/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes
  
   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html
  
   $Id$
  
   helper functions for WAVPACK data
  
   Written by Steve Lhomme <steve.lhomme@free.fr>.
   Based on a software from David Bryant <dbryant@impulse.net>.
   Modified by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "wavpack_common.h"
#include "common.h"

const uint32_t sample_rates [] = {
   6000,  8000,  9600, 11025, 12000, 16000, 22050,
  24000, 32000, 44100, 48000, 64000, 88200, 96000, 192000 };

static void
little_endian_to_native(void *data,
                        char *format) {
  uint8_t *cp = (uint8_t *)data;
  int32_t temp;

  while (*format) {
    switch (*format) {
      case 'L':
        temp = cp[0] + ((long)cp[1] << 8) + ((long)cp[2] << 16) +
          ((long)cp[3] << 24);
        *(long *)cp = temp;
        cp += 4;
        break;

      case 'S':
        temp = cp[0] + (cp[1] << 8);
        * (short *)cp = (short)temp;
        cp += 2;
        break;

      default:
        if (isdigit(*format))
          cp += *format - '0';

        break;
    }

    format++;
  }
}

static int32_t
read_next_header(mm_io_c *mm_io,
                 wavpack_header_t *wphdr) {
  char buffer[sizeof(*wphdr)], *sp = buffer + sizeof(*wphdr), *ep = sp;
  uint32_t bytes_skipped = 0;
  int bleft;

  while (1) {
    if (sp < ep) {
      bleft = ep - sp;
      memcpy(buffer, sp, bleft);
    }
    else
      bleft = 0;

    if (mm_io->read(buffer + bleft, sizeof(*wphdr) - bleft) !=
        (long)sizeof(*wphdr) - bleft)
      return -1;

    sp = buffer;

    if ((*sp++ == 'w') && (*sp == 'v') && (*++sp == 'p') && (*++sp == 'k') &&
        !(*++sp & 1) && (sp[2] < 16) && !sp[3] && (sp[5] == 4) &&
        (sp[4] >= 2) && (sp[4] <= 0xf)) {
      memcpy(wphdr, buffer, sizeof(*wphdr));
      little_endian_to_native(wphdr, "4LS2LLLLL");
      return bytes_skipped;
    }

    while ((sp < ep) && (*sp != 'w'))
      sp++;

    if ((bytes_skipped += sp - buffer) > 1024 * 1024)
      return -1;
  }
}

int32_t
wv_parse_frame(mm_io_c *mm_io,
               wavpack_header_t &wphdr,
               wavpack_meta_t &meta) {
  uint32_t bcount, total_bytes;

  // read next WavPack header
  bcount = read_next_header(mm_io, &wphdr);

  if (bcount == (uint32_t) -1) {
    return -1;
  }

  // if there's audio samples in there...
  if (wphdr.block_samples) {
    meta.sample_rate = (wphdr.flags & WV_SRATE_MASK) >> WV_SRATE_LSB;

    if (meta.sample_rate == 15)
      mxwarn("wavpack_reader: unknown sample rate!\n");
    else
      meta.sample_rate = sample_rates[meta.sample_rate];

    if (wphdr.flags & WV_INT32_DATA || wphdr.flags & WV_FLOAT_DATA)
      meta.bits_per_sample = 32;
    else
      meta.bits_per_sample = ((wphdr.flags & WV_BYTES_STORED) + 1) << 3;

    meta.samples_per_block = wphdr.block_samples;

    if (wphdr.flags & WV_INITIAL_BLOCK)  {
      meta.channel_count = (wphdr.flags & WV_MONO_FLAG) ? 1 : 2;
      total_bytes = wphdr.ck_size + 8;
      if (wphdr.flags & WV_FINAL_BLOCK) {
        mxverb(2, "wavpack_reader: %s block: %s, %d bytes\n",
               (wphdr.flags & WV_MONO_FLAG) ? "mono" : "stereo",
               (wphdr.flags & WV_HYBRID_FLAG) ? "hybrid" : "lossless",
               wphdr.ck_size + 8);
      }
    } else  {
      meta.channel_count += (wphdr.flags & WV_MONO_FLAG) ? 1 : 2;
      total_bytes = wphdr.ck_size + 8;

      if (wphdr.flags & WV_FINAL_BLOCK) {
        mxverb(2, "wavpack_reader: %d chans: %s, %d bytes\n",
               meta.channel_count, 
               (wphdr.flags & WV_HYBRID_FLAG) ? "hybrid" : "lossless",
               total_bytes);
      }
    }
  } else
    mxwarn("wavpack_reader: non-audio block found\n");

  return wphdr.ck_size - sizeof(wavpack_header_t) + 8;
}

