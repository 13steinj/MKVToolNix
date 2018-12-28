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

#include "common/mm_io_p.h"

class mm_mem_io_c;

class mm_mem_io_private_c : public mm_io_private_c {
public:
  std::size_t pos{}, mem_size{}, allocated{}, increase{};
  unsigned char *mem{};
  const unsigned char *ro_mem{};
  bool free_mem{}, read_only{};
  std::string file_name;

  explicit mm_mem_io_private_c(unsigned char *p_mem,
                               uint64_t p_mem_size,
                               std::size_t p_increase)
    : mem_size{p_mem_size}
    , allocated{p_mem_size}
    , increase{p_increase}
    , mem{p_mem}
  {
    if (0 >= increase)
      throw mtx::invalid_parameter_x();

    if (!mem && (0 < increase)) {
      if (0 == mem_size)
        allocated = increase;

      mem      = safemalloc(allocated);
      free_mem = true;

    }
  }

  explicit mm_mem_io_private_c(unsigned char const *p_mem,
                               uint64_t p_mem_size)
    : mem_size{p_mem_size}
    , allocated{p_mem_size}
    , ro_mem{p_mem}
    , read_only{true}
  {
    if (!ro_mem)
      throw mtx::invalid_parameter_x();
  }

  explicit mm_mem_io_private_c(memory_c const &p_mem)
    : mem_size{p_mem.get_size()}
    , allocated{p_mem.get_size()}
    , ro_mem{p_mem.get_buffer()}
    , read_only{true}
  {
    if (!ro_mem)
      throw mtx::invalid_parameter_x{};
  }
};
