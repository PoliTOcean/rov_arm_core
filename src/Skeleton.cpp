#include "Subscriber.h"
#include "Controller.h"
#include "PwmMotor.h"
#include "Stepper.h"
#include "Commands.h"

#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"

using namespace Politocean;
using namespace Politocean::RPi;

class Listener
{
    Direction shoulderDirection_, wristDirection_, handDirection_;
    int shoulderVelocity_, wristVelocity_, handVelocity_;

    std::string action_;

    bool updated_;

public:
    void listenForShoulder(const std::string& payload);

    void listenForWrist(const std::string& payload);
    void listenForWristDirectionAndVelocity(const std::string& payload);

    void listenForHand(const std::string& payload);
    void listenForHandDirectionAndVelocity(const std::string& payload);

    Direction shoulderDirection();
    Direction wristDirection();
    Direction handDirection();

    int shoulderVelocity();
    int wristVelocity();
    int handVelocity();

    std::string action();

    bool isUpdated();
};

void Listener::listenForShoulder(const std::string& payload)
{
    std::cout << payload << std::endl;

    if (payload == Constants::Commands::Actions::ON)
        action_ = Commands::Actions::SHOULDER_ON;
    else if (payload == Constants::Commands::Actions::OFF)
        action_ = Commands::Actions::SHOULDER_OFF;
    else if (payload == Constants::Commands::Actions::Arm::SHOULDER_UP)
    {
        action_ = Commands::Actions::SHOULDER_STEP;
        shoulderDirection_ = Direction::CCW;
    }
    else if (payload == Constants::Commands::Actions::Arm::SHOULDER_DOWN)
    {
        shoulderDirection_ = Direction::CW;
        action_ = Commands::Actions::SHOULDER_STEP;
    }
    else
    {
        action_ = Constants::Commands::Actions::NONE;
        shoulderDirection_ = Direction::NONE;
    }

    updated_ = true;
}

void Listener::listenForWrist(const std::string& payload)
{
    if (payload == Constants::Commands::Actions::ON)
        action_ = Commands::Actions::WRIST_ON;
    else if (payload == Constants::Commands::Actions::OFF)
        action_ = Commands::Actions::WRIST_OFF;
    else if (payload == Constants::Commands::Actions::START)
        action_ = Commands::Actions::WRIST_START;
    else if (payload == Constants::Commands::Actions::STOP)
        action_ = Commands::Actions::WRIST_STOP;
    else
        action_ = Constants::Commands::Actions::NONE;

    updated_ = true;
}

void Listener::listenForWristDirectionAndVelocity(const std::string& payload)
{
    int velocity = std::stoi(payload);

    if (velocity > 0)
    {
        wristDirection_ = Direction::CCW;
        wristVelocity_ = velocity;
    }
    else if (velocity < 0)
    {
        wristDirection_ = Direction::CW;
        wristVelocity_ = -velocity;
    }
    else
    {
        wristDirection_ = Direction::NONE;
        wristVelocity_ = 0;
    }

    updated_ = true;
}

void Listener::listenForHand(const std::string& payload)
{
    if (payload == Constants::Commands::Actions::START)
        action_ = Commands::Actions::HAND_START;
    else if (payload == Constants::Commands::Actions::STOP)
        action_ = Commands::Actions::HAND_STOP;
    else
        action_ = Constants::Commands::Actions::NONE;

    updated_ = true;
}

void Listener::listenForHandDirectionAndVelocity(const std::string& payload)
{
    int velocity = std::stoi(payload);

    if (velocity > 0)
    {
        handDirection_ = Direction::CCW;
        handVelocity_ = velocity;
    }
    else if (velocity < 0)
    {
        handDirection_ = Direction::CW;
        handVelocity_ = -velocity;
    }
    else
    {
        handDirection_ = Direction::NONE;
        handVelocity_ = 0;
    }

    updated_ = true;
}

Direction Listener::shoulderDirection()
{
    updated_ = false;

    return shoulderDirection_;
}

Direction Listener::wristDirection()
{
    updated_ = false;

    return wristDirection_;
}

Direction Listener::handDirection()
{
    updated_ = false;

    return handDirection_;
}

int Listener::shoulderVelocity()
{
    updated_ = false;

    return shoulderVelocity_;
}

int Listener::wristVelocity()
{
    updated_ = false;

    return wristVelocity_;
}

int Listener::handVelocity()
{
    updated_ = false;

    return handVelocity_;
}

string Listener::action()
{
    updated_ = false;

    return action_;
}

bool Listener::isUpdated()
{
    return updated_;
}


int main(int argc, const char *argv[])
{
    Subscriber subscriber(Constants::Rov::IP_ADDRESS, Constants::Rov::SKELETON_ID);
    Listener listener;

    subscriber.subscribeTo(Constants::Topics::SHOULDER,         &Listener::listenForShoulder,                   &listener);
    subscriber.subscribeTo(Constants::Topics::WRIST,            &Listener::listenForWrist,                      &listener);
    subscriber.subscribeTo(Constants::Topics::WRIST_VELOCITY,   &Listener::listenForWristDirectionAndVelocity,  &listener);
    subscriber.subscribeTo(Constants::Topics::HAND,             &Listener::listenForHand,                       &listener);
    subscriber.subscribeTo(Constants::Topics::HAND_VELOCITY,    &Listener::listenForHandDirectionAndVelocity,   &listener);

    try
    {
        subscriber.connect();
    }
    catch (const mqttException& e)
    {
        std::cerr << e.what() << '\n';
    }

    Controller controller;
    controller.setup();
    
    Stepper shoulder(&controller, Pinout::SHOULDER_EN, Pinout::SHOULDER_DIR, Pinout::SHOULDER_STEP);
    Stepper wrist(&controller, Pinout::WRIST_EN, Pinout::WRIST_DIR, Pinout::WRIST_STEP);
    PwmMotor hand(&controller, Pinout::HAND_DIR, Pinout::HAND_PWM, PwmMotor::PWM_MIN, PwmMotor::PWM_MAX);

    shoulder.setup();
    wrist.setup();
    hand.setup();

    while (subscriber.is_connected())
    {
        std::string action = listener.action();

        if (action == Commands::Actions::SHOULDER_ON)
            shoulder.enable();
        else if (action == Commands::Actions::SHOULDER_OFF)
            shoulder.disable();
        else if (action == Commands::Actions::SHOULDER_STEP)
        {
            shoulder.setDirection(listener.shoulderDirection());
            shoulder.setVelocity(Constants::Timing::Millisenconds::DFLT_STEPPER);
            shoulder.startStepping();
        }
        else if (action == Commands::Actions::WRIST_ON)
            wrist.enable();
        else if (action == Commands::Actions::WRIST_OFF)
            wrist.disable();
        else if (action == Commands::Actions::WRIST_START)
        {
            wrist.setDirection(listener.wristDirection());
            wrist.setVelocity(listener.wristVelocity());
            wrist.startStepping();
        }
        else if (action == Commands::Actions::WRIST_STOP)
        {
            wrist.setDirection(listener.wristDirection());
            wrist.setVelocity(listener.wristVelocity());
            wrist.startStepping();
        }
        else if (action == Commands::Actions::HAND_START)
        {
            hand.setDirection(listener.handDirection());
            hand.setVelocity(listener.handVelocity());
            hand.startPwm();
        }
        else if (action == Commands::Actions::HAND_STOP)
            hand.stopPwm();
        else continue;
    }
}