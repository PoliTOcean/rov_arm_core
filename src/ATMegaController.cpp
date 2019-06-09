/**
 * @author pettinz
 */

#include <string>
#include <thread>
#include <chrono>
#include <mutex>
#include <exception>
#include <cstdlib>

#include "MqttClient.h"
#include "Controller.h"

#include "Commands.h"
#include "PolitoceanConstants.h"
#include "PolitoceanExceptions.hpp"
#include "PolitoceanUtils.hpp"

#include "logger.h"
#include "mqttLogger.h"

#include "json.hpp"

using namespace Politocean;
using namespace Politocean::RPi;
using namespace Politocean::Constants;

class Listener
{
    std::vector<int> axes_;
    std::string action_;

    bool isAxesUpdated_, isActionUpdated_;

public:
    Listener() : axes_(3, 0), action_(), isAxesUpdated_(false), isActionUpdated_(false) {}

    void listenForAxes(const std::string& payload);
    void listenForAction(const std::string& payload);

    std::vector<int> axes();
    std::string action();

    bool isAxesUpdated();
    bool isActionUpdated();
};

void Listener::listenForAxes(const std::string& payload)
{
    auto c_map = nlohmann::json::parse(payload);
    axes_ = c_map.get<std::vector<int>>();

    isAxesUpdated_ = true;
}

void Listener::listenForAction(const std::string& payload)
{
    action_ = payload;

    isActionUpdated_ = true;
}

std::vector<int> Listener::axes()
{
    isAxesUpdated_ = false;
    return axes_;
}

std::string Listener::action()
{
    isActionUpdated_ = false;
    return action_;
}

bool Listener::isAxesUpdated()
{
    return isAxesUpdated_;
}

bool Listener::isActionUpdated()
{
    return isActionUpdated_;
}

class Talker
{
    std::thread *axesThread_, *actionThread_;
    std::mutex mutex_;

    Controller controller_;

    bool isTalking_;

    void send(const std::vector<unsigned char>& buffer);
    unsigned char command(const std::string& action);

public:
    Talker() : controller_(), isTalking_(false)
    {
        controller_.setup();
        controller_.setupMotors();
    }

    void startTalking(MqttClient& publisher, Listener& listener);
    void stopTalking();

    bool isTalking();
};

void Talker::startTalking(MqttClient& publisher, Listener& listener)
{
    if (isTalking_)
        return ;
    
    isTalking_ = true;

    controller_.spiOpen(0, 1000000);

    axesThread_ = new std::thread([&]() {
        std::vector<int> axes;

        while (isTalking_)
        {
            if (!listener.isAxesUpdated())
                continue ;
            
            axes = listener.axes();
            std::vector<unsigned char> buffer = {
                (unsigned char) 0xFF,
                (unsigned char) Politocean::map(axes[0], SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
                (unsigned char) Politocean::map(axes[1], SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1),
                (unsigned char) Politocean::map(axes[2], SHRT_MIN, SHRT_MAX, 1, UCHAR_MAX-1)
            };

            send(buffer);
        }
    });

    actionThread_ = new std::thread([&]() {
        std::string data;

        while (isTalking_)
        {
            if (!listener.isActionUpdated())
                continue ;

            data = listener.action();
            bool sendToSPI = false;

            if (data == Commands::Actions::RESET)
                controller_.reset();
            else if (data == Commands::Actions::ON)
            {
                controller_.startMotors();
                Politocean::publishComponents(publisher, Components::POWER, Commands::Actions::ON);
            }
            else if (data == Commands::Actions::OFF)
            {
                controller_.stopMotors();
                Politocean::publishComponents(publisher, Components::POWER, Commands::Actions::OFF);
            }
            else
                sendToSPI = true;

            if (!sendToSPI)
                continue ;

            std::vector<unsigned char> buffer = {
                (unsigned char) 0x00,
                (unsigned char) command(data)
            };

            send(buffer);
        }
    });
}

void Talker::stopTalking()
{
    if (!isTalking_)
        return ;
    
    isTalking_ = false;
    axesThread_->join(); actionThread_->join();
    controller_.spiClose();
}

bool Talker::isTalking()
{
    return isTalking_;
}

void Talker::send(const std::vector<unsigned char>& buffer)
{
    char tx, rx;

    for (auto it = buffer.begin(); it != buffer.end(); it++)
    {
        tx = *it;
        controller_.spiXfer(&tx, &rx, 1);
    }
}

unsigned char Talker::command(const std::string& action)
{
    unsigned char command = Commands::ATMega::SPI::NONE;

    if (action == Commands::Actions::ATMega::VDOWN_ON)
        command = Commands::ATMega::SPI::VDOWN_ON;
    else if (action == Commands::Actions::ATMega::VDOWN_OFF)
        command = Commands::ATMega::SPI::VDOWN_OFF;
    else if (action == Commands::Actions::ATMega::VUP_ON)
        command = Commands::ATMega::SPI::VUP_ON;
    else if (action == Commands::Actions::ATMega::VUP_OFF)
        command = Commands::ATMega::SPI::VUP_OFF;
    else if (action == Commands::Actions::ATMega::VUP_FAST_ON)
        command = Commands::ATMega::SPI::VUP_FAST_ON;
    else if (action == Commands::Actions::ATMega::VUP_FAST_OFF)
        command = Commands::ATMega::SPI::VUP_FAST_OFF;
    else if (action == Commands::Actions::ATMega::FAST)
        command = Commands::ATMega::SPI::FAST;
    else if (action == Commands::Actions::ATMega::SLOW)
        command = Commands::ATMega::SPI::SLOW;
    else if (action == Commands::Actions::ATMega::MEDIUM)
        command = Commands::ATMega::SPI::MEDIUM;
    else if (action == Commands::Actions::ATMega::START_AND_STOP)
        command = Commands::ATMega::SPI::START_AND_STOP;
    else
        command = Commands::ATMega::SPI::NONE;
    
    return command;
}

int main(int argc, const char *argv[])
{
    MqttClient publisher(Rov::ATMEGA_ID, Hmi::IP_ADDRESS);
    mqttLogger ptoLogger(&publisher);

    logger::enableLevel(logger::DEBUG, true);

    try
    {
        publisher.connect();
    }
    catch(const std::exception& e)
    {
        ptoLogger.logError(e);
    }

    MqttClient subscriber(Rov::ATMEGA_ID, Rov::IP_ADDRESS);
    Listener listener;

    try
    {
        subscriber.connect();
    }
    catch(const Politocean::mqttException& e)
    {
        std::cerr << "Error on subscriber connection : " << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    subscriber.subscribeTo(Topics::AXES,        &Listener::listenForAxes,   &listener);
    subscriber.subscribeTo(Topics::COMMANDS,    &Listener::listenForAction, &listener);

    Talker talker;
    talker.startTalking(publisher, listener);

    subscriber.wait();

    talker.stopTalking();

    return 0;
}