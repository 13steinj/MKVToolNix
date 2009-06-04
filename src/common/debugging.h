/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   definitions used in all programs, helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __MTX_COMMON_DEBUGGING_H
#define __MTX_COMMON_DEBUGGING_H

#include "common/os.h"

#include <string>

bool MTX_DLL_API debugging_requested(const char *option, std::string *arg = NULL);
bool MTX_DLL_API debugging_requested(const std::string &option, std::string *arg = NULL);
void MTX_DLL_API request_debugging(const std::string &options);

int MTX_DLL_API parse_debug_interval_arg(const std::string &option, int default_value = 1000, int invalid_value = -1);

#endif  // __MTX_COMMON_DEBUGGING_H
