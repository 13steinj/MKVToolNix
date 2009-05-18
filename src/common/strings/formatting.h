/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   std::string formatting functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __MTX_COMMON_STRING_FORMATTING_H
#define __MTX_COMMON_STRING_FORMATTING_H

#include "common/os.h"

#include <string>

#define FMT_TIMECODE "%02d:%02d:%02d.%03d"
#define ARG_TIMECODEINT(t) (int32_t)( (t) / 60 /   60 / 1000),         \
                           (int32_t)(((t) / 60        / 1000) %   60), \
                           (int32_t)(((t)             / 1000) %   60), \
                           (int32_t)( (t)                     % 1000)
#define ARG_TIMECODE(t)    ARG_TIMECODEINT((int64_t)(t))
#define ARG_TIMECODE_NS(t) ARG_TIMECODE((t) / 1000000)
#define FMT_TIMECODEN "%02d:%02d:%02d.%09d"
#define ARG_TIMECODENINT(t) (int32_t)( (t) / 60 / 60 / 1000000000),               \
                            (int32_t)(((t) / 60      / 1000000000) %         60), \
                            (int32_t)(((t)           / 1000000000) %         60), \
                            (int32_t)( (t)                         % 1000000000)
#define ARG_TIMECODEN(t) ARG_TIMECODENINT((int64_t)(t))

std::string MTX_DLL_API format_timecode(int64_t timecode, unsigned int precision = 9);

void MTX_DLL_API fix_format(const char *fmt, std::string &new_fmt);

std::string MTX_DLL_API to_string(int value);
std::string MTX_DLL_API to_string(unsigned int value);
std::string MTX_DLL_API to_string(int64_t value);
std::string MTX_DLL_API to_string(uint64_t value);
std::string MTX_DLL_API to_string(double value, unsigned int precision);
std::string MTX_DLL_API to_string(int64_t numerator, int64_t denominator, unsigned int precision);

#endif  // __MTX_COMMON_STRING_FORMATTING_H
