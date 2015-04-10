/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   OS dependant file system & system helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>
*/

#include "common/common_pch.h"

#include "common/fs_sys_helpers.h"

namespace mtx { namespace sys {

static bfs::path s_current_executable_path;

bfs::path
get_installation_path() {
  return s_current_executable_path;
}

void
determine_path_to_current_executable(std::string const &argv0) {
  s_current_executable_path = get_current_exe_path(argv0);
}

}}
