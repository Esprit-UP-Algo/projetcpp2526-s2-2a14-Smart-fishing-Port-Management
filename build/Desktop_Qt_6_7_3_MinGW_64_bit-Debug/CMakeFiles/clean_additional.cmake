# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\sans_titre_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sans_titre_autogen.dir\\ParseCache.txt"
  "sans_titre_autogen"
  )
endif()
