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
	bool shoulderEnable_, wristEnable_, isUpdated_;

	int action_;

public:
	Listener() :
		shoulderDirection_(Controller::Stepper::Direction::NONE), wristDirection_(Controller::Stepper::Direction::NONE),
		shoulderEnable_(false), wristEnable_(false), isUpdated_(false) {}

	void listenForShoulder(const std::string& payload);
	void listenForWrist(const std::string& payload);
	void listenForWristDirection(const std::string& payload);

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
		action_ = Constants::Commands::Actions::SHOULDER_UP;
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_DOWN))
		action_ = Constants::Commands::Actions::SHOULDER_DOWN;
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_ON))
		action_ = Constants::Commands::Actions::SHOULDER_ON;
	else if (payload == std::to_string(Constants::Commands::Actions::SHOULDER_OFF))
		action_ = Constants::Commands::Actions::SHOULDER_OFF;
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
	action_ = Constants::Commands::Actions::NONE;

	if (payload == std::to_string(Constants::Commands::Actions::WRIST_START))
		action_ = Constants::Commands::Actions::WRIST_START;
	else if (payload == std::to_string(Constants::Commands::Actions::WRIST_STOP))
		action_ = Constants::Commands::Actions::WRIST_STOP;
	else
		action_ = Constants::Commands::Actions::NONE;
	
	isUpdated_ = true;
}

void Listener::listenForWristDirection(const std::string& payload)
{
	if (std::stoi(payload) > 0)
		wristDirection_ = Controller::Stepper::Direction::CW;
	else if (std::stoi(payload) < 0)
		wristDirection_ = Controller::Stepper::Direction::CCW;
	else
		wristDirection_ = Controller::Stepper::Direction::NONE;
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

	void start(Controller::Stepper::Direction shoulderDirection, Controller::Stepper::Direction wristDirection);
	void stop();

	void startShoulder(Controller::Stepper::Direction direction);
	void stopShoulder();

	void startWrist(Controller::Stepper::Direction direction);
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
	controller.setupArm();
}

void Arm::start(Controller::Stepper::Direction shoulderDirection, Controller::Stepper::Direction wristDirection)
{
	startShoulder(shoulderDirection);
	startWrist(wristDirection);
}

void Arm::startShoulder(Controller::Stepper::Direction direction)
{
	if (isShouldering_)
		return ;

	shoulderThread = new std::thread([&]() {
		isShouldering_ 	= true;
		isMoving_ 		= true;

		while (isShouldering_)
		{
			if (direction == Controller::Stepper::Direction::NONE)
				continue;

			std::cout << "UPPPP" << std::endl;
			shoulder_.setDirection(direction);
			shoulder_.step();
		}
	});
}

void Arm::startWrist(Controller::Stepper::Direction direction)
{
	if (isWristing_)
		return ;
		
	wristThread = new std::thread([&]() {
		isWristing_ = true;
		isMoving_ 	= true;

		while (isWristing_)
		{
			if (direction == Controller::Stepper::Direction::NONE)
				continue;

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
	isShouldering_ 	= false;
	isMoving_ 		= isWristing_;

	shoulderThread->join();
}

void Arm::stopWrist()
{
	isWristing_	= false;
	isMoving_ 	= isWristing_;

	wristThread->join();
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

	subscriber.subscribeTo(Constants::Topics::SHOULDER, 		&Listener::listenForShoulder, &listener);
	subscriber.subscribeTo(Constants::Topics::WRIST, 			&Listener::listenForWrist, &listener);
	subscriber.subscribeTo(Constants::Topics::WRIST_VELOCITY,	&Listener::listenForWristDirection, &listener);

	subscriber.connect();

	Arm arm;
	arm.setup();

	while (subscriber.is_connected())
	{
		if (!listener.isUpdated())
			continue ;

		switch (listener.action())
		{
			case Constants::Commands::Actions::WRIST_OFF:
				arm.disableWrist();
				break;

			case Constants::Commands::Actions::WRIST_ON:
				arm.enableWrist();
				break;

			case Constants::Commands::Actions::SHOULDER_OFF:
				arm.disableShoulder();
				break;

			case Constants::Commands::Actions::SHOULDER_ON:
				arm.enableShoulder();
				break;

			case Constants::Commands::Actions::SHOULDER_UP:
				arm.startShoulder(listener.getShoulderDirection());
				break;
			
			case Constants::Commands::Actions::SHOULDER_DOWN:
				arm.stopShoulder();
				break;
			
			case Constants::Commands::Actions::WRIST_START:
				arm.startWrist(listener.getWristDirection());
				break;

			case Constants::Commands::Actions::WRIST_STOP:
				arm.stopWrist();
				break;

			case Constants::Commands::Actions::NONE:
				arm.stop();
				break;
		}
	}

	subscriber.wait();

	return 0;
}