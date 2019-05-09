/**
 * @author: pettinz
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "PolitoceanConstants.h"

namespace Politocean {

class Controller
{

public:
    class Pinout
    {
        static const int RESET              = 7;
        static const int MOTORS             = 12;

        static const int SHOULDER_EN        = 15;
        static const int SHOULDER_DIR       = 13;
        static const int SHOULDER_STEPPER   = 11;
            
        static const int WRIST_EN             = 40;
        static const int WRIST_DIR            = 38;
        static const int WRIST_STEPPER        = 36;
    }

    class Stepper
    {
    public:
        enum class Name { WRIST, SHOULDER };
        enum class Direction { CCW, CW, NONE };

        static const int ENABLE     = 0;
        static const int DISABLE    = 1;

    private:
        Name name_;
        Direction direction_;
        int velocity_;

        bool enable_;

        int getStepperPin();
        int getEnablePin();
        int getDirectionPin();

        void set(bool value);

    public:
        Stepper(Name name) : name_(name), direction_(Direction::NONE), velocity_(0), enable_(false) {}
         
        int velocity();
        Name name();
        Direction direction();

        void enable();
        void disable();

        void setDirection(Direction direction);
        void setVelocity(int velocity);
        
        void step();

        bool isEnable();
    };

private:
    bool motors_ = false;

public:
    static const int DEFAULT_SPI_CHANNEL    = 0;
    static const int DEFAULT_SPI_SPEED      = 1000000;

    Controller() = default;

    // It sets up the controller
    void setup();

    void setupSPI();
    void setupMotors();
    void setupArm();

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
