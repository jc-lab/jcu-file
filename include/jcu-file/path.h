/**
 * @file	path.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_FILE_PATH_H__
#define __JCU_FILE_PATH_H__

#include <string>

#ifdef _WIN32
#include <tchar.h>
#endif

namespace jcu {
    namespace file {

        class Path {
        public:
#ifdef _WIN32
            typedef std::basic_string<TCHAR> system_string_t;
#else
            typedef std::string system_string_t;
#endif

        private:
            system_string_t system_path_;
            Path(const system_string_t &system_string);

        public:
            static Path newFromUtf8(const std::string &text);
            static Path newFromUtf8(const char *text, int length = -1);
            static Path newFromSystem(const system_string_t &text);

            static Path dir();
            static Path cwd();
            static Path self();
            Path parent() const;

            static Path join(const Path &a, const Path &b);

            Path(const Path &obj);
            Path &operator=(const Path &obj);
            const system_string_t &getSystemString() const;
        };

    }
}

#endif //__JCU_FILE_PATH_H__
