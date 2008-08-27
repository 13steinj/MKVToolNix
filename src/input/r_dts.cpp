/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   $Id$

   DTS demultiplexer module

   Written by Peter Niemayer <niemayer@isg.de>.
   Modified by Moritz Bunkus <moritz@bunkus.org>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "dts_common.h"
#include "error.h"
#include "r_dts.h"
#include "p_dts.h"

#define READ_SIZE 16384

int
dts_reader_c::probe_file(mm_io_c *io,
                         int64_t size) {
  if (size < READ_SIZE)
    return 0;

  try {
    unsigned char buf[READ_SIZE];
    bool dts14_to_16 = false, swap_bytes = false;

    io->setFilePointer(0, seek_beginning);
    if (io->read(buf, READ_SIZE) != READ_SIZE)
      return 0;
    io->setFilePointer(0, seek_beginning);

    if (detect_dts(buf, READ_SIZE, dts14_to_16, swap_bytes))
      return 1;

  } catch (...) {
  }

  return 0;
}

dts_reader_c::dts_reader_c(track_info_c &_ti)
  throw (error_c):
  generic_reader_c(_ti),
  cur_buf(0),
  dts14_to_16(false),
  swap_bytes(false) {

  try {
    io     = new mm_file_io_c(ti.fname);
    size   = io->get_size();
    buf[0] = (unsigned short *)safemalloc(READ_SIZE);
    buf[1] = (unsigned short *)safemalloc(READ_SIZE);

    if (io->read(buf[cur_buf], READ_SIZE) != READ_SIZE)
      throw error_c("dts_reader: Could not read READ_SIZE bytes.");
    io->setFilePointer(0, seek_beginning);

  } catch (...) {
    throw error_c("dts_reader: Could not open the source file.");
  }

  detect_dts(buf[cur_buf], READ_SIZE, dts14_to_16, swap_bytes);

  mxverb(3, "DTS: 14->16 %d swap %d\n", dts14_to_16, swap_bytes);

  decode_buffer(READ_SIZE);
  int pos = find_dts_header((const unsigned char *)buf[cur_buf], READ_SIZE, &dtsheader);

  if (0 > pos)
    throw error_c("dts_reader: No valid DTS packet found in the first READ_SIZE bytes.\n");

  bytes_processed = 0;
  ti.id           = 0;          // ID for this track.

  if (verbose)
    mxinfo(FMT_FN "Using the DTS demultiplexer.\n", ti.fname.c_str());
}

dts_reader_c::~dts_reader_c() {
  delete io;
  safefree(buf[0]);
  safefree(buf[1]);
}

int
dts_reader_c::decode_buffer(int len) {
  if (swap_bytes) {
    swab((char *)buf[cur_buf], (char *)buf[cur_buf ^ 1], len);
    cur_buf ^= 1;
  }

  if (dts14_to_16) {
    dts_14_to_dts_16(buf[cur_buf], len / 2, buf[cur_buf ^ 1]);
    cur_buf ^= 1;
    len      = len * 7 / 8;
  }

  return len;
}

void
dts_reader_c::create_packetizer(int64_t) {
  if (NPTZR() != 0)
    return;

  add_packetizer(new dts_packetizer_c(this, dtsheader, ti));
  mxinfo(FMT_TID "Using the DTS output module.\n", ti.fname.c_str(), (int64_t)0);

  if (1 < verbose)
    print_dts_header(&dtsheader);
}

file_status_e
dts_reader_c::read(generic_packetizer_c *,
                   bool) {
  int nread  = io->read(buf[cur_buf], READ_SIZE);
  nread     &= ~0xf;

  if (0 >= nread) {
    PTZR0->flush();
    return FILE_STATUS_DONE;
  }

  nread = decode_buffer(nread);

  PTZR0->process(new packet_t(new memory_c(buf[cur_buf], nread, false)));
  bytes_processed += nread;

  if ((nread < READ_SIZE) || io->eof()) {
    PTZR0->flush();
    return FILE_STATUS_DONE;
  }

  return FILE_STATUS_MOREDATA;
}

int
dts_reader_c::get_progress() {
  return 100 * bytes_processed / size;
}

void
dts_reader_c::identify() {
  id_result_container("DTS");
  id_result_track(0, ID_RESULT_TRACK_AUDIO, "DTS");
}
