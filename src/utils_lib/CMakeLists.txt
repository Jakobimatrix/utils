
# change the version if there are API changes!
set(LIB_VERSION "3.0.0")
set(LIB_NAME "utils_lib")

if(NOT DEFINED LIBRARY_LIB_VERSION)
    set(LIBRARY_LIB_VERSION ${LIB_VERSION})

    add_library(${LIB_NAME}_${LIBRARY_LIB_VERSION} STATIC
      include/utils/data/BinaryDataReader.hpp
      include/utils/data/Serialize.hpp
      include/utils/debug/demangle.hpp
      include/utils/debug/benchmark.hpp
      include/utils/debug/logging.hpp
      include/utils/filesystem/filesystem.hpp
      include/utils/string/searchAndReplace.hpp
      include/utils/system/memory.hpp
      include/utils/templates/BitwiseEnums.hpp
      include/utils/templates/TypeContainer.hpp
      include/utils/templates/variadicFunction.hpp
      include/utils/types/constexpr_map.hpp
      include/utils/types/range.hpp
      src/demangle.cpp
      src/filesystem.cpp
      src/memory.cpp
      src/searchAndReplace.cpp
      src/Serialize.cpp
      src/variadicFunction.cpp
      include/utils/data/BinaryDataBuffer.hpp
      include/utils/data/BinaryDataWriter.hpp
      src/BinaryDataBuffer.cpp
      include/utils/string/utf8Conversion.hpp
    )

    target_link_libraries(${LIB_NAME}_${LIBRARY_LIB_VERSION}
      PRIVATE
      ${ENVIRONMENT_SETTINGS}
    )

    target_include_directories(${LIB_NAME}_${LIBRARY_LIB_VERSION} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )

elseif(LIBRARY_LIB_VERSION STREQUAL LIB_VERSION)
    # LIBRARY_LIB_VERSION is defined and matches LIB_VERSION
    # Do nothing

else()
    # LIBRARY_LIB_VERSION does not equal LIB_VERSION
    message(FATAL_ERROR "${LIB_NAME} already included with a different version. This Version: ${LIB_VERSION}, Other Version: ${LIBRARY_LIB_VERSION}")
endif()

