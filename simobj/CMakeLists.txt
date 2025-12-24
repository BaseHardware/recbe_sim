#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(simobj)

#----------------------------------------------------------------------------
# Find ROOT package, activating ROOT I/O and TTree by default
#
find_package(ROOT CONFIG REQUIRED COMPONENTS RIO Tree EG)

include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB simobj_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB simobj_lib_headers ${PROJECT_SOURCE_DIR}/simobj/*.h)

file(GLOB simobj_dict ${PROJECT_SOURCE_DIR}/dict/simobj_linkdef.h)

#----------------------------------------------------------------------------
# Add a library with sources and the local headers and the ROOT libraries
#
add_library(simobj SHARED ${simobj_lib_sources})

target_include_directories(simobj PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(simobj PRIVATE simobj)

target_link_libraries(simobj PRIVATE ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Create a dictionary file for simobj
#
ROOT_GENERATE_DICTIONARY(G__simobj
    ${simobj_lib_headers}
    LINKDEF ${simobj_dict}
    MODULE simobj
    NO_CXXMODULE
)
