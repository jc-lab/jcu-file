/**
 * @file	win-file-handler.h
 * @author	Joseph Lee <development@jc-lab.net>
 * @date	2019/11/12
 * @copyright Copyright (C) 2019 jichan.\n
 *            This software may be modified and distributed under the terms
 *            of the Apache License 2.0.  See the LICENSE file for details.
 */

#ifndef __JCU_FILE_WIN32_WIN_FILE_HANDLER_H__
#define __JCU_FILE_WIN32_WIN_FILE_HANDLER_H__

#include "../file-handler.h"

#include <windows.h>

namespace jcu {
namespace file {
namespace win32 {
class WinFileHandler : public FileHandler {
 protected:
  const std::basic_string<TCHAR> path_;
  std::basic_string<TCHAR> temp_path_;
  std::basic_string<TCHAR> old_path_;
  HANDLE handle_;
  int flags_;

  int removeOld();

 public:
  WinFileHandler(const std::basic_string<TCHAR> &path);
  int open(int flags) override;
  int read(void *buf, int size) override;
  int write(const void *buf, int size) override;
  int commit() override;
  int close() override;
  bool isOpen() const override;
  Path getOldName() const override;
};
}
}
}

#endif //__JCU_FILE_WIN32_WIN_FILE_HANDLER_H__
