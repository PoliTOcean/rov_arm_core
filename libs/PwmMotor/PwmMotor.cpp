#include "PwmMotor.h"

using namespace Politocean::RPi;

void PwmMotor::setup()
{
    controller_->pinMode(dirPin_, Controller::PinMode::PIN_OUTPUT);
    controller_->pinMode(pwmPin_, Controller::PinMode::PIN_OUTPUT);
}

void PwmMotor::setDirection(Direction direction)
{
    direction_ = direction;

    if (direction == Direction::CW)
        controller_->digitalWrite(dirPin_, Controller::PinLevel::PIN_LOW);
    else if (direction_ == Direction::CCW)
        controller_->digitalWrite(dirPin_, Controller::PinLevel::PIN_HIGH);
    else return ;
}

void PwmMotor::setVelocity(int velocity)
{
    velocity_ = velocity;
}

void PwmMotor::startPwm()
{
    if (isPwming_)
        return ;
    
    controller_->softPwmCreate(pwmPin_, minPwm_, maxPwm_);

    isPwming_ = true;
    th_ = new std::thread([&]() {
        while (isPwming_)
            controller_->softPwmWrite(pwmPin_, velocity_);
    });
}

void PwmMotor::stopPwm()
{
    isPwming_ = false;
    controller_->softPwmStop(pwmPin_);
}

bool PwmMotor::isPwming()
{
    isPwming_ = false;
}