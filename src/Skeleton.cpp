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

    void calculateFromAxes(int axes, Direction *direction, int *velocity);

public:
    void listenForShoulder(const std::string& payload, const std::string& topic);
    void listenForWrist(const std::string& payload, const std::string& topic);
    void listenForHand(const std::string& payload, const std::string& topic);
    
    Direction shoulderDirection();
    Direction wristDirection();
    Direction handDirection();

    int shoulderVelocity();
    int wristVelocity();
    int handVelocity();

    std::string action();
};

void Listener::listenForShoulder(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::SHOULDER)
    {
        if (payload == Commands::Actions::SHOULDER_ON)
            action_ = Commands::Actions::SHOULDER_ON;
        else if (payload == Commands::Actions::SHOULDER_OFF)
            action_ = Commands::Actions::SHOULDER_OFF;
        else if (payload == Commands::Actions::SHOULDER_UP)
            action_ = Commands::Actions::SHOULDER_UP;
        else if (payload == Commands::Actions::SHOULDER_DOWN)
            action_ = Commands::Actions::SHOULDER_DOWN;
        else
            action_ = Commands::Actions::NONE;
    }
    else if (topic == Constants::Topics::SHOULDER_VELOCITY)
        calculateFromAxes(std::stoi(payload), &shoulderDirection_, &shoulderVelocity_);
    else return ;
}

void Listener::listenForWrist(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::WRIST)
    {
        if (payload == Commands::Actions::WRIST_ON)
            action_ = Commands::Actions::WRIST_ON;
        else if (payload == Commands::Actions::WRIST_OFF)
            action_ = Commands::Actions::WRIST_OFF;
        else if (payload == Commands::Actions::WRIST_START)
            action_ = Commands::Actions::WRIST_START;
        else if (payload == Commands::Actions::WRIST_STOP)
            action_ = Commands::Actions::WRIST_STOP;
        else
            action_ = Commands::Actions::NONE;
    }
    else if (topic == Constants::Topics::WRIST_VELOCITY)
        calculateFromAxes(std::stoi(payload), &wristDirection_, &wristVelocity_);
    else return ;
}

void Listener::listenForHand(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::HAND)
    {
        if (payload == Commands::Actions::HAND_START)
            action_ = Commands::Actions::HAND_START;
        else if (payload == Commands::Actions::HAND_STOP)
            action_ = Commands::Actions::HAND_STOP;
        else
            action_ = Commands::Actions::NONE;
    }
    else if (topic == Constants::Topics::HAND_VELOCITY)
        calculateFromAxes(std::stoi(payload), &handDirection_, &handVelocity_);
    else return ;
    
}

Direction Listener::shoulderDirection()
{
    return shoulderDirection_;
}

Direction Listener::wristDirection()
{
    return wristDirection_;
}

Direction Listener::handDirection()
{
    return handDirection_;
}

int Listener::shoulderVelocity()
{
    return shoulderVelocity_;
}

int Listener::wristVelocity()
{
    return wristVelocity_;
}

int Listener::handVelocity()
{
    return handVelocity_;
}

std::string Listener::action()
{
    return action_;
}

void Listener::calculateFromAxes(int axes, Direction *direction, int *velocity)
{
    if (axes > 0)
    {
        *direction = Direction::CCW;
        *velocity = axes;
    }
    else if (axes < 0)
    {
        *direction = Direction::CW;
        *velocity = -axes;
    }
    else
    {
        *direction = Direction::NONE;
        *velocity = 0;
    }
}


int main(int argc, const char *argv[])
{
    Subscriber subscriber(Constants::Rov::IP_ADDRESS, Constants::Rov::SKELETON_ID);
    Listener listener;

    subscriber.subscribeTo(Constants::Topics::SHOULDER,         &Listener::listenForShoulder,                   &listener);
    subscriber.subscribeTo(Constants::Topics::WRIST,            &Listener::listenForWrist,                      &listener);
    subscriber.subscribeTo(Constants::Topics::HAND,             &Listener::listenForHand,                       &listener);

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
        else if (action == Commands::Actions::SHOULDER_UP)
        {
            shoulder.setDirection(listener.shoulderDirection());
            shoulder.setVelocity(listener.shoulderVelocity());
            shoulder.startStepping();
        }
        else if (action == Commands::Actions::SHOULDER_DOWN)
        {
            shoulder.setDirection(listener.shoulderDirection());
            shoulder.setVelocity(listener.shoulderVelocity());
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