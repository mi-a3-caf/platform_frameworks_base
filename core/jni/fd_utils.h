/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FRAMEWORKS_BASE_CORE_JNI_FD_UTILS_H_
#define FRAMEWORKS_BASE_CORE_JNI_FD_UTILS_H_

#include <set>
#include <string>
#include <unordered_map>

#include <dirent.h>
#include <inttypes.h>
#include <sys/stat.h>

#include <android-base/macros.h>

// Keeps track of all relevant information (flags, offset etc.) of an
// open zygote file descriptor.
class FileDescriptorInfo {
 public:
  // Create a FileDescriptorInfo for a given file descriptor. Returns
  // |NULL| if an error occurred.
  static FileDescriptorInfo* CreateFromFd(int fd);

  // Checks whether the file descriptor associated with this object
  // refers to the same description.
  bool Restat() const;

  bool ReopenOrDetach() const;

  const int fd;
  const struct stat stat;
  const std::string file_path;
  const int open_flags;
  const int fd_flags;
  const int fs_flags;
  const off_t offset;
  const bool is_sock;

 private:
  FileDescriptorInfo(int fd);

  FileDescriptorInfo(struct stat stat, const std::string& file_path, int fd, int open_flags,
                     int fd_flags, int fs_flags, off_t offset);

  static bool StartsWith(const std::string& str, const std::string& prefix);

  static bool EndsWith(const std::string& str, const std::string& suffix);

  // Returns true iff. a given path is whitelisted. A path is whitelisted
  // if it belongs to the whitelist (see kPathWhitelist) or if it's a path
  // under /system/framework that ends with ".jar" or if it is a system
  // framework overlay.
  static bool IsWhitelisted(const std::string& path);

  static bool Readlink(const int fd, std::string* result);

  // Returns the locally-bound name of the socket |fd|. Returns true
  // iff. all of the following hold :
  //
  // - the socket's sa_family is AF_UNIX.
  // - the length of the path is greater than zero (i.e, not an unnamed socket).
  // - the first byte of the path isn't zero (i.e, not a socket with an abstract
  //   address).
  static bool GetSocketName(const int fd, std::string* result);

  bool DetachSocket() const;

  DISALLOW_COPY_AND_ASSIGN(FileDescriptorInfo);
};

// A FileDescriptorTable is a collection of FileDescriptorInfo objects
// keyed by their FDs.
class FileDescriptorTable {
 public:
  // Creates a new FileDescriptorTable. This function scans
  // /proc/self/fd for the list of open file descriptors and collects
  // information about them. Returns NULL if an error occurs.
  static FileDescriptorTable* Create();

  bool Restat();

  // Reopens all file descriptors that are contained in the table. Returns true
  // if all descriptors were successfully re-opened or detached, and false if an
  // error occurred.
  bool ReopenOrDetach();

 private:
  FileDescriptorTable(const std::unordered_map<int, FileDescriptorInfo*>& map);

  bool RestatInternal(std::set<int>& open_fds);

  static int ParseFd(dirent* e, int dir_fd);

  // Invariant: All values in this unordered_map are non-NULL.
  std::unordered_map<int, FileDescriptorInfo*> open_fd_map_;

  DISALLOW_COPY_AND_ASSIGN(FileDescriptorTable);
};

#endif  // FRAMEWORKS_BASE_CORE_JNI_FD_UTILS_H_
