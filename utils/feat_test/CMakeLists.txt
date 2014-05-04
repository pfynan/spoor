cmake_minimum_required( VERSION 2.8 )
project( feat_test )

# Compile flag database
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )


add_definitions( -std=c++0x )

find_package( Threads )
find_package( OpenCV REQUIRED )
find_package( Boost 1.55 COMPONENTS system timer REQUIRED )

include_directories(
  SYSTEM
  ${OpenCV_INCLUDE_DIR}
  )


file( GLOB PROJECT_SOURCES feat_test.cpp )


add_executable( feat_test
    ${PROJECT_SOURCES}
    )

target_link_libraries( feat_test
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

