/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   header editor: unsigned integer value page class

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <wx/textctrl.h>
#include <wx/valtext.h>

#include <ebml/EbmlUInteger.h>

#include "common/string_parsing.h"
#include "common/wxcommon.h"
#include "mmg/he_unsigned_integer_value_page.h"

using namespace libebml;

he_unsigned_integer_value_page_c::he_unsigned_integer_value_page_c(header_editor_frame_c *parent,
                                                                   he_page_base_c *toplevel_page,
                                                                   EbmlMaster *master,
                                                                   const EbmlCallbacks &callbacks,
                                                                   const wxString &title,
                                                                   const wxString &description)
  : he_value_page_c(parent, toplevel_page, master, callbacks, vt_unsigned_integer, title, description)
  , m_tc_text(NULL)
  , m_original_value(0)
{
}

he_unsigned_integer_value_page_c::~he_unsigned_integer_value_page_c() {
}

wxControl *
he_unsigned_integer_value_page_c::create_input_control() {
  if (NULL != m_element)
    m_original_value = uint64(*static_cast<EbmlUInteger *>(m_element));

  m_tc_text = new wxTextCtrl(this, wxID_ANY, get_original_value_as_string());
  m_tc_text->SetValidator(wxTextValidator(wxFILTER_NUMERIC));

  return m_tc_text;
}

wxString
he_unsigned_integer_value_page_c::get_original_value_as_string() {
  if (NULL != m_element)
    return wxString::Format(_T("%") wxLongLongFmtSpec _T("u"), m_original_value);

  return wxEmptyString;
}

wxString
he_unsigned_integer_value_page_c::get_current_value_as_string() {
  return m_tc_text->GetValue();
}

void
he_unsigned_integer_value_page_c::reset_value() {
  m_tc_text->SetValue(get_original_value_as_string());
}

bool
he_unsigned_integer_value_page_c::validate_value() {
  uint64_t value;
  return parse_uint(wxMB(m_tc_text->GetValue()), value);
}

void
he_unsigned_integer_value_page_c::copy_value_to_element() {
  uint64_t value;
  parse_uint(wxMB(m_tc_text->GetValue()), value);
  *static_cast<EbmlUInteger *>(m_element) = value;
}
