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

    pinMode(Constants::Pinout::SHOULDER_DIR, OUTPUT);
    pinMode(Constants::Pinout::SHOULDER_STEPPER, OUTPUT);

    pinMode(Constants::Pinout::WRIST_DIR, OUTPUT);
    pinMode(Constants::Pinout::WRIST_STEPPER, OUTPUT);
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







int Controller::Stepper::getEnablePin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Constants::Pinout::SHOULDER_EN;
        case Name::WRIST:       return Constants::Pinout::WRIST_EN;
        default:                return -1;
    }
}

int Controller::Stepper::getDirectionPin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Constants::Pinout::SHOULDER_DIR;
        case Name::WRIST:       return Constants::Pinout::WRIST_DIR;
        default:                return -1;
    }
}

int Controller::Stepper::getStepperPin()
{
    switch (name_)
    {
        case Name::SHOULDER:    return Constants::Pinout::SHOULDER_STEPPER;
        case Name::WRIST:       return Constants::Pinout::WRIST_STEPPER;
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
    enable();

    int pin = getStepperPin();

    if (pin == -1)
        std::exit(EXIT_FAILURE);

    std::cout << "STEPPING" << std::endl;

    digitalWrite(pin, LOW);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    digitalWrite(pin, HIGH);
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

}