cmake_minimum_required(VERSION 3.5)
project(DCMotor VERSION 1.0.0 LANGUAGES CXX)

add_library(DCMotor SHARED
        DCMotor.cpp)

add_library(PolitoceanRov::DCMotor ALIAS DCMotor)

target_link_libraries(DCMotor -lpthread PolitoceanRovCommon::Controller)

target_include_directories(DCMotor
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(DCMotor PRIVATE cxx_auto_type)
target_compile_options(DCMotor PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall>)

include(GNUInstallDirs)
install(TARGETS DCMotor
        EXPORT PolitoceanRovTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
