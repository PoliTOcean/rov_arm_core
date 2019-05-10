#include "Subscriber.h"
#include "Controller.h"

#include <thread>
#include <chrono>

#include "PolitoceanConstants.h"
#include "PolitoceanUtils.hpp"

#include "json.hpp"

using namespace Politocean;

/*******************************************************
 * Listener class for stepper subscriber
 ******************************************************/

class Listener
{
	Controller::Stepper::Direction shoulderDirection_, wristDirection_;
	Controller::DCMotor::Direction handDirection_;
	bool shoulderEnable_, wristEnable_, isUpdated_;

	int action_;
	int wristVelocity_, handVelocity_;

public:
	Listener() :
		shoulderDirection_(Controller::Stepper::Direction::NONE), wristDirection_(Controller::Stepper::Direction::NONE),
		handDirection_(Controller::DCMotor::Direction::NONE),
		shoulderEnable_(false), wristEnable_(false), isUpdated_(false),
		action_(Constants::Commands::Actions::NONE),
		wristVelocity_(0), handVelocity_(0) {}

	void listenForShoulder(const std::string& payload);
	void listenForWrist(const std::string& payload);
	void listenForWristDirection(const std::string& payload);
	void listenForHand(const std::string& payload);
	void listenForHandVelocity(const std::string& payload);

	int action();
	int wristVelocity();
	int handVelocity();

	Controller::Stepper::Direction getShoulderDirection();
	Controller::Stepper::Direction getWristDirection();
	Controller::DCMotor::Direction getHandDirection();

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
		action_ = Constants::Commands::Actions::NONE;

	isUpdated_ = true;
}

void Listener::listenForWrist(const std::string& payload)
{
	action_ = Constants::Commands::Actions::NONE;

	if (payload == std::to_string(Constants::Commands::Actions::WRIST_START))
		action_ = Constants::Commands::Actions::WRIST_START;
	else if (payload == std::to_string(Constants::Commands::Actions::WRIST_STOP))
		action_ = Constants::Commands::Actions::WRIST_STOP;
	else if (payload == std::to_string(Constants::Commands::Actions::WRIST_ON))
		action_ = Constants::Commands::Actions::WRIST_ON;
	else if (payload == std::to_string(Constants::Commands::Actions::WRIST_OFF))
		action_ = Constants::Commands::Actions::WRIST_OFF;
	else
		action_ = Constants::Commands::Actions::NONE;
	
	isUpdated_ = true;
}

void Listener::listenForWristDirection(const std::string& payload)
{
	int velocity = std::stoi(payload);

	if (velocity)
		wristDirection_ = Controller::Stepper::Direction::CW;
	else if (velocity < 0)
		wristDirection_ = Controller::Stepper::Direction::CCW;
	else
		wristDirection_ = Controller::Stepper::Direction::NONE;

	isUpdated_ = true;
}

void Listener::listenForHand(const std::string& payload)
{	
	if (payload == std::to_string(Constants::Commands::Actions::HAND_START))
		action_ = Constants::Commands::Actions::HAND_START;
	else if (payload == std::to_string(Constants::Commands::Actions::HAND_STOP))
		action_ = Constants::Commands::Actions::HAND_STOP;
	else
		action_ = Constants::Commands::Actions::NONE;
	
	isUpdated_ = true;
}

void Listener::listenForHandVelocity(const std::string& payload)
{
	int velocity = std::stoi(payload);

	if (velocity > 0)
		handDirection_ = Controller::DCMotor::Direction::CCW;
	else if (velocity < 0)
		handDirection_ = Controller::DCMotor::Direction::CW;
	else
		handDirection_ = Controller::DCMotor::Direction::NONE;

	int const mask = velocity >> (sizeof(int) * __CHAR_BIT__ - 1);
	velocity = ((velocity + mask) ^ mask);

	handVelocity_ = Politocean::map(velocity, 0, SHRT_MAX, Controller::DCMotor::MIN_PWM, Controller::DCMotor::MAX_PWM);
}

int Listener::action()
{
	isUpdated_ = false;
	return action_;
}

int Listener::wristVelocity()
{
	isUpdated_ = false;
	return wristVelocity_;
}

int Listener::handVelocity()
{
	isUpdated_ = false;
	return handVelocity_;
}

Controller::Stepper::Direction Listener::getShoulderDirection()
{
	return shoulderDirection_;
}

Controller::Stepper::Direction Listener::getWristDirection()
{
	return wristDirection_;
}

Controller::DCMotor::Direction Listener::getHandDirection()
{
	return handDirection_;
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
	Controller::DCMotor hand_;

	std::thread *shoulderThread_, *wristThread_, *handThread_;
	
	bool isShouldering_, isWristing_, isHanding_, isMoving_;

public:
	Arm() :
		controller(),
		shoulder_(Controller::Stepper::Name::SHOULDER), wrist_(Controller::Stepper::Name::WRIST), hand_(Controller::DCMotor::Name::HAND),
		isShouldering_(false), isWristing_(false), isHanding_(false), isMoving_(false) {}

	void setup();

	void startShoulder();
	void stopShoulder();

	void startWrist();
	void stopWrist();

	void startHand(Controller::DCMotor::Direction direction, int velocity);
	void stopHand();

	void stop();

	void setShoulderDirection(Controller::Stepper::Direction direction);
	void setWristDirection(Controller::Stepper::Direction direction);
	void setHandDirection(Controller::DCMotor::Direction);

	bool isShouldering();
	bool isWristing();
	bool isHanding();
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

	hand_.setup();
}

void Arm::setShoulderDirection(Controller::Stepper::Direction direction)
{
	shoulder_.setDirection(direction);
}

void Arm::setWristDirection(Controller::Stepper::Direction direction)
{
	switch (direction)
	{
	case Controller::Stepper::Direction::CW:
		std::cout << "CW" << std::endl;
		break;
	case Controller::Stepper::Direction::CCW:
		std::cout << "CCW" << std::endl;
		break;
	
	default:
		break;
	}

	wrist_.setDirection(direction);
}

void Arm::setHandDirection(Controller::DCMotor::Direction direction)
{
	hand_.setDirection(direction);
}

void Arm::startShoulder()
{
	if (isShouldering_)
		return ;

	shoulderThread_ = new std::thread([&]() {
		isShouldering_ 	= true;
		isMoving_ 		= true;

		while (isShouldering_)
			shoulder_.step();
	});
}

void Arm::startWrist()
{
	if (isWristing_)
		return ;

	wristThread_ = new std::thread([&]() {
		isWristing_ = true;
		isMoving_ 	= true;

		while (isWristing_)
			wrist_.step();
	});
}

void Arm::startHand(Controller::DCMotor::Direction direction, int velocity)
{
	if (isHanding_)
		return ;
	
	hand_.setVelocity(velocity);
	hand_.setDirection(direction);

	hand_.startPWM();

	isHanding_ = true;
}

void Arm::stopHand()
{
	hand_.stopPWM();
	isHanding_	= false;
	isMoving_ 	= (isWristing_ || isShouldering_);
}

void Arm::stopShoulder()
{
	isShouldering_ 	= false;
	isMoving_ 		= (isWristing_ || isHanding_);
}

void Arm::stopWrist()
{
	isWristing_	= false;
	isMoving_ 	= (isShouldering_ || isHanding_);
}

void Arm::stop()
{
	stopShoulder();
	stopWrist();
	stopHand();
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

	subscriber.subscribeTo(Constants::Topics::SHOULDER, 		&Listener::listenForShoulder,		&listener);
	subscriber.subscribeTo(Constants::Topics::WRIST, 			&Listener::listenForWrist,			&listener);
	subscriber.subscribeTo(Constants::Topics::WRIST_VELOCITY,	&Listener::listenForWristDirection,	&listener);
	subscriber.subscribeTo(Constants::Topics::HAND,				&Listener::listenForHand, 			&listener);
	subscriber.subscribeTo(Constants::Topics::HAND_VELOCITY,	&Listener::listenForHandVelocity,	&listener);

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
				arm.setShoulderDirection(Controller::Stepper::Direction::CCW);
				arm.startShoulder();
				break;
			
			case Constants::Commands::Actions::SHOULDER_DOWN:
				arm.setShoulderDirection(Controller::Stepper::Direction::CW);
				arm.startShoulder();
				break;
			
			case Constants::Commands::Actions::WRIST_START:
				arm.setWristDirection(listener.getWristDirection());
				arm.startWrist();
				break;

			case Constants::Commands::Actions::WRIST_STOP:
				arm.stopWrist();
				break;

			case Constants::Commands::Actions::HAND_START:
				arm.startHand(listener.getHandDirection(), listener.handVelocity());
				break;

			case Constants::Commands::Actions::HAND_STOP:
				arm.stopHand();
				break;

			case Constants::Commands::Actions::NONE:
				arm.stop();
				break;
		}
	}

	subscriber.wait();

	return 0;
}