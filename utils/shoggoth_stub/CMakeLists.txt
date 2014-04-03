cmake_minimum_required( VERSION 2.8 )
project( shoggoth_stub )

# Compile flag database
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../cmake)

add_definitions( -std=c++0x )

find_package( Threads )
find_package( Boost 1.55 COMPONENTS system REQUIRED )
find_package( Thrift REQUIRED )

include_directories(
  SYSTEM
  ${Boost_INCLUDE_DIR}
  ${THRIFT_INCLUDE_DIR}
  )


file( GLOB PROJECT_SOURCES *.cpp
    gen-cpp/shoggoth_constants.cpp
    gen-cpp/shoggoth_types.cpp
    gen-cpp/Tracking.cpp
    )


add_executable( shoggoth_stub
    ${PROJECT_SOURCES}
    )

target_link_libraries( shoggoth_stub
    ${Boost_LIBRARIES}
    ${THRIFT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

