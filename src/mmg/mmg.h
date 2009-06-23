/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   main stuff - declarations

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __MMG_H
#define __MMG_H

#include <map>
#include <vector>

#include "common/os.h"

#include <wx/app.h>
#include <wx/combobox.h>
#include <wx/config.h>
#include <wx/string.h>

#include <ebml/EbmlUnicodeString.h>

#include "common/iso639.h"
#include "common/smart_pointers.h"
#include "common/wx.h"
#include "mmg/translation_table.h"

#ifdef SYS_WINDOWS
# define ALLFILES Z("All Files (*.*)|*.*")
# define PSEP '\\'
# define STDSPACING 3
# define TOPBOTTOMSPACING 5
# define LEFTRIGHTSPACING 5
#else
# define ALLFILES Z("All Files (*)|*")
# define PSEP '/'
# define STDSPACING 3
# define TOPBOTTOMSPACING 5
# define LEFTRIGHTSPACING 5
#endif

#define TRACK_ID_CHAPTERS    0x10000000
#define TRACK_ID_GLOBAL_TAGS 0x10000001
#define TRACK_ID_TAGS_BASE   0x10000002

// Config file versions and their differences
//
// Version 1: base settings
// Version 2: Added in v1.5.0:
//   Added splitting by timecodes. The old boolean "split_by_size" was
//   replaced by new string "split_mode".
// Version 3: Added after v2.4.2:
//   The per-file boolean "no attachments" was removed and replaced by
//   the handling of individual attached files.

#define MMG_CONFIG_FILE_VERSION_MAX 3

using namespace libebml;

struct mmg_track_t {
  char type;
  int64_t id;
  int source;
  wxString ctype;
  bool enabled, display_dimensions_selected;

  int default_track;
  bool aac_is_sbr, aac_is_sbr_detected, forced_track;
  bool track_name_was_present;
  wxString language, track_name, cues, delay, stretch, sub_charset;
  wxString tags, fourcc, aspect_ratio, compression, timecodes, fps;
  wxString packetizer;
  int nalu_size_length;
  wxString dwidth, dheight;
  int stereo_mode;

  wxString user_defined;

  bool appending;

  // For chapters and tags:
  int num_entries;

  mmg_track_t()
    : type(0)
    , id(0)
    , source(0)
    , enabled(false)
    , display_dimensions_selected(false)
    , default_track(0)
    , aac_is_sbr(false)
    , aac_is_sbr_detected(false)
    , forced_track(false)
    , track_name_was_present(false)
    , language(wxT("und"))
    , cues(wxT("default"))
    , sub_charset(wxT("default"))
    , nalu_size_length(0)
    , stereo_mode(0)
    , appending(false)
    , num_entries(0)
  {
  }

  wxString create_label();
};
typedef counted_ptr<mmg_track_t> mmg_track_cptr;

struct mmg_file_t;

struct mmg_attached_file_t {
  bool enabled;
  wxString name, description, mime_type;
  long id, size;
  mmg_file_t *source;

  mmg_attached_file_t()
    : enabled(true)
    , id(0)
    , size(0)
    , source(NULL)
  {
  }
};
typedef counted_ptr<mmg_attached_file_t> mmg_attached_file_cptr;

struct mmg_file_t {
  wxString file_name, title;
  bool title_was_present;
  int container;
  std::vector<mmg_track_cptr> tracks;
  std::vector<mmg_attached_file_cptr> attached_files;
  bool appending;

  mmg_file_t()
    : title_was_present(false)
    , container(0)
    , appending(false)
  {
  }
};
typedef counted_ptr<mmg_file_t> mmg_file_cptr;

struct mmg_attachment_t {
  wxString file_name, stored_name, description, mime_type;
  int style;

  mmg_attachment_t()
    : style(0)
  {
  }
};
typedef counted_ptr<mmg_attachment_t> mmg_attachment_cptr;

typedef enum {
  ODM_FROM_FIRST_INPUT_FILE = 0,
  ODM_PREVIOUS              = 1,
  ODM_FIXED                 = 2,
} output_directory_mode_e;

struct mmg_options_t {
  wxString mkvmerge;
  wxString output_directory;
  bool autoset_output_filename;
  output_directory_mode_e output_directory_mode;
  bool ask_before_overwriting;
  bool on_top;
  bool filenew_after_add_to_jobqueue;
  bool filenew_after_successful_mux;
  bool warn_usage;
  bool gui_debugging;
  bool set_delay_from_filename;
  wxString priority;
  wxArrayString popular_languages;

  mmg_options_t()
    : autoset_output_filename(false)
    , output_directory_mode(ODM_FROM_FIRST_INPUT_FILE)
    , ask_before_overwriting(false)
    , on_top(false)
    , filenew_after_add_to_jobqueue(false)
    , filenew_after_successful_mux(false)
    , warn_usage(false)
    , gui_debugging(false)
    , set_delay_from_filename(false)
  {
    init_popular_languages();
  }

  void validate();
  void init_popular_languages(const wxString &list = wxEmptyString);
};

extern wxString last_open_dir;
extern std::vector<wxString> last_settings;
extern std::vector<wxString> last_chapters;
extern std::vector<mmg_file_cptr> files;
extern std::vector<mmg_track_t *> tracks;
extern std::vector<mmg_attachment_cptr> attachments;
extern std::vector<mmg_attached_file_cptr> attached_files;
extern wxArrayString sorted_charsets;
extern wxArrayString sorted_iso_codes;
extern bool title_was_present;
extern std::map<wxString, wxString> capabilities;

wxString &break_line(wxString &line, int break_after = 80);
wxString extract_language_code(wxString source);
wxString shell_escape(wxString source, bool cmd_exe_mode = false);
std::vector<wxString> split(const wxString &src, const wxString &pattern, int max_num = -1);
wxString join(const wxString &pattern, std::vector<wxString> &strings);
wxString join(const wxString &pattern, wxArrayString &strings);
wxString &strip(wxString &s, bool newlines = false);
std::vector<wxString> & strip(std::vector<wxString> &v, bool newlines = false);
wxString no_cr(wxString source);
wxString UTFstring_to_wxString(const UTFstring &u);
wxString unescape(const wxString &src);
wxString format_date_time(time_t date_time);
wxString get_temp_dir();
wxString get_installation_dir();

wxString create_track_order(bool all);
wxString create_append_mapping();

int default_track_checked(char type);

void set_combobox_selection(wxComboBox *cb, const wxString wanted);
#if defined(USE_WXBITMAPCOMBOBOX)
void set_combobox_selection(wxBitmapComboBox *cb, const wxString wanted);
#endif  // USE_WXBITMAPCOMBOBOX

void wxdie(const wxString &errmsg);

#if defined(SYS_WINDOWS)
#define TIP(s) format_tooltip(Z(s))
wxString format_tooltip(const wxString &s);
#else
#define TIP(s) Z(s)
#endif

class mmg_app: public wxApp {
public:
  std::string m_ui_locale;
public:
  virtual bool OnInit();
  virtual int OnExit();
  virtual void init_ui_locale();
  virtual void handle_command_line_arguments();
};

extern mmg_app *app;

#endif // __MMG_H
