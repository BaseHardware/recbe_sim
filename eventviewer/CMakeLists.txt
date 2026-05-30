#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(eventviewer)

find_package(ROOT CONFIG REQUIRED COMPONENTS RIO Tree EG Gui Graf Graf3d Geom RGL)
find_package(Geant4 REQUIRED)

include(${ROOT_USE_FILE})

file(GLOB eventviewer_lib_sources ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB eventviewer_lib_headers ${PROJECT_SOURCE_DIR}/include/eventviewer/*.h)

add_library(eventviewer SHARED ${eventviewer_lib_sources})

target_include_directories(eventviewer PUBLIC include)
target_include_directories(eventviewer PUBLIC ${ROOT_INCLUDE_DIRS})
target_include_directories(eventviewer PRIVATE ${Geant4_INCLUDE_DIRS})
target_link_libraries(eventviewer PUBLIC simobj ${ROOT_LIBRARIES})
target_link_libraries(eventviewer PRIVATE ${Geant4_LIBRARIES})

ROOT_GENERATE_DICTIONARY(G__eventviewer
    ${eventviewer_lib_headers}
    LINKDEF ${PROJECT_SOURCE_DIR}/dict/linkdef.h
    MODULE eventviewer
    NO_CXXMODULE
)

add_executable(evt_viewer apps/evt_viewer.cxx)
target_link_libraries(evt_viewer PRIVATE eventviewer)
