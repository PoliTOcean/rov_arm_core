/**
 * @author: pettinz
 * 
 * TODO: Some logging
 */ 

#include <iostream>

#include "Controller.h"
#include "PolitoceanFunctions.hpp"

namespace Politocean {

#include "wiringPi.h"
#include "wiringPiSPI.h"

#include "unistd.h"

Controller::Controller()
{
    wiringPiSetup();
    wiringPiSPISetup(DEFAULT_SPI_CHANNEL, DEFAULT_SPI_SPEED);

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
    digitalWrite(PINOUT::RESET, LOW);
    usleep(100);
    digitalWrite(PINOUT::RESET, HIGH);
    sleep(3);
}

}