#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(kobetdsim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB kobetdsim_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)

file(GLOB kobetdsim_exec ${PROJECT_SOURCE_DIR}/exe/kobetd_mainsim.cxx)

#----------------------------------------------------------------------------
# Add a library with sources and the local headers and the Geant4 libraries
#
add_library(kobetdsim SHARED ${kobetdsim_lib_sources})

target_include_directories(kobetdsim PUBLIC include)
target_link_libraries(kobetdsim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(kobetdsim PUBLIC simcore)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(kobetd_mainsim ${kobetdsim_exec})

target_link_libraries(kobetd_mainsim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(kobetd_mainsim PUBLIC kobetdsim)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B4a. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(KOBETDSIM_USERFILES
    macros/gui.mac
    macros/init_vis.mac
    macros/run1.mac
    macros/run2.mac
    macros/vis.mac
    data/spectrum.txt
)

foreach(_userfile ${KOBETDSIM_USERFILES})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_userfile}
        ${PROJECT_BINARY_DIR}/${_userfile}
        COPYONLY
    )
endforeach()
