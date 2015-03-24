/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Byte buffer class

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_COMMON_BYTE_BUFFER_H
#define MTX_COMMON_BYTE_BUFFER_H

#include "common/common_pch.h"

#include "common/memory.h"

class byte_buffer_c {
private:
  memory_cptr m_data;
  size_t m_filled, m_offset, m_size, m_chunk_size;
  size_t m_num_reallocs, m_max_alloced_size;

public:
  byte_buffer_c(size_t chunk_size = 128 * 1024)
    : m_data{memory_c::alloc(chunk_size)}
    , m_filled(0)
    , m_offset(0)
    , m_size(chunk_size)
    , m_chunk_size(chunk_size)
    , m_num_reallocs(1)
    , m_max_alloced_size(chunk_size)
  {
  };

  void trim() {
    if (m_offset == 0)
      return;

    auto buffer = m_data->get_buffer();
    memmove(buffer, &buffer[m_offset], m_filled);

    m_offset        = 0;
    size_t new_size = (m_filled / m_chunk_size + 1) * m_chunk_size;

    if (new_size != m_size) {
      m_data->resize(new_size);
      m_size = new_size;

      count_alloc(new_size);
    }
  }

  void add(const unsigned char *new_data, int new_size) {
    if ((m_offset != 0) && ((m_offset + m_filled + new_size) >= m_chunk_size))
      trim();

    if ((m_offset + m_filled + new_size) > m_size) {
      m_size = ((m_offset + m_filled + new_size) / m_chunk_size + 1) * m_chunk_size;
      m_data->resize(m_size);
      count_alloc(m_size);
    }

    memcpy(m_data->get_buffer() + m_offset + m_filled, new_data, new_size);
    m_filled += new_size;
  }

  void add(memory_cptr &new_buffer) {
    add(new_buffer->get_buffer(), new_buffer->get_size());
  }

  void remove(size_t num) {
    if (num > m_filled)
      mxerror("byte_buffer_c: num > m_filled. Should not have happened. Please file a bug report.\n");
    m_offset += num;
    m_filled -= num;

    if (m_filled >= m_chunk_size)
      trim();
  }

  void clear() {
    if (m_filled)
      remove(m_filled);
  }

  unsigned char *get_buffer() {
    return m_data->get_buffer() + m_offset;
  }

  size_t get_size() {
    return m_filled;
  }

  void set_chunk_size(size_t chunk_size) {
    m_chunk_size = chunk_size;
    trim();
  }

private:

  void count_alloc(size_t filled) {
    ++m_num_reallocs;
    m_max_alloced_size = std::max(m_max_alloced_size, filled);
  }
};

using byte_buffer_cptr = std::shared_ptr<byte_buffer_c>;

#endif // MTX_COMMON_BYTE_BUFFER_H
