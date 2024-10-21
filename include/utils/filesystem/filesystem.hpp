#pragma once

#include <filesystem>

namespace util {

std::string inline getFileName(const std::filesystem::path& path){
  if(path.has_filename()){ // like abs/def/ but not marked as directory
    return path.parent_path().filename().string(); // --> def
  }
  return path.filename().string();
};

std::string inline getFileName(const std::filesystem::directory_entry& entry){
  if(std::filesystem::is_directory(entry.status())){
    return entry.path().parent_path().filename().string();
  }
  return getFileName(entry.path());
};


}

