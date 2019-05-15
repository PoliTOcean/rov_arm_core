#include <iostream>
#include <thread>
#include <chrono>

#include "Controller.h"

namespace WiringPi
{
    #include "wiringPi.h"
    #include "wiringPiSPI.h"
    #include "softPwm.h"
}

using namespace Politocean::RPi;

void Controller::setup()
{
    WiringPi::wiringPiSetupPhys();
}

void Controller::pinMode(int pin, PinMode mode)
{
    WiringPi::pinMode(pin, static_cast<int>(mode));
}

void Controller::digitalWrite(int pin, PinLevel level)
{
    WiringPi::digitalWrite(pin, static_cast<int>(level));
}

void Controller::softPwmCreate(int pwmPin, int start, int stop)
{
    WiringPi::softPwmCreate(pwmPin, start, stop);
}

void Controller::softPwmWrite(int pwmPin, int value)
{
    WiringPi::softPwmWrite(pwmPin, value);
}

void Controller::softPwmStop(int pwmPin)
{
    WiringPi::softPwmStop(pwmPin);
}

void Controller::setupSPI(int device, int frequency)
{
    WiringPi::wiringPiSPISetup(device, frequency);
}

unsigned char Controller::SPIDataRW(unsigned char data)
{
    unsigned char tmp = data;

    WiringPi::wiringPiSPIDataRW(spiDevice_, &tmp, sizeof(unsigned char));

    return tmp;
}

void Controller::reset()
{
    pinMode(Pinout::RESET, PinMode::PIN_OUTPUT);

    digitalWrite(Pinout::RESET, PinLevel::PIN_LOW);
    std::this_thread::sleep_for(std::chrono::microseconds(100));

    digitalWrite(Pinout::RESET, PinLevel::PIN_HIGH);
    std::this_thread::sleep_for(std::chrono::seconds(3));
}

void Controller::setupMotors()
{
    pinMode(Pinout::MOTORS, PinMode::PIN_OUTPUT);
}

void Controller::stopMotors()
{
    motors_ = false;

    digitalWrite(Pinout::MOTORS, PinLevel::PIN_LOW);
}

void Controller::startMotors()
{
    motors_ = true;

    digitalWrite(Pinout::MOTORS, PinLevel::PIN_HIGH);
}

