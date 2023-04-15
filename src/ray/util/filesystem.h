// Copyright 2020 The Ray Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

// check via https://stackoverflow.com/questions/1617877/how-to-detect-llvm-and-its-version-through-define-directives, echo 'echo | clang -dM -E -'
#if defined(__APPLE__) && defined(__MACH__)
#include <boost/filesystem.hpp>
#else
#include <filesystem>
#endif
#include <optional>
#include <string>
#include <utility>

// Filesystem and path manipulation APIs.
// (NTFS stream & attribute paths are not supported.)

namespace ray {

/// \return The portable directory separator (slash on all OSes).
static inline char GetAltDirSep() { return '/'; }

/// Equivalent to Python's os.path.basename() for file system paths.
std::string GetFileName(const std::string &path);

/// \return The non-volatile temporary directory for the current user (often /tmp).
std::string GetUserTempDir();

/// \return Whether or not the given character is a directory separator on this platform.
static inline bool IsDirSep(char ch) {
#if defined(__APPLE__) && defined(__MACH__)
  bool result = ch == '/';
#else
  bool result = ch == std::filesystem::path::preferred_separator;
#endif
#ifdef _WIN32
  result |= ch == GetAltDirSep();
#endif
  return result;
}

/// \return The result of joining multiple path components.
template <class... Paths>
std::string JoinPaths(std::string base, const Paths &...components) {
  auto join = [](auto &joined_path, const auto &component) {
#if defined(__APPLE__) && defined(__MACH__)
    // if the components begin with "/" or "////", just get the path name.
    if (!component.empty() &&
        component.front() == boost::filesystem::path::preferred_separator) {
      joined_path = boost::filesystem::path(std::string(joined_path).c_str())
                        .append(boost::filesystem::path(std::string(component).c_str()).filename().string())
                        .string();
    } else {
      joined_path = boost::filesystem::path(joined_path).append(component).string();
    }
#else
    // if the components begin with "/" or "////", just get the path name.
    if (!component.empty() &&
        component.front() == std::filesystem::path::preferred_separator) {
      joined_path = std::filesystem::path(joined_path)
                        .append(std::filesystem::path(component).filename().string())
                        .string();
    } else {
      joined_path = std::filesystem::path(joined_path).append(component).string();
    }
#endif
  };
  (join(base, std::string_view(std::string(components).c_str())), ...);
  return base;
}
}  // namespace ray
