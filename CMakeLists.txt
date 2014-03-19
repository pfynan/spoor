cmake_minimum_required( VERSION 2.8 )
project( spoor )

# Compile flag database
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )


add_definitions( -std=c++0x )

find_package( Threads )
find_package( Boost 1.55 COMPONENTS system timer program_options REQUIRED )
find_package( OpenCV 1.4.8 REQUIRED )

include_directories(
  SYSTEM
  ${Boost_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIR}
  )


file( GLOB PROJECT_SOURCES *.cpp )


add_executable( spoor
    ${PROJECT_SOURCES}
    )

target_link_libraries( spoor
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

