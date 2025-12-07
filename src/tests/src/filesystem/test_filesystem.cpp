/**
 * @file test_filesystem.cpp
 * @brief contains tests for the filesystem.hpp functions
 *
 * @date 2025.08.24
 * @author Jakob Wandel
 **/

#include <catch2/catch_test_macros.hpp>

#include <utils/filesystem/filesystem.hpp>

#include <array>
#include <filesystem>

namespace fs = std::filesystem;

TEST_CASE("util::getLastPathComponent works with paths", "[util]") {
  using util::getLastPathComponent;

  // SECTION("normal file path")
  {
    const fs::path path("/home/user/file.txt");
    REQUIRE(getLastPathComponent(path) == L"file.txt");
  }

  // SECTION("directory path with trailing slash")
  {
    const fs::path path("/home/user/folder/");
    REQUIRE(getLastPathComponent(path) == L"folder");
  }

  // SECTION("root directory")
  {
    const fs::path path("/");
    REQUIRE(getLastPathComponent(path).empty());
  }

  // SECTION("single file in current directory")
  {
    const fs::path path("file.txt");
    REQUIRE(getLastPathComponent(path) == L"file.txt");
  }

  // SECTION("directory_entry variant")
  {
    const fs::path path("/home/user/file.txt");
    const fs::directory_entry entry(path);
    REQUIRE(getLastPathComponent(entry) == L"file.txt");
  }
}

TEST_CASE("util::hasHiddenElement detects hidden files or directories",
          "[util]") {
  using util::hasHiddenElement;

  // SECTION("no hidden elements")
  {
    const fs::path path("/home/user/folder/file.txt");
    REQUIRE(!hasHiddenElement(path));
  }

  // SECTION("hidden file")
  {
    const fs::path path("/home/user/folder/.hidden");
    REQUIRE(hasHiddenElement(path));
  }

  // SECTION("hidden directory in path")
  {
    const fs::path path("/home/.user/folder/file.txt");
    REQUIRE(hasHiddenElement(path));
  }

  // SECTION("path starting with dot")
  {
    const fs::path path(".hiddenfile");
    REQUIRE(hasHiddenElement(path));
  }

  // SECTION("dot in the middle of filename is not hidden")
  {
    const fs::path path("/home/user/folder/file.name.txt");
    REQUIRE(!hasHiddenElement(path));
  }

  // SECTION("empty path")
  {
    const fs::path path;
    REQUIRE(!hasHiddenElement(path));
  }
}

TEST_CASE("util::getLastPathComponent never throws", "[util][noexcept]") {
  using util::getLastPathComponent;

  const std::array<fs::path, 8> pathList = {fs::path(""),
                                            fs::path("."),
                                            fs::path(".."),
                                            fs::path("normalfile.txt"),
                                            fs::path("/tmp/.hiddenfile"),
                                            fs::path("file.with.dots.ext"),
                                            fs::path("/tmp/folder/"),
                                            fs::path("/t\0mp/folder/")};

  for (const auto& pathEntry : pathList) {
    REQUIRE_NOTHROW(getLastPathComponent(pathEntry));
    auto resultPath = getLastPathComponent(pathEntry);
    (void)resultPath;  // just check it runs
  }
}

TEST_CASE("util::hasHiddenElement never throws", "[util][noexcept]") {
  using util::hasHiddenElement;

  const std::array<fs::path, 8> pathList = {fs::path(""),
                                            fs::path("."),
                                            fs::path(".."),
                                            fs::path("file.txt"),
                                            fs::path(".hidden"),
                                            fs::path("/tmp/.hiddenfile"),
                                            fs::path("/tmp/folder/"),
                                            fs::path("/tm\0p/folder/")};

  for (const auto& pathEntry : pathList) {
    REQUIRE_NOTHROW(hasHiddenElement(pathEntry));
    auto resultFlag = hasHiddenElement(pathEntry);
    (void)resultFlag;  // just check it runs
  }
}
