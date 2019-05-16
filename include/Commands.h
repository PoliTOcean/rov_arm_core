/**
 * @author: pettinz
 */

#ifndef COMMANDS_H
#define COMMANDS_H

using namespace std;

namespace Commands {

    namespace Actions
    {
        const string SHOULDER_ON    = "SHOULDER_ON";
        const string SHOULDER_OFF   = "SHOULDER_OFF";
        const string SHOULDER_UP    = "SHOULDER_UP";
        const string SHOULDER_DOWN  = "SHOULDER_DOWN";
        const string SHOULDER_STEP  = "SHOULDER_STEP";

        const string WRIST_ON       = "WRIST_ON";
        const string WRIST_OFF      = "WRIST_OFF";
        const string WRIST_START    = "WRIST_START";
        const string WRIST_STOP     = "WRIST_STOP";

        const string HAND_START     = "HAND_START";
        const string HAND_STOP      = "HAND_STOP";

        const string NONE           = "NONE";
    }

    namespace SPI
    {
        const unsigned char VDOWN_ON           = 0x04;
        const unsigned char VDOWN_OFF          = 0x05;
        const unsigned char VUP_ON             = 0x06;
        const unsigned char VUP_OFF            = 0x07;
        const unsigned char FAST               = 0x0D;
        const unsigned char SLOW               = 0x0E;
        const unsigned char MEDIUM             = 0x0C;
        const unsigned char AUTONOMOUS_ON      = 0x10;
        const unsigned char AUTONOMOUS_OFF     = 0x11;
        const unsigned char START_AND_STOP     = 0x12;
    }

}


#endif //COMMANDS_H