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

#include "wiringPi.h"
#include "wiringPiSPI.h"

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
    pinMode(Constants::Pinout::MOTORS, OUTPUT);
}

void Controller::setupArm()
{
    pinMode(Constants::Pinout::SHOULDER_EN, OUTPUT);
    digitalWrite(Constants::Pinout::SHOULDER_EN, HIGH);

    pinMode(Constants::Pinout::WRIST_EN, OUTPUT);
    digitalWrite(Constants::Pinout::WRIST_EN, HIGH);
}

void Controller::set(Stepper stepper, bool value)
{
    int pin = getStepperEnable(stepper);

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, value);
}

void Controller::set(Stepper stepper, Direction direction)
{
    int pin = getStepperDirection(stepper);

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, static_cast<int>(direction));
}

void Controller::step(Stepper stepper)
{
    set(stepper, LOW);

    int pin = getStepper(stepper);

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    digitalWrite(pin, LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    digitalWrite(pin, HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
    pinMode(Constants::Pinout::RESET, OUTPUT);

    digitalWrite(Constants::Pinout::RESET, LOW);
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    digitalWrite(Constants::Pinout::RESET, HIGH);
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void Controller::resetArm()
{
    pinMode(Constants::Pinout::SHOULDER_EN, INPUT);
    pinMode(Constants::Pinout::SHOULDER_DIR, INPUT);
    pinMode(Constants::Pinout::SHOULDER_STEPPER, INPUT);
}

void Controller::switchMotors()
{
    motors_ = !motors_;

    digitalWrite(Constants::Pinout::SHOULDER_EN, HIGH);
    digitalWrite(Constants::Pinout::WRIST_EN, HIGH);
    
    digitalWrite(Constants::Pinout::MOTORS, motors_);
}

int Controller::getStepper(Stepper stepper)
{
    switch (stepper)
    {
        case Stepper::SHOULDER:
            return Constants::Pinout::SHOULDER_STEPPER;
        case Stepper::WRIST:
            return Constants::Pinout::WRIST_STEPPER;
        default:
            break;
    }

    return -1;
}

int Controller::getStepperDirection(Stepper stepper)
{
    switch (stepper)
    {
        case Stepper::SHOULDER:
            return Constants::Pinout::SHOULDER_DIR;
        case Stepper::WRIST:
            return Constants::Pinout::WRIST_STEPPER;
        default:
            break;
    }

    return -1;
}

int Controller::getStepperEnable(Stepper stepper)
{
    switch (stepper)
    {
        case Stepper::SHOULDER:
            return Constants::Pinout::SHOULDER_EN;
        case Stepper::WRIST:
            return Constants::Pinout::WRIST_EN;
        default:
            break;
    }

    return -1;
}

}