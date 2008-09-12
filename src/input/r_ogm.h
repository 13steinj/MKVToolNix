/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   $Id$

   class definitions for the OGG media stream reader

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __R_OGM_H
#define __R_OGM_H

#include "os.h"

#include <stdio.h>

#include <ogg/ogg.h>

#include <vector>

#include "mm_io.h"
#include "common.h"
#include "pr_generic.h"
#include "theora_common.h"
#include "kate_common.h"

enum ogm_stream_type_e {
  OGM_STREAM_TYPE_UNKNOWN,
  OGM_STREAM_TYPE_A_AAC,
  OGM_STREAM_TYPE_A_AC3,
  OGM_STREAM_TYPE_A_FLAC,
  OGM_STREAM_TYPE_A_MP2,
  OGM_STREAM_TYPE_A_MP3,
  OGM_STREAM_TYPE_A_PCM,
  OGM_STREAM_TYPE_A_VORBIS,
  OGM_STREAM_TYPE_S_KATE,
  OGM_STREAM_TYPE_S_TEXT,
  OGM_STREAM_TYPE_V_AVC,
  OGM_STREAM_TYPE_V_MSCOMP,
  OGM_STREAM_TYPE_V_THEORA,
};

class ogm_reader_c;

class ogm_demuxer_c {
public:
  ogm_reader_c *reader;

  ogg_stream_state os;
  int ptzr;
  int64_t track_id;

  ogm_stream_type_e stype;
  int serialno, eos;
  int units_processed;
  int num_header_packets, num_non_header_packets;
  bool headers_read;
  string language, title;
  vector<memory_cptr> packet_data, nh_packet_data;
  int64_t first_granulepos, last_granulepos, default_duration;
  bool in_use;

public:
  ogm_demuxer_c(ogm_reader_c *p_reader);

  virtual ~ogm_demuxer_c();

  virtual const char *get_type() {
    return "unknown";
  };
  virtual string get_codec() {
    return "unknown";
  };

  virtual void initialize() {
  };
  virtual generic_packetizer_c *create_packetizer(track_info_c &ti) {
    return NULL;
  };
  virtual void process_page(int64_t granulepos);
  virtual void process_header_page();

  virtual void get_duration_and_len(ogg_packet &op, int64_t &duration, int &duration_len);
  virtual bool is_header_packet(ogg_packet &op) {
    return op.packet[0] & 1;
  };
};

typedef counted_ptr<ogm_demuxer_c> ogm_demuxer_cptr;

class ogm_reader_c: public generic_reader_c {
private:
  ogg_sync_state oy;
  mm_io_c *io;
  vector<ogm_demuxer_cptr> sdemuxers;
  int bos_pages_read;
  int64_t file_size;

public:
  ogm_reader_c(track_info_c &_ti) throw (error_c);
  virtual ~ogm_reader_c();

  virtual file_status_e read(generic_packetizer_c *ptzr, bool force = false);
  virtual void identify();
  virtual void create_packetizers();
  virtual void create_packetizer(int64_t tid);
  virtual void add_available_track_ids();

  virtual int get_progress();

  static int probe_file(mm_io_c *io, int64_t size);

private:
  virtual ogm_demuxer_cptr find_demuxer(int serialno);
  virtual int read_page(ogg_page *);
  virtual void handle_new_stream(ogg_page *);
  virtual void handle_new_stream_and_packets(ogg_page *);
  virtual void process_page(ogg_page *);
  virtual int packet_available();
  virtual int read_headers();
  virtual void process_header_page(ogg_page *pg);
  virtual void process_header_packets(ogm_demuxer_cptr dmx);
  virtual void handle_stream_comments();
};


#endif  // __R_OGM_H
