/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   declarations for ComboBox data

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef MTX_COMMON_EXTERN_DATA_H
#define MTX_COMMON_EXTERN_DATA_H

#include "common/common_pch.h"

struct mime_type_t {
  std::string const name;
  std::vector<std::string> const extensions;

  mime_type_t(std::string const &p_name, std::vector<std::string> const p_extensions)
    : name{p_name}
    , extensions{p_extensions}
  {
  }
};

struct iso3166_1alpha_2_t {
  std::string code, country;
};

extern std::vector<std::string> const sub_charsets, g_popular_character_sets;
extern std::vector<iso3166_1alpha_2_t> const g_cctlds;
extern std::vector<std::string> const g_popular_country_codes;
extern std::vector<mime_type_t> const mime_types;

std::string guess_mime_type(std::string ext, bool is_file);
bool is_valid_cctld(const std::string &s);

#endif // MTX_COMMON_EXTERN_DATA_H
