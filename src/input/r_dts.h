/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definitions for the DTS demultiplexer module

   Written by Peter Niemayer <niemayer@isg.de>.
   Modified by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __R_DTS_H
#define __R_DTS_H

#include "common/os.h"

#include <stdio.h>

#include "common/common.h"
#include "common/dts_common.h"
#include "common/error.h"
#include "common/mm_io.h"
#include "merge/pr_generic.h"

class dts_reader_c: public generic_reader_c {
private:
  unsigned short *buf[2];
  int cur_buf;
  mm_io_c *io;
  int64_t bytes_processed, size;
  dts_header_t dtsheader;
  bool dts14_to_16, swap_bytes;

public:
  dts_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~dts_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual int get_progress();
  virtual void identify();
  virtual void create_packetizer(int64_t id);

  static int probe_file(mm_io_c *io, int64_t size);

protected:
  virtual int decode_buffer(int len);
};

#endif // __R_DTS_H
