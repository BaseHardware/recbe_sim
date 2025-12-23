#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(RecbeSim)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB recbesim_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB recbesim_lib_headers ${PROJECT_SOURCE_DIR}/include/*.hh)

file(GLOB recbesim_exec ${PROJECT_SOURCE_DIR}/exe/recbe_mainsim.cc)

#----------------------------------------------------------------------------
# Add a library with sources and the local headers and the Geant4 libraries
#
add_library(recbesim SHARED ${recbesim_lib_sources})

target_include_directories(recbesim PRIVATE include/recbesim)
target_link_libraries(recbesim PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(recbe_mainsim ${recbesim_exec})

target_include_directories(recbe_mainsim PRIVATE include)
target_link_libraries(recbe_mainsim PRIVATE ${Geant4_LIBRARIES})
target_link_libraries(recbe_mainsim PRIVATE recbesim)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B4a. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(RECBESIM_SCRIPTS
    macros/gui.mac
    macros/init_vis.mac
    macros/run1.mac
    macros/run2.mac
    macros/vis.mac
)

foreach(_script ${RECBESIM_SCRIPTS})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_script}
        ${PROJECT_BINARY_DIR}/${_script}
        COPYONLY
    )
endforeach()
