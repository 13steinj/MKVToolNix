/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   ISO 15924 script codes

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/iso15924.h"
#include "common/strings/formatting.h"

namespace mtx::iso15924 {

std::optional<std::size_t>
look_up(std::string const &s) {
  if (s.empty())
    return {};

  auto s_lower = mtx::string::to_lower_ascii(s);
  auto itr     = std::find_if(g_scripts.begin(), g_scripts.end(), [&s_lower](auto const &script) {
    return s_lower == mtx::string::to_lower_ascii(script.code);
  });

  if (itr == g_scripts.end())
    return {};

  return std::distance(g_scripts.begin(), itr);
}

} // namespace mtx::iso15924
