project(test_utils_lib)

set(BOOST_INCLUDE_DIRS $boost_installation_prefix/include)

include_directories(${Boost_INCLUDE_DIRS})
link_directories (${Boost_LIBRARY_DIRS})

find_package(Boost COMPONENTS system unit_test_framework filesystem)

if( Boost_FOUND )
  add_executable(test_utils_type_container test_utils_type_container.cpp)

  target_link_libraries(test_utils_type_container
                        utils_lib
                        ${Boost_FILESYSTEM_LIBRARY}
                        ${Boost_SYSTEM_LIBRARY}
                        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                        )

  # declares a test with executable
  add_test(NAME utils_type_container_test COMMAND test_utils_type_container)
else()
  MESSAGE("BOOST NOT FOUND!! CAN NOT RUN TESTS.")
endif()
