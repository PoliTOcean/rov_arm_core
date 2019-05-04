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
    std::vector<int> axes_;
    unsigned char button_;

    std::vector<Sensor<unsigned char>> sensors_;

    /**
     * @current_: the index of the current value of the SPI buffer
     */
    unsigned int current_;

    /**
     * Callback functions.
     * They read the joystick data (@payload) from JoystickPublisher
     * 
     * @payload: the string that recives from the JoystickPublisher
     *
     * listenForButtons : converts the string @payload into an unsigned char value and stores it inside @button_.
     * listenForAxes    : parses the string @payload into a JSON an stores the axes values inside @axes_ vector.
     */
    bool isListening_;
    std::thread *listeningThread_;
    void listenForButtons(const std::string& payload);
    void listenForAxes(const std::string& payload);

public:
    /**
     * @DFLT_ADDRESS    : default mosquitto address
     * @DFLT_CLIENT_ID  : default subscriber client id
     */
    static const std::string DFLT_ADDRESS, DFLT_CLIENT_ID;

    /**
     * Default constructor.
     * It uses default values @DFLT_ADDRESS, @DFLT_CLIENT_ID, @DFLT_TOPIC
     */
    JoystickSubscriber() : JoystickSubscriber(DFLT_ADDRESS, DFLT_CLIENT_ID) {}
    /**
     * Constructor.
     * 
     * @address : mosquitto address
     * @clientID: subscriber client id
     */
    JoystickSubscriber(const std::string& address, const std::string& clientID) 
        : Subscriber(address, clientID), controller_(), current_(0), isListening_(false), listeningThread_(nullptr)
    {
        for (auto sensor_type : sensor_t())
            sensors_.emplace_back(Sensor<unsigned char>(sensor_type, 0));
    }
    
    // Starts listening to topics
    void startListening();

    // Stops listening to topics
    void stopListening();
};

}