/** MPEG video helper functions (MPEG 1, 2 and 4)

   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   \file

   \author Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __MTX_COMMON_MPEG4_P10_H
#define __MTX_COMMON_MPEG4_P10_H

#include "common/os.h"

#include "common/memory.h"

#define NALU_START_CODE 0x00000001

#define NALU_TYPE_NON_IDR_SLICE  0x01
#define NALU_TYPE_DP_A_SLICE     0x02
#define NALU_TYPE_DP_B_SLICE     0x03
#define NALU_TYPE_DP_C_SLICE     0x04
#define NALU_TYPE_IDR_SLICE      0x05
#define NALU_TYPE_SEI            0x06
#define NALU_TYPE_SEQ_PARAM      0x07
#define NALU_TYPE_PIC_PARAM      0x08
#define NALU_TYPE_ACCESS_UNIT    0x09
#define NALU_TYPE_END_OF_SEQ     0x0a
#define NALU_TYPE_END_OF_STREAM  0x0b
#define NALU_TYPE_FILLER_DATA    0x0c

#define AVC_SLICE_TYPE_P   0
#define AVC_SLICE_TYPE_B   1
#define AVC_SLICE_TYPE_I   2
#define AVC_SLICE_TYPE_SP  3
#define AVC_SLICE_TYPE_SI  4
#define AVC_SLICE_TYPE2_P  5
#define AVC_SLICE_TYPE2_B  6
#define AVC_SLICE_TYPE2_I  7
#define AVC_SLICE_TYPE2_SP 8
#define AVC_SLICE_TYPE2_SI 9

#define AVC_EXTENDED_SAR        0xff
#define AVC_NUM_PREDEFINED_PARS   17

namespace mpeg4 {
  namespace p10 {
    struct sps_info_t {
      unsigned id;

      unsigned profile_idc;
      unsigned profile_compat;
      unsigned level_idc;
      unsigned log2_max_frame_num;
      unsigned pic_order_cnt_type;
      unsigned log2_max_pic_order_cnt_lsb;
      int offset_for_non_ref_pic;
      int offset_for_top_to_bottom_field;
      unsigned num_ref_frames_in_pic_order_cnt_cycle;
      bool delta_pic_order_always_zero_flag;
      bool frame_mbs_only;

      // vui:
      bool vui_present, ar_found;
      unsigned par_num, par_den;

      // timing_info:
      bool timing_info_present;
      unsigned num_units_in_tick, time_scale;
      bool fixed_frame_rate;

      unsigned crop_left, crop_top, crop_right, crop_bottom;
      unsigned width, height;

      uint32_t checksum;

      sps_info_t() {
        memset(this, 0, sizeof(*this));
      }
    };

    struct pps_info_t {
      unsigned id;
      unsigned sps_id;

      bool pic_order_present;

      uint32_t checksum;

      pps_info_t() {
        memset(this, 0, sizeof(*this));
      }
    };

    struct slice_info_t {
      unsigned char nalu_type;
      unsigned char nal_ref_idc;
      unsigned char type;
      unsigned char pps_id;
      unsigned frame_num;
      bool field_pic_flag, bottom_field_flag;
      unsigned idr_pic_id;
      unsigned pic_order_cnt_lsb;
      int delta_pic_order_cnt_bottom;
      int delta_pic_order_cnt[2];
      int first_mb_in_slice;

      int sps;
      int pps;

      slice_info_t() {
        memset(this, 0, sizeof(*this));
      }
    };

    void MTX_DLL_API nalu_to_rbsp(memory_cptr &buffer);
    void MTX_DLL_API rbsp_to_nalu(memory_cptr &buffer);

    bool MTX_DLL_API parse_sps(memory_cptr &buffer, sps_info_t &sps, bool keep_ar_info = false);
    bool MTX_DLL_API parse_pps(memory_cptr &buffer, pps_info_t &pps);

    bool MTX_DLL_API extract_par(uint8_t *&buffer, int &buffer_size, uint32_t &par_num, uint32_t &par_den);
    bool MTX_DLL_API is_avc_fourcc(const char *fourcc);
    memory_cptr MTX_DLL_API avcc_to_nalus(const unsigned char *buffer, int size);

    struct avc_frame_t {
      memory_cptr m_data;
      int64_t m_start, m_end, m_ref1, m_ref2;
      bool m_keyframe;
      slice_info_t m_si;

      avc_frame_t() {
        clear();
      };

      avc_frame_t(const avc_frame_t &f) {
        *this = f;
      };

      avc_frame_t &operator =(const avc_frame_t &f) {
        m_data     = f.m_data;
        m_start    = f.m_start;
        m_end      = f.m_end;
        m_ref1     = f.m_ref1;
        m_ref2     = f.m_ref2;
        m_keyframe = f.m_keyframe;
        m_si       = f.m_si;

        return *this;
      };

      void clear() {
        m_start    = 0;
        m_end      = 0;
        m_ref1     = 0;
        m_ref2     = 0;
        m_keyframe = false;
        m_data     = memory_cptr(NULL);

        memset(&m_si, 0, sizeof(m_si));
      };
    };

    class MTX_DLL_API nalu_size_length_error_c {
    protected:
      int m_required_length;

    public:
      nalu_size_length_error_c(int required_length)
        : m_required_length(required_length)
        {
      };

      int get_required_length() {
        return m_required_length;
      };
    };

    class MTX_DLL_API avc_es_parser_c {
    protected:
      int m_nalu_size_length;

      bool m_keep_ar_info;
      bool m_avcc_ready, m_avcc_changed;

      int64_t m_default_duration;
      int m_frame_number, m_num_skipped_frames;
      bool m_first_keyframe_found, m_recovery_point_valid, m_b_frames_since_keyframe;

      std::deque<avc_frame_t> m_frames, m_frames_out;
      std::deque<int64_t> m_timecodes;

      bool m_generate_timecodes;

      std::deque<memory_cptr> m_sps_list, m_pps_list, m_extra_data;
      std::vector<sps_info_t> m_sps_info_list;
      std::vector<pps_info_t> m_pps_info_list;

      memory_cptr m_unparsed_buffer;

      avc_frame_t m_incomplete_frame;
      bool m_have_incomplete_frame;
      std::deque<memory_cptr> m_unhandled_nalus;

      bool m_ignore_nalu_size_length_errors, m_discard_actual_frames;

      std::vector<int> m_num_slices_by_type;
      bool m_debug_keyframe_detection;

    public:
      avc_es_parser_c();
      ~avc_es_parser_c();

      void enable_timecode_generation(int64_t default_duration) {
        m_default_duration   = default_duration;
        m_generate_timecodes = true;
      };

      void set_keep_ar_info(bool keep) {
        m_keep_ar_info = keep;
      };

      void add_bytes(unsigned char *buf, int size);
      void add_bytes(memory_cptr &buf) {
        add_bytes(buf->get_buffer(), buf->get_size());
      };

      void flush();

      bool frame_available() {
        return !m_frames_out.empty();
      };

      avc_frame_t get_frame() {
        assert(!m_frames_out.empty());

        avc_frame_t frame(*m_frames_out.begin());
        m_frames_out.erase(m_frames_out.begin(), m_frames_out.begin() + 1);

        return frame;
      };

      memory_cptr get_avcc();

      bool avcc_changed() {
        return m_avcc_changed;
      };

      int get_width() {
        assert(!m_sps_info_list.empty());
        return m_sps_info_list.begin()->width;
      };

      int get_height() {
        assert(!m_sps_info_list.empty());
        return m_sps_info_list.begin()->height;
      };

      void handle_nalu(memory_cptr nalu);

      void add_timecode(int64_t timecode);

      bool headers_parsed() {
        return m_avcc_ready;
      };

      void set_nalu_size_length(int nalu_size_length) {
        m_nalu_size_length = nalu_size_length;
      };

      void ignore_nalu_size_length_errors() {
        m_ignore_nalu_size_length_errors = true;
      };

      void discard_actual_frames(bool discard = true);

      int get_num_skipped_frames() {
        return m_num_skipped_frames;
      };

      void dump_info();

    protected:
      bool parse_slice(memory_cptr &buffer, slice_info_t &si);
      void handle_sps_nalu(memory_cptr &nalu);
      void handle_pps_nalu(memory_cptr &nalu);
      void handle_sei_nalu(memory_cptr &nalu);
      void handle_slice_nalu(memory_cptr &nalu);
      void cleanup();
      void default_cleanup();
      bool flush_decision(slice_info_t &si, slice_info_t &ref);
      void flush_incomplete_frame();
      void flush_unhandled_nalus();
      void write_nalu_size(unsigned char *buffer, int size, int nalu_size_length = -1);
      memory_cptr create_nalu_with_size(const memory_cptr &src, bool add_extra_data = false);
    };
    typedef counted_ptr<avc_es_parser_c> avc_es_parser_cptr;
  };
};

#endif  // __MTX_COMMON_MPEG4_P10_H
