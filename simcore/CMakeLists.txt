#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(simcore)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)
find_package(ROOT CONFIG REQUIRED COMPONENTS RIO Tree)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB simcore_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)

#----------------------------------------------------------------------------
# Add a library with sources and the local headers and the Geant4 libraries
#
add_library(simcore SHARED ${simcore_lib_sources})

target_include_directories(simcore PUBLIC include)

target_link_libraries(simcore PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(simcore PRIVATE simobj)
target_link_libraries(simcore PRIVATE ${ROOT_LIBRARIES})
