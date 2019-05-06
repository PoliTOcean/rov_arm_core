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
	std::string button_;

	/**
	 * @axesUpdated_	: it is true if @axes_ values has changed
	 * @buttonUpdated_	: it is true if @button_ value has changed
	 */
	bool axesUpdated_ = false, buttonUpdated_ = false;

public:
	// Returns the @axes_ vector
	std::vector<int> axes();

	// Returns the @button_ variable
	std::string button();

	/**
	 * Callback functions.
	 * They read the joystick data (@payload) from JoystickPublisher
	 * 
	 * @payload: the string that recives from the JoystickPublisher
	 * 
	 * listenForButtons	: converts the string @payload into an unsigned char value and stores it inside @button_.
	 * listenForAxes	: parses the string @payload into a JSON an stores the axes values inside @axes_ vector.
	 */
	void listenForAxes(const std::string& payload);
	void listenForButton(const std::string& payload);

	// To check if @axes_ values or @button_ value has changed
	bool isAxesUpdated();
	bool isButtonUpdated();

};

void Listener::listenForAxes(const std::string& payload)
{
	auto c_map = nlohmann::json::parse(payload);
	axes_ = c_map.get<std::vector<int>>();
	
	axesUpdated_ = true;
}

void Listener::listenForButton(const std::string& payload)
{
	button_ = static_cast<unsigned char>(std::stoi(payload));

	buttonUpdated_ = true;
}

std::vector<int> Listener::axes()
{
	axesUpdated_ = false;
	return axes_;
}

std::string Listener::button()
{
	buttonUpdated_ = false;
	return button_;
}

bool Listener::isAxesUpdated(){
	return axesUpdated_;
}

bool Listener::isButtonUpdated(){
	return buttonUpdated_;
}

/***************************************************
 * Main section
 **************************************************/


using namespace Politocean;
using namespace Politocean::Constants;
using namespace std;

/**
 * @mutex : to handle the critical sections
 */
std::mutex mutex_;

/**
 * @sensors : vector of sensors object with value type unsigned char (8 bit)
 */
std::vector<Sensor<unsigned char>> sensors;
unsigned int sensor = 0;

void bufferToSPI(Controller &controller, const std::vector<unsigned char>& buffer){

	std::lock_guard<std::mutex> lock(mutex_);

	for (auto it = buffer.begin(); it != buffer.end(); it++)
	{
		unsigned char data = controller.SPIDataRW(*it);

		if (*it == 0xFF)
		{
			sensor=0;
			it = buffer.begin();
			continue;
		}

		sensors[sensor++].setValue(data);

		// Check if I received the last sensor
		if (sensor >= sensors.size())
			sensor = 0;
	}
}

int main(int argc, const char *argv[])
{
	// Enable logging
	Publisher pub(Hmi::IP_ADDRESS, Rov::SPI_ID_PUB);
	mqttLogger ptoLogger(&pub);
	logger::enableLevel(logger::DEBUG, true);

	// Try to connect to publisher logger
	try
	{
		pub.connect();
	} catch (const mqtt::exception& e)
	{
		ptoLogger.logError(e);
	} catch(const std::exception& e)
	{
		ptoLogger.logError(e);
	}

	/**
	 * @joystickSubscriber	: the subscriber listening to JoystickPublisher topics
	 * @listener			: object with the callbacks for @joystickSubscriber and methods to retreive data read
	 */
	Subscriber joystickSubscriber(Hmi::IP_ADDRESS, Rov::SPI_ID_SUB);
	Listener listener;

	// Subscribe @joystickSubscriber to joystick publisher topics
	joystickSubscriber.subscribeTo(Topics::JOYSTICK_AXES, 	&Listener::listenForAxes, 		&listener);
	joystickSubscriber.subscribeTo(Topics::BUTTONS,			&Listener::listenForButton, 	&listener);

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

	Controller controller;

	// Try to setup @controller
	try
	{
		controller.setup();
	} catch (Politocean::controllerException &e)
	{
		std::cerr << "Error on controller setup : " << e.what() << std::endl;
		ptoLogger.logError(e);
		std::exit(EXIT_FAILURE);
	}

	// Setup sensors
	for (auto sensor_type : Politocean::sensor_t())
		sensors.emplace_back(Politocean::Sensor<unsigned char>(sensor_type, 0));

	/**
	 * Threads that handle with SPI
	 *
	 * @SPIAxesThread 	: sends axes via SPI
	 * @SPIButtonThread	: sends buttons via SPI
	 */
	std::thread SPIAxesThread([&]() {
		while (joystickSubscriber.is_connected())
		{
			if(!listener.isAxesUpdated()) continue;

			std::vector<int> axes = listener.axes();

			std::vector<unsigned char> buffer = {
				(unsigned char) Politocean::map(axes[Commands::Axes::X],	0, INT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::Axes::Y], 	0, INT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::Axes::RZ], 	0, INT_MAX, 1, UCHAR_MAX-1)
			};

			bufferToSPI(controller, buffer);
		}
	});
	
	std::thread SPIButtonThread([&]() {
		bool started = false;
		while (joystickSubscriber.is_connected())
		{
			if(!listener.isButtonUpdated()) continue;

			std::string btn = listener.button();
			unsigned char data;
			if(btn==Commands::Actions::MOTORS_SWAP){
				started = !started;
				if(started) data = 0x01;
				else 0x02;
			}
		/*	

			bool value 				= (data >> 7) & 0x01;
      		unsigned short int id 	= data & 0x7F;

			bool sendToSPI 			= false;
			switch (id)
			{
				case Constants::Commands::Buttons::RESET:
					if (value)
						controller.reset();
					break;
				case Constants::Commands::Buttons::MOTORS:
					if (value)
						controller.switchMotors();
					break;
				default:
					sendToSPI = true;
			}

			if (!sendToSPI)
				continue;*/

			std::vector<unsigned char> buffer = {
				0x00,
				data
			};

			bufferToSPI(controller, buffer);
		}
	});

	// Wait for threads ending	
	SPIAxesThread.join();
	SPIButtonThread.join();

	//safe reset at the end
	controller.reset();
	
	return 0;
}
