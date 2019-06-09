/**
 * @author pettinz
 */

#include "MqttClient.h"
#include "Controller.h"
#include "DCMotor.h"
#include "Stepper.h"
#include "Commands.h"

#include <climits>
#include <queue>

#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

#include "ComponentsManager.hpp"

using namespace Politocean;
using namespace Politocean::RPi;
using namespace Politocean::Constants;

class Listener
{
    Direction shoulderDirection_, wristDirection_, handDirection_, headDirection_;
    int shoulderVelocity_, wristVelocity_, handVelocity_, headVelocity_;

    std::queue<std::string> actions_;

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
            actions_.push(Commands::Skeleton::SHOULDER_ON);
        else if (payload == Commands::Actions::OFF)
            actions_.push(Commands::Skeleton::SHOULDER_OFF);
        else if (payload == Commands::Actions::Stepper::UP)
        {
            shoulderDirection_ = Direction::CCW;
            actions_.push(Commands::Skeleton::SHOULDER_STEP);
        }
        else if (payload == Commands::Actions::Stepper::DOWN)
        {
            shoulderDirection_ = Direction::CW;
            actions_.push(Commands::Skeleton::SHOULDER_STEP);
        }
        else if (payload == Commands::Actions::STOP)
            actions_.push(Commands::Skeleton::SHOULDER_STOP);
        else
        {
            shoulderDirection_ = Direction::NONE;
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
            actions_.push(Commands::Skeleton::WRIST_ON);
        else if (payload == Commands::Actions::OFF)
            actions_.push(Commands::Skeleton::WRIST_OFF);
        else if (payload == Commands::Actions::START)
            actions_.push(Commands::Skeleton::WRIST_START);
        else if (payload == Commands::Actions::STOP)
            actions_.push(Commands::Skeleton::WRIST_STOP);
        
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
            actions_.push(Commands::Skeleton::HAND_START);
        else if (payload == Commands::Actions::STOP)
            actions_.push(Commands::Skeleton::HAND_STOP);

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
            actions_.push(Commands::Skeleton::HEAD_ON);
        else if (payload == Commands::Actions::OFF)
            actions_.push(Commands::Skeleton::HEAD_OFF);
        else if (payload == Commands::Actions::Stepper::UP)
        {
            headDirection_ = Direction::CCW;
            actions_.push(Commands::Skeleton::HEAD_STEP);
        }
        else if (payload == Commands::Actions::Stepper::DOWN)
        {
            headDirection_ = Direction::CW;
            actions_.push(Commands::Skeleton::HEAD_STEP);
        }
        else if (payload == Commands::Actions::STOP)
            actions_.push(Commands::Skeleton::HEAD_STOP);
        else
        {
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

    updated_ = true;
    if (velocity > 0)
        direction = Direction::CW;
    else if (velocity < 0)
    {
        direction = Direction::CCW;
        velocity = -axis;
    }
    else {
        wristVelocity_  = 0;
        wristDirection_ = Direction::NONE;
        return;
    }
    
    wristVelocity_  = Politocean::map(velocity, 0, SHRT_MAX, Timing::Microseconds::WRIST_MAX, Timing::Microseconds::WRIST_MIN);
    wristDirection_ = direction;
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
    return shoulderDirection_;
}

Direction Listener::wristDirection()
{    return wristDirection_;
}

Direction Listener::handDirection()
{
    return handDirection_;
}

Direction Listener::headDirection()
{
    return headDirection_;
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

int Listener::headVelocity()
{
    return headVelocity_;
}

std::string Listener::action()
{
    updated_ = false;

    if (actions_.empty()) return Commands::Actions::NONE;
    std::string action = actions_.front();
    actions_.pop();
    return action;
}

bool Listener::isUpdated()
{
    return !actions_.empty();
}

int main(int argc, const char *argv[])
{
    logger::enableLevel(logger::INFO);

    MqttClient& subscriber = MqttClient::getInstance(Rov::SKELETON_ID, Rov::IP_ADDRESS);
    Listener listener;

    subscriber.subscribeToFamily(Topics::SHOULDER,  &Listener::listenForShoulder,  &listener);
    subscriber.subscribeToFamily(Topics::WRIST,     &Listener::listenForWrist,     &listener);
    subscriber.subscribeToFamily(Topics::HAND,      &Listener::listenForHand,      &listener);
    subscriber.subscribeToFamily(Topics::HEAD,      &Listener::listenForHead,      &listener);

    ComponentsManager::Init(Rov::SKELETON_ID);

    Controller controller;
    controller.setup();
    
    Stepper head(&controller, Pinout::CAMERA_EN, Pinout::CAMERA_DIR, Pinout::CAMERA_STEP);
    Stepper shoulder(&controller, Pinout::SHOULDER_EN, Pinout::SHOULDER_DIR, Pinout::SHOULDER_STEP);
    Stepper wrist(&controller, Pinout::WRIST_EN, Pinout::WRIST_DIR, Pinout::WRIST_STEP);
    DCMotor hand(&controller, Pinout::HAND_DIR, Pinout::HAND_PWM, DCMotor::PWM_MIN, DCMotor::PWM_MAX);

    head.setup();
    ComponentsManager::SetComponentState(component_t::HEAD, Component::Status::DISABLED);

    shoulder.setup();
    ComponentsManager::SetComponentState(component_t::SHOULDER, Component::Status::DISABLED);

    wrist.setup();
    ComponentsManager::SetComponentState(component_t::WRIST, Component::Status::DISABLED);

    hand.setup();

    while (subscriber.is_connected())
    {
        if (!listener.isUpdated())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(Timing::Milliseconds::JOYSTICK));
            continue ;
        }

        std::string action = listener.action();

        if (action == Commands::Skeleton::SHOULDER_ON)
        {
            shoulder.enable();
            ComponentsManager::SetComponentState(component_t::SHOULDER, Component::Status::ENABLED);
        }
        else if (action == Commands::Skeleton::SHOULDER_OFF)
        {
            shoulder.disable();
            ComponentsManager::SetComponentState(component_t::SHOULDER, Component::Status::DISABLED);
        }
        else if (action == Commands::Skeleton::SHOULDER_STEP)
        {
            shoulder.setDirection(listener.shoulderDirection());
            shoulder.setVelocity(Timing::Microseconds::DFLT_STEPPER);
            shoulder.startStepping();
        }
        else if (action == Commands::Skeleton::SHOULDER_STOP)
            shoulder.stopStepping();
        else if (action == Commands::Skeleton::WRIST_ON)
        {
            wrist.enable();
            ComponentsManager::SetComponentState(component_t::WRIST, Component::Status::ENABLED);
        }
        else if (action == Commands::Skeleton::WRIST_OFF)
        {
            wrist.disable();
            ComponentsManager::SetComponentState(component_t::WRIST, Component::Status::DISABLED);
        }
        else if (action == Commands::Skeleton::WRIST_START)
        {
            wrist.setDirection(listener.wristDirection());
            wrist.setVelocity(listener.wristVelocity());
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
        {
            head.enable();
            ComponentsManager::SetComponentState(component_t::HEAD, Component::Status::ENABLED);
        }
        else if (action == Commands::Skeleton::HEAD_OFF)
        {
            head.disable();
            ComponentsManager::SetComponentState(component_t::HEAD, Component::Status::DISABLED);
        }
        else if (action == Commands::Skeleton::HEAD_STEP)
        {
            head.setDirection(listener.headDirection());
            head.setVelocity(Timing::Microseconds::DFLT_HEAD);
            head.startStepping();
        }
        else if (action == Commands::Skeleton::HEAD_STOP)
            head.stopStepping();
        else continue;
    }
}