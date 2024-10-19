set(UTILS_LIB_VERSION "0.0.1")

add_library(utils_lib INTERFACE)
target_include_directories(utils_lib INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(utils_lib PROPERTIES
  VERSION ${UTILS_LIB_VERSION}
)

# Create an alias with the version in its name
add_library(utils_lib_${UTILS_LIB_VERSION} ALIAS utils_lib)

# Function to warn if the library is already included
function(check_utils_lib_inclusion)
  if(TARGET utils_lib)
    get_target_property(existing_version utils_lib VERSION)
    message(WARNING "utils_lib version ${existing_version} has already been included.")
    message(WARNING "The current version being included is ${UTILS_LIB_VERSION}.")
  else()
    message(STATUS "Including utils_lib version ${UTILS_LIB_VERSION}.")
  endif()
endfunction()

check_utils_lib_inclusion()
