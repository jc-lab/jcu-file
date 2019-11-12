/**
 * @file	file-path.cc
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jc-lab. All rights reserved.
 */

#include "jcu-file/path.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <vector>

namespace jcu {
    namespace file {

        Path::Path(const Path::system_string_t &system_string)
            : system_path_(system_string) {
        }

        Path::Path(const Path &obj)
            : system_path_(obj.system_path_) {
        }

        Path &Path::operator=(const Path &obj) {
            system_path_ = obj.system_path_;
            return *this;
        }

        const Path::system_string_t &Path::getSystemString() const {
            return system_path_;
        }

        Path Path::newFromSystem(const Path::system_string_t &text) {
            return Path(text);
        }

        Path Path::newFromUtf8(const std::string &text) {
#ifdef _WIN32
            return newFromUtf8(text.c_str(), text.length());
#else
            return Path(text);
#endif
        }

        Path Path::newFromUtf8(const char *text, int length) {
#ifdef _WIN32
            std::vector<wchar_t> wbuf;
            if (length < 0)
                length = strlen(text);
            int wLen = MultiByteToWideChar(CP_UTF8, 0, text, length, NULL, NULL);
            wbuf.reserve(wLen + 1);
            MultiByteToWideChar(CP_UTF8, 0, text, length, wbuf.data(), wLen);
#ifdef _UNICODE
            return Path(std::basic_string<wchar_t>(wbuf.data(), wbuf.data() + wLen));
#else
            std::vector<char> cbuf;
            int cLen = WideCharToMultiByte(CP_ACP, 0, wbuf.data(), wLen, NULL, 0, NULL, NULL);
            cbuf.reserve(cLen + 1);
            WideCharToMultiByte(CP_ACP, 0, wbuf.data(), wLen, cbuf.data(), cLen, NULL, NULL);
            return Path(std::basic_string<char>(cbuf.data(), cbuf.data() + cLen));
#endif
#else
            return Path(std::string(text, length));
#endif
        }

        Path Path::dir() {
            Path self_path(self());
            return self_path.parent();
        }

#ifdef _WIN32
        Path Path::cwd() {
            std::vector<TCHAR> buf(1024);
            DWORD nLength = ::GetCurrentDirectory(buf.size(), buf.data());
            return Path(system_string_t(buf.data(), nLength));
        }
        Path Path::self() {
            std::vector<TCHAR> buf(1024);
            DWORD nLength = ::GetModuleFileName(NULL, buf.data(), buf.size());
            return Path(system_string_t(buf.data(), nLength));
        }
        Path Path::parent() const {
            size_t pos = system_path_.find_last_of("/\\");
            if (system_string_t::npos == pos) {
                return Path(_T(""));
            }
            return Path(system_path_.substr(0, pos));
        }

        Path Path::join(const Path &a, const Path &b) {
            system_string_t joined;
            if (!a.system_path_.empty()) {
                joined = a.system_path_;
                if ((joined.at(joined.length() - 1) != '\\') && (joined.at(joined.length() - 1) != '/'))
                    joined.append("\\");
            }
            joined.append(b.system_path_);
            return Path(joined);
        }
#else
        Path Path::cwd() {
            std::vector<char> buf(1024);
            const char *path = getcwd(buf.data(), buf.size());
            return Path(path);
        }
        Path Path::self() {
            std::vector<char> buf(1024);
            int len = readlink("/proc/self/exe", buf.data(), buf.size());
            return Path(std::string(buf.data(), len));
        }
        Path Path::parent() const {
            size_t pos = system_path_.find_last_of("/");
            if(system_string_t::npos == pos) {
                return Path("");
            }
            return Path(system_path_.substr(0, pos));
        }

        Path Path::join(const Path& a, const Path& b) {
            system_string_t joined;
            if(!a.system_path_.empty()) {
                joined = a.system_path_;
                if(joined.at(joined.length() - 1) != '/')
                    joined.append("/");
            }
            joined.append(b.system_path_);
            return Path(joined);
        }
#endif
    }
}
