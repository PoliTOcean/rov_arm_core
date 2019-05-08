/**
 * @author: pettinz
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "PolitoceanConstants.h"

namespace Politocean {

class Controller {
public:
    enum class Stepper { WRIST, SHOULDER };
    enum class Direction { CCW, CW, NONE };

private:
    bool motors_ = false;

    int getStepperEnable(Stepper stepper);
    int getStepperDirection(Stepper stepper);
    int getStepper(Stepper stepper);
public:
    static const int DEFAULT_SPI_CHANNEL    = 0;
    static const int DEFAULT_SPI_SPEED      = 1000000;

    Controller() {}

    // It sets up the controller
    void setup();

    void setupSPI();
    void setupMotors();
    void setupArm();

    void set(Stepper stepper, bool value);
    void set(Stepper stepper, Direction direction);
    
    void step(Stepper stepper);

    /**
     * It returns the value read from SPI
     * 
     * @data : value to send via SPI
     */
    unsigned char SPIDataRW(unsigned char data);

    void switchMotors();

    // It resets the controller
    void reset();

    void resetArm();
};

}


#endif //CONTROLLER_H
