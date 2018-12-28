/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

#include "common/mm_proxy_io_p.h"

class mm_write_buffer_io_c;

class mm_write_buffer_io_private_c : public mm_proxy_io_private_c {
public:
  memory_cptr af_buffer;
  unsigned char *buffer{};
  std::size_t fill{};
  std::size_t const size{};

  explicit mm_write_buffer_io_private_c(mm_io_cptr const &p_proxy_io,
                                        std::size_t p_buffer_size)
    : mm_proxy_io_private_c{p_proxy_io}
    , af_buffer{memory_c::alloc(p_buffer_size)}
    , buffer{af_buffer->get_buffer()}
    , size{p_buffer_size}
  {
  }
};
