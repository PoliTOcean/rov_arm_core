/**
 * @author: pettinz
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

namespace Politocean
{
    namespace RPi
    {
        class Pinout
        {
        public:
            static const int RESET              = 7;
            static const int MOTORS             = 12;
            
            static const int SHOULDER_EN        = 15;
            static const int SHOULDER_DIR       = 13;
            static const int SHOULDER_STEP      = 11;
            
            static const int WRIST_EN           = 40;
            static const int WRIST_DIR          = 38;
            static const int WRIST_STEP         = 36;
            
            static const int HAND_DIR           = 16;
            static const int HAND_PWM           = 18;
            
            static const int CAMERA_EN          = 33;
            static const int CAMERA_DIR         = 31;
            static const int CAMERA_STEP        = 29;
        };
        
        class Controller
        {
            int spiDevice_;
            
            bool motors_;
        
        public:
            enum class PinLevel { PIN_LOW, PIN_HIGH };
            enum class PinMode { PIN_OUTPUT, PIN_INPUT };
            
            static const int DEFAULT_SPI_CHANNEL    = 0;
            static const int DEFAULT_SPI_SPEED      = 1000000;
            
            Controller() : spiDevice_(DEFAULT_SPI_CHANNEL), motors_(false) {}
            
            void setup();

            void pinMode(int pin, PinMode mode);
            void digitalWrite(int pin, PinLevel level);
            
            void softPwmCreate(int pwmPin, int start, int stop);
            void softPwmWrite(int pwmPin, int value);
            void softPwmStop(int pwmPin);
            
            void setupSPI(int device, int frequency);
            void setupMotors();
            
            unsigned char SPIDataRW(unsigned char data);
            
            void switchMotors();
            
            void reset();
        };
    }
}


#endif //CONTROLLER_H
