/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   objects for managing translations

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __TRANSLATION_TABLE_H
#define __TRANSLATION_TABLE_H

#include "os.h"

#include <vector>

#include <wx/string.h>

struct translation_t {
  wxString english, translated;

  translation_t(const wxString &n_english,
                const wxString &n_translated)
    : english(n_english)
    , translated(n_translated)
  { }
};

class translation_table_c {
public:
  std::vector<translation_t> entries;

public:
  translation_table_c();

  void add(const wxString &english, const wxString &translated);
  wxString to_translated(const wxString &english);
  wxString to_english(const wxString &translated);
};

#endif  // __TRANSLATION_TABLE_H
