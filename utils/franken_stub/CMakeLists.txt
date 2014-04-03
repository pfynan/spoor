cmake_minimum_required( VERSION 2.8 )
project( franken_stub )

# Compile flag database
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )


add_definitions( -std=c++0x -DNO_OPENCV)

find_package( Threads )
find_package( Boost 1.55 COMPONENTS system thread REQUIRED )

include_directories(
  SYSTEM
  ${Boost_INCLUDE_DIR}
  )


file( GLOB PROJECT_SOURCES *.cpp )


add_executable( franken_stub
    ${PROJECT_SOURCES}
    )

target_link_libraries( franken_stub
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

