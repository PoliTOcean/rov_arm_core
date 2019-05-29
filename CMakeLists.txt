cmake_minimum_required(VERSION 3.5)
project(PolitoceanRovArmCore)

set(CMAKE_CXX_STANDARD 11)

# Add politocean common libs
add_subdirectory(politocean_common)
include_directories(politocean_common/include)

add_subdirectory(rov_common)
include_directories(rov_common/include)

add_subdirectory(libs)

include_directories(include)

# Add executable
add_executable(ATMegaController src/ATMegaController.cpp)
add_executable(Skeleton src/Skeleton.cpp)

# Linking the libraries
target_link_libraries(ATMegaController -lpthread
    RovCommon::Controller

    PolitoceanCommon::Sensor
    PolitoceanCommon::mqttLogger
    PolitoceanCommon::logger
    PolitoceanCommon::MqttClient
)


target_link_libraries(Skeleton -lpthread
    RovCommon::Controller
    PolitoceanRovArmCore::Stepper
    PolitoceanRovArmCore::DCMotor
    
    PolitoceanCommon::MqttClient
)

