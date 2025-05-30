

if (FUZZER_ENABLED)

  add_executable(fuzzer_searchAndReplace src/fuzzer_searchAndReplace.cpp)

  target_link_libraries(fuzzer_searchAndReplace 
    PRIVATE 
    utils_lib_0.0.1
    ${ENVIRONMENT_SETTINGS}
  )

endif()

