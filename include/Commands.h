/**
 * @author: pettinz
 */

#ifndef COMMANDS_H
#define COMMANDS_H

using namespace std;

namespace Politocean {
namespace Constants {
namespace Commands {

    namespace ATMega
    {
        namespace Axis
        {
            const short X_AXES  = 0;
            const short Y_AXES  = 1;
            const short RZ_AXES = 2;
        }

        namespace SPI
        {
            const unsigned char NONE                = 0x00;
            const unsigned char VDOWN_ON            = 0x04;
            const unsigned char VDOWN_OFF           = 0x05;
            const unsigned char VUP_ON              = 0x06;
            const unsigned char VUP_OFF             = 0x07;
            const unsigned char FAST                = 0x0D;
            const unsigned char SLOW                = 0x0E;
            const unsigned char MEDIUM              = 0x0C;
            const unsigned char START_AND_STOP      = 0x12;
            const unsigned char VUP_FAST_ON         = 0x13;
            const unsigned char VUP_FAST_OFF        = 0x14;

            namespace Delims
            {
                const unsigned char AXES         = 0xFF;
                const unsigned char COMMAND      = 0x00;
                const unsigned char SENSORS      = 0xFF;
            }
        }
    }

    namespace Skeleton
    {
        const string SHOULDER_ON    = "SHOULDER_ON";
        const string SHOULDER_OFF   = "SHOULDER_OFF";
        const string SHOULDER_UP    = "SHOULDER_UP";
        const string SHOULDER_DOWN  = "SHOULDER_DOWN";
        const string SHOULDER_STEP  = "SHOULDER_STEP";
        const string SHOULDER_STOP  = "SHOULDER_STOP";

        const string WRIST_ON       = "WRIST_ON";
        const string WRIST_OFF      = "WRIST_OFF";
        const string WRIST_START    = "WRIST_START";
        const string WRIST_STOP     = "WRIST_STOP";
        
        const string HAND_START     = "HAND_START";
        const string HAND_STOP      = "HAND_STOP";

        const string HEAD_ON        = "HEAD_ON";
        const string HEAD_OFF       = "HEAD_OFF";
        const string HEAD_UP        = "HEAD_UP";
        const string HEAD_DOWN      = "HEAD_DOWN";
        const string HEAD_STEP      = "HEAD_STEP";
        const string HEAD_STOP      = "HEAD_STOP";

        const string NONE           = "NONE";
    }

}
}
}

#endif //COMMANDS_H