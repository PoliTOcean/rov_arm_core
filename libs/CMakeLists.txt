cmake_minimum_required(VERSION 3.5)
project(PolitoceanRov)

include_directories(../include)

# Add here all libraries' directories
#add_subdirectory(name_of_directory)

add_subdirectory(Stepper)
add_subdirectory(DCMotor)

# Install and export (do not touch this part)
install(
  EXPORT PolitoceanRovTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRov
  NAMESPACE PolitoceanRov::
  FILE PolitoceanRovTargets.cmake
  )

include(CMakePackageConfigHelpers)
configure_package_config_file( 
  "Config.cmake.in" 
  "PolitoceanRovConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRov
  PATH_VARS
    CMAKE_INSTALL_LIBDIR
  )

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovConfigVersion.cmake
  VERSION 1.0.0
  COMPATIBILITY SameMajorVersion
  )

### Install Config and ConfigVersion files
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/PolitoceanRovConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/PolitoceanRov"
  )
