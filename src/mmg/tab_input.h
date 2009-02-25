/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   declarations for the input tab

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#ifndef __TAB_INPUT_H
#define __TAB_INPUT_H

#include "os.h"

#include "wx/config.h"

#define ID_LB_INPUTFILES                  11000
#define ID_B_ADDFILE                      11001
#define ID_B_REMOVEFILE                   11002
#define ID_CLB_TRACKS                     11003
#define ID_CB_LANGUAGE                    11004
#define ID_TC_TRACKNAME                   11005
#define ID_CB_CUES                        11006
#define ID_CB_MAKEDEFAULT                 11007
#define ID_TC_DELAY                       11008
#define ID_TC_STRETCH                     11009
#define ID_CB_NOCHAPTERS                  11010
#define ID_CB_SUBTITLECHARSET             11011
#define ID_CB_AACISSBR                    11012
#define ID_TC_TAGS                        11013
#define ID_B_BROWSETAGS                   11014
#define ID_CB_ASPECTRATIO                 11015
#define ID_CB_FOURCC                      11016
#define ID_CB_NOATTACHMENTS               11017
#define ID_CB_NOTAGS                      11018
#define ID_CB_COMPRESSION                 11019
#define ID_TC_TIMECODES                   11020
#define ID_B_BROWSE_TIMECODES             11021
#define ID_RB_ASPECTRATIO                 11022
#define ID_RB_DISPLAYDIMENSIONS           11023
#define ID_TC_DISPLAYWIDTH                11024
#define ID_TC_DISPLAYHEIGHT               11025
#define ID_B_TRACKUP                      11028
#define ID_B_TRACKDOWN                    11029
#define ID_T_INPUTVALUES                  11030
#define ID_B_APPENDFILE                   11031
#define ID_CB_STEREO_MODE                 11032
#define ID_CB_FPS                         11033
#define ID_CB_NALU_SIZE_LENGTH            11034
#define ID_TC_USER_DEFINED                11035
#define ID_B_REMOVE_ALL_FILES             11036
#define ID_NB_OPTIONS                     11999

extern const wxChar *predefined_aspect_ratios[];
extern const wxChar *predefined_fourccs[];

class tab_input;

class tab_input_general: public wxPanel {
  DECLARE_CLASS(tab_input_general);
  DECLARE_EVENT_TABLE();
public:
  wxMTX_COMBOBOX_TYPE *cob_default, *cob_language, *cob_cues;
  wxTextCtrl *tc_track_name, *tc_tags, *tc_timecodes;
  wxButton *b_browse_tags, *b_browse_timecodes;

  wxStaticText *st_language, *st_track_name, *st_default;
  wxStaticText *st_cues, *st_tags, *st_timecodes;

  tab_input *input;

  translation_table_c cob_cues_translations;
  translation_table_c cob_default_translations;

public:
  tab_input_general(wxWindow *parent, tab_input *ti);

  void setup_languages();
  void setup_cues();
  void setup_default_track();
  void set_track_mode(mmg_track_t *t);

  void on_default_track_selected(wxCommandEvent &evt);
  void on_language_selected(wxCommandEvent &evt);
  void on_cues_selected(wxCommandEvent &evt);
  void on_browse_tags(wxCommandEvent &evt);
  void on_tags_changed(wxCommandEvent &evt);
  void on_track_name_changed(wxCommandEvent &evt);
  void on_timecodes_changed(wxCommandEvent &evt);
  void on_browse_timecodes_clicked(wxCommandEvent &evt);
};

class tab_input_format: public wxPanel {
  DECLARE_CLASS(tab_input_format);
  DECLARE_EVENT_TABLE();
public:
  wxCheckBox *cb_aac_is_sbr;
  wxMTX_COMBOBOX_TYPE *cob_sub_charset, *cob_aspect_ratio, *cob_fourcc, *cob_compression, *cob_stereo_mode, *cob_fps, *cob_nalu_size_length;
  wxTextCtrl *tc_delay, *tc_stretch;
  wxRadioButton *rb_aspect_ratio, *rb_display_dimensions;
  wxTextCtrl *tc_display_width, *tc_display_height;

  wxStaticText *st_delay, *st_stretch, *st_stereo_mode;
  wxStaticText *st_x, *st_sub_charset, *st_fourcc, *st_compression;
  wxStaticText *st_fps, *st_nalu_size_length;

  tab_input *input;

  translation_table_c cob_sub_charset_translations;
  translation_table_c cob_compression_translations;

public:
  tab_input_format(wxWindow *parent, tab_input *ti);

  void setup_control_contents();
  void set_track_mode(mmg_track_t *t);

  void on_aac_is_sbr_clicked(wxCommandEvent &evt);
  void on_subcharset_selected(wxCommandEvent &evt);
  void on_delay_changed(wxCommandEvent &evt);
  void on_stretch_changed(wxCommandEvent &evt);
  void on_aspect_ratio_selected(wxCommandEvent &evt);
  void on_aspect_ratio_changed(wxCommandEvent &evt);
  void on_display_dimensions_selected(wxCommandEvent &evt);
  void on_display_width_changed(wxCommandEvent &evt);
  void on_display_height_changed(wxCommandEvent &evt);
  void on_fourcc_changed(wxCommandEvent &evt);
  void on_fps_changed(wxCommandEvent &evt);
  void on_nalu_size_length_changed(wxCommandEvent &evt);
  void on_stereo_mode_changed(wxCommandEvent &evt);
  void on_compression_selected(wxCommandEvent &evt);
};

class tab_input_extra: public wxPanel {
  DECLARE_CLASS(tab_input_extra);
  DECLARE_EVENT_TABLE();
public:
  wxTextCtrl *tc_user_defined;

  wxStaticText *st_user_defined;

  tab_input *input;

public:
  tab_input_extra(wxWindow *parent, tab_input *ti);

  void set_track_mode(mmg_track_t *t);

  void on_user_defined_changed(wxCommandEvent &evt);
};

class tab_input: public wxPanel {
  DECLARE_CLASS(tab_input);
  DECLARE_EVENT_TABLE();
public:
  tab_input_general *ti_general;
  tab_input_format *ti_format;
  tab_input_extra *ti_extra;

  wxListBox *lb_input_files;
  wxButton *b_add_file, *b_remove_file, *b_remove_all_files;
  wxButton *b_track_up, *b_track_down, *b_append_file;
  wxCheckBox *cb_no_chapters, *cb_no_tags;
  wxCheckListBox *clb_tracks;
  wxStaticText *st_file_options, *st_tracks;
  wxNotebook *nb_options;

  wxTimer value_copy_timer;
  bool dont_copy_values_now;
  bool avc_es_fps_warning_shown;

  int selected_file, selected_track;

public:
  tab_input(wxWindow *parent);

  void on_add_file(wxCommandEvent &evt);
  void add_file(const wxString &file_name, bool append);
  void select_file(bool append);
  void on_remove_file(wxCommandEvent &evt);
  void on_remove_all_files(wxCommandEvent &evt);
  void on_append_file(wxCommandEvent &evt);
  void on_file_selected(wxCommandEvent &evt);
  void on_move_track_up(wxCommandEvent &evt);
  void on_move_track_down(wxCommandEvent &evt);
  void on_track_selected(wxCommandEvent &evt);
  void on_track_enabled(wxCommandEvent &evt);
  void on_nochapters_clicked(wxCommandEvent &evt);
  void on_notags_clicked(wxCommandEvent &evt);
  void on_value_copy_timer(wxTimerEvent &evt);
  void on_file_new(wxCommandEvent &evt);

  void set_track_mode(mmg_track_t *t);

  void save(wxConfigBase *cfg);
  void load(wxConfigBase *cfg, int version);
  bool validate_settings();
};

#endif // __TAB_INPUT_H
