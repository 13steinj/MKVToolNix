/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   definitions for the W64 file format

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

namespace mtx::w64 {

struct chunk_t {
  unsigned char guid[16];
  uint64_t size;
};

struct header_t {
  chunk_t riff;
  unsigned char wave_guid[16];
};

extern unsigned char const g_guid_riff[16], g_guid_wave[16], g_guid_fmt[16], g_guid_data[16];

}
