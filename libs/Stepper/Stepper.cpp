#include "Stepper.h"
#include <iostream>

using namespace Politocean::RPi;

void Stepper::setup()
{
    controller_->pinMode(enPin_, Controller::PinMode::PIN_OUTPUT);
    controller_->pinMode(dirPin_, Controller::PinMode::PIN_OUTPUT);
    controller_->pinMode(stepPin_, Controller::PinMode::PIN_OUTPUT);
            
    controller_->digitalWrite(enPin_, Controller::PinLevel::PIN_HIGH);
}

void Stepper::enable()
{
    controller_->digitalWrite(enPin_, Controller::PinLevel::PIN_LOW);
}

void Stepper::disable()
{
    controller_->digitalWrite(enPin_, Controller::PinLevel::PIN_HIGH);
}

void Stepper::setDirection(Direction direction)
{
    direction_ = direction;

    if (direction_ == Direction::CW)
        controller_->digitalWrite(dirPin_, Controller::PinLevel::PIN_LOW);
    else if (direction_ == Direction::CCW)
        controller_->digitalWrite(dirPin_, Controller::PinLevel::PIN_HIGH);
    else return ;
}

void Stepper::setVelocity(int velocity)
{
    velocity_ = velocity;
}

void Stepper::step()
{   
    controller_->digitalWrite(stepPin_, Controller::PinLevel::PIN_LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(velocity_));

    controller_->digitalWrite(stepPin_, Controller::PinLevel::PIN_HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(velocity_));
}

void Stepper::startStepping()
{
    if (isStepping_)
        return;

    isStepping_ = true;
    th_ = new std::thread([&] {
        while (isStepping_)
            step();
    });

}

void Stepper::stopStepping()
{
    isStepping_ = false;
}

bool Stepper::isStepping()
{
    return isStepping_;
}