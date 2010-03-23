/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   OGG media stream reader

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include <ogg/ogg.h>
#include <vorbis/codec.h>

#include "common/matroska.h"
#include "input/flac_common.h"
#include "input/r_flac.h"
#include "merge/output_control.h"
#include "merge/pr_generic.h"

#define BUFFER_SIZE 4096

#if defined(HAVE_FLAC_FORMAT_H)

#ifdef LEGACY_FLAC
static FLAC__SeekableStreamDecoderReadStatus
flac_read_cb(const FLAC__SeekableStreamDecoder *,
             FLAC__byte buffer[],
             unsigned *bytes,
             void *client_data) {
  return ((flac_reader_c *)client_data)->read_cb(buffer, bytes);
}
#else
static FLAC__StreamDecoderReadStatus
flac_read_cb(const FLAC__StreamDecoder *,
             FLAC__byte buffer[],
             size_t *bytes,
             void *client_data) {
  return ((flac_reader_c *)client_data)->read_cb(buffer, bytes);
}
#endif

#ifdef LEGACY_FLAC
static FLAC__StreamDecoderWriteStatus
flac_write_cb(const FLAC__SeekableStreamDecoder *,
              const FLAC__Frame *frame,
              const FLAC__int32 * const data[],
              void *client_data) {
  return ((flac_reader_c *)client_data)->write_cb(frame, data);
}
#else
static FLAC__StreamDecoderWriteStatus
flac_write_cb(const FLAC__StreamDecoder *,
              const FLAC__Frame *frame,
              const FLAC__int32 * const data[],
              void *client_data) {
  return ((flac_reader_c *)client_data)->write_cb(frame, data);
}
#endif

#ifdef LEGACY_FLAC
static void
flac_metadata_cb(const FLAC__SeekableStreamDecoder *,
                 const FLAC__StreamMetadata *metadata,
                 void *client_data) {
  ((flac_reader_c *)client_data)->metadata_cb(metadata);
}
#else
static void
flac_metadata_cb(const FLAC__StreamDecoder *,
                 const FLAC__StreamMetadata *metadata,
                 void *client_data) {
  ((flac_reader_c *)client_data)->metadata_cb(metadata);
}
#endif

#ifdef LEGACY_FLAC
static void
flac_error_cb(const FLAC__SeekableStreamDecoder *,
              FLAC__StreamDecoderErrorStatus status,
              void *client_data) {
  ((flac_reader_c *)client_data)->error_cb(status);
}
#else
static void
flac_error_cb(const FLAC__StreamDecoder *,
              FLAC__StreamDecoderErrorStatus status,
              void *client_data) {
  ((flac_reader_c *)client_data)->error_cb(status);
}
#endif

#ifdef LEGACY_FLAC
static FLAC__SeekableStreamDecoderSeekStatus
flac_seek_cb(const FLAC__SeekableStreamDecoder *,
             FLAC__uint64 absolute_byte_offset,
             void *client_data) {
  return ((flac_reader_c *)client_data)->seek_cb(absolute_byte_offset);
}
#else
static FLAC__StreamDecoderSeekStatus
flac_seek_cb(const FLAC__StreamDecoder *,
             FLAC__uint64 absolute_byte_offset,
             void *client_data) {
  return ((flac_reader_c *)client_data)->seek_cb(absolute_byte_offset);
}
#endif

#ifdef LEGACY_FLAC
static FLAC__SeekableStreamDecoderTellStatus
flac_tell_cb(const FLAC__SeekableStreamDecoder *,
             FLAC__uint64 *absolute_byte_offset,
             void *client_data) {
  return ((flac_reader_c *)client_data)->tell_cb(*absolute_byte_offset);
}
#else
static FLAC__StreamDecoderTellStatus
flac_tell_cb(const FLAC__StreamDecoder *,
             FLAC__uint64 *absolute_byte_offset,
             void *client_data) {
  return ((flac_reader_c *)client_data)->tell_cb(*absolute_byte_offset);
}
#endif

#ifdef LEGACY_FLAC
static FLAC__SeekableStreamDecoderLengthStatus
flac_length_cb(const FLAC__SeekableStreamDecoder *,
               FLAC__uint64 *stream_length,
               void *client_data) {
  return ((flac_reader_c *)client_data)->length_cb(*stream_length);
}
#else
static FLAC__StreamDecoderLengthStatus
flac_length_cb(const FLAC__StreamDecoder *,
               FLAC__uint64 *stream_length,
               void *client_data) {
  return ((flac_reader_c *)client_data)->length_cb(*stream_length);
}
#endif

#ifdef LEGACY_FLAC
static FLAC__bool
flac_eof_cb(const FLAC__SeekableStreamDecoder *,
            void *client_data) {
  return ((flac_reader_c *)client_data)->eof_cb();
}
#else
static FLAC__bool
flac_eof_cb(const FLAC__StreamDecoder *,
            void *client_data) {
  return ((flac_reader_c *)client_data)->eof_cb();
}
#endif

int
flac_reader_c::probe_file(mm_io_c *io,
                          int64_t size) {
  unsigned char data[4];

  if (4 > size)
    return 0;
  try {
    io->setFilePointer(0, seek_beginning);
    if (io->read(data, 4) != 4)
      return 0;
    io->setFilePointer(0, seek_beginning);
  } catch (...) {
    return 0;
  }

  if (strncmp((char *)data, "fLaC", 4))
    return 0;

  return 1;
}

flac_reader_c::flac_reader_c(track_info_c &_ti)
  throw (error_c):
  generic_reader_c(_ti),
  samples(0),
  header(NULL) {

  try {
    file      = new mm_file_io_c(m_ti.m_fname);
    file_size = file->get_size();
  } catch (...) {
    throw error_c(Y("flac_reader: Could not open the source file."));
  }

  if (g_identifying)
    return;

  mxinfo_fn(m_ti.m_fname, Y("Using the FLAC demultiplexer.\n"));

  if (!parse_file())
    throw error_c(Y("flac_reader: Could not read all header packets."));

  try {
    uint32_t block_size = 0;

    for (current_block = blocks.begin(); (current_block != blocks.end()) && (FLAC_BLOCK_TYPE_HEADERS == current_block->type); current_block++)
      block_size += current_block->len;

    unsigned char *buf = (unsigned char *)safemalloc(block_size);

    block_size         = 0;
    for (current_block = blocks.begin(); (current_block != blocks.end()) && (FLAC_BLOCK_TYPE_HEADERS == current_block->type); current_block++) {
      file->setFilePointer(current_block->filepos);
      if (file->read(&buf[block_size], current_block->len) != current_block->len)
        mxerror(Y("flac_reader: Could not read a header packet.\n"));
      block_size += current_block->len;
    }

    header      = buf;
    header_size = block_size;

  } catch (error_c &error) {
    mxerror(Y("flac_reader: could not initialize the FLAC packetizer.\n"));
  }
}

flac_reader_c::~flac_reader_c() {
  delete file;
  safefree(header);
}

void
flac_reader_c::create_packetizer(int64_t) {
  if (NPTZR() != 0)
    return;

  add_packetizer(new flac_packetizer_c(this, m_ti, header, header_size));
  mxinfo_tid(m_ti.m_fname, 0, Y("Using the FLAC output module.\n"));
}

bool
flac_reader_c::parse_file() {
#ifdef LEGACY_FLAC
  FLAC__SeekableStreamDecoder *decoder;
  FLAC__SeekableStreamDecoderState state;
#else
  FLAC__StreamDecoder *decoder;
  FLAC__StreamDecoderState state;
#endif
  flac_block_t block;
  uint64_t u, old_pos;
  int result, progress, old_progress;
  bool ok;

  file->setFilePointer(0);
  metadata_parsed = false;

  mxinfo(Y("+-> Parsing the FLAC file. This can take a LONG time.\n"));
#ifdef LEGACY_FLAC
  decoder = FLAC__seekable_stream_decoder_new();
#else
  decoder = FLAC__stream_decoder_new();
#endif
  if (decoder == NULL)
    mxerror(Y("flac_reader: FLAC__stream_decoder_new() failed.\n"));
#ifdef LEGACY_FLAC
  FLAC__seekable_stream_decoder_set_client_data(decoder, this);
  if (!FLAC__seekable_stream_decoder_set_read_callback(decoder, flac_read_cb))
    mxerror(Y("flac_reader: Could not set the read callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_write_callback(decoder, flac_write_cb))
    mxerror(Y("flac_reader: Could not set the write callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_metadata_callback(decoder, flac_metadata_cb))
    mxerror(Y("flac_reader: Could not set the metadata callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_error_callback(decoder, flac_error_cb))
    mxerror(Y("flac_reader: Could not set the error callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_metadata_respond_all(decoder))
    mxerror(Y("flac_reader: Could not set metadata_respond_all.\n"));
  if (!FLAC__seekable_stream_decoder_set_seek_callback(decoder, flac_seek_cb))
    mxerror(Y("flac_reader: Could not set the seek callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_tell_callback(decoder, flac_tell_cb))
    mxerror(Y("flac_reader: Could not set the tell callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_length_callback(decoder, flac_length_cb))
    mxerror(Y("flac_reader: Could not set the length callback.\n"));
  if (!FLAC__seekable_stream_decoder_set_eof_callback(decoder, flac_eof_cb))
    mxerror(Y("flac_reader: Could not set the eof callback.\n"));
  if (FLAC__seekable_stream_decoder_init(decoder) != FLAC__SEEKABLE_STREAM_DECODER_OK)
    mxerror(Y("flac_reader: Could not initialize the FLAC decoder.\n"));
#else  // LEGACY_FLAC
  if (!FLAC__stream_decoder_set_metadata_respond_all(decoder))
    mxerror(Y("flac_reader: Could not set metadata_respond_all.\n"));
  if (FLAC__stream_decoder_init_stream(decoder, flac_read_cb, flac_seek_cb, flac_tell_cb, flac_length_cb, flac_eof_cb, flac_write_cb,
                                       flac_metadata_cb, flac_error_cb, this) != FLAC__STREAM_DECODER_INIT_STATUS_OK)
    mxerror(Y("flac_reader: Could not initialize the FLAC decoder.\n"));
#endif  // LEGACY_FLAC

#ifdef LEGACY_FLAC
  result = (int)FLAC__seekable_stream_decoder_process_until_end_of_metadata(decoder);
#else
  result = (int)FLAC__stream_decoder_process_until_end_of_metadata(decoder);
#endif

  mxverb(2, boost::format("flac_reader: extract->metadata, result: %1%, mdp: %2%, num blocks: %3%\n") % result % metadata_parsed % blocks.size());

  if (!metadata_parsed)
    mxerror_fn(m_ti.m_fname, Y("No metadata block found. This file is broken.\n"));

#ifdef LEGACY_FLAC
  FLAC__seekable_stream_decoder_get_decode_position(decoder, &u);
#else
  FLAC__stream_decoder_get_decode_position(decoder, &u);
#endif

  block.type    = FLAC_BLOCK_TYPE_HEADERS;
  block.filepos = 0;
  block.len     = u;
  old_pos       = u;

  blocks.push_back(block);

  mxverb(2, boost::format("flac_reader: headers: block at %1% with size %2%\n") % block.filepos % block.len);

  old_progress = -5;
#if defined(HAVE_FLAC_DECODER_SKIP)
# ifdef LEGACY_FLAC
  ok = FLAC__seekable_stream_decoder_skip_single_frame(decoder);
# else  // LEGACY_FLAC
  ok = FLAC__stream_decoder_skip_single_frame(decoder);
# endif  // LEGACY_FLAC
#else    // HAVE_FLAC_DECODER_SKIP
  ok = FLAC__seekable_stream_decoder_process_single(decoder);
#endif  // HAVE_FLAC_DECODER_SKIP
  while (ok) {
#ifdef LEGACY_FLAC
    state = FLAC__seekable_stream_decoder_get_state(decoder);
#else
    state = FLAC__stream_decoder_get_state(decoder);
#endif

    progress = (int)(file->getFilePointer() * 100 / file_size);
    if ((progress - old_progress) >= 5) {
      mxinfo(boost::format(Y("+-> Pre-parsing FLAC file: %1%%%%2%")) % progress % "\r");
      old_progress = progress;
    }

    if (
#ifdef LEGACY_FLAC
        FLAC__seekable_stream_decoder_get_decode_position(decoder, &u) &&
#else
        FLAC__stream_decoder_get_decode_position(decoder, &u) &&
#endif
        (u != old_pos)) {
      block.type    = FLAC_BLOCK_TYPE_DATA;
      block.filepos = old_pos;
      block.len     = u - old_pos;
      old_pos       = u;
      blocks.push_back(block);

      mxverb(2, boost::format("flac_reader: skip/decode frame, block at %1% with size %2%\n") % block.filepos % block.len);
    }

#ifdef LEGACY_FLAC
    if (   (state == FLAC__SEEKABLE_STREAM_DECODER_END_OF_STREAM)
        || (state == FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR)
        || (state == FLAC__SEEKABLE_STREAM_DECODER_MEMORY_ALLOCATION_ERROR)
        || (state == FLAC__SEEKABLE_STREAM_DECODER_READ_ERROR)
        || (state == FLAC__SEEKABLE_STREAM_DECODER_SEEK_ERROR))
      break;
#else
    if (state > FLAC__STREAM_DECODER_READ_FRAME)
      break;
#endif

#if defined(HAVE_FLAC_DECODER_SKIP)
# ifdef LEGACY_FLAC
    ok = FLAC__seekable_stream_decoder_skip_single_frame(decoder);
# else  // LEGACY_FLAC
    ok = FLAC__stream_decoder_skip_single_frame(decoder);
# endif  // LEGACY_FLAC
#else    // HAVE_FLAC_DECODER_SKIP
    ok = FLAC__seekable_stream_decoder_process_single(decoder);
#endif  // HAVE_FLAC_DECODER_SKIP
  }

  if (100 != old_progress)
    mxinfo(Y("+-> Pre-parsing FLAC file: 100%\n"));
  else
    mxinfo("\n");

  if ((blocks.size() == 0) || (blocks[0].type != FLAC_BLOCK_TYPE_HEADERS))
    mxerror(Y("flac_reader: Could not read all header packets.\n"));

#ifdef LEGACY_FLAC
  FLAC__seekable_stream_decoder_reset(decoder);
  FLAC__seekable_stream_decoder_delete(decoder);
#else
  FLAC__stream_decoder_reset(decoder);
  FLAC__stream_decoder_delete(decoder);
#endif

  file->setFilePointer(0);
  blocks[0].len     -= 4;
  blocks[0].filepos  = 4;

  return metadata_parsed;
}

file_status_e
flac_reader_c::read(generic_packetizer_c *,
                    bool) {
  unsigned char *buf;
  int samples_here;

  if (current_block == blocks.end())
    return FILE_STATUS_DONE;

  buf = (unsigned char *)safemalloc(current_block->len);
  file->setFilePointer(current_block->filepos);
  if (file->read(buf, current_block->len) != current_block->len) {
    safefree(buf);
    PTZR0->flush();
    return FILE_STATUS_DONE;
  }

  samples_here = flac_get_num_samples(buf, current_block->len, stream_info);
  PTZR0->process(new packet_t(new memory_c(buf, current_block->len, true), samples * 1000000000 / sample_rate));

  samples += samples_here;
  current_block++;

  if (current_block == blocks.end()) {
    PTZR0->flush();
    return FILE_STATUS_DONE;
  }
  return FILE_STATUS_MOREDATA;
}

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderReadStatus
flac_reader_c::read_cb(FLAC__byte buffer[],
                       unsigned *bytes)
#else
FLAC__StreamDecoderReadStatus
flac_reader_c::read_cb(FLAC__byte buffer[],
                       size_t *bytes)
#endif
{
  unsigned bytes_read, wanted_bytes;

  wanted_bytes = *bytes;
  bytes_read   = file->read((unsigned char *)buffer, wanted_bytes);
  *bytes       = bytes_read;

#ifdef LEGACY_FLAC
  return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
#else
  return bytes_read == wanted_bytes ? FLAC__STREAM_DECODER_READ_STATUS_CONTINUE : FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
#endif
}

FLAC__StreamDecoderWriteStatus
flac_reader_c::write_cb(const FLAC__Frame *,
                        const FLAC__int32 * const []) {
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void
flac_reader_c::metadata_cb(const FLAC__StreamMetadata *metadata) {
  switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
      memcpy(&stream_info, &metadata->data.stream_info, sizeof(FLAC__StreamMetadata_StreamInfo));
      sample_rate     = metadata->data.stream_info.sample_rate;
      metadata_parsed = true;

      mxverb(2, boost::format("flac_reader: STREAMINFO block (%1% bytes):\n") % metadata->length);
      mxverb(2, boost::format("flac_reader:   sample_rate: %1% Hz\n")         % metadata->data.stream_info.sample_rate);
      mxverb(2, boost::format("flac_reader:   channels: %1%\n")               % metadata->data.stream_info.channels);
      mxverb(2, boost::format("flac_reader:   bits_per_sample: %1%\n")        % metadata->data.stream_info.bits_per_sample);

      break;
    default:
      mxverb(2,
             boost::format("%1% (%2%) block (%3% bytes)\n")
             % (  metadata->type == FLAC__METADATA_TYPE_PADDING        ? "PADDING"
                : metadata->type == FLAC__METADATA_TYPE_APPLICATION    ? "APPLICATION"
                : metadata->type == FLAC__METADATA_TYPE_SEEKTABLE      ? "SEEKTABLE"
                : metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT ? "VORBIS COMMENT"
                : metadata->type == FLAC__METADATA_TYPE_CUESHEET       ? "CUESHEET"
                :                                                        "UNDEFINED")
             % metadata->type % metadata->length);
      break;
  }
}

void
flac_reader_c::error_cb(FLAC__StreamDecoderErrorStatus status) {
  mxerror(boost::format(Y("flac_reader: Error parsing the file: %1%\n")) % (int)status);
}

#ifdef LEGACY_FLAC
FLAC__SeekableStreamDecoderSeekStatus
flac_reader_c::seek_cb(uint64_t new_pos) {
  file->setFilePointer(new_pos, seek_beginning);
  if (file->getFilePointer() == new_pos)
    return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
  return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
}

FLAC__SeekableStreamDecoderTellStatus
flac_reader_c::tell_cb(uint64_t &absolute_byte_offset) {
  absolute_byte_offset = file->getFilePointer();
  return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__SeekableStreamDecoderLengthStatus
flac_reader_c::length_cb(uint64_t &stream_length) {
  stream_length = file_size;
  return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
}

#else  // LEGACY_FLAC

FLAC__StreamDecoderSeekStatus
flac_reader_c::seek_cb(uint64_t new_pos) {
  file->setFilePointer(new_pos, seek_beginning);
  if (file->getFilePointer() == new_pos)
    return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
  return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

FLAC__StreamDecoderTellStatus
flac_reader_c::tell_cb(uint64_t &absolute_byte_offset) {
  absolute_byte_offset = file->getFilePointer();
  return FLAC__STREAM_DECODER_TELL_STATUS_OK;
}

FLAC__StreamDecoderLengthStatus
flac_reader_c::length_cb(uint64_t &stream_length) {
  stream_length = file_size;
  return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
}
#endif  // LEGACY_FLAC

FLAC__bool
flac_reader_c::eof_cb() {
  return file->getFilePointer() >= file_size;
}

int
flac_reader_c::get_progress() {
  return 100 * distance(blocks.begin(), current_block) / blocks.size();
}

void
flac_reader_c::identify() {
  id_result_container("FLAC");
  id_result_track(0, ID_RESULT_TRACK_AUDIO, "FLAC");
}

#else  // HAVE_FLAC_FORMAT_H

int
flac_reader_c::probe_file(mm_io_c *io,
                          int64_t size) {
  unsigned char data[4];

  if (4 > size)
    return 0;
  try {
    io->setFilePointer(0, seek_beginning);
    if (io->read(data, 4) != 4)
      return 0;
    io->setFilePointer(0, seek_beginning);
  } catch (...) {
    return 0;
  }
  if (strncmp((char *)data, "fLaC", 4))
    return 0;
  id_result_container_unsupported(io->get_file_name(), "FLAC");
  return 1;
}

#endif // HAVE_FLAC_FORMAT_H
