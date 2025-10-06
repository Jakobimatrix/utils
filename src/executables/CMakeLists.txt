

if (FUZZER_ENABLED)

  add_executable(fuzzer_searchAndReplace src/fuzzer_searchAndReplace.cpp)

  target_link_libraries(fuzzer_searchAndReplace 
    PRIVATE 
    utils_lib_3.0.0
    ${ENVIRONMENT_SETTINGS}
  )

  add_executable(fuzzer_filesystem src/fuzzer_filesystem.cpp)

  target_link_libraries(fuzzer_filesystem
    PRIVATE
    utils_lib_3.0.0
    ${ENVIRONMENT_SETTINGS}
  )

endif()

