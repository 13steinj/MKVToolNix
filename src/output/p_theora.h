/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definition for the Theora video output module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __P_THEORA_H
#define __P_THEORA_H

#include "common/common.h"

#include "output/p_video.h"

class theora_video_packetizer_c: public video_packetizer_c {
public:
  theora_video_packetizer_c(generic_reader_c *p_reader, track_info_c &p_ti, double fps, int width, int height);
  virtual void set_headers();
  virtual int process(packet_cptr packet);

protected:
  virtual void extract_aspect_ratio();
};

#endif  // __P_THEORA_H
