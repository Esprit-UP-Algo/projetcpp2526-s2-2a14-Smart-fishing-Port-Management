# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\user_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\user_autogen.dir\\ParseCache.txt"
  "user_autogen"
  )
endif()
