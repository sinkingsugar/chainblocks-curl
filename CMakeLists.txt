project("chainblocks-curl")
cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -march=core2)

#### Files to format and tidy
set(
  MY_PROJECT_SOURCE_FILES
  ${MY_PROJECT_SOURCE_FILES}
  ${CMAKE_CURRENT_LIST_DIR}/tgbot.cpp
  )

#### Header paths for tidy
set(
  MY_PROJECT_HEADER_PATHS
  ${MY_PROJECT_HEADER_PATHS}
  -I${CHAINBLOCKS_DIR}/include
  -I${CHAINBLOCKS_DIR}/deps/stb
  -I${CHAINBLOCKS_DIR}/deps/nameof/include
  -I${CHAINBLOCKS_DIR}/deps/magic_enum/include
  -I${CMAKE_CURRENT_LIST_DIR}/deps/tgbot-cpp/include
  )

### setup clang format
find_program(
  CLANG_FORMAT_EXE
  NAMES "clang-format"
  DOC "Path to clang-format executable"
  )
if(NOT CLANG_FORMAT_EXE)
  message(STATUS "clang-format not found.")
else()
  message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")
endif()

#### Format target
if(CLANG_FORMAT_EXE)
  add_custom_target(
    format
    COMMAND ${CLANG_FORMAT_EXE} -i -style=LLVM ${MY_PROJECT_SOURCE_FILES}
    )
endif()
###

### setup clang tidy
find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
  )
if(NOT CLANG_TIDY_EXE)
  message(STATUS "clang-tidy not found.")
else()
  message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
endif()

#### tidy target
set(tidyfied)
foreach(_file ${MY_PROJECT_SOURCE_FILES})
  if(CLANG_TIDY_EXE)
    add_custom_command(
      OUTPUT ${_file}.noop
      COMMAND ${CLANG_TIDY_EXE} -checks=-*,clang-analyzer-*,performance-*,bugprone-* -fix ${_file} -- -std=c++17 -DDEBUG ${MY_PROJECT_HEADER_PATHS}
      )
    list(APPEND tidyfied ${_file}.noop)
  endif()
endforeach()
add_custom_target(tidy DEPENDS ${tidyfied})
###

if(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

if(APPLE)
  include_directories(/usr/local/include)
  link_directories(/usr/local/lib)
endif()

include_directories(
  ${CHAINBLOCKS_DIR}/include
  ${CHAINBLOCKS_DIR}/deps/stb
  ${CHAINBLOCKS_DIR}/deps/nameof/include
  ${CHAINBLOCKS_DIR}/deps/magic_enum/include
  )

add_library(
  cbcurl SHARED
  ${CMAKE_CURRENT_LIST_DIR}/curl.cpp
  )
add_dependencies(cbtgbot format)
if(WIN32)
  target_link_libraries(cbcurl -static)
else()
  target_link_libraries(cbcurl)
endif()
