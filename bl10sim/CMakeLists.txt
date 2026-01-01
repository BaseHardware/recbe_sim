#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(bl10sim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB bl10sim_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)

file(GLOB bl10sim_exec ${PROJECT_SOURCE_DIR}/exe/bl10_mainsim.cxx)
file(GLOB bl10ductsim_exec ${PROJECT_SOURCE_DIR}/exe/bl10_ductsim.cxx)

#----------------------------------------------------------------------------
# Add a library with sources and the local headers and the Geant4 libraries
#
add_library(bl10sim SHARED ${bl10sim_lib_sources})

target_include_directories(bl10sim PUBLIC include)
target_link_libraries(bl10sim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(bl10sim PUBLIC simcore)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(bl10_mainsim ${bl10sim_exec})

target_link_libraries(bl10_mainsim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(bl10_mainsim PRIVATE bl10sim)

add_executable(bl10_ductsim ${bl10ductsim_exec})

target_link_libraries(bl10_ductsim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(bl10_ductsim PRIVATE bl10sim)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B4a. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(BL10SIM_USERDATA
    macros/gui.mac
    macros/init_vis.mac
    macros/run1.mac
    macros/run2.mac
    macros/vis.mac
    data/port10_inp.txt
)

foreach(_script ${BL10SIM_USERDATA})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_script}
        ${PROJECT_BINARY_DIR}/${_script}
        COPYONLY
    )
endforeach()
