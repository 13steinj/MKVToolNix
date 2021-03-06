/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

   Option with a priority/source

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

enum option_source_e {
    OPTION_SOURCE_NONE         =  0
  , OPTION_SOURCE_BITSTREAM    = 10
  , OPTION_SOURCE_CONTAINER    = 20
  , OPTION_SOURCE_COMMAND_LINE = 30
};

template<typename T>
class option_with_source_c {
public:
protected:
  option_source_e m_source;
  std::optional<T> m_value;

public:
  option_with_source_c()
    : m_source{OPTION_SOURCE_NONE}
  {
  }

  option_with_source_c(T const &value,
                       option_source_e source)
    : m_source{OPTION_SOURCE_NONE}
  {
    set(value, source);
  }

  operator bool()
    const {
    return !!m_value;
  }

  T const &
  get()
    const {
    if (!*this)
      throw std::logic_error{"not set yet"};
    return m_value.value();
  }

  option_source_e
  get_source()
    const {
    if (!*this)
      throw std::logic_error{"not set yet"};
    return m_source;
  }

  void
  set(T const &value,
      option_source_e source) {
    if (source < m_source)
      return;

    m_value  = value;
    m_source = source;
  }
};
