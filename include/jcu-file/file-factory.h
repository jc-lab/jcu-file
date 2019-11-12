/**
 * @file	file-factory.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_FILE_FACTORY_H__
#define __JCU_FILE_FACTORY_H__

#include "file-handler.h"
#include "path.h"

#include <memory>

namespace jcu {
    namespace file {

        class FileFactory {
        public:
            virtual std::unique_ptr<FileHandler> createFileHandle(const Path &file_path) const = 0;

            virtual int makeDirectory(const Path& path, bool recursive = false) const = 0;
        };

        extern FileFactory *fs();

    }
}

#endif //__JCU_FILE_FACTORY_H__
