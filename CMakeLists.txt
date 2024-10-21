set(UTILS_LIB_VERSION "0.0.1")

if(TARGET utils_lib)
      message(WARNING "utils_lib has already been included.")
      message(WARNING "The current version being included is ${UTILS_LIB_VERSION}.")
else()

    add_library(utils_lib_${UTILS_LIB_VERSION} INTERFACE)
    target_include_directories(utils_lib_${UTILS_LIB_VERSION} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

    # Create an alias with the version in its name
    add_library(utils_lib ALIAS utils_lib_${UTILS_LIB_VERSION} )

    option(TEST_UTIL "Build Boost Tests fur util library" FALSE)
    if(TEST_UTIL)
      enable_testing()
      add_subdirectory(tests)
    endif()

endif()
