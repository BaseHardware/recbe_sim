set(_LOCAL_INCLUDE_ROOT "${CMAKE_CURRENT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${_LOCAL_INCLUDE_ROOT}")
include_directories(${_LOCAL_INCLUDE_ROOT})

function(add_sim_subdir subdir_name)
    set(src_inc "${CMAKE_CURRENT_SOURCE_DIR}/${subdir_name}/include")

    if(NOT IS_DIRECTORY "${src_inc}")
        message(STATUS "[link_subdir_include] Skip '${subdir_name}': no include dir at ${src_inc}")
        return()
    endif()


    set(dst_link "${_LOCAL_INCLUDE_ROOT}/${subdir_name}")

    if(EXISTS "${dst_link}")
        if(IS_SYMLINK "${dst_link}")
            file(REMOVE "${dst_link}")
        else()
            file(REMOVE_RECURSE "${dst_link}")
        endif()
    endif()

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "${src_inc}" "${dst_link}"
        RESULT_VARIABLE rv
    )

    if(NOT rv EQUAL 0)
        message(FATAL_ERROR "Failed to create symlink: ${dst_link} -> ${src_inc}")
    else()
        message(STATUS "Creating symlink: ${dst_link} -> ${src_inc}")
    endif()

    add_subdirectory(${subdir_name})
endfunction(add_sim_subdir)
