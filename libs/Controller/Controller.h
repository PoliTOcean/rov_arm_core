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

        void setup();

        Name name();
        Direction direction();
        int velocity();

        void enable();
        void disable();

        void setDirection(Direction direction);
        void setVelocity(int velocity);
        
        void step();

        bool isEnable();
    };

    class DCMotor
    {
    public:
        enum class Name { HAND };
        enum class Direction { CCW, CW, NONE };

        static const int MIN_PWM    = 20;
        static const int MAX_PWM    = 200;

    private:
        Name name_;
        Direction direction_;
        int velocity_;

        int getDirPin();
        int getPWMPin();

    public:
        DCMotor(Name name) : name_(name), direction_(Direction::NONE) {};

        Name name();
        Direction direction();
        int velocity();
        
        void setup();

        void setDirection(Direction direction);
        void setVelocity(int velocity);

        void startPWM();
        void stopPWM();
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
