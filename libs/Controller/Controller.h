/**
 * @author: pettinz
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "PolitoceanConstants.h"

namespace Politocean {

class Controller {
    bool motors_;

public:
    static const int DEFAULT_SPI_CHANNEL    = 0;
    static const int DEFAULT_SPI_SPEED      = 1000000;

    Controller() : motors_(false) {}

    // It sets up the controller
    void setup();

    /**
     * It returns the value read from SPI
     * 
     * @data : value to send via SPI
     */
    unsigned char SPIDataRW(unsigned char data);

    void switchMotors();

    // It resets the controller
    void reset();
};

}


#endif //CONTROLLER_H
