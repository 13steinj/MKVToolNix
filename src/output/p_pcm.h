/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definition for the PCM output module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __P_PCM_H
#define __P_PCM_H

#include "common/os.h"

#include "common/byte_buffer.h"
#include "common/common.h"
#include "common/samples_timecode_conv.h"
#include "merge/pr_generic.h"

class pcm_packetizer_c: public generic_packetizer_c {
private:
  int m_packetno, m_bytes_per_second, m_samples_per_sec, m_channels, m_bits_per_sample, m_samples_per_packet;
  int64_t m_packet_size, m_samples_output;
  bool m_big_endian, m_ieee_float;
  byte_buffer_c m_buffer;
  samples_to_timecode_converter_c m_s2tc;

public:
  pcm_packetizer_c(generic_reader_c *p_reader, track_info_c &p_ti, int p_samples_per_sec, int channels, int bits_per_sample,
                   bool big_endian = false, bool ieee_float = false) throw (error_c);
  virtual ~pcm_packetizer_c();

  virtual int process(packet_cptr packet);
  virtual void set_headers();
  virtual void flush();

  virtual const char *get_format_name() {
    return "PCM";
  }
  virtual connection_result_e can_connect_to(generic_packetizer_c *src, string &error_message);
};

#endif // __P_PCM_H
