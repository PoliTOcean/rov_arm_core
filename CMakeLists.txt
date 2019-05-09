cmake_minimum_required(VERSION 3.5)
project(PolitoceanRovArmCore)

set(CMAKE_CXX_STANDARD 11)

# Add politocean common libs
add_subdirectory(politocean_common)
include_directories(politocean_common/include)

# Add project internal libs
add_subdirectory(libs)

include_directories(include)

# Add executable
add_executable(JoystickSubscriber src/JoystickSubscriber.cpp)
add_executable(Arm src/Arm.cpp)
add_executable(ImuCalibration src/ImuCalibration.cpp)

# Linking the libraries
target_link_libraries(JoystickSubscriber
    -lpthread
    PolitoceanRovArmCore::Sensor PolitoceanRovArmCore::Controller
    PolitoceanCommon::Subscriber PolitoceanCommon::mqttLogger PolitoceanCommon::logger PolitoceanCommon::Publisher)


target_link_libraries(Arm -lpthread
    PolitoceanRovArmCore::Controller
    PolitoceanCommon::Subscriber)

target_link_libraries(ImuCalibration
    PolitoceanRovArmCore::Controller
    PolitoceanCommon::Publisher
    PolitoceanCommon::mqttLogger)
