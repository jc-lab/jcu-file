/**
 * @file	win32-file-factory.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jc-lab. All rights reserved.
 */

#include "jcu-file/file-factory.h"
#include "jcu-file/file-handler.h"

#include <sstream>
#include <vector>
#include <time.h>

#ifdef _WIN32
#include "jcu-file/win32/win-file-handler.h"

namespace jcu {
    namespace file {

        namespace win32 {

            class WinFileFactory : public FileFactory {
            public:
                std::unique_ptr<FileHandler> createFileHandle(const Path &file_path) const override {
                    return std::unique_ptr<FileHandler>(new WinFileHandler(file_path.getSystemString()));
                }

                int makeDirectory(const Path& path, bool recursive) const override {
                    int rc = 0;
                    DWORD dwFileAttri = ::GetFileAttributes(path.getSystemString().c_str());
                    if(dwFileAttri == INVALID_FILE_ATTRIBUTES) {
                        if(recursive) {
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
            };

            WinFileHandler::WinFileHandler(const std::basic_string<TCHAR> &path)
                : path_(path), flags_(0), handle_(NULL) {
            }
            int WinFileHandler::removeOld() {
                DWORD dwOldFileAttri = ::GetFileAttributes(path_.c_str());
                if(dwOldFileAttri != INVALID_FILE_ATTRIBUTES) {
                    if(flags_ & RENAME_IF_EXISTS) {
                        std::vector<TCHAR> fnbuf(1024);
                        _stprintf_s(fnbuf.data(), fnbuf.size(), _T("%s.%u.old"), path_.c_str(), (unsigned int)time(NULL));
                        old_path_ = fnbuf.data();
                        if(!::MoveFileEx(path_.c_str(), old_path_.c_str(), 0)) {
                            return ::GetLastError();
                        }
                    }
                    if(flags_ & REMOVE_IF_EXISTS) {
                        std::basic_string<TCHAR> to_remove_file = old_path_.empty() ? path_ : old_path_;
                        if(!::DeleteFile(to_remove_file.c_str())) {
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

                if(flags & MODE_READ)
                    dwDesiredAccess |= GENERIC_READ;
                if(flags & MODE_WRITE)
                    dwDesiredAccess |= GENERIC_WRITE;
                if(flags & MODE_CREATE)
                    dwCreationDisposition = CREATE_ALWAYS;
                else if(flags & MODE_EXISTS)
                    dwCreationDisposition = OPEN_EXISTING;
                else
                    dwCreationDisposition = OPEN_ALWAYS;
                if(flags & SHARE_READ)
                    dwShareMode = FILE_SHARE_READ;

                if(flags & USE_TEMPNAME) {
                    std::vector<TCHAR> fnbuf(1024);
                    _stprintf_s(fnbuf.data(), fnbuf.size(), _T("%s.%u.new"), path_.c_str(), (unsigned int)time(NULL));
                    open_path = fnbuf.data();
                    temp_path_ = open_path;
                }else{
                    removeOld();
                    open_path = path_;
                }

                handle_ = ::CreateFile(open_path.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
                if(handle_ && (handle_ != INVALID_HANDLE_VALUE))
                    return 0;

                return ::GetLastError();
            }
            int WinFileHandler::read(void *buf, int size) {
                DWORD dwReadBytes = 0;
                if(!ReadFile(handle_, buf, size, &dwReadBytes, NULL)) {
                    return -((int)::GetLastError());
                }
                return dwReadBytes;
            }
            int WinFileHandler::write(const void *buf, int size) {
                DWORD dwWrittenBytes = 0;
                if(!WriteFile(handle_, buf, size, &dwWrittenBytes, NULL)) {
                    return -((int)::GetLastError());
                }
                return dwWrittenBytes;
            }
            int WinFileHandler::commit() {
                int rc;

                if(!temp_path_.empty()) {
                    rc = removeOld();
                    if (rc)
                        return rc;

                    if(!::MoveFileEx(temp_path_.c_str(), path_.c_str(), 0)) {
                        return ::GetLastError();
                    }
                }

                return 0;
            }
            int WinFileHandler::close() {
                if(handle_ && (handle_ != INVALID_HANDLE_VALUE)) {
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
        }

        FileFactory* fs() {
            static std::unique_ptr<win32::WinFileFactory> file_factory(new win32::WinFileFactory());
            return file_factory.get();
        }

    }
}
#endif