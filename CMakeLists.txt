cmake_minimum_required(VERSION 3.5)
project(PolitoceanRov)

set(CMAKE_CXX_STANDARD 11)

# Add politocean common libs
add_subdirectory(politocean_common)
include_directories(politocean_common/include)

# Add project internal libs
#add_subdirectory(libs)

# Add executable
add_executable(RovCore main.cpp)

# Linking the libraries
target_link_libraries(RovCore 
    # Add here all the libraries as:
    # Name_of_the_library       --- if it's internal (e.g. Publisher)
    # -lname_of_the_library     --- if it's a standard c++ library (e.g. -lpthread)
    )
