#include <jcu-file/file-factory.h>

#include <iostream>

int main() {
  auto fs = jcu::file::fs();
  std::list<jcu::file::Path> file_list;

  fs->readdir(file_list, jcu::file::Path::newFromUtf8("G:\\temp"));

  for(auto it = file_list.begin(); it != file_list.end(); it++) {
    std::cout << "FILE ITEM : [" << it->toUtf8() << "]" << std::endl;
  }

  return 0;
}