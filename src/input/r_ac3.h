/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes
  
   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html
  
   $Id$
  
   class definitions for the AC3 demultiplexer module
  
   Written by Moritz Bunkus <moritz@bunkus.org>.
 */

#ifndef __R_AC3_H
#define __R_AC3_H

#include "os.h"

#include <stdio.h>

#include "ac3_common.h"
#include "common.h"
#include "error.h"
#include "mm_io.h"
#include "pr_generic.h"

class ac3_reader_c: public generic_reader_c {
private:
  unsigned char *chunk;
  mm_io_c *mm_io;
  int64_t bytes_processed, size;
  ac3_header_t ac3header;

public:
  ac3_reader_c(track_info_c *nti) throw (error_c);
  virtual ~ac3_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual int get_progress();
  virtual void identify();
  virtual void create_packetizer(int64_t id);

  static int probe_file(mm_io_c *mm_io, int64_t size, int64_t probe_size,
                        int num_headers);

protected:
  static int find_valid_headers(mm_io_c *mm_io, int64_t probe_range,
                                int num_headers);
};

#endif // __R_AC3_H
