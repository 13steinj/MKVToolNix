/*
   mkvextract -- extract tracks from Matroska files into other files

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   extracts chapters from Matroska files into other files
   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include <avilib.h>
}

#include <ebml/EbmlHead.h>
#include <ebml/EbmlSubHead.h>
#include <ebml/EbmlStream.h>
#include <ebml/EbmlVoid.h>

#include <matroska/FileKax.h>
#include <matroska/KaxChapters.h>

#include "common/chapters/chapters.h"
#include "common/common.h"
#include "common/ebml.h"
#include "common/mm_io.h"
#include "common/kax_analyzer.h"
#include "extract/mkvextract.h"

using namespace libmatroska;

void
extract_chapters(const std::string &file_name,
                 bool chapter_format_simple,
                 kax_analyzer_c::parse_mode_e parse_mode) {
  kax_analyzer_cptr analyzer;

  // open input file
  try {
    analyzer = kax_analyzer_cptr(new kax_analyzer_c(file_name));
    if (!analyzer->process(parse_mode, MODE_READ))
      throw false;
  } catch (...) {
    show_error(boost::format(Y("The file '%1%' could not be opened for reading (%2%).")) % file_name % strerror(errno));
    return;
  }

  EbmlMaster *master = analyzer->read_all(EBML_INFO(KaxChapters));
  if (NULL == master)
    return;

  KaxChapters *chapters = dynamic_cast<KaxChapters *>(master);
  assert(NULL != chapters);

  if (!chapter_format_simple) {
    g_mm_stdio->write_bom("UTF-8");
    g_mm_stdio->puts("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "\n"
                     "<!-- <!DOCTYPE Tags SYSTEM \"matroskatags.dtd\"> -->\n"
                     "\n"
                     "<Chapters>\n");
    write_chapters_xml(chapters, g_mm_stdio.get_object());
    g_mm_stdio->puts("</Chapters>\n");

  } else {
    int dummy = 1;
    write_chapters_simple(dummy, chapters, g_mm_stdio.get_object());
  }

  delete chapters;
}
