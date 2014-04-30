cmake_minimum_required( VERSION 2.8 )
project( spoor )

# Compile flag database
set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )


add_definitions( -std=c++0x )

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package( Threads REQUIRED)
find_package( Boost 1.55 COMPONENTS system timer program_options thread REQUIRED )
find_package( OpenCV 1.4.8 REQUIRED )
find_package( Thrift REQUIRED )
find_package( GLIB COMPONENTS gobject gthread gmodule REQUIRED )
find_package( LibXml2 REQUIRED )
find_package( GStreamer COMPONENTS app base video REQUIRED )

include_directories(
  SYSTEM
  ${Boost_INCLUDE_DIR}
  ${OpenCV_INCLUDE_DIR}
  ${THRIFT_INCLUDE_DIR}
  ${GLIB_INCLUDE_DIRS}
  ${LIBXML2_INCLUDE_DIR}
  ${GSTREAMER_INCLUDE_DIRS}
  ${GSTREAMER_APP_INCLUDE_DIRS}
  ${GSTREAMER_VIDEO_INCLUDE_DIRS}
  ${GSTREAMER_BASE_INCLUDE_DIRS}
  )


set( PROJECT_SOURCES 
    feature.cpp
    franken.cpp
    ImLogger.cpp
    spoor.cpp
    shoggoth.cpp
    tracking.cpp
    vision.cpp
    cap_gstreamer.cpp
    gen-cpp/shoggoth_constants.cpp
    gen-cpp/shoggoth_types.cpp
    gen-cpp/Tracking.cpp
    )


add_executable( spoor
    ${PROJECT_SOURCES}
    )

target_link_libraries( spoor
    ${Boost_LIBRARIES}
    ${OpenCV_LIBRARIES}
    ${THRIFT_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${GSTREAMER_VIDEO_LIBRARIES}
    ${GSTREAMER_APP_LIBRARIES}
    ${GSTREAMER_BASE_LIBRARIES}
    ${GSTREAMER_LIBRARIES}
    ${GLIB_GOBJECT_LIBRARIES}
    ${GLIB_GMODULE_LIBRARIES}
    ${GLIB_GTHREAD_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    )


set_source_files_properties(tags PROPERTIES GENERATED true)
add_custom_target(tags
    COMMAND ctags -R --c++-kinds=+p --fields=+iaS --extra=+q .
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_dependencies(spoor tags)

