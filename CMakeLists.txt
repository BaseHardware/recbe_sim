cmake_minimum_required(VERSION 3.16...3.27)
project(recbesim)

include(cmake/add_sim_subdir.cmake)

include_directories(include)

add_sim_subdir(simcore)
add_sim_subdir(bl10sim)
