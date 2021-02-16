/**
 * @file	win32-file-factory.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jc-lab. All rights reserved.
 */

#include "jcu-file/file-factory.h"
#include "jcu-file/file-handler.h"

#include <jcu-random/secure-random-factory.h>

#include <sstream>
#include <vector>
#include <time.h>
#include <list>

#ifdef _WIN32
#include "jcu-file/win32/win-file-handler.h"

namespace jcu {
namespace file {

namespace win32 {

class WinFileFactory : public FileFactory {
 private:
  std::unique_ptr<jcu::random::SecureRandom> secure_random_;

 public:
  WinFileFactory() {
    secure_random_ = std::move(jcu::random::getSecureRandomFactory()->create());
  }

  ~WinFileFactory() {
  }

  unsigned int genRandomUint() const {
    return (unsigned int) secure_random_->nextInt();
  }

  std::unique_ptr<FileHandler> createFileHandle(const Path &file_path) const override {
    return std::unique_ptr<FileHandler>(new WinFileHandler(file_path.getSystemString()));
  }

  int makeDirectory(const Path &path, bool recursive) const override {
    int rc = 0;
    DWORD dwFileAttri = ::GetFileAttributes(path.getSystemString().c_str());
    if (dwFileAttri == INVALID_FILE_ATTRIBUTES) {
      if (recursive) {
        rc = makeDirectory(path.parent(), recursive);
        if (rc != 0)
          return rc;
      }
      if (!::CreateDirectory(path.getSystemString().c_str(), NULL)) {
        return ::GetLastError();
      }
    }
    return 0;
  }

  Path getTempDir(int *perr) const override {
    TCHAR szBuffer[MAX_PATH];
    DWORD dwTempDirLen = ::GetTempPath(MAX_PATH - 1, szBuffer);
    szBuffer[dwTempDirLen] = 0;
    if (dwTempDirLen == 0) {
      DWORD dwError = ::GetLastError();
      if (perr)
        *perr = (int) dwError;
      return Path();
    }

    if (perr)
      *perr = 0;

    return Path::newFromSystem(szBuffer);
  }

  Path generateTempPath(const char *prefix, int *perr) const override {
    int err = 0;
    Path tempDir(getTempDir(&err));
    if (err) {
      if (perr)
        *perr = err;
      return Path();
    }

    std::string cstrPrefix(prefix);
    std::basic_string<TCHAR> tstrPrefix(cstrPrefix.begin(), cstrPrefix.end());

    TCHAR szBuffer[MAX_PATH];
    UINT nResult = GetTempFileName(tempDir.getSystemString().c_str(), tstrPrefix.c_str(), genRandomUint(), szBuffer);
    if (nResult == 0) {
      if (perr)
        *perr = (int) ::GetLastError();
      return Path();
    }

    return Path::newFromSystem(szBuffer);
  }
  bool isFile(const Path &path) const override;
  bool isDirectory(const Path &path) const override;
  bool isDevice(const Path &path) const override;
  int readdir(std::list<Path> &out, const Path &path) const override;
  int64_t WinFileFactory::getFileSize(const Path& path) const override;
};

WinFileHandler::WinFileHandler(const std::basic_string<TCHAR> &path)
    : path_(path), flags_(0), handle_(NULL) {
}
int WinFileHandler::removeOld() {
  DWORD dwOldFileAttri = ::GetFileAttributes(path_.c_str());
  if (dwOldFileAttri != INVALID_FILE_ATTRIBUTES) {
    if (flags_ & RENAME_IF_EXISTS) {
      std::vector<TCHAR> fnbuf(1024);
      _stprintf_s(fnbuf.data(), fnbuf.size(), _T("%s.%u.old"), path_.c_str(), (unsigned int) time(NULL));
      old_path_ = fnbuf.data();
      if (!::MoveFileEx(path_.c_str(), old_path_.c_str(), 0)) {
        return ::GetLastError();
      }
    }
    if (flags_ & REMOVE_IF_EXISTS) {
      std::basic_string<TCHAR> to_remove_file = old_path_.empty() ? path_ : old_path_;
      if (!::DeleteFile(to_remove_file.c_str())) {
        ::MoveFileEx(to_remove_file.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
      }
    }
  }
  return 0;
}

int WinFileHandler::open(int flags) {
  DWORD dwDesiredAccess = 0;
  DWORD dwShareMode = 0;
  DWORD dwCreationDisposition = 0;
  std::basic_string<TCHAR> open_path;

  flags_ = flags;

  if (flags & MODE_READ)
    dwDesiredAccess |= GENERIC_READ;
  if (flags & MODE_WRITE)
    dwDesiredAccess |= GENERIC_WRITE;
  if (flags & MODE_CREATE)
    dwCreationDisposition = CREATE_ALWAYS;
  else if (flags & MODE_EXISTS)
    dwCreationDisposition = OPEN_EXISTING;
  else
    dwCreationDisposition = OPEN_ALWAYS;
  if (flags & SHARE_READ)
    dwShareMode = FILE_SHARE_READ;

  if (flags & USE_TEMPNAME) {
    std::vector<TCHAR> fnbuf(1024);
    _stprintf_s(fnbuf.data(), fnbuf.size(), _T("%s.%u.new"), path_.c_str(), (unsigned int) time(NULL));
    open_path = fnbuf.data();
    temp_path_ = open_path;
  } else {
    removeOld();
    open_path = path_;
  }

  handle_ = ::CreateFile(open_path.c_str(),
                         dwDesiredAccess,
                         dwShareMode,
                         NULL,
                         dwCreationDisposition,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  if (handle_ && (handle_ != INVALID_HANDLE_VALUE))
    return 0;

  return ::GetLastError();
}
int WinFileHandler::read(void *buf, int size) {
  DWORD dwReadBytes = 0;
  if (!ReadFile(handle_, buf, size, &dwReadBytes, NULL)) {
    return -((int) ::GetLastError());
  }
  return dwReadBytes;
}
int WinFileHandler::write(const void *buf, int size) {
  DWORD dwWrittenBytes = 0;
  if (!WriteFile(handle_, buf, size, &dwWrittenBytes, NULL)) {
    return -((int) ::GetLastError());
  }
  return dwWrittenBytes;
}
int WinFileHandler::commit() {
  int rc;

  if (!temp_path_.empty()) {
    rc = removeOld();
    if (rc)
      return rc;

    if (!::MoveFileEx(temp_path_.c_str(), path_.c_str(), 0)) {
      return ::GetLastError();
    }
  }

  return 0;
}
int WinFileHandler::close() {
  if (handle_ && (handle_ != INVALID_HANDLE_VALUE)) {
    CloseHandle(handle_);
  }
  handle_ = NULL;
  return 0;
}
bool WinFileHandler::isOpen() const {
  return handle_ && (handle_ != INVALID_HANDLE_VALUE);
}
Path WinFileHandler::getOldName() const {
  return Path::newFromSystem(old_path_);
}

bool WinFileFactory::isFile(const Path &path) const {
  std::basic_string<TCHAR> str_path = path.getSystemString();
  DWORD attrs = ::GetFileAttributes(str_path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return !(attrs & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE));
}

bool WinFileFactory::isDirectory(const Path &path) const {
  std::basic_string<TCHAR> str_path = path.getSystemString();
  DWORD attrs = ::GetFileAttributes(str_path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

bool WinFileFactory::isDevice(const Path &path) const {
  std::basic_string<TCHAR> str_path = path.getSystemString();
  DWORD attrs = ::GetFileAttributes(str_path.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  return (attrs & FILE_ATTRIBUTE_DEVICE);
}

int WinFileFactory::readdir(std::list<Path> &out, const Path &path) const {
  std::basic_string<TCHAR> str_dir = path.getSystemString();
  size_t dir_len;
  const TCHAR last_chr = str_dir.empty() ? 0 : str_dir.at(str_dir.length() - 1);
  WIN32_FIND_DATA ffd = {0};
  HANDLE find_handle;

  if (str_dir.empty()) {
    return -1;
  }

  if (last_chr == _T('\\') || last_chr == _T('/')) {
    str_dir.pop_back();
  }

  str_dir.append(_T("\\"));
  dir_len = str_dir.length();
  str_dir.append(_T("*"));

  find_handle = ::FindFirstFile(str_dir.c_str(), &ffd);
  if (!find_handle || find_handle == INVALID_HANDLE_VALUE) {
    return ::GetLastError();
  }

  do {
    if (_tcscmp(ffd.cFileName, _T(".")) && _tcscmp(ffd.cFileName, _T(".."))) {
      str_dir.resize(dir_len);
      str_dir.append(ffd.cFileName);
      out.emplace_back(Path::newFromSystem(str_dir));
    }
  } while (FindNextFile(find_handle, &ffd));

  ::FindClose(find_handle);

  return 0;
}

int64_t WinFileFactory::getFileSize(const Path& path) const {
  WIN32_FILE_ATTRIBUTE_DATA data = { 0 };
  if(::GetFileAttributesEx(
    path.getSystemString().c_str(),
    GetFileExInfoStandard,
    &data
    )) {
    return ((((int64_t)data.nFileSizeHigh) & 0xffffffffLL) << 32) |
      (((int64_t)data.nFileSizeLow) & 0xffffffffLL);
  }
  return -((int)::GetLastError());
}

}

FileFactory *fs() {
  static std::unique_ptr<win32::WinFileFactory> file_factory(new win32::WinFileFactory());
  return file_factory.get();
}

}
}
#endif
