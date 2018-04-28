#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ivsensor" for configuration "Release"
set_property(TARGET ivsensor APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ivsensor PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ivcommon;/opt/ros/kinetic/lib/libroslib.so;/opt/ros/kinetic/lib/librospack.so;/usr/lib/x86_64-linux-gnu/libpython2.7.so;/usr/lib/x86_64-linux-gnu/libboost_program_options.so;/usr/lib/x86_64-linux-gnu/libtinyxml.so;/opt/ros/kinetic/lib/libtf.so;/opt/ros/kinetic/lib/libtf2_ros.so;/opt/ros/kinetic/lib/libactionlib.so;/opt/ros/kinetic/lib/libmessage_filters.so;/opt/ros/kinetic/lib/libroscpp.so;/usr/lib/x86_64-linux-gnu/libboost_filesystem.so;/usr/lib/x86_64-linux-gnu/libboost_signals.so;/opt/ros/kinetic/lib/libxmlrpcpp.so;/opt/ros/kinetic/lib/libtf2.so;/opt/ros/kinetic/lib/libroscpp_serialization.so;/opt/ros/kinetic/lib/librosconsole.so;/opt/ros/kinetic/lib/librosconsole_log4cxx.so;/opt/ros/kinetic/lib/librosconsole_backend_interface.so;/usr/lib/x86_64-linux-gnu/liblog4cxx.so;/usr/lib/x86_64-linux-gnu/libboost_regex.so;/opt/ros/kinetic/lib/librostime.so;/opt/ros/kinetic/lib/libcpp_common.so;/usr/lib/x86_64-linux-gnu/libboost_system.so;/usr/lib/x86_64-linux-gnu/libboost_thread.so;/usr/lib/x86_64-linux-gnu/libboost_chrono.so;/usr/lib/x86_64-linux-gnu/libboost_date_time.so;/usr/lib/x86_64-linux-gnu/libboost_atomic.so;/usr/lib/x86_64-linux-gnu/libpthread.so;/usr/lib/x86_64-linux-gnu/libconsole_bridge.so;pcap"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libivsensor.so"
  IMPORTED_SONAME_RELEASE "libivsensor.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS ivsensor )
list(APPEND _IMPORT_CHECK_FILES_FOR_ivsensor "${_IMPORT_PREFIX}/lib/libivsensor.so" )

# Import target "utils" for configuration "Release"
set_property(TARGET utils APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(utils PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ivcommon;ivsensor"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libutils.so"
  IMPORTED_SONAME_RELEASE "libutils.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS utils )
list(APPEND _IMPORT_CHECK_FILES_FOR_utils "${_IMPORT_PREFIX}/lib/libutils.so" )

# Import target "utils2" for configuration "Release"
set_property(TARGET utils2 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(utils2 PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "ivcommon;ivsensor"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libutils2.so"
  IMPORTED_SONAME_RELEASE "libutils2.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS utils2 )
list(APPEND _IMPORT_CHECK_FILES_FOR_utils2 "${_IMPORT_PREFIX}/lib/libutils2.so" )

# Import target "masternode" for configuration "Release"
set_property(TARGET masternode APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(masternode PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/masternode"
  )

list(APPEND _IMPORT_CHECK_TARGETS masternode )
list(APPEND _IMPORT_CHECK_FILES_FOR_masternode "${_IMPORT_PREFIX}/bin/masternode" )

# Import target "getgpsdata" for configuration "Release"
set_property(TARGET getgpsdata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getgpsdata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getgpsdata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getgpsdata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getgpsdata "${_IMPORT_PREFIX}/bin/getgpsdata" )

# Import target "getinsdata" for configuration "Release"
set_property(TARGET getinsdata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getinsdata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getinsdata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getinsdata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getinsdata "${_IMPORT_PREFIX}/bin/getinsdata" )

# Import target "getrslidardata" for configuration "Release"
set_property(TARGET getrslidardata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getrslidardata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getrslidardata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getrslidardata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getrslidardata "${_IMPORT_PREFIX}/bin/getrslidardata" )

# Import target "getsinglerslidardata" for configuration "Release"
set_property(TARGET getsinglerslidardata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getsinglerslidardata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getsinglerslidardata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getsinglerslidardata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getsinglerslidardata "${_IMPORT_PREFIX}/bin/getsinglerslidardata" )

# Import target "getmutivelodynedata" for configuration "Release"
set_property(TARGET getmutivelodynedata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getmutivelodynedata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getmutivelodynedata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getmutivelodynedata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getmutivelodynedata "${_IMPORT_PREFIX}/bin/getmutivelodynedata" )

# Import target "getvelodynedata" for configuration "Release"
set_property(TARGET getvelodynedata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getvelodynedata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getvelodynedata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getvelodynedata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getvelodynedata "${_IMPORT_PREFIX}/bin/getvelodynedata" )

# Import target "kittidata" for configuration "Release"
set_property(TARGET kittidata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(kittidata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/kittidata"
  )

list(APPEND _IMPORT_CHECK_TARGETS kittidata )
list(APPEND _IMPORT_CHECK_FILES_FOR_kittidata "${_IMPORT_PREFIX}/bin/kittidata" )

# Import target "getecudata" for configuration "Release"
set_property(TARGET getecudata APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(getecudata PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/getecudata"
  )

list(APPEND _IMPORT_CHECK_TARGETS getecudata )
list(APPEND _IMPORT_CHECK_FILES_FOR_getecudata "${_IMPORT_PREFIX}/bin/getecudata" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
