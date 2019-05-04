/**
 * @author: pettinz
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <climits>

namespace Politocean {

namespace PINOUT {
    static const int RESET = 7;
}

class Controller {
public:
    static const int DEFAULT_SPI_CHANNEL    = 0;
    static const int DEFAULT_SPI_SPEED      = 1000000;

    Controller() = default;

    // It sets up the controller
    void setup();

    /**
     * It returns the value read from SPI
     * 
     * @data : value to send via SPI
     */
    unsigned char SPIDataRW(unsigned char data);

    // It resets the controller
    void reset();
};

}


#endif //CONTROLLER_H
