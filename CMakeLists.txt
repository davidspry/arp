cmake_minimum_required(VERSION 3.20)

project(arp CXX)
set(TARGET ${PROJECT_NAME})
set(TARGET_ALIAS ${TARGET}::${TARGET})

include(FetchContent)
FetchContent_Declare(fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 12.0.0
  GIT_SHALLOW on)
FetchContent_MakeAvailable(fmt)

add_library(${TARGET} INTERFACE)
add_library(${TARGET_ALIAS} ALIAS ${TARGET})
add_subdirectory(app)

target_sources(${TARGET}
  INTERFACE)

target_compile_features(${TARGET}
  INTERFACE
    cxx_std_23)

target_include_directories(${TARGET}
  INTERFACE
    include)

target_link_libraries(${TARGET}
  INTERFACE
    fmt::fmt)
