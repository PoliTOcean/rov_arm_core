/**
 * @author pettinz
 */

#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <exception>
#include <Commands.h>

#include "Publisher.h"
#include "Subscriber.h"
#include "Sensor.h"
#include "Controller.h"
#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

#include "logger.h"
#include "mqttLogger.h"

#include "json.hpp"

/***************************************************
 * Listener class for subscriber
 **************************************************/

using namespace Politocean;
using namespace Politocean::RPi;
using namespace Politocean::Constants;

class Listener
{
	/**
	 * @axes_		: it is a vector with the following structure:
						(*) indices represent the axes identifiers
						(*) values represent the axes values
	 * @button_		: contains the value of button on 8 bit with the following structure:
						(*) MSB for the value (0 if released, 1 if pressed)
						(*) the remeining 7 bit for the identifier
	 */
	std::vector<int> axes_;
	string button_;

	std::vector<Sensor<unsigned char>> sensors_;
	sensor_t currentSensor_;

	/**
	 * @axesUpdated_	: it is true if @axes_ values has changed
	 * @buttonUpdated_	: it is true if @button_ value has changed
	 */
	bool axesUpdated_, buttonUpdated_, sensorsUpdated_;

public:
	// Constructor
	// It setup class variables and sensors
	Listener() : axesUpdated_(false), buttonUpdated_(false), currentSensor_(sensor_t::First)
	{
		for (auto sensor_type : Politocean::sensor_t())
			sensors_.emplace_back(Politocean::Sensor<unsigned char>(sensor_type, 0));
	}

	// Returns the @axes_ vector
	std::vector<int> axes();
	// Returns the @button_ variable
	std::string action();
	// Returns the @sensor_ vector
	std::vector<int> sensors();

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
	void listenForSensor(unsigned char data);

	void resetCurrentSensor();

	// To check if @axes_ values or @button_ value has changed
	bool isAxesUpdated();
	bool isButtonUpdated();
	bool isSensorsUpdated();

};

void Listener::listenForAxes(const std::string& payload)
{
	auto c_map = nlohmann::json::parse(payload);
	axes_ = c_map.get<std::vector<int>>();
	
	axesUpdated_ = true;
}

void Listener::listenForButton(const std::string& payload)
{
	std::cout << payload << std::endl;
	button_ = payload;

	buttonUpdated_ = true;
}

void Listener::listenForSensor(unsigned char data)
{
	sensors_[static_cast<int>(currentSensor_)].setValue(data);

	if (++currentSensor_ > sensor_t::Last)
		currentSensor_ = sensor_t::First;

	sensorsUpdated_ = true;
}

void Listener::resetCurrentSensor()
{
	currentSensor_ = sensor_t::First;
}

std::vector<int> Listener::axes()
{
	axesUpdated_ = false;
	return axes_;
}

std::string Listener::action()
{
	buttonUpdated_ = false;
	return button_;
}

std::vector<int> Listener::sensors()
{
	sensorsUpdated_ = false;

	std::vector<int> sensors;

	for (auto it = sensors_.begin(); it != sensors_.end(); it++)
		sensors.emplace_back(it->getValue());

	return sensors;
}

bool Listener::isAxesUpdated()
{
	return axesUpdated_;
}

bool Listener::isButtonUpdated()
{
	return buttonUpdated_;
}

bool Listener::isSensorsUpdated()
{
	return sensorsUpdated_;
}

/***************************************************
 * Talker class for sensors
 **************************************************/

class Talker
{
	std::thread *sensorThread_;
	bool isTalking_;

public:
	Talker() : isTalking_(false) {}

	void startTalking(Publisher& publisher, Listener& listener);
	void stopTalking();

	bool isTalking();
};

void Talker::startTalking(Publisher& publisher, Listener& listener)
{
	if (isTalking_)
		return ;

	isTalking_ = true;
	sensorThread_ = new std::thread([&]() {
		while (publisher.is_connected() && isTalking_)
		{
			if (!listener.isSensorsUpdated())
				continue ;

			nlohmann::json j_map = listener.sensors();
			publisher.publish(Topics::SENSORS, j_map.dump());

			std::this_thread::sleep_for(std::chrono::seconds(Timing::Seconds::SENSORS));
		}
	});
}

void Talker::stopTalking()
{
	if (!isTalking_)
		return ;
	
	isTalking_ = false;
	sensorThread_->join();
}

bool Talker::isTalking()
{
	return isTalking_;
}


/***************************************************
 * SPI Class
 **************************************************/

class SPI
{
	Controller *controller_;
	std::mutex mutex_;

	std::thread *SPIAxesThread_, *SPIButtonThread_;
	bool isUsing_;

	void send(const std::vector<unsigned char>& buffer, Listener &listener);

public:
	SPI(Controller *controller) : controller_(controller), isUsing_(false) {}

	void setup();

	void startSPI(Listener& listener, Publisher& publisher);
	void stopSPI();

	bool isUsing();
};

void SPI::setup()
{
	controller_->setupSPI(Controller::DEFAULT_SPI_CHANNEL, Controller::DEFAULT_SPI_SPEED);
}

unsigned char setAction(std::string action)
{
    if(action == Commands::Actions::ATMega::VDOWN_ON)
        return Commands::ATMega::SPI::VDOWN_ON;
    else if(action == Commands::Actions::ATMega::VDOWN_OFF)
        return Commands::ATMega::SPI::VDOWN_OFF;
    else if(action == Commands::Actions::ATMega::VUP_ON)
        return Commands::ATMega::SPI::VUP_ON;
    else if(action == Commands::Actions::ATMega::VUP_OFF)
        return Commands::ATMega::SPI::VUP_OFF;
	else if(action == Commands::Actions::ATMega::VUP_FAST_ON)
		return Commands::ATMega::SPI::VUP_FAST_ON;
	else if(action == Commands::Actions::ATMega::VUP_FAST_OFF)
		return Commands::ATMega::SPI::VUP_FAST_OFF;
    else if(action == Commands::Actions::ATMega::FAST)
        return Commands::ATMega::SPI::FAST;
    else if(action == Commands::Actions::ATMega::SLOW)
        return Commands::ATMega::SPI::SLOW;
    else if(action == Commands::Actions::ATMega::MEDIUM)
        return Commands::ATMega::SPI::MEDIUM;
    else if(action == Commands::Actions::ATMega::START_AND_STOP)
        return Commands::ATMega::SPI::START_AND_STOP;
    else
        return 0;
}

void SPI::startSPI(Listener& listener, Publisher& publisher)
{
	if (isUsing_)
		return ;

	isUsing_ = true;

	SPIAxesThread_ = new std::thread([&]() {

		long long threshold = 20 / ( static_cast<int>(sensor_t::Last) + 1 );
		int counter = 0;

		while (isUsing_)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			counter++;
			
			if(!listener.isAxesUpdated() && counter < threshold) continue;

			std::vector<int> axes = listener.axes();

			std::vector<unsigned char> buffer = {
				(unsigned char) Commands::ATMega::SPI::Delims::AXES,
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axis::X_AXES],	SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axis::Y_AXES],	SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axis::RZ_AXES],	SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1)
			};

			send(buffer, listener);

			counter = 0;
		}
	});

	SPIButtonThread_ = new std::thread([&]() {
		while (isUsing_)
		{
			if(!listener.isButtonUpdated()) continue;

			std::string data = listener.action();
			std::cout << data << std::endl;
			bool sendToSPI = false;

			if (data == Commands::Actions::RESET)
			    controller_->reset();
			else if (data == Commands::Actions::ON)
            {
                Politocean::publishComponents(publisher,Components::POWER, Commands::Actions::ON);
                controller_->startMotors();
            }
            else if (data == Commands::Actions::OFF)
            {
                Politocean::publishComponents(publisher,Components::POWER, Commands::Actions::OFF);
                controller_->stopMotors();
            } else {
                sendToSPI = true;
			}

            if (!sendToSPI)
                    continue;

            unsigned char action = setAction(data);

			std::vector<unsigned char> buffer = {
				Commands::ATMega::SPI::Delims::COMMAND,
				action
			};

			send(buffer, listener);
		}
	});
}

void SPI::stopSPI()
{
	if (!isUsing_)
		return ;

	isUsing_ = false;
	SPIAxesThread_->join(); SPIButtonThread_->join();
}

void SPI::send(const std::vector<unsigned char>& buffer, Listener& listener)
{
	std::lock_guard<std::mutex> lock(mutex_);

	for (auto it = buffer.begin(); it != buffer.end(); it++)
	{
		unsigned char data = controller_->SPIDataRW(*it);

		if (data == Commands::ATMega::SPI::Delims::SENSORS)
		{
			listener.resetCurrentSensor();
			continue;
		}
		
		listener.listenForSensor(data);
	}
}

bool SPI::isUsing()
{
	return isUsing_;
}

/***************************************************
 * Main section
 **************************************************/

int main(int argc, const char *argv[])
{
	// Enable logging
	Publisher publisher(Hmi::IP_ADDRESS, Rov::ATMEGA_ID);
	mqttLogger ptoLogger(&publisher);
	logger::enableLevel(logger::DEBUG, true);

	// Try to connect to publisher logger
	try
	{
		publisher.connect();
	}
	catch (const mqtt::exception& e)
	{
		ptoLogger.logError(e);
	}

	/**
	 * @subscriber	: the subscriber listening to JoystickPublisher topics
	 * @listener	: object with the callbacks for @subscriber and methods to retreive data read
	 */
	Subscriber subscriber(Rov::IP_ADDRESS, Rov::ATMEGA_ID);
	Listener listener;

	// Subscribe @subscriber to joystick publisher topics
	subscriber.subscribeTo(Topics::AXES, 			&Listener::listenForAxes, 		&listener);
	subscriber.subscribeTo(Topics::COMMANDS,		&Listener::listenForButton, 	&listener);

	// Try to connect @subscriber
	try
	{	
		subscriber.connect();
	}
	catch (Politocean::mqttException& e)
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
		controller.setupMotors();
	} catch (Politocean::controllerException &e)
	{
		std::cerr << "Error on controller setup : " << e.what() << std::endl;
		ptoLogger.logError(e);
		std::exit(EXIT_FAILURE);
	}

	SPI spi(&controller);

	// Try to setup @spi
	try
	{
		spi.setup();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
	}	
	
	spi.startSPI(listener, publisher);

	Talker talker;
	talker.startTalking(publisher, listener);

	// wait until subscriber is is_connected
	subscriber.wait();

	// Stop sensors talker and SPI
	talker.stopTalking();
	spi.stopSPI();

	//safe reset at the end
	controller.reset();
	
	return 0;
}
