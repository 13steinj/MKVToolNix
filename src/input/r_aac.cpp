/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   AAC demultiplexer module

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include <algorithm>

#include "common/error.h"
#include "common/id3.h"
#include "input/r_aac.h"
#include "output/p_aac.h"
#include "merge/output_control.h"

int
aac_reader_c::probe_file(mm_io_c *io,
                         uint64_t size,
                         int64_t probe_range,
                         int num_headers) {
  return (find_valid_headers(io, probe_range, num_headers) != -1) ? 1 : 0;
}

#define INITCHUNKSIZE 16384

aac_reader_c::aac_reader_c(track_info_c &_ti)
  throw (error_c)
  : generic_reader_c(_ti)
  , m_bytes_processed(0)
  , m_emphasis_present(false)
  , m_sbr_status_set(false)
{
  try {
    m_io               = mm_io_cptr(new mm_file_io_c(m_ti.m_fname));
    m_size             = m_io->get_size();

    int tag_size_start = skip_id3v2_tag(*m_io);
    int tag_size_end   = id3_tag_present_at_end(*m_io);

    if (0 > tag_size_start)
      tag_size_start = 0;
    if (0 < tag_size_end)
      m_size -= tag_size_end;

    size_t init_read_len = std::min(m_size - tag_size_start, (int64_t)INITCHUNKSIZE);
    m_chunk              = memory_c::alloc(INITCHUNKSIZE);

    if (m_io->read(m_chunk, init_read_len) != init_read_len)
      throw error_c(boost::format(Y("aac_reader: Could not read %1% bytes.")) % init_read_len);

    m_io->setFilePointer(tag_size_start, seek_beginning);

    if (parse_aac_adif_header(*m_chunk, init_read_len, &m_aacheader))
      throw error_c(Y("aac_reader: ADIF header files are not supported."));

    if (find_aac_header(*m_chunk, init_read_len, &m_aacheader, m_emphasis_present) < 0)
      throw error_c(boost::format(Y("aac_reader: No valid AAC packet found in the first %1% bytes.\n")) % init_read_len);

    guess_adts_version();

    m_ti.m_id            = 0;       // ID for this track.
    int detected_profile = m_aacheader.profile;

    if (24000 >= m_aacheader.sample_rate)
      m_aacheader.profile = AAC_PROFILE_SBR;

    if (   (map_has_key(m_ti.m_all_aac_is_sbr,  0) && m_ti.m_all_aac_is_sbr[ 0])
        || (map_has_key(m_ti.m_all_aac_is_sbr, -1) && m_ti.m_all_aac_is_sbr[-1]))
      m_aacheader.profile = AAC_PROFILE_SBR;

    if (   (map_has_key(m_ti.m_all_aac_is_sbr,  0) && !m_ti.m_all_aac_is_sbr[ 0])
        || (map_has_key(m_ti.m_all_aac_is_sbr, -1) && !m_ti.m_all_aac_is_sbr[-1]))
      m_aacheader.profile = detected_profile;

    if (   map_has_key(m_ti.m_all_aac_is_sbr,  0)
        || map_has_key(m_ti.m_all_aac_is_sbr, -1))
      m_sbr_status_set = true;

  } catch (...) {
    throw error_c(Y("aac_reader: Could not open the file."));
  }

  if (verbose)
    mxinfo_fn(m_ti.m_fname, Y("Using the AAC demultiplexer.\n"));
}

aac_reader_c::~aac_reader_c() {
}

void
aac_reader_c::create_packetizer(int64_t) {
  if (!demuxing_requested('a', 0) || (NPTZR() != 0))
    return;

  if (!m_sbr_status_set)
    mxwarn(Y("AAC files may contain HE-AAC / AAC+ / SBR AAC audio. "
             "This can NOT be detected automatically. Therefore you have to "
             "specifiy '--aac-is-sbr 0' manually for this input file if the "
             "file actually contains SBR AAC. The file will be muxed in the "
             "WRONG way otherwise. Also read mkvmerge's documentation.\n"));

  generic_packetizer_c *aacpacketizer = new aac_packetizer_c(this, m_ti, m_aacheader.id, m_aacheader.profile, m_aacheader.sample_rate, m_aacheader.channels, m_emphasis_present);
  add_packetizer(aacpacketizer);

  if (AAC_PROFILE_SBR == m_aacheader.profile)
    aacpacketizer->set_audio_output_sampling_freq(m_aacheader.sample_rate * 2);

  mxinfo_tid(m_ti.m_fname, 0, Y("Using the AAC output module.\n"));
}

// Try to guess if the MPEG4 header contains the emphasis field (2 bits)
void
aac_reader_c::guess_adts_version() {
  aac_header_t tmp_aacheader;

  m_emphasis_present = false;

  // Due to the checks we do have an ADTS header at 0.
  find_aac_header(*m_chunk, INITCHUNKSIZE, &tmp_aacheader, m_emphasis_present);
  if (tmp_aacheader.id != 0)        // MPEG2
    return;

  // Now make some sanity checks on the size field.
  if (tmp_aacheader.bytes > 8192) {
    m_emphasis_present = true;    // Looks like it's borked.
    return;
  }

  // Looks ok so far. See if the next ADTS is right behind this packet.
  int pos = find_aac_header(m_chunk->get_buffer() + tmp_aacheader.bytes, INITCHUNKSIZE - tmp_aacheader.bytes, &tmp_aacheader, m_emphasis_present);
  if (0 != pos)                 // Not ok - what do we do now?
    m_emphasis_present = true;
}

file_status_e
aac_reader_c::read(generic_packetizer_c *,
                   bool) {
  int remaining_bytes = m_size - m_io->getFilePointer();
  int read_len        = std::min(INITCHUNKSIZE, remaining_bytes);
  int num_read        = m_io->read(m_chunk, read_len);

  if (0 < num_read) {
    PTZR0->process(new packet_t(new memory_c(*m_chunk, num_read, false)));
    m_bytes_processed += num_read;

    if (0 < (remaining_bytes - num_read))
      return FILE_STATUS_MOREDATA;
  }

  return flush_packetizers();
}

int
aac_reader_c::get_progress() {
  return 100 * m_bytes_processed / m_size;
}

void
aac_reader_c::identify() {
  std::string verbose_info = std::string("aac_is_sbr:") + std::string(AAC_PROFILE_SBR == m_aacheader.profile ? "true" : "unknown");

  id_result_container("AAC");
  id_result_track(0, ID_RESULT_TRACK_AUDIO, "AAC", verbose_info);
}

int
aac_reader_c::find_valid_headers(mm_io_c *io,
                                 int64_t probe_range,
                                 int num_headers) {
  try {
    io->setFilePointer(0, seek_beginning);
    memory_cptr buf = memory_c::alloc(probe_range);
    int num_read    = io->read(buf->get_buffer(), probe_range);
    int pos         = find_consecutive_aac_headers(buf->get_buffer(), num_read, num_headers);
    io->setFilePointer(0, seek_beginning);

    return pos;
  } catch (...) {
    return -1;
  }
}
