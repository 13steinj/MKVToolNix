/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definitions for the TrueHD demultiplexer module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __R_TRUEHD_H
#define __R_TRUEHD_H

#include "os.h"

#include <stdio.h>

#include "truehd_common.h"
#include "common.h"
#include "error.h"
#include "mm_io.h"
#include "pr_generic.h"

class truehd_reader_c: public generic_reader_c {
private:
  memory_cptr m_chunk;
  mm_io_cptr m_io;
  int64_t m_bytes_processed, m_file_size;
  truehd_frame_cptr m_header;

public:
  truehd_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~truehd_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual int get_progress();
  virtual void identify();
  virtual void create_packetizer(int64_t id);

  static int probe_file(mm_io_c *io, int64_t size);

protected:
  static bool find_valid_headers(mm_io_c *io, int64_t probe_range, int num_headers);
};

#endif // __R_TRUEHD_H
