/**
 * @author: pettinz
 * 
 * TODO: Some logging
 */ 

#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "Controller.h"

namespace Politocean {

#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <softPwm.h>

/***********************************************************************
 * Controller class implementation
 **********************************************************************/


class Pinout
{
public:
    static const int RESET              = 7;
    static const int MOTORS             = 12;

    static const int SHOULDER_EN        = 15;
    static const int SHOULDER_DIR       = 13;
    static const int SHOULDER_STEPPER   = 11;

    static const int WRIST_EN           = 40;
    static const int WRIST_DIR          = 38;
    static const int WRIST_STEPPER      = 36;

    static const int HAND_DIR           = 16;
    static const int HAND_PWM           = 18;
};

void Controller::setup()
{
    wiringPiSetupPhys();
}

void Controller::setupSPI()
{
    wiringPiSPISetup(DEFAULT_SPI_CHANNEL, DEFAULT_SPI_SPEED);
}

void Controller::setupMotors()
{
    pinMode(Pinout::MOTORS, OUTPUT);
}

void Controller::setupArm()
{
    pinMode(Pinout::SHOULDER_EN, OUTPUT);
    digitalWrite(Pinout::SHOULDER_EN, HIGH);

    pinMode(Pinout::WRIST_EN, OUTPUT);
    digitalWrite(Pinout::WRIST_EN, HIGH);

    pinMode(Pinout::SHOULDER_DIR, OUTPUT);
    pinMode(Pinout::SHOULDER_STEPPER, OUTPUT);

    pinMode(Pinout::WRIST_DIR, OUTPUT);
    pinMode(Pinout::WRIST_STEPPER, OUTPUT);
}

unsigned char Controller::SPIDataRW(unsigned char data)
{
    unsigned char tmp;

    tmp = data;
    wiringPiSPIDataRW(DEFAULT_SPI_CHANNEL, &tmp, sizeof(unsigned char));

    return tmp;
}

void Controller::reset()
{
    pinMode(Pinout::RESET, OUTPUT);

    digitalWrite(Pinout::RESET, LOW);
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    digitalWrite(Pinout::RESET, HIGH);
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void Controller::resetArm()
{
    pinMode(Pinout::SHOULDER_EN, INPUT);
    pinMode(Pinout::SHOULDER_DIR, INPUT);
    pinMode(Pinout::SHOULDER_STEPPER, INPUT);
}

void Controller::switchMotors()
{
    motors_ = !motors_;

    digitalWrite(Pinout::SHOULDER_EN, HIGH);
    digitalWrite(Pinout::WRIST_EN, HIGH);
    
    digitalWrite(Pinout::MOTORS, motors_);
}

/***********************************************************************
 * Stepper class implementation
 **********************************************************************/


int Controller::Stepper::getEnablePin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Pinout::SHOULDER_EN;
        case Name::WRIST:       return Pinout::WRIST_EN;
        default:                return -1;
    }
}

int Controller::Stepper::getDirectionPin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Pinout::SHOULDER_DIR;
        case Name::WRIST:       return Pinout::WRIST_DIR;
        default:                return -1;
    }
}

int Controller::Stepper::getStepperPin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Pinout::SHOULDER_STEPPER;
        case Name::WRIST:       return Pinout::WRIST_STEPPER;
        default:                return -1;
    }
}

void Controller::Stepper::set(bool value)
{
    int pin = getEnablePin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, value);
}

void Controller::Stepper::enable()
{
    set(Controller::Stepper::ENABLE);
}

void Controller::Stepper::disable()
{
    set(Controller::Stepper::DISABLE);
}

void Controller::Stepper::setDirection(Direction direction)
{
    int pin = getDirectionPin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, static_cast<int>(direction));
}

void Controller::Stepper::setVelocity(int velocity)
{
    velocity_ = velocity;
}

void Controller::Stepper::step()
{
    int pin = getStepperPin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(Constants::Timing::Millisenconds::DFLT_STEPPER));

    digitalWrite(pin, HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(Constants::Timing::Millisenconds::DFLT_STEPPER));
}

/***********************************************************************
 * DCMotor class implementation
 **********************************************************************/

Controller::DCMotor::Name Controller::DCMotor::name()
{
    return name_;
}

Controller::DCMotor::Direction Controller::DCMotor::direction()
{
    return direction_;
}

int Controller::DCMotor::velocity()
{
    return velocity_;
}

void Controller::DCMotor::setup()
{
    switch (name_)
    {
    case Name::HAND:
        pinMode(Pinout::HAND_DIR, OUTPUT);
        pinMode(Pinout::HAND_PWM, OUTPUT);

        softPwmCreate(Pinout::HAND_PWM, 0, 200);
        break;

    default:
        break;
    }
}

void Controller::DCMotor::setDirection(Direction direction)
{
    int pin = getDirPin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, static_cast<int>(direction));
}

void Controller::DCMotor::setVelocity(int velocity)
{
    velocity_ = 50;
}

void Controller::DCMotor::pwm()
{
    int pin = getPWMPin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    softPwmWrite(pin, velocity_);
}

int Controller::DCMotor::getDirPin()
{
    switch (name_)
    {
        case Name::HAND:    return Pinout::HAND_DIR;
        default:            return -1;
    }
}

int Controller::DCMotor::getPWMPin()
{
    switch (name_)
    {
        case Name::HAND:        return Pinout::HAND_PWM;
        default:                return -1;
    }
}

}