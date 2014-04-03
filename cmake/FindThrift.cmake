find_package(PkgConfig)
pkg_check_modules(PC_THRIFT QUIET thrift)
set(THRIFT_DEFINITIONS ${PC_THRIFT_CFLAGS_OTHER})

find_path(THRIFT_INCLUDE_DIR thrift/Thrift.h
          HINTS ${PC_THRIFT_INCLUDEDIR} ${PC_THRIFT_INCLUDE_DIRS}
          PATH_SUFFIXES thrift )

find_library(THRIFT_LIBRARY NAMES thrift libthrift
             HINTS ${PC_THRIFT_LIBDIR} ${PC_THRIFT_LIBRARY_DIRS} )

find_program(THRIFT_COMPILER thrift)

set(THRIFT_LIBRARIES ${THRIFT_LIBRARY} )
set(THRIFT_INCLUDE_DIRS ${THRIFT_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set THRIFT_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Thrift  DEFAULT_MSG
    THRIFT_LIBRARY THRIFT_INCLUDE_DIR THRIFT_COMPILER)

mark_as_advanced(THRIFT_INCLUDE_DIR THRIFT_LIBRARY )

