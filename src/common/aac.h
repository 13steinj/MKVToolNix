/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   definitions and helper functions for AAC data

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_COMMON_AACCOMMON_H
#define MTX_COMMON_AACCOMMON_H

#include "common/common_pch.h"

#include <ostream>

#include "common/bit_cursor.h"
#include "common/byte_buffer.h"
#include "common/timecode.h"

#define AAC_ID_MPEG4 0
#define AAC_ID_MPEG2 1

#define AAC_PROFILE_MAIN 0
#define AAC_PROFILE_LC   1
#define AAC_PROFILE_SSR  2
#define AAC_PROFILE_LTP  3
#define AAC_PROFILE_SBR  4

#define AAC_SYNC_EXTENSION_TYPE 0x02b7

#define AAC_MAX_PRIVATE_DATA_SIZE 5

#define AAC_ADTS_SYNC_WORD       0xfff000
#define AAC_ADTS_SYNC_WORD_MASK  0xfff000 // first 12 of 24 bits

#define AAC_LOAS_SYNC_WORD       0x56e000 // 0x2b7
#define AAC_LOAS_SYNC_WORD_MASK  0xffe000 // first 11 of 24 bits
#define AAC_LOAS_FRAME_SIZE_MASK 0x001fff // last 13 of 24 bits

namespace aac {

unsigned int get_sampling_freq_idx(unsigned int sampling_freq);
bool parse_codec_id(const std::string &codec_id, int &id, int &profile);
bool parse_audio_specific_config(const unsigned char *data, size_t size, int &profile, int &channels, int &sample_rate, int &output_sample_rate, bool &sbr);
int create_audio_specific_config(unsigned char *data, int profile, int channels, int sample_rate, int output_sample_rate, bool sbr);

class header_c {
public:
  unsigned int object_type, extension_object_type, profile, sample_rate, output_sample_rate, bit_rate, channels, bytes;
  unsigned int id;                       // 0 = MPEG-4, 1 = MPEG-2
  size_t header_bit_size, header_byte_size, data_byte_size;

  bool is_sbr, is_valid;

protected:
  bit_reader_c *m_bc;

public:
  header_c();

  std::string to_string() const;

public:
  static header_c from_audio_specific_config(const unsigned char *data, size_t size);

  void parse_audio_specific_config(const unsigned char *data, size_t size, bool look_for_sync_extension = true);
  void parse_audio_specific_config(bit_reader_c &bc, bool look_for_sync_extension = true);

protected:
  int read_object_type();
  int read_sample_rate();
  void read_eld_specific_config();
  void read_ga_specific_config();
  void read_program_config_element();
  void read_error_protection_specific_config();
};

bool operator ==(const header_c &h1, const header_c &h2);

inline std::ostream &
operator <<(std::ostream &out,
            header_c const &header) {
  out << header.to_string();
  return out;
}

class latm_parser_c {
protected:
  int m_audio_mux_version, m_audio_mux_version_a;
  size_t m_fixed_frame_length, m_frame_length_type, m_frame_bit_offset, m_frame_length;
  header_c m_header;
  bit_reader_c *m_bc;
  bool m_config_parsed;
  debugging_option_c m_debug;

public:
  latm_parser_c();

  bool config_parsed() const;
  header_c const &get_header() const;
  size_t get_frame_bit_offset() const;
  size_t get_frame_length() const;

  void parse(bit_reader_c &bc);

protected:
  unsigned int get_value();
  void parse_audio_specific_config(size_t asc_length);
  void parse_stream_mux_config();
  void parse_payload_mux(size_t length);
  void parse_audio_mux_element();
  size_t parse_payload_length_info();
};

class frame_c {
public:
  header_c m_header;
  uint64_t m_stream_position;
  size_t m_garbage_size;
  timecode_c m_timecode;
  memory_cptr m_data;

public:
  frame_c();
  void init();

  std::string to_string(bool verbose = false) const;
};

class parser_c {
public:
  enum multiplex_type_e {
      unknown_multiplex = 0
    , adts_multiplex
    , adif_multiplex
    , loas_latm_multiplex
  };

protected:
  enum parse_result_e {
      failure
    , success
    , need_more_data
  };

protected:
  std::deque<frame_c> m_frames;
  std::deque<timecode_c> m_provided_timecodes;
  byte_buffer_c m_buffer;
  unsigned char const *m_fixed_buffer;
  size_t m_fixed_buffer_size;
  uint64_t m_parsed_stream_position, m_total_stream_position;
  size_t m_garbage_size, m_num_frames_found, m_abort_after_num_frames;
  bool m_require_frame_at_first_byte, m_copy_data;
  multiplex_type_e m_multiplex_type;
  header_c m_header;
  latm_parser_c m_latm_parser;
  debugging_option_c m_debug;

public:
  parser_c();
  void add_timecode(timecode_c const &timecode);

  void add_bytes(memory_cptr const &mem);
  void add_bytes(unsigned char const *buffer, size_t size);

  void parse_fixed_buffer(unsigned char const *fixed_buffer, size_t fixed_buffer_size);
  void parse_fixed_buffer(memory_cptr const &fixed_buffer);

  void flush();

  size_t frames_available() const;
  bool headers_parsed() const;

  frame_c get_frame();
  uint64_t get_parsed_stream_position() const;
  uint64_t get_total_stream_position() const;

  void abort_after_num_frames(size_t num_frames);
  void require_frame_at_first_byte(bool require);
  void copy_data(bool copy);

public:                         // static functions
  static int find_consecutive_frames(unsigned char const *buffer, size_t buffer_size, size_t num_required_frames);

protected:
  void parse();
  std::pair<parse_result_e, size_t> decode_header(unsigned char const *buffer, size_t buffer_size);
  std::pair<parse_result_e, size_t> decode_adts_header(unsigned char const *buffer, size_t buffer_size);
  std::pair<parse_result_e, size_t> decode_loas_latm_header(unsigned char const *buffer, size_t buffer_size);
  void push_frame(frame_c &frame);
};
typedef std::shared_ptr<parser_c> parser_cptr;

} // namespace aac

#endif // MTX_COMMON_AACCOMMON_H
