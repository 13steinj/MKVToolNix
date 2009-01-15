/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Subripper subtitle reader

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "os.h"

#include "matroska.h"
#include "r_srt.h"
#include "subtitles.h"

int
srt_reader_c::probe_file(mm_text_io_c *io,
                         int64_t) {
  return srt_parser_c::probe(io);
}

srt_reader_c::srt_reader_c(track_info_c &p_ti)
  throw (error_c)
  : generic_reader_c(p_ti)
{

  try {
    m_io = mm_text_io_cptr(new mm_text_io_c(new mm_file_io_c(ti.fname)));
    if (!srt_parser_c::probe(m_io.get()))
      throw error_c(Y("srt_reader: Source is not a valid SRT file."));

    ti.id  = 0;                 // ID for this track.
    m_subs = srt_parser_cptr(new srt_parser_c(m_io.get(), ti.fname, 0));

  } catch (...) {
    throw error_c(Y("srt_reader: Could not open the source file."));
  }

  if (verbose)
    mxinfo_fn(ti.fname, Y("Using the SRT subtitle reader.\n"));

  m_subs->parse();
}

srt_reader_c::~srt_reader_c() {
}

void
srt_reader_c::create_packetizer(int64_t) {
  if (NPTZR() != 0)
    return;

  bool is_utf8 = m_io->get_byte_order() != BO_NONE;
  add_packetizer(new textsubs_packetizer_c(this, ti, MKV_S_TEXTUTF8, NULL, 0, true, is_utf8));

  mxinfo_tid(ti.fname, 0, Y("Using the text subtitle output module.\n"));
}

file_status_e
srt_reader_c::read(generic_packetizer_c *,
                   bool) {
  if (m_subs->empty())
    return FILE_STATUS_DONE;

  m_subs->process(PTZR0);

  if (m_subs->empty()) {
    flush_packetizers();
    return FILE_STATUS_DONE;
  }

  return FILE_STATUS_MOREDATA;
}

int
srt_reader_c::get_progress() {
  int num_entries = m_subs->get_num_entries();

  return 0 == num_entries ? 100 : 100 * m_subs->get_num_processed() / num_entries;
}

void
srt_reader_c::identify() {
  id_result_container("SRT");
  id_result_track(0, ID_RESULT_TRACK_SUBTITLES, "SRT");
}
