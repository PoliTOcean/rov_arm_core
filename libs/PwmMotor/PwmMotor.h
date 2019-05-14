#ifndef PWM_MOTOR_H
#define PWM_MOTOR_H

#include <thread>

#include "Direction.h"
#include "Controller.h"

namespace Politocean
{
    namespace RPi
    {
        class PwmMotor
        {
            Controller *controller_;

            int dirPin_, pwmPin_, minPwm_, maxPwm_;
            
            Direction direction_;
            int velocity_;

            std::thread *th_;
            bool isPwming_;
        
        public:
            static const int PWM_MIN = 20;
            static const int PWM_MAX = 200;

            PwmMotor(Controller *controller, int dirPin, int pwmPin, int minPwm, int maxPwm) :
                controller_(controller), dirPin_(dirPin), pwmPin_(pwmPin), minPwm_(minPwm), maxPwm_(maxPwm), isPwming_(false) {}
            
            void setup();
            
            void setDirection(Direction direction);
            void setVelocity(int velocity);

            void startPwm();
            void stopPwm();

            bool isPwming();
        };
    }
}

#endif // PWM_MOTOR_H