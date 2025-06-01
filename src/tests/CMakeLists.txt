
if (TARGET Catch2::Catch2WithMain)

  add_executable(test_searchAndReplace src/string/test_searchAndReplace.cpp)
  add_executable(test_BinaryDataInterpreter src/data/test_BinaryDataInterpreter.cpp)


  target_link_libraries(test_searchAndReplace 
    PRIVATE
    Catch2::Catch2WithMain
    utils_lib_1.0.0
    ${ENVIRONMENT_SETTINGS}
    )


  target_link_libraries(test_BinaryDataInterpreter 
    PRIVATE
    Catch2::Catch2WithMain
    utils_lib_1.0.0
    ${ENVIRONMENT_SETTINGS}
    )


  catch_discover_tests(test_searchAndReplace)
  catch_discover_tests(test_BinaryDataInterpreter)


endif()
