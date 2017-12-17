/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   AAC demultiplexer module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/endian.h"
#include "common/file_types.h"
#include "merge/id_result.h"
#include "input/r_aac_adif.h"

int
aac_adif_reader_c::probe_file(mm_io_c &in,
                              uint64_t size) {
  try {
    if (4 > size)
      return 0;

    unsigned char buf[4];

    in.setFilePointer(0, seek_beginning);
    if (in.read(buf, 4) != 4)
      return 0;
    in.setFilePointer(0, seek_beginning);

    if (FOURCC('A', 'D', 'I', 'F') == get_uint32_be(buf)) {
      id_result_container_unsupported(in.get_file_name(), Y("AAC with ADIF headers"));
      // Never reached:
      return 1;
    }

    return 0;

  } catch (...) {
    return 0;
  }
}
