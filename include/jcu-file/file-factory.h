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

#include <memory>
#include <list>
#include <string>

#include "file-handler.h"
#include "path.h"

namespace jcu {
namespace file {

class FileFactory {
 public:
  virtual std::unique_ptr<FileHandler> createFileHandle(const Path &file_path) const = 0;

  virtual int makeDirectory(const Path &path, bool recursive = false) const = 0;

  virtual Path getTempDir(int *perr = NULL) const = 0;
  virtual Path generateTempPath(const char *prefix, int *perr = NULL) const = 0;

  virtual bool isFile(const Path &path) const = 0;
  virtual bool isDirectory(const Path &path) const = 0;
  virtual int readdir(std::list<Path> &out, const Path &path) const = 0;
};

extern FileFactory *fs();

}
}

#endif //__JCU_FILE_FACTORY_H__
