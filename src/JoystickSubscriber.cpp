/**
 * @author pettinz
 */

#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>

#include "Subscriber.h"
#include "Sensor.h"
#include "Controller.h"
#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

#include "Publisher.h"
#include "logger.h"
#include "mqttLogger.h"

#include "json.hpp"

#define DFLT_ADDRESS 	"tcp://localhost:1883"
#define DFLT_CLIENT_ID	"JoystickSubscriber"

/***************************************************
 * Listener class for JoystickSubscriber
 **************************************************/

class Listener
{
	/**
	 * @axes_	: it is a vector with the following structure:
					(*) indices represent the axes identifiers
					(*) values represent the axes values
	 * @button_	: contains the value of button on 8 bit with the following structure:
					(*) MSB for the value (0 if released, 1 if pressed)
					(*) the remeining 7 bit for the identifier
	 */
	std::vector<int> axes_;
    unsigned char button_;

public:
	// Returns the @axes_ vector
	std::vector<int> axes();

	// Returns the @button_ variable
	unsigned char button();

	/**
     * Callback functions.
     * They read the joystick data (@payload) from JoystickPublisher
     * 
     * @payload: the string that recives from the JoystickPublisher
     *
     * listenForButtons : converts the string @payload into an unsigned char value and stores it inside @button_.
     * listenForAxes    : parses the string @payload into a JSON an stores the axes values inside @axes_ vector.
     */
	void listenForAxes(const std::string& payload);
	void listenForButton(const std::string& payload);

};

void Listener::listenForAxes(const std::string& payload)
{
	auto c_map = nlohmann::json::parse(payload);

	axes_ = c_map["axes"].get<std::vector<int>>();
}

void Listener::listenForButton(const std::string& payload)
{
	button_ = static_cast<unsigned char>(std::stoi(payload));
}

std::vector<int> Listener::axes()
{
	return axes_;
}

unsigned char Listener::button()
{
	return button_;
}

/***************************************************
 * Main section
 **************************************************/

std::mutex mutex;

int main(int argc, const char *argv[])
{
	// Enable logging
	Politocean::Publisher pub("127.0.0.1", Politocean::Constants::Rov::CLIENT_ID);
	Politocean::mqttLogger ptoLogger(&pub);
	Politocean::logger::enableLevel(Politocean::logger::DEBUG, true);

	// Try to connect to publisher logger
    try {
        pub.connect();
    } catch(const mqtt::exception& e) {
        ptoLogger.logError(e);
    } catch(const std::exception& e) {
        ptoLogger.logError(e);
    }

	/**
	 * @sensors : vector of sensors object with value type unsigned char (8 bit)
	 */
	std::vector<Politocean::Sensor<unsigned char>> sensors;

	/**
	 * @joystickSubscriber 	: the subscriber listening to JoystickPublisher topics
	 * @listener			: object with the callbacks for @joystickSubscriber and methods to retreive data read
	 */
	Politocean::Subscriber joystickSubscriber(DFLT_ADDRESS, DFLT_CLIENT_ID);
	Listener listener;

	// Subscribe @joystickSubscriber to joystick publisher topics
	joystickSubscriber.subscribeTo(Politocean::Constants::Topics::JOYSTICK_AXES, 	&Listener::listenForAxes, 		&listener);
	joystickSubscriber.subscribeTo(Politocean::Constants::Topics::JOYSTICK_BUTTONS,	&Listener::listenForButton, 	&listener);

	// Try to connect @joystickSubscriber
	try
	{	
		joystickSubscriber.connect();
	} catch (Politocean::mqttException& e)
	{
		std::cerr << "Error on subscriber connection : " << e.what() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	/**
	 * @controller : to access to Raspberry Pi features
	 */
	Politocean::Controller controller;

	// Try to setup @controller
	try
	{
		controller.setup();
	} catch (Politocean::controllerException &e)
	{
		std::cerr << "Error on controller setup : " << e.what() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	// Setup sensors
	for (auto sensor_type : Politocean::sensor_t())
            sensors.emplace_back(Politocean::Sensor<unsigned char>(sensor_type, 0));

	// Setup the SPI communication
	bool isCommunicating = true;
	int sensor = 0;

	/**
	 * Threads that handle with SPI
	 *
	 * @mutex			: to handle with critical section (shared sensors variables)
	 * @SPIAxesThread 	: sends axes via SPI
	 * @SPIButtonThread	: sends buttons via SPI
	 */
	std::thread SPIAxesThread([&]() {
		while (isCommunicating)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
			
			std::vector<int> axes = listener.axes();

			std::vector<int> axesBuffer = {
                axes[Politocean::Constants::Commands::Axes::X],
                axes[Politocean::Constants::Commands::Axes::Y],
                axes[Politocean::Constants::Commands::Axes::RZ]
            };

			unsigned char data = Politocean::map(axesBuffer[sensor], 0, INT_MAX);
			
			std::lock_guard<std::mutex> lock(mutex);

			sensors[sensor].setValue(controller.SPIDataRW(data));

			if (++sensor > sensors.size())
                sensor = 0;
		}
	});
	std::thread SPIButtonThread([&]() {
		unsigned char lastButton = -1;

		while (isCommunicating)
		{
			unsigned char data = listener.button();

			if (data == lastButton)
				continue;

			lastButton = data;
			
			std::lock_guard<std::mutex> lock(mutex);

			sensors[sensor].setValue(controller.SPIDataRW(data));			

			if (++sensor > sensors.size())
				sensor = 0;
		}
	});

	// Wait for threads ending	
	SPIAxesThread.join();
	SPIButtonThread.join();

	return 0;
}