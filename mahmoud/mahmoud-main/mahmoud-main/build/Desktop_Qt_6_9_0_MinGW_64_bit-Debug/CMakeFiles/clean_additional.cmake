# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\SystemMonitor_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\SystemMonitor_autogen.dir\\ParseCache.txt"
  "SystemMonitor_autogen"
  )
endif()
