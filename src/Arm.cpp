#include "Subscriber.h"
#include "Controller.h"

#include <thread>
#include <chrono>

#include "PolitoceanConstants.h"

using namespace Politocean;

/*******************************************************
 * Listener class for button 
 ******************************************************/

class ButtonListener {
	int button_;

	bool isListening_ = false, isUpdated_ = false;

public:
	void listen(const std::string& payload);

	int button();

	bool isListening();
	bool isUpdated();
};

void ButtonListener::listen(const std::string& payload)
{
	button_ = std::stoi(payload);
	isUpdated_ = true;
	isListening_ = true;
}

int ButtonListener::button()
{
	isUpdated_ = false;

	return button_;
}

bool ButtonListener::isListening()
{
	return isListening_;
}

bool ButtonListener::isUpdated()
{
	return isUpdated_;
}

/*******************************************************
 * Listener class for stepper subscriber
 ******************************************************/

class StepperListener {
	Controller::Direction direction_;

public:
	void listen(const std::string& payload);

	Controller::Direction getDirection();
};

void StepperListener::listen(const std::string& payload)
{
	if (payload == to_string(Constants::Commands::Actions::SHOULDER_UP))
		direction_ = Controller::Direction::CW;
	else if (payload == to_string(Constants::Commands::Actions::SHOULDER_DOWN))
		direction_ = Controller::Direction::CCW;
	else
		direction_ = Controller::Direction::NONE;
}

Controller::Direction StepperListener::getDirection()
{
	return direction_;
}

/*******************************************************
 * Arm class
 ******************************************************/

class Arm {
	std::thread *shoulderThread, *wristThread;
	bool isShouldering_ = false, isWristing_ = false, isMoving_ = false;

	Controller controller;

public:
	Arm() : controller() {}

	void setup();

	void start(StepperListener &shoulderListener, StepperListener &wristListener);
	void start(Controller::Stepper stepper, StepperListener &listener);

	void stop();
	void stop(Controller::Stepper stepper);

	bool isShouldering();
	bool isWristing();
	bool isMoving();
};

void Arm::setup()
{
	controller.setupArm();
}

void Arm::start(StepperListener &shoulderListener, StepperListener &wristListener)
{
	start(Controller::Stepper::SHOULDER, shoulderListener);
	start(Controller::Stepper::WRIST, wristListener);
}

void Arm::start(Controller::Stepper stepper, StepperListener &listener)
{
	switch (stepper)
	{
		case Controller::Stepper::SHOULDER:
			if (isShouldering_)
				break;

			std::cout << "Starting shoulder...\n";

			isShouldering_ = true;
			isMoving_ = true;
			shoulderThread = new std::thread([&]() {
				while (isShouldering_)
				{
					Controller::Direction direction = listener.getDirection();

					if (direction == Controller::Direction::NONE)
					{
						controller.set(Controller::Stepper::SHOULDER, 1);
						continue;
					}

					controller.set(Controller::Stepper::SHOULDER, direction);
					controller.step(Controller::Stepper::SHOULDER);		
				}
			});
		break;

		case Controller::Stepper::WRIST:
			if (isWristing_)
				break;

			std::cout << "Starting wrist...\n";

			isWristing_ = true;
			isMoving_ = true;
			wristThread = new std::thread([&]() {
				while (isWristing_)
				{
					Controller::Direction direction = listener.getDirection();

					if (direction == Controller::Direction::NONE)
					{
						controller.set(Controller::Stepper::WRIST, 1);
						continue;
					}

					controller.set(Controller::Stepper::WRIST, direction);
					controller.step(Controller::Stepper::WRIST);
				}
			});
		break;

		default: break;
	}
}

void Arm::stop()
{
	isShouldering_ 	= false;
	isWristing_ 	= false;
	isMoving_ 		= false;

	std::cout << "Stopping all...\n";
}

void Arm::stop(Controller::Stepper stepper)
{
	switch (stepper)
	{
		case Controller::Stepper::SHOULDER:
			isShouldering_ = false;

			std::cout << "Stopping shoulder...\n";
			if (!isWristing_)
				isMoving_ = false;
		break;

		case Controller::Stepper::WRIST:
			isWristing_ = false;

			std::cout << "Stopping wrist...\n";
			if (!isShouldering_)
				isMoving_ = false;
		break;

		default: break;
	}
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

/*******************************************************
 * Main section
 ******************************************************/

int main (void)
{
	Subscriber subscriber(Constants::Hmi::IP_ADDRESS, Constants::Rov::ARM_ID);
	StepperListener shoulderListener, wristListener;
	ButtonListener buttonListener;

	Controller controller;

	subscriber.subscribeTo(Constants::Topics::ARM_SHOULDER, &StepperListener::listen, &shoulderListener);
	subscriber.subscribeTo(Constants::Topics::ARM_WRIST, 	&StepperListener::listen, &wristListener);
	subscriber.subscribeTo(Constants::Topics::BUTTONS, 		&ButtonListener::listen, &buttonListener);

	subscriber.connect();

	Arm arm;

	arm.setup();
	
	while (subscriber.is_connected())
	{
		if (!buttonListener.isUpdated())
			continue ;

		switch (buttonListener.button())
		{
			case Constants::Commands::Actions::WRIST_OFF:
				arm.stop(Controller::Stepper::WRIST);
				break;

			case Constants::Commands::Actions::WRIST_ON:
				arm.start(Controller::Stepper::WRIST, wristListener);
				break;

			case Constants::Commands::Actions::SHOULDER_OFF:
				arm.stop(Controller::Stepper::SHOULDER);
				break;

			case Constants::Commands::Actions::SHOULDER_ON:
				arm.start(Controller::Stepper::SHOULDER, shoulderListener);
				break;
		}
	}

	subscriber.wait();

	controller.resetArm();

	return 0;
}
