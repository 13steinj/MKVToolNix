/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#if defined(HAVE_CMARK)

# include <cmark.h>

# include "common/markdown.h"

namespace mtx::markdown {

std::string
to_html(std::string const &markdown_text,
        int options) {
  auto html = cmark_markdown_to_html(markdown_text.c_str(), markdown_text.length(), options);
  if (!html)
    return {};

  auto html_str = std::string{html};
  free(html);

  return html_str;
}

}

#endif  // defined(HAVE_CMARK)
