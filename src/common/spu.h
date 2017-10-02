/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   helper functions for SPU data (SubPicture Unist — subtitles on DVDs)

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

#include "common/timestamp.h"

namespace mtx { namespace spu {

timestamp_c get_duration(unsigned char const *data, std::size_t const buf_size);
void set_duration(unsigned char *data, std::size_t const buf_size, timestamp_c const &duration);

}}
