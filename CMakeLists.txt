cmake_minimum_required(VERSION 3.16...3.27)
project(recbesim)

include(cmake/add_sim_subdir.cmake)

include_directories(include)

add_sim_subdir(simobj)
add_sim_subdir(simcore)
add_sim_subdir(bl10sim)
add_sim_subdir(kobetdsim)

#----------------------------------------------------------------------------
# Create a bash source script for the MC libraries
#
configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/environment.sh.in
    ${PROJECT_BINARY_DIR}/environment.sh
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                     GROUP_READ GROUP_EXECUTE
                     WORLD_READ WORLD_EXECUTE
    @ONLY
)
