/**
 * @file	file-handler.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_FILE_HANDLER_H__
#define __JCU_FILE_HANDLER_H__

#include <stdint.h>

#include "path.h"

namespace jcu {
namespace file {

enum Flag {
  MODE_READ = 0x00000001,
  MODE_WRITE = 0x00000002,
  MODE_CREATE = 0x00000010,
  MODE_EXISTS = 0x00000020,
  SHARE_READ = 0x00000100,
  RENAME_IF_EXISTS = 0x00010000,
  REMOVE_IF_EXISTS = 0x00020000,
  USE_TEMPNAME = 0x00040000,
};

class FileHandler {
 public:
  /**
   * Open the file
   *
   * @param flags
   * @return
   */
  virtual int open(int flags) = 0;

  /**
   * Read from file
   *
   * @param buf
   * @param size
   * @return read bytes
   */
  virtual int read(void *buf, int size) = 0;

  /**
   * Write to file
   *
   * @param buf
   * @param size
   * @return written bytes
   */
  virtual int write(const void *buf, int size) = 0;

  /**
   * remove temp file to real name
   *
   * @return
   */
  virtual int commit() = 0;

  /**
   * Close the file
   *
   * @return
   */
  virtual int close() = 0;

  virtual bool isOpen() const = 0;

  virtual Path getOldName() const = 0;
};

}
}

#endif //__JCU_FILE_HANDLER_H__
