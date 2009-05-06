/*
   mkvextract -- extract tracks from Matroska files into other files

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   extracts tracks from Matroska files into other files

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __XTR_VOBSUB_H
#define __XTR_VOBSUB_H

#include "common/os.h"

#include "common/smart_pointers.h"
#include "extract/xtr_base.h"

class xtr_vobsub_c: public xtr_base_c {
public:
  vector<int64_t> m_positions, m_timecodes;
  vector<xtr_vobsub_c *> m_slaves;
  memory_cptr m_private_data;
  string m_base_name, m_language;
  int m_stream_id;

public:
  xtr_vobsub_c(const string &codec_id, int64_t tid, track_spec_t &tspec);

  virtual void create_file(xtr_base_c *master, KaxTrackEntry &track);
  virtual void handle_frame(memory_cptr &frame, KaxBlockAdditions *additions, int64_t timecode, int64_t duration, int64_t bref, int64_t fref,
                            bool keyframe, bool discardable, bool references_valid);
  virtual void finish_file();
  virtual void write_idx(mm_io_c &idx, int index);

  virtual const char *get_container_name() {
    return "VobSubs";
  };
};

#endif
