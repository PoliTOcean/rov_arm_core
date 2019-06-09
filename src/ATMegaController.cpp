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
#include <queue>

#include "MqttClient.h"
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
	 * @commands_	: contains the value of button on 8 bit with the following structure:
						(*) MSB for the value (0 if released, 1 if pressed)
						(*) the remeining 7 bit for the identifier
	 */
	std::vector<int> axes_;
	std::queue<string> commands_;

	std::vector<Sensor<unsigned char>> sensors_;
	sensor_t currentSensor_;

	std::mutex mutexSnr_, mutexAxs_, mutexCmd_;

	/**
	 * @axesUpdated_		: it is true if @axes_ values has changed
	 * @commandsUpdated_	: it is true if @button_ value has changed
	 */
	bool axesUpdated_, commandsUpdated_, sensorsUpdated_;

public:
	// Constructor
	// It setup class variables and sensors
	Listener() : axes_(3, 0), axesUpdated_(false), commandsUpdated_(false), currentSensor_(sensor_t::First)
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
	 * They read the joystick data (@payload) from CommandParser
	 * 
	 * @payload: the string that recives from the CommandParser
	 * 
	 * listenForButtons	: converts the string @payload into an unsigned char value and stores it inside @button_.
	 * listenForAxes	: parses the string @payload into a JSON an stores the axes values inside @axes_ vector.
	 */
	void listenForAxes(const std::string& payload);
	void listenForCommands(const std::string& payload);
	void listenForSensor(unsigned char data);

	void resetCurrentSensor();

	// To check if @axes_ values or @commands_ values has changed
	bool isAxesUpdated();
	bool isCommandsUpdated();
	bool isSensorsUpdated();

};

void Listener::listenForAxes(const std::string& payload)
{
	auto c_map = nlohmann::json::parse(payload);

	std::lock_guard<std::mutex> lock(mutexAxs_);

	axes_ = c_map.get<std::vector<int>>();
	
	axesUpdated_ = true;
}

void Listener::listenForCommands(const std::string& payload)
{
	logger::getInstance().log(logger::DEBUG, "Received: "+payload);

	std::lock_guard<std::mutex> lock(mutexCmd_);

    commands_.push( payload );

	commandsUpdated_ = true;
}

void Listener::listenForSensor(unsigned char data)
{
	std::lock_guard<std::mutex> lock(mutexSnr_);

	if(currentSensor_ == sensor_t::ROLL || currentSensor_ == sensor_t::PITCH){
		float f=(float)data;
		f /= 10;
		sensors_[static_cast<int>(currentSensor_)].setValue(f);
	}else if(currentSensor_ == sensor_t::PRESSURE)
		sensors_[static_cast<int>(currentSensor_)].setValue(data + 990);
	else
		sensors_[static_cast<int>(currentSensor_)].setValue(data);
	

	if (++currentSensor_ > sensor_t::Last)
		currentSensor_ = sensor_t::First;

	sensorsUpdated_ = true;
}

void Listener::resetCurrentSensor()
{
	std::lock_guard<std::mutex> lock(mutexSnr_);
	currentSensor_ = sensor_t::First;
}

std::vector<int> Listener::axes()
{
	std::lock_guard<std::mutex> lock(mutexAxs_);
	axesUpdated_ = false;
	return axes_;
}

std::string Listener::action()
{
	std::lock_guard<std::mutex> lock(mutexCmd_);

	commandsUpdated_ = false;
	if (commands_.empty()) return Commands::Actions::NONE;

	string action = commands_.front();
	commands_.pop();
	return action;
}

std::vector<int> Listener::sensors()
{
	std::lock_guard<std::mutex> lock(mutexSnr_);
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

bool Listener::isCommandsUpdated()
{
	return !commands_.empty();
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

	void startTalking(MqttClient& publisher, Listener& listener);
	void stopTalking();

	bool isTalking();
};

void Talker::startTalking(MqttClient& publisher, Listener& listener)
{
	if (isTalking_)
		return ;

	isTalking_ = true;
	sensorThread_ = new std::thread([&]() {
		while (publisher.is_connected() && isTalking_)
		{
			if (!listener.isSensorsUpdated())
			{
				std::this_thread::sleep_for(std::chrono::seconds(Timing::Seconds::SENSORS));
				continue ;
			}

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

	std::thread *SPIAxesThread_, *SPICommandsThread_;
	bool isUsing_;

	void send(const std::vector<unsigned char>& buffer, Listener &listener);

public:
	SPI(Controller *controller) : controller_(controller), isUsing_(false) {}

	void setup();

	void startSPI(Listener& listener, MqttClient& publisher);
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
    else if(action == Commands::Actions::ATMega::PITCH_CONTROL)
		return Commands::ATMega::SPI::PITCH_CONTROL;
	else
        return 0;
}

void SPI::startSPI(Listener& listener, MqttClient& publisher)
{
	if (isUsing_)
		return ;

	isUsing_ = true;

	SPIAxesThread_ = new std::thread([&]() {

		long long threshold = (Timing::Milliseconds::SENSORS_UPDATE_DELAY / Timing::Milliseconds::AXES_DELAY) 
								/ ( static_cast<int>(sensor_t::Last) + 1 );
		int counter = 0;

		while (isUsing_)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(Timing::Milliseconds::AXES_DELAY));

			counter++;
			
			if(!listener.isAxesUpdated() && counter < threshold) continue;

			std::vector<int> axes = listener.axes();

			std::vector<unsigned char> buffer = {
				(unsigned char) Commands::ATMega::SPI::Delims::AXES,
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axes::X_AXIS],		SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axes::Y_AXIS],		SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axes::RZ_AXIS],		SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
				(unsigned char) Politocean::map(axes[Commands::ATMega::Axes::PITCH_AXIS],	SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
			};

			send(buffer, listener);

			counter = 0;
		}
	});

	SPICommandsThread_ = new std::thread([&]() {
		while (isUsing_)
		{
			if(!listener.isCommandsUpdated()){
            	std::this_thread::sleep_for(std::chrono::milliseconds(Timing::Milliseconds::JOYSTICK));
				continue;
			}

			std::string data = listener.action();

			bool sendToSPI = false;

			if (data == Commands::Actions::RESET)
			    controller_->reset();
			else if (data == Commands::Actions::ON)
            {
                Politocean::publishComponents(Rov::ATMEGA_ID, Components::POWER, Components::Status::ENABLED);
                controller_->startMotors();
            }
            else if (data == Commands::Actions::OFF)
            {
                Politocean::publishComponents(Rov::ATMEGA_ID, Components::POWER, Components::Status::DISABLED);
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
	SPIAxesThread_->join(); SPICommandsThread_->join();
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
	MqttClient& publisher = MqttClient::getInstance(Rov::ATMEGA_ID, Hmi::IP_ADDRESS);
	mqttLogger& ptoLogger = mqttLogger::getInstance(publisher);
	logger::enableLevel(logger::INFO);

	/**
	 * @subscriber	: the subscriber listening to JoystickMqttClient topics
	 * @listener	: object with the callbacks for @subscriber and methods to retreive data read
	 */
	MqttClient& subscriber = MqttClient::getInstance(Rov::ATMEGA_ID, Rov::IP_ADDRESS);
	Listener listener;

	// Subscribe @subscriber to joystick publisher topics
	subscriber.subscribeTo(Topics::AXES, 			&Listener::listenForAxes, 		&listener);
	subscriber.subscribeTo(Topics::COMMANDS,		&Listener::listenForCommands, 	&listener);


	/**
	 * @controller : to access to Raspberry Pi features
	 */

	Controller controller;
	
	Components::Status motorsStatus = Components::Status::ERROR;
	// Try to setup @controller
	try
	{
		controller.setup();
		motorsStatus = controller.setupMotors() == Controller::PinLevel::PIN_LOW ? Components::Status::DISABLED : Components::Status::ENABLED;
	} catch (Politocean::controllerException &e)
	{
		ptoLogger.log(logger::ERROR, e); // TODO mettere in ciclo come per Joystick
	}

	Politocean::publishComponents(Rov::ATMEGA_ID, Components::POWER, motorsStatus);
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
