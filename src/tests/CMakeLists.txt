

add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/data utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})

target_sources(test_data
  PRIVATE
    src/string/test_utf8Conversion.cpp
    src/data/test_BinaryDataWriter.cpp
    src/data/test_BinaryDataReader.cpp
)
add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/string utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})
add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/templates utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})
add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/debug utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})
add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/filesystem utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})
add_catch_test(${CMAKE_CURRENT_SOURCE_DIR}/src/types utils_lib_3.0.0 ${ENVIRONMENT_SETTINGS})
