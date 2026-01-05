# Usage:
# cmake -DSOURCE_DIR=... -DOUTPUT_FILE=... -P generate_git_header.cmake

find_package(Git REQUIRED QUIET)

set(GIT_SHA "unknown")
set(GIT_DESCRIBE "unknown")
set(GIT_BRANCH "unknown")

if(GIT_FOUND)
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DESCRIBE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
endif()

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/GitInformation.h.in
    ${OUTPUT_FILE}
    @ONLY
)

#set(content
#"#ifndef __recbesim_GitInformation_h__\n"
#"#define __recbesim_GitInformation_h__\n"
#"#define GIT_SHA \"${GIT_SHA}\"\n"
#"#define GIT_DESCRIBE \"${GIT_DESCRIBE}\"\n"
#"#define GIT_BRANCH \"${GIT_BRANCH}\"\n"
#"#endif\n"
#)

#if(EXISTS "${OUTPUT_FILE}")
#  file(READ "${OUTPUT_FILE}" old)
#  if(old STREQUAL content)
#    return()
#  endif()
#endif()

#file(WRITE ${OUTPUT_FILE} ${content})
