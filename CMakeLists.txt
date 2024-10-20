set(UTILS_LIB_VERSION "0.0.1")

add_library(utils_lib_${UTILS_LIB_VERSION} INTERFACE)
target_include_directories(utils_lib_${UTILS_LIB_VERSION} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Create an alias with the version in its name
add_library(utils_lib ALIAS utils_lib_${UTILS_LIB_VERSION} )

# Function to warn if the library is already included
function(check_utils_lib_inclusion)
  if(TARGET utils_lib)
    message(WARNING "utils_lib has already been included.")
    message(WARNING "The current version being included is ${UTILS_LIB_VERSION}.")
  else()
    message(STATUS "Including utils_lib version ${UTILS_LIB_VERSION}.")
  endif()
endfunction()

check_utils_lib_inclusion()


option(TEST_UTIL "Build Boost Tests fur util library" FALSE)
if(TEST_UTIL)
  enable_testing()
  add_subdirectory(tests)
endif()
