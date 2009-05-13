/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   XML chapter writer functions

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <expat.h>

#include <string>

#include <matroska/KaxChapters.h>

#include "common/chapters.h"
#include "common/ebml.h"
#include "common/locale.h"
#include "common/mm_io.h"
#include "common/xml_element_writer.h"

using namespace libmatroska;

// {{{ simple chapter output

class chapter_entry_c {
public:
  std::string m_name;
  int64_t m_start, m_level;

  chapter_entry_c(std::string name, int64_t start, int64_t level);

  bool operator < (const chapter_entry_c &cmp) const;
};

chapter_entry_c::chapter_entry_c(std::string name,
                                 int64_t start,
                                 int64_t level)
  : m_name(name)
  , m_start(start)
  , m_level(level)
{
}

bool
chapter_entry_c::operator <(const chapter_entry_c &cmp)
  const {
  return m_start < cmp.m_start;
}

static std::vector<chapter_entry_c> s_chapter_start_times, s_chapter_names;
static std::vector<chapter_entry_c> s_chapter_entries;

static void
handle_name(int level,
            const std::string &name) {
  int i;

  for (i = 0; s_chapter_start_times.size() > i; i++) {
    chapter_entry_c &e = s_chapter_start_times[i];
    if (e.m_level == level) {
      s_chapter_entries.push_back(chapter_entry_c(name, e.m_start, level));
      s_chapter_start_times.erase(s_chapter_start_times.begin() + i);
      return;
    }
  }

  s_chapter_names.push_back(chapter_entry_c(name, 0, level));
}

static void
handle_start_time(int level,
                  int64_t start_time) {
  int i;

  for (i = 0; s_chapter_names.size() > i; i++) {
    chapter_entry_c &e = s_chapter_names[i];
    if (e.m_level == level) {
      s_chapter_entries.push_back(chapter_entry_c(e.m_name, start_time, level));
      s_chapter_names.erase(s_chapter_names.begin() + i);
      return;
    }
  }

  s_chapter_start_times.push_back(chapter_entry_c("", start_time, level));
}

static void write_chapter_atom_simple(KaxChapterAtom *atom, int level);

static void
write_chapter_display_simple(KaxChapterDisplay *display,
                             int level) {
  int i;

  for (i = 0; i < display->ListSize(); i++) {
    EbmlElement *e = (*display)[i];
    if (is_id(e, KaxChapterString))
      handle_name(level - 1, UTFstring_to_cstrutf8(UTFstring(*static_cast<EbmlUnicodeString *>(e)).c_str()));

    else if (is_id(e, KaxChapterAtom))
      write_chapter_atom_simple((KaxChapterAtom *)e, level + 1);

  }
}

static void
write_chapter_track_simple(KaxChapterTrack *track,
                           int level) {
  int i;

  for (i = 0; i < track->ListSize(); i++) {
    EbmlElement *e = (*track)[i];

    if (is_id(e, KaxChapterAtom))
      write_chapter_atom_simple(static_cast<KaxChapterAtom *>(e), level + 1);

  }
}

static void
write_chapter_atom_simple(KaxChapterAtom *atom,
                          int level) {
  int i;

  for (i = 0; i < atom->ListSize(); i++) {
    EbmlElement *e = (*atom)[i];

    if (is_id(e, KaxChapterTimeStart))
      handle_start_time(level, uint64(*static_cast<EbmlUInteger *>(e)) / 1000000);

    else if (is_id(e, KaxChapterTrack))
      write_chapter_track_simple(static_cast<KaxChapterTrack *>(e), level + 1);

    else if (is_id(e, KaxChapterDisplay))
      write_chapter_display_simple(static_cast<KaxChapterDisplay *>(e), level + 1);

    else if (is_id(e, KaxChapterAtom))
      write_chapter_atom_simple(static_cast<KaxChapterAtom *>(e), level + 1);

  }
}

void
write_chapters_simple(int &chapter_num,
                      KaxChapters *chapters,
                      mm_io_c *out) {
  s_chapter_start_times.clear();
  s_chapter_names.clear();
  s_chapter_entries.clear();

  int chapter_idx;
  for (chapter_idx = 0; chapters->ListSize() > chapter_idx; chapter_idx++) {
    if (is_id((*chapters)[chapter_idx], KaxEditionEntry)) {
      KaxEditionEntry *edition = static_cast<KaxEditionEntry *>((*chapters)[chapter_idx]);

      int edition_idx;
      for (edition_idx = 0; edition->ListSize() > edition_idx; edition_idx++)
        if (is_id((*edition)[edition_idx], KaxChapterAtom))
          write_chapter_atom_simple(static_cast<KaxChapterAtom *>((*edition)[edition_idx]), 2);
    }
  }

  for (chapter_idx = 0; s_chapter_entries.size() > chapter_idx; chapter_idx++) {
    int64_t v = s_chapter_entries[chapter_idx].m_start;
    out->puts(g_cc_stdio->native((boost::format("CHAPTER%|1$02d|=%|2$02d|:%|3$02d|:%|4$02d|.%|5$03d|\n")
                                  % chapter_num % (v / 1000 / 60 / 60) % ((v / 1000 / 60) % 60) % ((v / 1000) % 60) % (v % 1000)).str()));
    out->puts(g_cc_stdio->native((boost::format("CHAPTER%|1$02d|NAME=%2%\n") % chapter_num % s_chapter_entries[chapter_idx].m_name).str()));
    chapter_num++;
  }

  s_chapter_start_times.clear();
  s_chapter_names.clear();
  s_chapter_entries.clear();
}

// }}}

// {{{ XML chapter output

static void
pt(xml_writer_cb_t *cb,
   const char *tag) {
  int i;

  for (i = 0; i < cb->level; i++)
    cb->out->puts("  ");
  cb->out->puts(tag);
}

static int
cet_index(const char *name) {
  int i;

  for (i = 0; NULL != chapter_elements[i].name; i++)
    if (!strcmp(name, chapter_elements[i].name))
      return i;

  mxerror(boost::format(Y("cet_index: '%1%' not found\n")) % name);

  return -1;
}

static void
end_write_chapter_atom(void *data) {
  xml_writer_cb_t *cb  = static_cast<xml_writer_cb_t *>(data);
  KaxChapterAtom *atom = dynamic_cast<KaxChapterAtom *>(cb->e);

  assert(NULL != atom);

  if (FINDFIRST(atom, KaxChapterTimeStart) == NULL)
    pt(cb, "<ChapterTimeStart>00:00:00.000</ChapterTimeStart>\n");
}

static void
end_write_chapter_display(void *data) {
  xml_writer_cb_t *cb        = static_cast<xml_writer_cb_t *>(data);
  KaxChapterDisplay *display = dynamic_cast<KaxChapterDisplay *>(cb->e);

  assert(NULL != display);

  if (FINDFIRST(display, KaxChapterString) == NULL)
    pt(cb, "<ChapterString></ChapterString>\n");

  if (FINDFIRST(display, KaxChapterLanguage) == NULL)
    pt(cb, "<ChapterLanguage>eng</ChapterLanguage>\n");
}

void
write_chapters_xml(KaxChapters *chapters,
                   mm_io_c *out) {
  int i;

  for (i = 0; NULL != chapter_elements[i].name; i++) {
    chapter_elements[i].start_hook = NULL;
    chapter_elements[i].end_hook   = NULL;
  }

  chapter_elements[cet_index("ChapterAtom")].end_hook    = end_write_chapter_atom;
  chapter_elements[cet_index("ChapterDisplay")].end_hook = end_write_chapter_display;

  for (i = 0; chapters->ListSize() > i; i++)
    write_xml_element_rec(1, 0, (*chapters)[i], out, chapter_elements);
}

// }}}
