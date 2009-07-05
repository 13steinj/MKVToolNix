/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   XML segment info parser functions

   Written by Steve Lhomme <steve.lhomme@free.fr> and
   Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <expat.h>
#include <ctype.h>
#include <setjmp.h>

#include <string>

#include <matroska/KaxInfo.h>
#include <matroska/KaxInfoData.h>
#include <matroska/KaxSegment.h>

#include "common/common.h"
#include "common/ebml.h"
#include "common/error.h"
#include "common/iso639.h"
#include "common/mm_io.h"
#include "common/segmentinfo.h"
#include "common/xml/element_mapping.h"
#include "common/xml/element_parser.h"

using namespace libmatroska;

#define CPDATA (parser_data_t *)pdata

static void
end_segmentinfo_data(void *) {
/* TODO verify that the FamilyUID is set
  EbmlMaster *m;

  m = static_cast<EbmlMaster *>(xmlp_pelt);
  if (m->FindFirstElt(KaxChapterString::ClassInfos, false) == NULL)
    xmlp_error(CPDATA, "<ChapterDisplay> is missing the <ChapterString> "
               "child.");
  if (m->FindFirstElt(KaxChapterLanguage::ClassInfos, false) == NULL) {
    KaxChapterLanguage *cl;

    cl = new KaxChapterLanguage;
    *static_cast<EbmlString *>(cl) = "und";
    m->PushElement(*cl);
  }
*/
}

static void
end_segmentinfo_family(void *) {
  /* TODO verify that the FamilyUID is 128 bits */
}

static void
end_segmentinfo_links(void *) {
  /* TODO verify that the ChapterLinkCodec and ChapterLinkID are set */
}

KaxInfo *
parse_xml_segmentinfo(mm_text_io_c *in,
                      bool exception_on_error) {
  int i;
  for (i = 0; segmentinfo_elements[i].name != NULL; i++) {
    segmentinfo_elements[i].start_hook = NULL;
    segmentinfo_elements[i].end_hook   = NULL;
  }

  segmentinfo_elements[chapter_element_map_index("Info")].end_hook      = end_segmentinfo_data;
  segmentinfo_elements[chapter_element_map_index("FamilyUID")].end_hook = end_segmentinfo_family;
  segmentinfo_elements[chapter_element_map_index("Links")].end_hook     = end_segmentinfo_links;

  std::string error;
  KaxInfo *info = NULL;
  try {
    EbmlMaster *m = parse_xml_elements("Info", segmentinfo_elements, in);
    info          = dynamic_cast<KaxInfo *>(sort_ebml_master(m));
    assert(NULL != info);

  } catch (error_c e) {
    if (!exception_on_error)
      mxerror(e.get_error());
    error = e.get_error();
    info  = NULL;
  }

  if (!error.empty())
    throw error_c(error);

  fix_mandatory_segmentinfo_elements(info);

  return info;
}
