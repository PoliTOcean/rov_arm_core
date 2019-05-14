cmake_minimum_required(VERSION 3.5)
project(PwmMotor VERSION 1.0.0 LANGUAGES CXX)

add_library(PwmMotor
        PwmMotor.cpp)

add_library(PolitoceanRovArmCore::PwmMotor ALIAS PwmMotor)

target_link_libraries(PwmMotor -lpthread PolitoceanRovArmCore::Controller)

target_include_directories(PwmMotor
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(PwmMotor PRIVATE cxx_auto_type)
target_compile_options(PwmMotor PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS PwmMotor
        EXPORT PolitoceanRovArmCoreTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
