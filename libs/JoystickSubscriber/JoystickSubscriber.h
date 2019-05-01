/**
 * @author pettinz
 */

#include "Subscriber.h"
#include "Sensor.h"
#include "Controller.h"

#include <string>
#include <vector>

namespace Politocean
{

class JoystickSubscriber : public Subscriber
{
    Controller controller_;

    /**
     * @axesBuffer_     : contains the axes values to send via SPI
     * @buttonsBuffer_  : contains the buttons values to send via SPI
     */
    std::vector<int> axesBuffer_, buttonsBuffer_;
    std::vector<Sensor<unsigned char>> sensors_;

    /**
     * @current_: the index of the current value of the SPI buffer
     */
    int current_;

public:
    /**
     * @DFLT_ADDRESS    : default mosquitto address
     * @DFLT_CLIENT_ID  : default subscriber client id
     * @DFLT_TOPIC      : default listening topic
     */
    static const std::string DFLT_ADDRESS, DFLT_CLIENT_ID, DFLT_TOPIC;

    /**
     * Default constructor.
     * It uses default values @DFLT_ADDRESS, @DFLT_CLIENT_ID, @DFLT_TOPIC
     */
    JoystickSubscriber() : JoystickSubscriber(DFLT_ADDRESS, DFLT_CLIENT_ID, DFLT_TOPIC) {}
    /**
     * Constructor.
     * 
     * @address : mosquitto address
     * @clientID: subscriber client id
     * @topic   : listening topic
     */
    JoystickSubscriber(const std::string& address, const std::string& clientID, const std::string& topic) 
        : Subscriber(address, clientID, topic, &JoystickSubscriber::callback, this), controller_(), current_(0)
    {
        for (auto sensor_type : sensor_t())
            sensors_.emplace_back(Sensor<unsigned char>(sensor_type, 0));
    }
    
    /**
     * Callback function.
     * It elaborates the joystick data (@payload) and use the SPI to send values and receive sensors data.
     * 
     * @payload: the string that recives from the JoystickPublisher
     */
    void callback(const std::string& payload);
};

}