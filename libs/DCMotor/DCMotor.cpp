#include "DCMotor.h"

using namespace Politocean::RPi;

void DCMotor::setup()
{
    controller_.pinMode(dirPin_, Controller::PinMode::PIN_OUTPUT);
    controller_.pinMode(pwmPin_, Controller::PinMode::PIN_OUTPUT);
}

void DCMotor::setDirection(Direction direction)
{
    direction_ = direction;

    if (direction == Direction::CW)
        controller_.digitalWrite(dirPin_, Controller::PinLevel::PIN_LOW);
    else if (direction_ == Direction::CCW)
        controller_.digitalWrite(dirPin_, Controller::PinLevel::PIN_HIGH);
    else return ;
}

void DCMotor::setVelocity(int velocity)
{
    velocity_ = velocity;
}

void DCMotor::startPwm()
{
    if (isPwming_)
        return ;

    controller_.setPwm(pwmPin_, velocity_);
}

void DCMotor::stopPwm()
{
    isPwming_ = false;

    controller_.setPwm(pwmPin_, 0);
}

bool DCMotor::isPwming()
{
    return isPwming_;
}