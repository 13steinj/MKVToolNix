/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   regular expression helper functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

namespace mtx::regex {

std::string escape(std::string const &s);

template<typename Tunary_function>
std::string
replace(std::string::const_iterator first,
        std::string::const_iterator last,
        std::regex const &re,
        Tunary_function formatter) {
  std::string s;

  std::smatch::difference_type last_match_pos = 0;
  auto last_match_end = first;

  auto callback = [&](std::smatch const &match) {
    auto this_match_start = last_match_end;
    auto this_match_pos   = match.position(0);
    auto diff             = this_match_pos - last_match_pos;

    std::advance(this_match_start, diff);

    s.append(last_match_end, this_match_start);
    s.append(formatter(match));

    auto match_length = match.length(0);
    last_match_pos    = this_match_pos + match_length;
    last_match_end    = this_match_start;

    std::advance(last_match_end, match_length);
  };

  std::sregex_iterator re_begin(first, last, re), re_end;
  std::for_each(re_begin, re_end, callback);

  s.append(last_match_end, last);

  return s;
}

template<typename Tunary_function>
std::string
replace(std::string const &s,
        std::regex const &re,
        Tunary_function formatter) {
  return replace(s.cbegin(), s.cend(), re, formatter);
}

}