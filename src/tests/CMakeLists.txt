
if (NOT FUZZER_ENABLED)

  # only enable other executables, if Fuzzer is not linked. Fuzzer brings its own main!
  Include(FetchContent)

  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.4.0 # or a later release
  )

  FetchContent_MakeAvailable(Catch2)

  enable_testing()

  add_executable(test_searchAndReplace src/string/test_searchAndReplace.cpp)
  add_executable(test_BinaryDataInterpreter src/data/test_BinaryDataInterpreter.cpp)


  target_link_libraries(test_searchAndReplace 
    PRIVATE
    Catch2::Catch2WithMain
    utils_lib_0.0.1
    ${ENVIRONMENT_SETTINGS}
    )


  target_link_libraries(test_BinaryDataInterpreter 
    PRIVATE
    Catch2::Catch2WithMain
    utils_lib_0.0.1
    ${ENVIRONMENT_SETTINGS}
    )

  include(CTest)
  include(Catch)

  catch_discover_tests(test_searchAndReplace)
  catch_discover_tests(test_BinaryDataInterpreter)


endif()