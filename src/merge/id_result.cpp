/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/translation.h"
#include "merge/id_result.h"
#include "merge/output_control.h"

static void
output_container_unsupported_text(std::string const &filename,
                                  translatable_string_c const &info) {
  if (g_identifying) {
    if (identification_output_format_e::gui == g_identification_output_format)
      mxinfo(boost::format("File '%1%': unsupported container: %2%\n") % filename % info);
    else
      mxinfo(boost::format(Y("File '%1%': unsupported container: %2%\n")) % filename % info);
    mxexit(3);

  } else
    mxerror(boost::format(Y("The file '%1%' is a non-supported file type (%2%).\n")) % filename % info);
}

static void
output_container_unsupported_json(std::string const &filename,
                                  translatable_string_c const &info) {
  auto json = nlohmann::json{
    { "identification_format_version", 2        },
    { "file_name",                     filename },
    { "container", {
        { "recognized", true                  },
        { "supported",  false                 },
        { "type",       info.get_translated() },
      } },
  };

  display_json_output(json);

  mxexit(0);
}

void
id_result_container_unsupported(std::string const &filename,
                                translatable_string_c const &info) {
  if (identification_output_format_e::json == g_identification_output_format)
    output_container_unsupported_json(filename, info);
  else
    output_container_unsupported_text(filename, info);
}
