/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definitions for the TTA demultiplexer module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __R_TTA_H
#define __R_TTA_H

#include "common/os.h"

#include <stdio.h>

#include "common/common.h"
#include "common/error.h"
#include "common/mm_io.h"
#include "merge/pr_generic.h"
#include "common/tta.h"

class tta_reader_c: public generic_reader_c {
private:
  unsigned char *chunk;
  mm_io_c *io;
  int64_t bytes_processed, size;
  vector<uint32_t> seek_points;
  int pos;
  tta_file_header_t header;

public:
  tta_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~tta_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual int get_progress();
  virtual void identify();
  virtual void create_packetizer(int64_t id);

  static int probe_file(mm_io_c *io, int64_t size);
};

#endif // __R_TTA_H
