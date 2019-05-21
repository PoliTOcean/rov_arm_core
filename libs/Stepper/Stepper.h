#ifndef STEPPER_H
#define STEPPER_H

#include <thread>

#include "Direction.h"
#include "Controller.h"

namespace Politocean
{
    namespace RPi
    {
        class Stepper
        {
            Controller *controller_;
            
            int enPin_, dirPin_, stepPin_;
            
            Direction direction_;
            int velocity_;

            std::thread *th_;
            bool isStepping_;
        
        public:
            Stepper(Controller *controller, int enPin, int dirPin, int stepPin) : controller_(controller), enPin_(enPin), dirPin_(dirPin), stepPin_(stepPin), isStepping_(false) {}
            
            void setup();

            void enable();
            void disable();
            
            void setDirection(Direction direction);
            void setVelocity(int velocity);

            void step();

            void startStepping();
            void stopStepping();

            bool isStepping();
        };
    }
}

#endif // STEPPER_H