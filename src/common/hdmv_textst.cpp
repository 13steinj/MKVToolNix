/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper functions for HDMV TextST subtitles

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/endian.h"
#include "common/hdmv_textst.h"

namespace mtx { namespace hdmv_textst {

::timestamp_c
get_timestamp(unsigned char const *buf) {
  auto value = (static_cast<uint64_t>(buf[0] & 0x01) << 32) | get_uint32_be(&buf[1]);
  return timestamp_c::mpeg(value);
}

void
put_timestamp(unsigned char *buf,
             ::timestamp_c const &timestamp) {
  auto pts = timestamp.to_mpeg();

  buf[0] = (buf[0] & 0xfe) | ((pts >> 32) & 0x01);
  put_uint32_be(&buf[1], pts & ((1ull << 32) - 1));
}


}}
