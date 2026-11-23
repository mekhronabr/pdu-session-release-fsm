cmake_minimum_required(VERSION 3.16)
project(pdu-session-release-fsm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pdu-session-release-fsm harness.cpp release_procedure_fsm.cpp)
target_include_directories(pdu-session-release-fsm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
