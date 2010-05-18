/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   VP8 video output module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/matroska.h"
#include "output/p_vp8.h"

using namespace libmatroska;

vp8_video_packetizer_c::vp8_video_packetizer_c(generic_reader_c *p_reader,
                                               track_info_c &p_ti)
  : generic_packetizer_c(p_reader, p_ti)
{
  m_timecode_factory_application_mode = TFA_SHORT_QUEUEING;

  if (get_cue_creation() == CUE_STRATEGY_UNSPECIFIED)
    set_cue_creation(CUE_STRATEGY_IFRAMES);

  set_track_type(track_video);
  set_codec_id(MKV_V_VP8);
}

void
vp8_video_packetizer_c::set_headers() {
  generic_packetizer_c::set_headers();
}

int
vp8_video_packetizer_c::process(packet_cptr packet) {
  add_packet(packet);

  return FILE_STATUS_MOREDATA;
}

#define CMP(member) (member != psrc->member)

connection_result_e
vp8_video_packetizer_c::can_connect_to(generic_packetizer_c *src,
                                       std::string &error_message) {
  vp8_video_packetizer_c *psrc = dynamic_cast<vp8_video_packetizer_c *>(src);
  if (NULL == psrc)
    return CAN_CONNECT_NO_FORMAT;

  connect_check_codec_id(m_hcodec_id, psrc->m_hcodec_id);

  if (CMP(m_htrack_type))
    return CAN_CONNECT_NO_PARAMETERS;

  connect_check_v_width(m_hvideo_pixel_width,      psrc->m_hvideo_pixel_width);
  connect_check_v_height(m_hvideo_pixel_height,    psrc->m_hvideo_pixel_height);
  connect_check_v_dwidth(m_hvideo_display_width,   psrc->m_hvideo_display_width);
  connect_check_v_dheight(m_hvideo_display_height, psrc->m_hvideo_display_height);

  if (CMP(m_htrack_min_cache) || CMP(m_htrack_max_cache) || CMP(m_htrack_default_duration))
    return CAN_CONNECT_NO_PARAMETERS;

  return CAN_CONNECT_YES;
}

bool
vp8_video_packetizer_c::is_compatible_with(output_compatibility_e compatibility) {
  return (OC_MATROSKA == compatibility) || (OC_WEBMEDIA == compatibility);
}
