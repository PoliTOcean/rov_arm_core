#include "Subscriber.h"
#include "Controller.h"

#include <thread>
#include <chrono>

#include "PolitoceanConstants.h"

#include "json.hpp"

using namespace Politocean;

/*******************************************************
 * Listener class for stepper subscriber
 ******************************************************/

class Listener
{
	Controller::Stepper::Direction shoulderDirection_, wristDirection_;
	bool shoulderEnable_, wristEnable_, isUpdated_, isListening_;

	int action_;

public:
	Listener() :
		shoulderDirection_(Controller::Stepper::Direction::NONE), wristDirection_(Controller::Stepper::Direction::NONE),
		shoulderEnable_(false), wristEnable_(false), isUpdated_(false), isListening_(false) {}

	void listenForShoulder(const std::string& payload);
	void listenForWrist(const std::string& payload);

	int action();

	Controller::Stepper::Direction getShoulderDirection();
	Controller::Stepper::Direction getWristDirection();

	bool isShoulderEnable();
	bool isWristEnable();

	bool isUpdated();
};

void Listener::listenForShoulder(const std::string& payload)
{
	action_ = Constants::Commands::Actions::NONE;

	if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_UP))
	{
		shoulderDirection_ = Controller::Stepper::Direction::CW;
		action_ = Constants::Commands::Actions::SHOULDER_ON;
	}
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_DOWN))
	{
		shoulderDirection_ = Controller::Stepper::Direction::CCW;
		action_ = Constants::Commands::Actions::SHOULDER_ON;
	}
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_ON))
		shoulderEnable_ = true;
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_OFF))
		shoulderEnable_ = false;
	else
	{
		action_ = Constants::Commands::Actions::SHOULDER_OFF;
		shoulderDirection_ = Controller::Stepper::Direction::NONE;
		shoulderEnable_ = false;
	}

	isUpdated_ = true;
}

void Listener::listenForWrist(const std::string& payload)
{
	//
}

int Listener::action()
{
	isUpdated_ = false;
	return action_;
}

Controller::Stepper::Direction Listener::getShoulderDirection()
{
	return shoulderDirection_;
}

Controller::Stepper::Direction Listener::getWristDirection()
{
	return wristDirection_;
}

bool Listener::isUpdated()
{
	return isUpdated_;
}

/*******************************************************
 * Arm class
 ******************************************************/

class Arm
{
	Controller controller;
	Controller::Stepper shoulder_, wrist_;

	std::thread *shoulderThread, *wristThread;
	
	bool isShouldering_ = false, isWristing_ = false, isMoving_ = false;

public:
	Arm() : controller(), shoulder_(Controller::Stepper::Name::SHOULDER), wrist_(Controller::Stepper::Name::WRIST) {}

	void setup();

	void start(Listener& shoulderListener, Listener& wristListener);
	void stop();

	void startShoulder(Listener& wristListener);
	void stopShoulder();

	void startWrist(Listener& wristListener);
	void stopWrist();

	bool isShouldering();
	bool isWristing();
	bool isMoving();

	void enableShoulder();
	void disableShoulder();

	void enableWrist();
	void disableWrist();

	void enable();
	void disable();
};

void Arm::setup()
{
	controller.setup();
}

void Arm::start(Listener& shoulderListener, Listener& wristListener)
{
	startShoulder(shoulderListener);
	startWrist(wristListener);
}

void Arm::startShoulder(Listener& listener)
{
	if (isShouldering_)
		return ;

	shoulderThread = new std::thread([&]() {
		isShouldering_ 	= true;
		isMoving_ 		= true;

		shoulder_.enable();

		while (isShouldering_)
		{
			Controller::Stepper::Direction direction = listener.getShoulderDirection();

			if (direction == Controller::Stepper::Direction::NONE)
			{
				shoulder_.disable();
				continue;
			}

			shoulder_.setDirection(direction);
			shoulder_.step();
		}
	});
}

void Arm::startWrist(Listener& listener)
{
	if (isWristing_)
		return ;
		
	wristThread = new std::thread([&]() {
		isWristing_ = true;
		isMoving_ 	= true;

		wrist_.enable();

		while (isWristing_)
		{
			Controller::Stepper::Direction direction = listener.getWristDirection();

			if (direction == Controller::Stepper::Direction::NONE)
			{
				wrist_.disable();
				continue;
			}

			wrist_.setDirection(direction);
			wrist_.step();
		}
	});
}

void Arm::stop()
{
	stopShoulder();
	stopWrist();
}

void Arm::stopShoulder()
{
	shoulder_.disable();

	isShouldering_ 	= false;
	isMoving_ 		= isWristing_;
}

void Arm::stopWrist()
{
	wrist_.disable();

	isWristing_	= false;
	isMoving_ 	= isWristing_;
}

bool Arm::isShouldering()
{
	return isShouldering_;
}

bool Arm::isWristing()
{
	return isWristing_;
}

bool Arm::isMoving()
{
	return isMoving_;
}

void Arm::enableShoulder()
{
	shoulder_.enable();
}

void Arm::disableShoulder()
{
	shoulder_.disable();
}

void Arm::enableWrist()
{
	wrist_.enable();
}

void Arm::disableWrist()
{
	wrist_.disable();
}

void Arm::enable()
{
	enableShoulder();
	enableWrist();
}

void Arm::disable()
{
	disableShoulder();
	disableWrist();
}

/*******************************************************
 * Main section
 ******************************************************/

int main (void)
{
	Subscriber subscriber(Constants::Rov::IP_ADDRESS, Constants::Rov::ARM_ID);
	Listener listener;

	subscriber.subscribeTo(Constants::Topics::SHOULDER, &Listener::listenForShoulder, &listener);
	subscriber.subscribeTo(Constants::Topics::WRIST, 	&Listener::listenForWrist, &listener);

	subscriber.connect();

	Arm arm;


	while (subscriber.is_connected())
	{
		if (!listener.isUpdated())
			continue ;

		std::cout << "I'm in!" << std::endl;

		switch (listener.action())
		{
			case Constants::Commands::Actions::WRIST_OFF:
				std::cout << "WRIST OFF" << std::endl;
				arm.stopWrist();
				break;

			case Constants::Commands::Actions::WRIST_ON:
				std::cout << "WRIST ON" << std::endl;
				arm.startWrist(listener);
				break;

			case Constants::Commands::Actions::SHOULDER_OFF:
				std::cout << "SHOULDER OFF" << std::endl;
				arm.stopShoulder();
				break;

			case Constants::Commands::Actions::SHOULDER_ON:
				std::cout << "SHOULDER ON" << std::endl;
				arm.startShoulder(listener);
				break;
		}
	}

	subscriber.wait();

	return 0;
}