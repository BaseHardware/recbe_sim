find_package(Git REQUIRED QUIET)

set(GEN_DIR "${CMAKE_BINARY_DIR}/include/recbesim/")
file(MAKE_DIRECTORY "${GEN_DIR}")
set(GIT_HEADER "${GEN_DIR}/GitInformation.h")

function(_git_path out relpath)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --git-path "${relpath}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE _p
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    set(${out} ${_p} PARENT_SCOPE)
endfunction()

_git_path(GIT_HEAD_PATH "HEAD")
_git_path(GIT_INDEX_PATH "index")

add_custom_command(
    OUTPUT  ${GIT_HEADER}
    COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_CURRENT_LIST_DIR}
            -DOUTPUT_FILE=${GIT_HEADER}
            -P ${CMAKE_CURRENT_LIST_DIR}/git_version/generate_git_header.cmake
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/git_version/generate_git_header.cmake
            ${CMAKE_CURRENT_LIST_DIR}/git_version/GitInformation.h.in
            ${GIT_HEAD_PATH}
            ${GIT_INDEX_PATH}
    VERBATIM
)

add_custom_target(git_version_header DEPENDS "${GIT_HEADER}")
