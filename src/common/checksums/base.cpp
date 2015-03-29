/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   checksum calculations – base class implementations

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/checksums/base.h"
#include "common/checksums/adler32.h"
#include "common/checksums/crc.h"
#include "common/checksums/md5.h"
#include "common/make_unique.h"

namespace mtx { namespace checksum {

base_uptr
for_algorithm(algorithm_e algorithm,
              uint64_t initial_value) {
  if (algorithm_e::adler32 == algorithm)
    return std::make_unique<adler32_c>();

  else if (algorithm_e::crc8_atm == algorithm)
    return std::make_unique<crc8_atm_c>(initial_value);

  else if (algorithm_e::crc16_ansi == algorithm)
    return std::make_unique<crc16_ansi_c>(initial_value);

  else if (algorithm_e::crc16_ccitt == algorithm)
    return std::make_unique<crc16_ccitt_c>(initial_value);

  else if (algorithm_e::crc32_ieee == algorithm)
    return std::make_unique<crc32_ieee_c>(initial_value);

  else if (algorithm_e::crc32_ieee_le == algorithm)
    return std::make_unique<crc32_ieee_le_c>(initial_value);

  else if (algorithm_e::md5 == algorithm)
    return std::make_unique<md5_c>();

  else
    mxerror(boost::format("Programming error: unknown checksum algorithm %1%\n") % static_cast<int>(algorithm));

  return {};
}

memory_cptr
calculate(algorithm_e algorithm,
          memory_c const &buffer,
          uint64_t initial_value) {
  return calculate(algorithm, buffer.get_buffer(), buffer.get_size(), initial_value);
}

memory_cptr
calculate(algorithm_e algorithm,
          void const *buffer,
          size_t size,
          uint64_t initial_value) {
  auto worker = for_algorithm(algorithm, initial_value);

  worker->add(buffer, size);
  worker->finish();
  return worker->get_result();
}

uint64_t
calculate_as_uint(algorithm_e algorithm,
                  memory_c const &buffer,
                  uint64_t initial_value) {
  return calculate_as_uint(algorithm, buffer.get_buffer(), buffer.get_size(), initial_value);
}

uint64_t
calculate_as_uint(algorithm_e algorithm,
                  void const *buffer,
                  size_t size,
                  uint64_t initial_value) {
  auto worker = for_algorithm(algorithm, initial_value);

  worker->add(buffer, size);
  worker->finish();
  return dynamic_cast<uint_result_c &>(*worker).get_result_as_uint();
}

// ----------------------------------------------------------------------

base_c::base_c() {
}

base_c::~base_c() {
}

base_c &
base_c::add(void const *buffer,
            size_t size) {
  add_impl(static_cast<unsigned char const *>(buffer), size);
  return *this;
}

base_c &
base_c::add(memory_c const &buffer) {
  add_impl(buffer.get_buffer(), buffer.get_size());
  return *this;
}

base_c &
base_c::finish() {
  return *this;
}

// ----------------------------------------------------------------------

uint_result_c::~uint_result_c() {
}

// ----------------------------------------------------------------------

set_initial_value_c::~set_initial_value_c() {
}

void
set_initial_value_c::set_initial_value(uint64_t initial_value) {
  set_initial_value_impl(initial_value);
}

void
set_initial_value_c::set_initial_value(memory_c const &initial_value) {
  set_initial_value_impl(initial_value.get_buffer(), initial_value.get_size());
}

void
set_initial_value_c::set_initial_value(unsigned char const *buffer,
                                       size_t size) {
  set_initial_value_impl(buffer, size);
}

}} // namespace mtx { namespace checksum {
