/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   class definitions for the Quicktime & MP4 reader

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __R_QTMP4_H
#define __R_QTMP4_H

#include "os.h"

#include <stdio.h>

#include <vector>

#include "ac3_common.h"
#include "common.h"
#include "mm_io.h"
#include "p_video.h"
#include "qtmp4_atoms.h"
#include "smart_pointers.h"

struct qt_durmap_t {
  uint32_t number;
  uint32_t duration;

  qt_durmap_t():
    number(0),
    duration(0) {
  }
};

struct qt_chunk_t {
  uint32_t samples;
  uint32_t size;
  uint32_t desc;
  uint64_t pos;

  qt_chunk_t():
    samples(0),
    size(0),
    desc(0),
    pos(0) {
  }
};

struct qt_chunkmap_t {
  uint32_t first_chunk;
  uint32_t samples_per_chunk;
  uint32_t sample_description_id;

  qt_chunkmap_t():
    first_chunk(0),
    samples_per_chunk(0),
    sample_description_id(0) {
  }
};

struct qt_editlist_t {
  uint64_t duration;
  uint64_t pos;
  uint32_t speed;
  uint32_t frames;
  uint64_t start_sample;
  uint64_t start_frame;
  int64_t  pts_offset;

  qt_editlist_t():
    duration(0),
    pos(0),
    speed(0),
    frames(0),
    start_sample(0),
    start_frame(0),
    pts_offset(0) {
  }
};

struct qt_sample_t {
  uint64_t pts;
  uint32_t size;
  uint64_t pos;

  qt_sample_t():
    pts(0),
    size(0),
    pos(0) {
  }
};

struct qt_frame_offset_t {
  uint32_t count;
  uint32_t offset;

  qt_frame_offset_t():
    count(0),
    offset(0) {
  }
};

struct qt_index_t {
  int64_t file_pos, size;
  int64_t timecode, duration;
  bool    is_keyframe;

  qt_index_t(int64_t file_pos_, int64_t size_, int64_t timecode_, int64_t duration_, bool is_keyframe_):
    file_pos(file_pos_),
    size(size_),
    timecode(timecode_),
    duration(duration_),
    is_keyframe(is_keyframe_) {
  };
};

struct qtmp4_demuxer_c {
  bool ok;

  char type;
  uint32_t id;
  char fourcc[4];
  uint32_t pos;

  uint32_t time_scale;
  uint64_t global_duration;
  uint32_t sample_size;

  uint64_t duration;

  vector<qt_sample_t> sample_table;
  vector<qt_chunk_t> chunk_table;
  vector<qt_chunkmap_t> chunkmap_table;
  vector<qt_durmap_t> durmap_table;
  vector<uint32_t> keyframe_table;
  vector<qt_editlist_t> editlist_table;
  vector<qt_frame_offset_t> raw_frame_offset_table;
  vector<int32_t> frame_offset_table;

  vector<int64_t> timecodes, durations, frame_indices;

  vector<qt_index_t> m_index;

  int64_t min_timecode, max_timecode;
  double fps;

  esds_t esds;
  bool esds_parsed;

  memory_cptr v_stsd;
  uint32_t v_stsd_size;
  uint32_t v_width, v_height, v_bitdepth;
  deque<int64_t> references;
  bool v_is_avc, avc_use_bframes;
  memory_cptr a_stsd;
  uint32_t a_channels, a_bitdepth;
  float a_samplerate;
  int a_aac_profile, a_aac_output_sample_rate;
  bool a_aac_is_sbr, a_aac_config_parsed;
  ac3_header_t m_ac3_header;

  unsigned char *priv;
  uint32_t priv_size;

  bool warning_printed;

  int ptzr;

  qtmp4_demuxer_c():
    ok(false),
    type('?'),
    id(0),
    pos(0),
    time_scale(1),
    global_duration(0), //avg_duration(0),
    sample_size(0),
    duration(0),
    min_timecode(0),
    max_timecode(0),
    fps(0.0),
    esds_parsed(false),
    v_width(0),
    v_height(0),
    v_bitdepth(0),
    v_is_avc(false),
    avc_use_bframes(false),
    a_channels(0),
    a_bitdepth(0),
    a_samplerate(0.0),
    a_aac_profile(0),
    a_aac_output_sample_rate(0),
    a_aac_is_sbr(false),
    a_aac_config_parsed(false),
    priv(NULL),
    priv_size(0),
    warning_printed(false),
    ptzr(-1) {

    memset(fourcc, 0, 4);
    memset(&esds, 0, sizeof(esds_t));
  }

  ~qtmp4_demuxer_c() {
    safefree(priv);
    safefree(esds.decoder_config);
    safefree(esds.sl_config);
  }

  void calculate_fps();
  int64_t to_nsecs(int64_t value);
  void calculate_timecodes();
  void adjust_timecodes(int64_t delta);

  void update_tables(int64_t global_time_scale);
  void update_editlist_table(int64_t global_time_scale);

  void build_index();

  bool read_first_bytes(memory_cptr &buf, int num_bytes, mm_io_c *io);

private:
  void build_index_chunk_mode();
  void build_index_constant_sample_size_mode();
};
typedef counted_ptr<qtmp4_demuxer_c> qtmp4_demuxer_cptr;

struct qt_atom_t {
  uint32_t fourcc;
  uint64_t size;
  uint64_t pos;
  uint32_t hsize;

  qt_atom_t():
    fourcc(0),
    size(0),
    pos(0),
    hsize(0) {}

  qt_atom_t to_parent() {
    qt_atom_t parent;

    parent.fourcc = fourcc;
    parent.size   = size - hsize;
    parent.pos    = pos  + hsize;
    return parent;
  }
};

class qtmp4_reader_c: public generic_reader_c {
private:
  mm_io_c *io;
  vector<qtmp4_demuxer_cptr> demuxers;
  int64_t file_size, mdat_pos, mdat_size;
  uint32_t time_scale, compression_algorithm;
  int main_dmx;

public:
  qtmp4_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~qtmp4_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual int get_progress();
  virtual void identify();
  virtual void create_packetizers();
  virtual void create_packetizer(int64_t tid);
  virtual void add_available_track_ids();

  static int probe_file(mm_io_c *in, int64_t size);

protected:
  virtual void parse_headers();
  virtual void calculate_timecodes();
  virtual qt_atom_t read_atom(mm_io_c *read_from = NULL, bool exit_on_error = true);
  virtual void parse_video_header_priv_atoms(qtmp4_demuxer_cptr &dmx, unsigned char *mem, int size, int level);
  virtual void parse_audio_header_priv_atoms(qtmp4_demuxer_cptr &dmx, unsigned char *mem, int size, int level);
  virtual bool parse_esds_atom(mm_mem_io_c &memio, qtmp4_demuxer_cptr &dmx, int level);
  virtual uint32_t read_esds_descr_len(mm_mem_io_c &memio);

  virtual void handle_cmov_atom(qt_atom_t parent, int level);
  virtual void handle_cmvd_atom(qt_atom_t parent, int level);
  virtual void handle_ctts_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_dcom_atom(qt_atom_t parent, int level);
  virtual void handle_hdlr_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_mdhd_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_mdia_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_minf_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_moov_atom(qt_atom_t parent, int level);
  virtual void handle_mvhd_atom(qt_atom_t parent, int level);
  virtual void handle_stbl_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stco_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_co64_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stsc_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stsd_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stss_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stsz_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_sttd_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_stts_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_tkhd_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_trak_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_edts_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);
  virtual void handle_elst_atom(qtmp4_demuxer_cptr &new_dmx, qt_atom_t parent, int level);

  virtual memory_cptr create_bitmap_info_header(qtmp4_demuxer_cptr &dmx, const char *fourcc, int extra_size = 0, const void *extra_data = NULL);

  virtual void create_video_packetizer_svq1(qtmp4_demuxer_cptr &dmx);
  virtual bool create_audio_packetizer_ac3(qtmp4_demuxer_cptr &dmx);
};

#endif  // __R_QTMP4_H
