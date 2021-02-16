#include <string>
#include <map>
#include <list>

#include <test-config.h>

#include <gtest/gtest.h>

#include <jcu-file/path.h>
#include <jcu-file/file-factory.h>

using namespace jcu::file;

// PathTest
namespace {

TEST(PathTest, parent) {
  Path a = Path::newFromUtf8("aaaa");
  Path b = Path::newFromUtf8("bbbb");
  Path c = Path::join(a, b);

  EXPECT_EQ(c.parent().toUtf8(), a.toUtf8());
}

} // namespace

// FileSystemTest
namespace {

std::string getTestFilesDir() {
  std::string temp = TEST_FILES_DIR;
  for (auto it = temp.begin(); it != temp.end(); it++) {
    if (*it == '/') {
      *it = '\\';
    }
  }
  return temp;
}

TEST(FileSystemTest, getFileSizeFile1) {
  std::string test_dir = getTestFilesDir();
  std::string filename = "file-1";

  auto file_factory = fs();
  auto file_path = Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8(filename));
  int64_t size = file_factory->getFileSize(file_path);

  EXPECT_EQ(size, 11);
}

TEST(FileSystemTest, getFileSizeFile2) {
  std::string test_dir = getTestFilesDir();
  std::string filename = "file-2";

  auto file_factory = fs();
  auto file_path = Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8(filename));
  int64_t size = file_factory->getFileSize(file_path);

  EXPECT_EQ(size, 0);
}

TEST(FileSystemTest, readdir) {
  std::string test_dir = getTestFilesDir();
  auto file_factory = fs();
  std::list<Path> file_list;
  int rc = file_factory->readdir(file_list, Path::newFromUtf8(test_dir));

  EXPECT_EQ(rc, 0);

  std::map<std::string, unsigned int> expect_values;
  expect_values.emplace(Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8("file-1")).toUtf8(), 1);
  expect_values.emplace(Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8("file-2")).toUtf8(), 2);
  expect_values.emplace(Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8("dir-a")).toUtf8(), 3);
  expect_values.emplace(Path::join(Path::newFromUtf8(test_dir), Path::newFromUtf8("dir-b")).toUtf8(), 4);

  unsigned int result_mask = 0;
  unsigned int expect_mask = 0x1e;

  for (auto it = file_list.cbegin(); it != file_list.cend(); it++) {
    const auto found = expect_values.find(it->toUtf8());
    EXPECT_TRUE(found != expect_values.cend());
    if (found != expect_values.cend()) {
      result_mask |= (1U << found->second);
    }
  }

  EXPECT_EQ(result_mask, expect_mask);
}

} // namespace
