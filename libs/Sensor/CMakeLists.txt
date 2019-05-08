cmake_minimum_required(VERSION 3.5)
project(Sensor VERSION 1.0.0 LANGUAGES CXX)

add_library(Sensor
        Sensor.cpp)

add_library(PolitoceanRovArmCore::Sensor ALIAS Sensor)

target_include_directories(Sensor
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(Sensor PRIVATE cxx_auto_type)
target_compile_options(Sensor PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS Sensor
        EXPORT PolitoceanRovArmCoreTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
