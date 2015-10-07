/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definition for the AAC output module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_P_AAC_H
#define MTX_P_AAC_H

#include "common/common_pch.h"

#include "common/aac.h"
#include "merge/generic_packetizer.h"
#include "merge/timecode_calculator.h"

class aac_packetizer_c: public generic_packetizer_c {
private:
  int m_samples_per_sec, m_channels, m_profile;
  bool m_headerless;
  aac::parser_c m_parser;
  timecode_calculator_c m_timecode_calculator;
  int64_t m_packet_duration;

  static const int ms_samples_per_packet = 1024;

public:
  aac_packetizer_c(generic_reader_c *p_reader, track_info_c &p_ti, int profile, int samples_per_sec, int channels, bool headerless);
  virtual ~aac_packetizer_c();

  virtual int process(packet_cptr packet);
  virtual void set_headers();

  virtual translatable_string_c get_format_name() const {
    return YT("AAC");
  }

  virtual connection_result_e can_connect_to(generic_packetizer_c *src, std::string &error_message);

private:
  virtual int process_headerless(packet_cptr packet);
};

#endif // MTX_P_AAC_H
