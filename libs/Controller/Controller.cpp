/**
 * @author: pettinz
 * 
 * TODO: Some logging
 */ 

#include <iostream>

#include "Controller.h"

namespace Politocean {

#include "wiringPi.h"
#include "wiringPiSPI.h"

#include "unistd.h"

void Controller::setup()
{
    wiringPiSetup();
    wiringPiSPISetup(DEFAULT_SPI_CHANNEL, DEFAULT_SPI_SPEED);

    pinMode(Constants::Pinout::MOTORS, OUTPUT);

    std::cout << "Controller is ready" << std::endl;
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
    digitalWrite(Constants::Pinout::RESET, LOW);
    usleep(100);
    digitalWrite(Constants::Pinout::RESET, HIGH);
    sleep(3);
}

void Controller::switchMotors()
{
    motors_ = !motors_;
    
    digitalWrite(Constants::Pinout::MOTORS, motors_);
}

}