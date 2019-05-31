/**
 * @author pettinz
 */

#include "MqttClient.h"
#include "Controller.h"
#include "DCMotor.h"
#include "Stepper.h"
#include "Commands.h"

#include <climits>

#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

using namespace Politocean;
using namespace Politocean::RPi;
using namespace Politocean::Constants;

class Listener
{
    Direction shoulderDirection_, wristDirection_, handDirection_, headDirection_;
    int shoulderVelocity_, wristVelocity_, handVelocity_, headVelocity_;

    std::string action_;

    bool updated_;

    void wristAxis(int axes);
    void handAxis(int axes);

public:
    Listener() :    shoulderDirection_(Direction::NONE), wristDirection_(Direction::NONE), handDirection_(Direction::NONE),
                    headDirection_(Direction::NONE), shoulderVelocity_(0), wristVelocity_(0), handVelocity_(0), headVelocity_(0),
                    updated_(false) {}

    void listenForShoulder(const std::string& payload, const std::string& topic);
    void listenForWrist(const std::string& payload, const std::string& topic);
    void listenForHand(const std::string& payload, const std::string& topic);
    void listenForHead(const std::string& payload, const std::string& topic);

    Direction shoulderDirection();
    Direction wristDirection();
    Direction handDirection();
    Direction headDirection();

    int shoulderVelocity();
    int wristVelocity();
    int handVelocity();
    int headVelocity();

    std::string action();

    bool isUpdated();
};

void Listener::listenForShoulder(const std::string& payload, const std::string& topic)
{
    if (topic == Topics::SHOULDER)
    {
        if (payload == Commands::Actions::ON)
            action_ = Commands::Skeleton::SHOULDER_ON;
        else if (payload == Commands::Actions::OFF)
            action_ = Commands::Skeleton::SHOULDER_OFF;
        else if (payload == Commands::Actions::Stepper::UP)
        {
            shoulderDirection_ = Direction::CCW;
            action_ = Commands::Skeleton::SHOULDER_STEP;
        }
        else if (payload == Commands::Actions::Stepper::DOWN)
        {
            shoulderDirection_ = Direction::CW;
            action_ = Commands::Skeleton::SHOULDER_STEP;
        }
        else if (payload == Commands::Actions::STOP)
            action_ = Commands::Skeleton::SHOULDER_STOP;
        else
        {
            shoulderDirection_ = Direction::NONE;
            action_ = Commands::Actions::NONE;
        }

        updated_ = true;
    }
    else if (topic == Topics::SHOULDER_VELOCITY)
    {

    }
    else return ;
}

void Listener::listenForWrist(const std::string& payload, const std::string& topic)
{
    if (topic == Topics::WRIST)
    {
        if (payload == Commands::Actions::ON)
            action_ = Commands::Skeleton::WRIST_ON;
        else if (payload == Commands::Actions::OFF)
            action_ = Commands::Skeleton::WRIST_OFF;
        else if (payload == Commands::Actions::START)
            action_ = Commands::Skeleton::WRIST_START;
        else if (payload == Commands::Actions::STOP)
            action_ = Commands::Skeleton::WRIST_STOP;
        else
            action_ = Commands::Actions::NONE;

        updated_ = true;
    }
    else if (topic == Topics::WRIST_VELOCITY)
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
    if (topic == Topics::HAND)
    {
        if (payload == Commands::Actions::START)
            action_ = Commands::Skeleton::HAND_START;
        else if (payload == Commands::Actions::STOP)
            action_ = Commands::Skeleton::HAND_STOP;
        else
            action_ = Commands::Actions::NONE;

        updated_ = true;
    }
    else if (topic == Topics::HAND_VELOCITY)
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

void Listener::listenForHead(const std::string& payload, const std::string& topic)
{
    if (topic == Topics::HEAD)
    {
        if (payload == Commands::Actions::ON)
            action_ = Commands::Skeleton::HEAD_ON;
        else if (payload == Commands::Actions::OFF)
            action_ = Commands::Skeleton::HEAD_OFF;
        else if (payload == Commands::Actions::Stepper::UP)
        {
            headDirection_ = Direction::CCW;
            action_ = Commands::Skeleton::HEAD_STEP;
        }
        else if (payload == Commands::Actions::Stepper::DOWN)
        {
            headDirection_ = Direction::CW;
            action_ = Commands::Skeleton::HEAD_STEP;
        }
        else if (payload == Commands::Actions::STOP)
            action_ = Commands::Skeleton::HEAD_STOP;
        else
        {
            action_ = Commands::Actions::NONE;
            headDirection_ = Direction::NONE;
        }
        
        updated_ = true;
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

    velocity = Politocean::map(velocity, 0, SHRT_MAX, DCMotor::PWM_MIN, DCMotor::PWM_MAX);

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

Direction Listener::headDirection()
{
    updated_ = false;
    
    return headDirection_;
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

int Listener::headVelocity()
{
    updated_ = false;

    return headVelocity_;
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
    logger::enableLevel(logger::INFO);
    
    MqttClient subscriber(Rov::SKELETON_ID, Rov::IP_ADDRESS);
    Listener listener;

    subscriber.subscribeTo(Topics::SHOULDER+"#",    &Listener::listenForShoulder,   &listener);
    subscriber.subscribeTo(Topics::WRIST+"#",       &Listener::listenForWrist,      &listener);
    subscriber.subscribeTo(Topics::HAND+"#",        &Listener::listenForHand,       &listener);
    subscriber.subscribeTo(Topics::HEAD+"#",        &Listener::listenForHead,       &listener);


    Controller controller;
    controller.setup();
    
    Stepper shoulder(&controller, Pinout::SHOULDER_EN, Pinout::SHOULDER_DIR, Pinout::SHOULDER_STEP);
    Stepper wrist(&controller, Pinout::WRIST_EN, Pinout::WRIST_DIR, Pinout::WRIST_STEP);
    DCMotor hand(&controller, Pinout::HAND_DIR, Pinout::HAND_PWM, DCMotor::PWM_MIN, DCMotor::PWM_MAX);

    Stepper head(&controller, Pinout::CAMERA_EN, Pinout::CAMERA_DIR, Pinout::CAMERA_STEP);

    shoulder.setup();
    wrist.setup();
    hand.setup();

    head.setup();

    while (subscriber.is_connected())
    {
        if (!listener.isUpdated()) continue ;

        std::string action = listener.action();

        if (action == Commands::Skeleton::SHOULDER_ON)
        {
            Politocean::publishComponents(Rov::SKELETON_ID, Components::SHOULDER, Commands::Actions::ON);
            shoulder.enable();
        }
        else if (action == Commands::Skeleton::SHOULDER_OFF)
        {
            Politocean::publishComponents(Rov::SKELETON_ID, Components::SHOULDER, Commands::Actions::OFF);
            shoulder.disable();
        }
        else if (action == Commands::Skeleton::SHOULDER_STEP)
        {
            shoulder.setDirection(listener.shoulderDirection());
            shoulder.setVelocity(Timing::Milliseconds::DFLT_STEPPER);
            shoulder.startStepping();
        }
        else if (action == Commands::Skeleton::SHOULDER_STOP)
            shoulder.stopStepping();
        else if (action == Commands::Skeleton::WRIST_ON)
        {
            Politocean::publishComponents(Rov::SKELETON_ID, Components::WRIST, Commands::Actions::ON);
            wrist.enable();
        }
        else if (action == Commands::Skeleton::WRIST_OFF)
        {
            Politocean::publishComponents(Rov::SKELETON_ID, Components::WRIST, Commands::Actions::OFF);
            wrist.disable();
        }
        else if (action == Commands::Skeleton::WRIST_START)
        {
            wrist.setDirection(listener.wristDirection());
            wrist.setVelocity(Timing::Milliseconds::DFLT_STEPPER);
            wrist.startStepping();
        }
        else if (action == Commands::Skeleton::WRIST_STOP)
            wrist.stopStepping();
        else if (action == Commands::Skeleton::HAND_START)
        {
            hand.setDirection(listener.handDirection());
            hand.setVelocity(listener.handVelocity());
            hand.startPwm();
        }
        else if (action == Commands::Skeleton::HAND_STOP)
            hand.stopPwm();
        else if (action == Commands::Skeleton::HEAD_ON)
            head.enable();
        else if (action == Commands::Skeleton::HEAD_OFF)
            head.disable();
        else if (action == Commands::Skeleton::HEAD_STEP)
        {
            head.setDirection(listener.headDirection());
            head.setVelocity(10);
            head.startStepping();
        }
        else if (action == Commands::Skeleton::HEAD_STOP)
            head.stopStepping();
        else continue;
    }
}