cmake_minimum_required(VERSION 3.20)

project(arp_app CXX)
set(TARGET ${PROJECT_NAME})

add_executable(${TARGET} main.cpp)

add_custom_target(run_${TARGET}
  COMMAND ${TARGET}
  DEPENDS ${TARGET}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(${TARGET}
  PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED on)

target_link_libraries(${TARGET}
  PRIVATE
    arp::arp)
