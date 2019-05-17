#include "Subscriber.h"
#include "Controller.h"
#include "PwmMotor.h"
#include "Stepper.h"
#include "Commands.h"

#include <climits>

#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

using namespace Politocean;
using namespace Politocean::RPi;

class Listener
{
    Direction shoulderDirection_, wristDirection_, handDirection_;
    int shoulderVelocity_, wristVelocity_, handVelocity_;

    std::string action_;

    bool updated_;

    void wristAxis(int axes);
    void handAxis(int axes);

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

    bool isUpdated();
};

void Listener::listenForShoulder(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::SHOULDER)
    {
        if (payload == Constants::Commands::Actions::ON)
            action_ = Commands::Actions::SHOULDER_ON;
        else if (payload == Constants::Commands::Actions::OFF)
            action_ = Commands::Actions::SHOULDER_OFF;
        else if (payload == Constants::Commands::Actions::Arm::SHOULDER_UP)
        {
            shoulderDirection_ = Direction::CCW;
            action_ = Commands::Actions::SHOULDER_STEP;
        }
        else if (payload == Constants::Commands::Actions::Arm::SHOULDER_DOWN)
        {
            shoulderDirection_ = Direction::CW;
            action_ = Commands::Actions::SHOULDER_STEP;
        }
        else if (payload == Constants::Commands::Actions::STOP)
            action_ = Commands::Actions::SHOULDER_STOP;
        else
        {
            shoulderDirection_ = Direction::NONE;
            action_ = Constants::Commands::Actions::NONE;
        }

        updated_ = true;
    }
    else if (topic == Constants::Topics::SHOULDER_VELOCITY)
    {

    }
    else return ;
}

void Listener::listenForWrist(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::WRIST)
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
    else if (topic == Constants::Topics::WRIST_VELOCITY)
        try
        {
            int axis = std::stoi(payload);
            wristAxis(axis);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    else return ;
}

void Listener::listenForHand(const std::string& payload, const std::string& topic)
{
    if (topic == Constants::Topics::HAND)
    {
        if (payload == Constants::Commands::Actions::START)
            action_ = Commands::Actions::HAND_START;
        else if (payload == Constants::Commands::Actions::STOP)
            action_ = Commands::Actions::HAND_STOP;
        else
            action_ = Constants::Commands::Actions::NONE;

        updated_ = true;
    }
    else if (topic == Constants::Topics::HAND_VELOCITY)
    {
        try
        {
            int axis = std::stoi(payload);
            handAxis(axis);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    else return ;
}

void Listener::wristAxis(int axis)
{
    int velocity        = axis;
    Direction direction = Direction::NONE;

    if (velocity > 0)
        direction = Direction::CCW;
    else if (velocity < 0)
    {
        direction = Direction::CW;
        velocity = -axis;
    }
    else {}

    if (wristVelocity_ == velocity && wristDirection_ == direction)
        return ;

    wristVelocity_  = velocity;
    wristDirection_ = direction;

    updated_ = true;
}

void Listener::handAxis(int axis)
{
    int velocity        = axis;
    Direction direction = Direction::NONE;

    if (velocity > 0)
        direction = Direction::CCW;
    else if (velocity < 0)
    {
        direction = Direction::CW;
        velocity = -axis;
    }
    else {}

    velocity = Politocean::map(velocity, 0, SHRT_MAX, PwmMotor::PWM_MIN, PwmMotor::PWM_MAX);

    if (handVelocity_ == velocity && handDirection_ == direction)
        return ;

    handVelocity_  = velocity;
    handDirection_ = direction;

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

std::string Listener::action()
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
        if (!listener.isUpdated()) continue ;

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
        else if (action == Commands::Actions::SHOULDER_STOP)
            shoulder.stopStepping();
        else if (action == Commands::Actions::WRIST_ON)
            wrist.enable();
        else if (action == Commands::Actions::WRIST_OFF)
            wrist.disable();
        else if (action == Commands::Actions::WRIST_START)
        {
            wrist.setDirection(listener.wristDirection());
            wrist.setVelocity(Constants::Timing::Millisenconds::DFLT_STEPPER);
            wrist.startStepping();
        }
        else if (action == Commands::Actions::WRIST_STOP)
            wrist.stopStepping();
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