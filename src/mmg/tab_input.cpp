/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   "input" tab

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include <errno.h>

#include <algorithm>

#include "wx/wxprec.h"

#include "wx/wx.h"
#include "wx/dnd.h"
#include "wx/file.h"
#include "wx/listctrl.h"
#include "wx/notebook.h"
#include "wx/regex.h"
#include "wx/statline.h"

#include "common.h"
#include "extern_data.h"
#include "iso639.h"
#include "mkvmerge.h"
#include "mmg.h"
#include "mmg_dialog.h"
#include "tab_input.h"
#include "tab_global.h"

using namespace std;

wxArrayString sorted_iso_codes;
wxArrayString sorted_charsets;
bool title_was_present = false;

const wxChar *predefined_aspect_ratios[] = {
  wxEmptyString,
  wxT("4/3"),
  wxT("1.66"),
  wxT("16/9"),
  wxT("1.85"),
  wxT("2.00"),
  wxT("2.21"),
  wxT("2.35"),
  NULL
};

const wxChar *predefined_fourccs[] = {
  wxEmptyString,
  wxT("DIVX"),
  wxT("DIV3"),
  wxT("DX50"),
  wxT("MP4V"),
  wxT("XVID"),
  NULL
};

class input_drop_target_c: public wxFileDropTarget {
private:
  tab_input *owner;
public:
  input_drop_target_c(tab_input *n_owner):
    owner(n_owner) {}
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &dropped_files) {
    int i;

    for (i = 0; i < dropped_files.Count(); i++)
      owner->add_file(dropped_files[i], false);

    return true;
  }
};

tab_input::tab_input(wxWindow *parent):
  wxPanel(parent, -1, wxDefaultPosition, wxSize(100, 400),
          wxTAB_TRAVERSAL) {
  wxBoxSizer *siz_line, *siz_column, *siz_all;

  siz_all = new wxBoxSizer(wxVERTICAL);
  siz_all->AddSpacer(TOPBOTTOMSPACING);

  siz_line = new wxBoxSizer(wxHORIZONTAL);
  siz_line->Add(new wxStaticText(this, wxID_STATIC, Z("Input files:")), 0, wxALL, STDSPACING);
  siz_all->Add(siz_line, 0, wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  siz_line = new wxBoxSizer(wxHORIZONTAL);
  lb_input_files = new wxListBox(this, ID_LB_INPUTFILES);
  siz_line->Add(lb_input_files, 1, wxGROW | wxALL, STDSPACING);

  siz_column = new wxBoxSizer(wxVERTICAL);
  b_add_file = new wxButton(this, ID_B_ADDFILE, Z("add"));
  b_remove_file = new wxButton(this, ID_B_REMOVEFILE, Z("remove"));
  b_remove_file->Enable(false);
  b_append_file = new wxButton(this, ID_B_APPENDFILE, Z("append"));
  b_append_file->Enable(false);

  b_remove_all_files = new wxButton(this, ID_B_REMOVE_ALL_FILES, Z("remove all"));
  b_remove_all_files->Enable(false);

  siz_column->Add(b_add_file, 0, wxGROW | wxALL, STDSPACING);
  siz_column->Add(b_remove_file, 0, wxGROW | wxALL, STDSPACING);
  siz_line->Add(siz_column);

  siz_column = new wxBoxSizer(wxVERTICAL);
  siz_column->Add(b_append_file, 0, wxGROW | wxALL, STDSPACING);
  siz_column->Add(b_remove_all_files, 0, wxGROW | wxALL, STDSPACING);
  siz_line->Add(siz_column);

  siz_all->Add(siz_line, 0, wxGROW | wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  siz_line = new wxBoxSizer(wxHORIZONTAL);
  st_file_options = new wxStaticText(this, wxID_STATIC, Z("File options:"));
  st_file_options->Enable(false);
  siz_line->Add(st_file_options, 0, wxALIGN_CENTER_VERTICAL | wxALL, STDSPACING);

  cb_no_chapters = new wxCheckBox(this, ID_CB_NOCHAPTERS, Z("No chapters"));
  cb_no_chapters->SetValue(false);
  cb_no_chapters->SetToolTip(TIP("Do not copy chapters from this file. Only applies to Matroska files."));
  cb_no_chapters->Enable(false);
  siz_line->Add(cb_no_chapters, 0, wxALIGN_CENTER_VERTICAL | wxALL, STDSPACING);

  cb_no_attachments = new wxCheckBox(this, ID_CB_NOATTACHMENTS, Z("No attachments"));
  cb_no_attachments->SetValue(false);
  cb_no_attachments->SetToolTip(TIP("Do not copy attachments from this file. Only applies to Matroska files."));
  cb_no_attachments->Enable(false);
  siz_line->Add(cb_no_attachments, 0, wxALIGN_CENTER_VERTICAL | wxALL, STDSPACING);

  cb_no_tags = new wxCheckBox(this, ID_CB_NOTAGS, Z("No tags"));
  cb_no_tags->SetValue(false);
  cb_no_tags->SetToolTip(TIP("Do not copy tags from this file. Only applies to Matroska files."));
  cb_no_tags->Enable(false);
  siz_line->Add(cb_no_tags, 0, wxALIGN_CENTER_VERTICAL | wxALL, STDSPACING);
  siz_all->Add(siz_line, 0, wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  siz_line = new wxBoxSizer(wxHORIZONTAL);
  st_tracks = new wxStaticText(this, wxID_STATIC, Z("Tracks:"));
  st_tracks->Enable(false);
  siz_line->Add(st_tracks, 0, wxALL, STDSPACING);
  siz_all->Add(siz_line, 0, wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  siz_line = new wxBoxSizer(wxHORIZONTAL);
  siz_column = new wxBoxSizer(wxVERTICAL);
  clb_tracks = new wxCheckListBox(this, ID_CLB_TRACKS);
  clb_tracks->Enable(false);
  siz_line->Add(clb_tracks, 1, wxGROW | wxALIGN_TOP | wxALL, STDSPACING);
  b_track_up = new wxButton(this, ID_B_TRACKUP, Z("up"));
  b_track_up->Enable(false);
  siz_column->Add(b_track_up, 0, wxGROW | wxALL, STDSPACING);
  b_track_down = new wxButton(this, ID_B_TRACKDOWN, Z("down"));
  b_track_down->Enable(false);
  siz_column->Add(b_track_down, 0, wxGROW | wxALL, STDSPACING);
  siz_line->Add(siz_column);
  siz_all->Add(siz_line, 3, wxGROW | wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  nb_options = new wxNotebook(this, ID_NB_OPTIONS, wxDefaultPosition, wxDefaultSize, wxNB_TOP);

  ti_general = new tab_input_general(nb_options, this);
  ti_format  = new tab_input_format(nb_options, this);
  ti_extra   = new tab_input_extra(nb_options, this);

  nb_options->AddPage(ti_general, Z("General track options"));
  nb_options->AddPage(ti_format,  Z("Format specific options"));
  nb_options->AddPage(ti_extra,   Z("Extra options"));

  siz_all->Add(nb_options, 0, wxGROW | wxLEFT | wxRIGHT, LEFTRIGHTSPACING);

  siz_all->AddSpacer(TOPBOTTOMSPACING);

  SetSizer(siz_all);

  set_track_mode(NULL);
  selected_file  = -1;
  selected_track = -1;

  dont_copy_values_now = false;
  value_copy_timer.SetOwner(this, ID_T_INPUTVALUES);
  value_copy_timer.Start(333);

  SetDropTarget(new input_drop_target_c(this));

  wxConfig *cfg = (wxConfig *)wxConfigBase::Get();

  cfg->SetPath(wxT("/GUI"));
  cfg->Read(wxT("avc_es_fps_warning_shown"), &avc_es_fps_warning_shown, false);
}

void
tab_input::set_track_mode(mmg_track_t *t) {
  ti_general->set_track_mode(t);
  ti_format->set_track_mode(t);
  ti_extra->set_track_mode(t);
}

struct file_type_t {
  wxString title, extensions;

  file_type_t(const wxString &n_title, const wxString &n_extensions)
    : title(n_title)
    , extensions(n_extensions) {
  }
};

void
tab_input::select_file(bool append) {
  static vector<file_type_t> file_types;

  if (file_types.empty()) {
    file_types.push_back(file_type_t(Z("A/52 (aka AC3)"),                      wxU("ac3")));
    file_types.push_back(file_type_t(Z("AAC (Advanced Audio Coding)"),         wxU("aac m4a mp4")));
    file_types.push_back(file_type_t(Z("AVC/h.264 elementary streams"),        wxU("264 avc h264 x264")));
    file_types.push_back(file_type_t(Z("AVI (Audio/Video Interleaved)"),       wxU("avi")));
    file_types.push_back(file_type_t(Z("Dirac"),                               wxU("drc")));
    file_types.push_back(file_type_t(Z("DTS (Digital Theater System)"),        wxU("dts")));
    if (capabilities[wxT("FLAC")] == wxT("true"))
      file_types.push_back(file_type_t(Z("FLAC (Free Lossless Audio Codec)"),  wxU("flac ogg")));
    file_types.push_back(file_type_t(Z("MP4 audio/video files"),               wxU("mp4")));
    file_types.push_back(file_type_t(Z("MPEG audio files"),                    wxU("mp2 mp3")));
    file_types.push_back(file_type_t(Z("MPEG program streams"),                wxU("mpg mpeg m2v evo evob vob")));
    file_types.push_back(file_type_t(Z("MPEG video elementary streams"),       wxU("m1v m2v")));
    file_types.push_back(file_type_t(Z("Matroska audio/video files"),          wxU("mka mks mkv")));
    file_types.push_back(file_type_t(Z("QuickTime audio/video files"),         wxU("mov")));
    file_types.push_back(file_type_t(Z("Ogg/OGM audio/video files"),           wxU("ogg ogm")));
    file_types.push_back(file_type_t(Z("RealMedia audio/video files"),         wxU("ra ram rm rmvb rv")));
    file_types.push_back(file_type_t(Z("SRT text subtitles"),                  wxU("srt")));
    file_types.push_back(file_type_t(Z("SSA/ASS text subtitles"),              wxU("ass ssa")));
    file_types.push_back(file_type_t(Z("TTA (The lossless True Audio codec)"), wxU("tta")));
    file_types.push_back(file_type_t(Z("USF text subtitles"),                  wxU("usf xml")));
    file_types.push_back(file_type_t(Z("VC1 elementary streams"),              wxU("vc1")));
    file_types.push_back(file_type_t(Z("VobSub subtitles"),                    wxU("idx")));
    file_types.push_back(file_type_t(Z("WAVE (uncompressed PCM audio)"),       wxU("wav")));
    file_types.push_back(file_type_t(Z("WAVPACK v4 audio"),                    wxU("wv")));
    file_types.push_back(file_type_t(Z("VobButtons"),                          wxU("btn")));
  }

  wxString media_files, rest, a_exts;
  vector<wxString> all_extensions;
  int ft, ae;

  for (ft = 0; file_types.size() > ft; ++ft) {
    wxString s_exts;
    int e;

    vector<wxString> extensions = split(wxString(file_types[ft].extensions), wxU(" "));
    for (e = 0; e < extensions.size(); e++) {

      if (s_exts.Length() > 0)
        s_exts += wxT(";");
      s_exts += wxString::Format(wxT("*.%s"), extensions[e].c_str());

      bool found = false;
      for (ae = 0; ae < all_extensions.size(); ae++)
        if (all_extensions[ae] == extensions[e]) {
          found = true;
          break;
        }
      if (!found)
        all_extensions.push_back(extensions[e]);
    }

    rest += wxString::Format(wxT("|%s (%s)|%s"), file_types[ft].title.c_str(), s_exts.c_str(), s_exts.c_str());
  }

  sort(all_extensions.begin(), all_extensions.end());

  for (ae = 0; ae < all_extensions.size(); ae++) {
    if (a_exts.Length() > 0)
      a_exts += wxT(";");
    a_exts += wxString::Format(wxT("*.%s"), all_extensions[ae].c_str());
  }

  media_files.Printf(Z("All supported media files|%s%s|%s"), a_exts.c_str(), rest.c_str(), ALLFILES.c_str());

  wxFileDialog dlg(NULL, append ? Z("Choose an input file to append") : Z("Choose an input file to add"), last_open_dir, wxEmptyString, media_files, wxOPEN | wxMULTIPLE);
  if(dlg.ShowModal() != wxID_OK)
    return;

  wxArrayString selected_files;
  int i;

  dlg.GetPaths(selected_files);
  for (i = 0; i < selected_files.Count(); i++)
    add_file(selected_files[i], append);
}

void
tab_input::on_add_file(wxCommandEvent &evt) {
  select_file(false);
}

void
tab_input::on_append_file(wxCommandEvent &evt) {
  select_file(true);
}

void
tab_input::add_file(const wxString &file_name,
                    bool append) {
  mmg_file_t file;
  wxString name, command, id, type, exact, video_track_name, opt_file_name;
  wxArrayString output, errors;
  vector<wxString> args, pair;
  int result, pos, new_file_pos;
  unsigned int i, k;
  wxFile *opt_file;
  string arg_utf8;
  bool default_video_track_found, default_audio_track_found;
  bool default_subtitle_track_found;

  wxFileName file_name_obj(file_name);
  last_open_dir = file_name_obj.GetPath();

  opt_file_name.Printf(wxT("%smmg-mkvmerge-options-%d-%d"), get_temp_dir().c_str(), (int)wxGetProcessId(), (int)time(NULL));
  try {
    const unsigned char utf8_bom[3] = {0xef, 0xbb, 0xbf};
    opt_file = new wxFile(opt_file_name, wxFile::write);
    opt_file->Write(utf8_bom, 3);
  } catch (...) {
    wxString error;
    error.Printf(Z("Could not create a temporary file for mkvmerge's command line option called '%s' (error code %d, %s)."), opt_file_name.c_str(), errno, wxUCS(strerror(errno)));
    wxMessageBox(error, Z("File creation failed"), wxOK | wxCENTER | wxICON_ERROR);
    return;
  }

  opt_file->Write(wxT("--output-charset\nUTF-8\n--identify-for-mmg\n"));
  arg_utf8 = to_utf8(file_name);
  opt_file->Write(arg_utf8.c_str(), arg_utf8.length());
  opt_file->Write(wxT("\n"));
  delete opt_file;

  command = wxT("\"") + mdlg->options.mkvmerge + wxT("\" \"@") + opt_file_name + wxT("\"");

  wxLogMessage(wxT("identify 1: command: ``%s''"), command.c_str());

  result = wxExecute(command, output, errors);

  wxLogMessage(wxT("identify 1: result: %d"), result);
  for (i = 0; i < output.Count(); i++)
    wxLogMessage(wxT("identify 1: output[%d]: ``%s''"), i, output[i].c_str());
  for (i = 0; i < errors.Count(); i++)
    wxLogMessage(wxT("identify 1: errors[%d]: ``%s''"), i, errors[i].c_str());

  wxRemoveFile(opt_file_name);

  if (3 == result) {
    wxString container = Z("unknown");

    if (output.Count() && (0 <= (pos = output[0].Find(wxT("container:")))))
      container = output[0].Mid(pos + 11);

    wxString info;
    info.Printf(Z("The file is an unsupported container format (%s)."), container.c_str());
    break_line(info, 60);

    wxMessageBox(info, Z("Unsupported format"), wxOK | wxCENTER | wxICON_ERROR);
    return;

  } else if ((0 > result) || (1 < result)) {
    name.Printf(Z("File identification failed for '%s'. Return code: %d\n\n"), file_name.c_str(), result);
    for (i = 0; i < output.Count(); i++)
      name += break_line(output[i]) + wxT("\n");
    name += wxT("\n");
    for (i = 0; i < errors.Count(); i++)
      name += break_line(errors[i]) + wxT("\n");
    wxMessageBox(name, Z("File identification failed"), wxOK | wxCENTER | wxICON_ERROR);
    return;

  } else if (0 < result) {
    name.Printf(Z("File identification failed. Return code: %d. Errno: %d (%s). Make sure that you've selected a mkvmerge executable in the settings dialog."),
                result, errno, wxUCS(strerror(errno)));
    break_line(name, 60);
    wxMessageBox(name, Z("File identification failed"), wxOK | wxCENTER | wxICON_ERROR);
    return;
  }

  wxString delay_from_file_name;
  if (mdlg->options.set_delay_from_filename) {
    wxRegEx re_delay(wxT("delay[[:blank:]]+(-?[[:digit:]]+)"), wxRE_ICASE);
    if (re_delay.Matches(file_name))
      delay_from_file_name = re_delay.GetMatch(file_name, 1);
  }

  default_audio_track_found    = -1 != default_track_checked('a');
  default_video_track_found    = -1 != default_track_checked('v');
  default_subtitle_track_found = -1 != default_track_checked('s');
  for (i = 0; i < output.Count(); i++) {
    if (output[i].Find(wxT("Track")) == 0) {
      wxString info;
      mmg_track_ptr track(new mmg_track_t);

      id = output[i].AfterFirst(wxT(' ')).AfterFirst(wxT(' ')).
        BeforeFirst(wxT(':'));
      type = output[i].AfterFirst(wxT(':')).BeforeFirst(wxT('(')).Mid(1).
        RemoveLast();
      exact = output[i].AfterFirst(wxT('(')).BeforeFirst(wxT(')'));
      info = output[i].AfterFirst(wxT('[')).BeforeLast(wxT(']'));
      if (type == wxT("audio"))
        track->type = 'a';
      else if (type == wxT("video"))
        track->type = 'v';
      else if (type == wxT("subtitles"))
        track->type = 's';
      else
        track->type = '?';
      parse_int(wxMB(id), track->id);
      track->ctype = exact;
      track->enabled = true;

      if ('a' == track->type)
        track->delay = delay_from_file_name;

      if (info.length() > 0) {
        args = split(info, wxU(" "));
        for (k = 0; k < args.size(); k++) {
          pair = split(args[k], wxU(":"), 2);
          if (pair.size() != 2)
            continue;

          if (pair[0] == wxT("track_name")) {
            track->track_name             = from_utf8(unescape(pair[1]));
            track->track_name_was_present = true;

          } else if (pair[0] == wxT("language"))
            track->language = unescape(pair[1]);

          else if (pair[0] == wxT("display_dimensions")) {
            int64_t width, height;

            vector<wxString> dims = split(pair[1], wxU("x"));
            if ((dims.size() == 2) && parse_int(wxMB(dims[0]), width) && parse_int(wxMB(dims[1]), height)) {
              string format;
              fix_format(LLD, format);
              track->dwidth.Printf(wxU(format.c_str()), width);
              track->dheight.Printf(wxU(format.c_str()), height);
              track->display_dimensions_selected = true;
            }

          } else if ((pair[0] == wxT("default_track")) && (pair[1] == wxT("1"))) {
            if (('a' == track->type) && !default_audio_track_found) {
              track->default_track      = true;
              default_audio_track_found = true;

            } else if (('v' == track->type) && !default_video_track_found) {
              track->default_track      = true;
              default_video_track_found = true;

            } else if (('s' == track->type) && !default_subtitle_track_found) {
              track->default_track         = true;
              default_subtitle_track_found = true;
            }

          } else if (pair[0] == wxT("stereo_mode")) {
            parse_int(wxMB(pair[1]), track->stereo_mode);
            track->stereo_mode += 1;

          } else if (pair[0] == wxT("aac_is_sbr"))
            track->aac_is_sbr = track->aac_is_sbr_detected = pair[1] == wxT("true");

          else if (pair[0] == wxT("packetizer"))
            track->packetizer = pair[1];

        }
      }

      if (('v' == track->type) && (track->track_name.Length() > 0))
        video_track_name = track->track_name;

      track->appending = append;

      file.tracks.push_back(track);

      if (   (FILE_TYPE_AVC_ES == file.container)
          && ('v' == track->type)
          && (track->ctype.Find(wxT("MPEG-4 part 10 ES")) >= 0)
          && (!avc_es_fps_warning_shown || mdlg->options.warn_usage)) {
        wxMessageBox(Z("You're adding an AVC/h.264 elementary stream to the output file. "
                       "mkvmerge cannot determine the number of frames per second for such files itself. "
                       "Therefore you have to set this parameter yourself on the 'format specific options' page.\n\n"
                       "If you don't do this then mkvmerge will assume 25 fps.\n\n"
                       "This message will only be shown once unless you have enabled mmg's warnings on its 'settings' page."));
        avc_es_fps_warning_shown = true;

        wxConfig *cfg = (wxConfig *)wxConfigBase::Get();
        cfg->SetPath(wxT("/GUI"));
        cfg->Write(wxT("avc_es_fps_warning_shown"), true);
        cfg->Flush();
      }

    } else if ((pos = output[i].Find(wxT("container:"))) > 0) {
      wxString container = output[i].Mid(pos + 11).BeforeFirst(wxT(' '));
      wxString info      = output[i].Mid(pos + 11).AfterFirst(wxT('[')).BeforeLast(wxT(']'));

      if (container == wxT("AAC"))
        file.container = FILE_TYPE_AAC;
      else if (container == wxT("AC3"))
        file.container = FILE_TYPE_AC3;
      else if (container == wxT("AVC/h.264"))
        file.container = FILE_TYPE_AVC_ES;
      else if (container == wxT("AVI"))
        file.container = FILE_TYPE_AVI;
      else if (container == wxT("Dirac elementary stream"))
        file.container = FILE_TYPE_DIRAC;
      else if (container == wxT("DTS"))
        file.container = FILE_TYPE_DTS;
      else if (container == wxT("Matroska"))
        file.container = FILE_TYPE_MATROSKA;
      else if (container == wxT("MP2/MP3"))
        file.container = FILE_TYPE_MP3;
      else if (container == wxT("Ogg/OGM"))
        file.container = FILE_TYPE_OGM;
      else if (container == wxT("Quicktime/MP4"))
        file.container = FILE_TYPE_QTMP4;
      else if (container == wxT("RealMedia"))
        file.container = FILE_TYPE_REAL;
      else if (container == wxT("SRT"))
        file.container = FILE_TYPE_SRT;
      else if (container == wxT("SSA/ASS"))
        file.container = FILE_TYPE_SSA;
      else if (container == wxT("VC1 elementary stream"))
        file.container = FILE_TYPE_VC1;
      else if (container == wxT("VobSub"))
        file.container = FILE_TYPE_VOBSUB;
      else if (container == wxT("WAV"))
        file.container = FILE_TYPE_WAV;
      else
        file.container = FILE_TYPE_IS_UNKNOWN;

      if (info.length() > 0) {
        args = split(info, wxU(" "));
        for (k = 0; k < args.size(); k++) {
          pair = split(args[k], wxU(":"), 2);
          if ((pair.size() == 2) && (pair[0] == wxT("title"))) {
            file.title = from_utf8(unescape(pair[1]));
            file.title_was_present = true;
            title_was_present = true;
          }
        }
      }
    }
  }

  if (file.tracks.empty()) {
    wxMessageBox(wxString::Format(Z("The input file '%s' does not contain any tracks."), file_name.c_str()), Z("No tracks found"));
    return;
  }

  // Look for a place to insert the new file. If the file is only "added",
  // then it will be added to the back of the list. If it is "appended",
  // then it should be inserted right after the currently selected file.
  if (!append)
    new_file_pos = lb_input_files->GetCount();
  else {
    new_file_pos = lb_input_files->GetSelection();
    if (wxNOT_FOUND == new_file_pos)
      new_file_pos = lb_input_files->GetCount();
    else {
      do {
        ++new_file_pos;
      } while ((files.size() > new_file_pos) && files[new_file_pos].appending);
    }
  }

  name.Printf(wxT("%s%s (%s)"), append ? wxT("++> ") : wxEmptyString, file_name.AfterLast(wxT(PSEP)).c_str(), file_name.BeforeLast(wxT(PSEP)).c_str());
  lb_input_files->Insert(name, new_file_pos);

  file.file_name = file_name;
  mdlg->set_title_maybe(file.title);
  if ((file.container == FILE_TYPE_OGM) && (video_track_name.Length() > 0))
    mdlg->set_title_maybe(video_track_name);
  file.appending = append;
  files.insert(files.begin() + new_file_pos, file);

  // After inserting the file the "source" index is wrong for all files
  // after the insertion position.
  for (i = 0; i < tracks.size(); i++)
    if (tracks[i]->source >= new_file_pos)
      ++tracks[i]->source;

  if (name.StartsWith(wxT("++> ")))
    name.Remove(0, 4);

  for (i = 0; i < file.tracks.size(); i++) {
    string format;
    wxString label;
    int new_track_pos;

    mmg_track_ptr &t = file.tracks[i];
    t->enabled = true;
    t->source  = new_file_pos;

    fix_format(Y("%s%s (ID %lld, type: %s) from %s"), format);
    wxString type = t->type == 'a' ? Z("audio")
                  : t->type == 'v' ? Z("video")
                  : t->type == 's' ? Z("subtitles")
                  :                  Z("unknown");

    label.Printf(wxU(format.c_str()), t->appending ? wxT("++> ") : wxEmptyString, t->ctype.c_str(), t->id, type.c_str(), name.c_str());

    // Look for a place to insert this new track. If the file is "added" then
    // the new track is simply appended to the list of existing tracks.
    // If the file is "appended" then it should be put to the end of the
    // chain of tracks being appended. The n'th track from this new file
    // should be appended to the n'th track of the "old" file this new file is
    // appended to. So first I have to find the n'th track of the "old" file.
    // Then I have to skip over all the other tracks that are already appended
    // to that n'th track. The insertion point is right after that.
    if (append) {
      int nth_old_track = 0;
      new_track_pos     = 0;
      while ((tracks.size() > new_track_pos) && (nth_old_track < (i + 1))) {
        if (tracks[new_track_pos]->source == (new_file_pos - 1))
          ++nth_old_track;
        ++new_track_pos;
      }

      // Either we've found the n'th track from the "old" file or we're at
      // the end of the list. In the latter case we just append the track
      // at the end and let the user figure out which track he really wants
      // to append it to.
      if (nth_old_track == (i + 1))
        while ((tracks.size() > new_track_pos) && tracks[new_track_pos]->appending)
          ++new_track_pos;

    } else
      new_track_pos = tracks.size();

    tracks.insert(tracks.begin() + new_track_pos, t.get());
    clb_tracks->Insert(label, new_track_pos);
    clb_tracks->Check(new_track_pos, true);
  }

  mdlg->set_output_maybe(file.file_name);

  st_tracks->Enable(true);
  clb_tracks->Enable(true);
  b_append_file->Enable(true);
  b_remove_all_files->Enable(true);
}

void
tab_input::on_remove_file(wxCommandEvent &evt) {
  mmg_track_t *t;
  vector<mmg_file_t>::iterator eit;

  if (-1 == selected_file)
    return;

  mmg_file_t *f = &files[selected_file];

  // Files may not be removed if something else is still being appended to
  // them.
  unsigned int i;
  for (i = 0; i < (tracks.size() - 1); i++)
    if ((tracks[i]->source == selected_file) && tracks[i + 1]->appending && (tracks[i]->source != tracks[i + 1]->source)) {
      wxString err;

      err.Printf(Z("The current file (number %d) cannot be removed. "
                   "There are other files -- at least file number %d -- whose tracks are supposed to be appended to tracks from this file. "
                   "Please remove those files first."),
                 selected_file + 1, tracks[i + 1]->source + 1);
      wxMessageBox(err, Z("File removal not possible"), wxOK | wxICON_ERROR | wxCENTER);
      return;
    }

  dont_copy_values_now = true;

  i = 0;
  while (i < tracks.size()) {
    t = tracks[i];
    if (t->source == selected_file) {
      clb_tracks->Delete(i);
      tracks.erase(tracks.begin() + i);
    } else {
      if (t->source > selected_file)
        t->source--;
      i++;
    }
  }

  // Unset the "file/segment title" box if the segment title came from this
  // source file and the user hasn't changed it since.
  if ((f->title != wxEmptyString) && f->title_was_present && (f->title == mdlg->global_page->tc_title->GetValue()))
    mdlg->global_page->tc_title->SetValue(wxEmptyString);

  files.erase(files.begin() + selected_file);
  lb_input_files->Delete(selected_file);
  selected_file = lb_input_files->GetSelection();
  st_file_options->Enable(-1 != selected_file);
  cb_no_chapters->Enable(-1 != selected_file);
  cb_no_attachments->Enable(-1 != selected_file);
  cb_no_tags->Enable(-1 != selected_file);
  b_remove_file->Enable(-1 != selected_file);
  b_remove_all_files->Enable(-1 != selected_file);
  b_append_file->Enable(tracks.size() > 0);
  b_track_up->Enable(-1 != selected_file);
  b_track_down->Enable(-1 != selected_file);

  selected_track = clb_tracks->GetSelection();
  set_track_mode(-1 == selected_track ? NULL : tracks[selected_track]);
  if (tracks.empty()) {
    st_tracks->Enable(false);
    clb_tracks->Enable(false);
    mdlg->remove_output_filename();
  }

  dont_copy_values_now = false;
}

void
tab_input::on_remove_all_files(wxCommandEvent &evt) {
  dont_copy_values_now = true;

  lb_input_files->Clear();
  clb_tracks->Clear();

  tracks.clear();
  files.clear();

  selected_file  = -1;
  selected_track = -1;
  st_tracks->Enable(false);
  clb_tracks->Enable(false);
  st_file_options->Enable(false);
  cb_no_chapters->Enable(false);
  cb_no_attachments->Enable(false);
  cb_no_tags->Enable(false);
  b_remove_file->Enable(false);
  b_remove_all_files->Enable(false);
  b_append_file->Enable(false);
  b_track_up->Enable(false);
  b_track_down->Enable(false);

  set_track_mode(NULL);

  mdlg->remove_output_filename();

  dont_copy_values_now = false;
}

void
tab_input::on_move_track_up(wxCommandEvent &evt) {
  if (1 > selected_track)
    return;

  // Appended tracks may not be at the top.
  if ((1 == selected_track) && tracks[selected_track]->appending)
    return;

  dont_copy_values_now      = true;

  int current_track         = selected_track;

  mmg_track_t *t            = tracks[current_track - 1];
  tracks[current_track - 1] = tracks[current_track];
  tracks[current_track]     = t;
  wxString s                = clb_tracks->GetString(current_track - 1);

  clb_tracks->SetString(current_track - 1, clb_tracks->GetString(current_track));
  clb_tracks->SetString(current_track, s);
  clb_tracks->SetSelection(current_track - 1);
  clb_tracks->Check(current_track - 1, tracks[current_track - 1]->enabled);
  clb_tracks->Check(current_track, tracks[current_track]->enabled);
  current_track--;

  b_track_up->Enable(current_track > 0);
  b_track_down->Enable(true);

  selected_track       = current_track;
  dont_copy_values_now = false;
}

void
tab_input::on_move_track_down(wxCommandEvent &evt) {
  if ((0 > selected_track) || (selected_track >= tracks.size() - 1))
    return;

  // Appended tracks may not be at the top.
  if ((0 == selected_track) && (tracks.size() > 1) && tracks[1]->appending)
    return;

  dont_copy_values_now      = true;

  int current_track         = selected_track;

  mmg_track_t *t            = tracks[current_track + 1];
  tracks[current_track + 1] = tracks[current_track];
  tracks[current_track]     = t;
  wxString s                = clb_tracks->GetString(current_track + 1);

  clb_tracks->SetString(current_track + 1, clb_tracks->GetString(current_track));
  clb_tracks->SetString(current_track, s);
  clb_tracks->SetSelection(current_track + 1);
  clb_tracks->Check(current_track + 1, tracks[current_track + 1]->enabled);
  clb_tracks->Check(current_track, tracks[current_track]->enabled);
  current_track++;

  b_track_up->Enable(true);
  b_track_down->Enable(current_track < (tracks.size() - 1));

  selected_track       = current_track;
  dont_copy_values_now = false;
}

void
tab_input::on_file_selected(wxCommandEvent &evt) {
  selected_file = -1;
  int new_sel   = lb_input_files->GetSelection();
  if (0 > new_sel)
    return;

  b_remove_file->Enable(true);
  b_append_file->Enable(true);

  int old_track  = selected_track;
  selected_track = -1;
  mmg_file_t *f  = &files[new_sel];

  if (FILE_TYPE_MATROSKA == f->container) {
    st_file_options->Enable(true);
    cb_no_chapters->Enable(true);
    cb_no_attachments->Enable(true);
    cb_no_tags->Enable(true);
    cb_no_chapters->SetValue(f->no_chapters);
    cb_no_attachments->SetValue(f->no_attachments);
    cb_no_tags->SetValue(f->no_tags);

  } else {
    st_file_options->Enable(false);
    cb_no_chapters->Enable(false);
    cb_no_attachments->Enable(false);
    cb_no_tags->Enable(false);
    cb_no_chapters->SetValue(false);
    cb_no_attachments->SetValue(false);
    cb_no_tags->SetValue(false);
  }

  selected_file  = new_sel;
  selected_track = old_track;
}

void
tab_input::on_nochapters_clicked(wxCommandEvent &evt) {
  if (dont_copy_values_now || (-1 != selected_file))
    files[selected_file].no_chapters = cb_no_chapters->GetValue();
}

void
tab_input::on_noattachments_clicked(wxCommandEvent &evt) {
  if (dont_copy_values_now || (-1 != selected_file))
    files[selected_file].no_attachments = cb_no_attachments->GetValue();
}

void
tab_input::on_notags_clicked(wxCommandEvent &evt) {
  if (dont_copy_values_now || (-1 != selected_file))
    files[selected_file].no_tags = cb_no_tags->GetValue();
}

void
tab_input::on_track_selected(wxCommandEvent &evt) {
  dont_copy_values_now = true;

  selected_track       = -1;
  int new_sel          = clb_tracks->GetSelection();

  if (0 > new_sel)
    return;

  mmg_track_t *t = tracks[new_sel];

  b_track_up->Enable(new_sel > 0);
  b_track_down->Enable(new_sel < (tracks.size() - 1));

  set_track_mode(t);

  wxString lang = t->language;
  unsigned int i;
  for (i = 0; i < sorted_iso_codes.Count(); i++)
    if (extract_language_code(sorted_iso_codes[i]) == lang) {
      lang = sorted_iso_codes[i];
      break;
    }

  ti_general->cob_language->SetValue(lang);
  ti_general->tc_track_name->SetValue(t->track_name);
  ti_general->cob_cues->SetValue(t->cues);
  ti_format->tc_delay->SetValue(t->delay);
  ti_format->tc_stretch->SetValue(t->stretch);
  ti_format->cob_sub_charset->SetValue(t->sub_charset);
  ti_general->tc_tags->SetValue(t->tags);
  ti_general->cob_default->SetSelection(t->default_track);
  ti_format->cb_aac_is_sbr->SetValue(t->aac_is_sbr);
  set_combobox_selection(ti_format->cob_aspect_ratio, t->aspect_ratio);
  ti_format->tc_display_width->SetValue(t->dwidth);
  ti_format->tc_display_height->SetValue(t->dheight);
  selected_track = new_sel;
  ti_format->cob_compression->SetValue(t->compression);
  ti_general->tc_timecodes->SetValue(t->timecodes);
  set_combobox_selection(ti_format->cob_fourcc, t->fourcc);
  set_combobox_selection(ti_format->cob_fps, t->fps);
  ti_format->cob_nalu_size_length->SetSelection(t->nalu_size_length / 2);
  ti_general->tc_track_name->SetFocus();
  ti_format->cob_stereo_mode->SetSelection(t->stereo_mode);
  ti_extra->tc_user_defined->SetValue(t->user_defined);

  dont_copy_values_now = false;
}

void
tab_input::on_track_enabled(wxCommandEvent &evt) {
  unsigned int i;
  for (i = 0; i < tracks.size(); i++)
    tracks[i]->enabled = clb_tracks->IsChecked(i);
}

void
tab_input::on_value_copy_timer(wxTimerEvent &evt) {
  if (dont_copy_values_now || (selected_track == -1))
    return;

  mmg_track_t *t  = tracks[selected_track];
  t->aspect_ratio = ti_format->cob_aspect_ratio->GetValue();
  t->fourcc       = ti_format->cob_fourcc->GetValue();
  t->fps         = ti_format->cob_fps->GetValue();
}

void
tab_input::on_file_new(wxCommandEvent &evt) {
  b_append_file->Enable(false);
  b_remove_file->Enable(false);
  st_file_options->Enable(false);
  cb_no_chapters->Enable(false);
  cb_no_attachments->Enable(false);
  cb_no_tags->Enable(false);
  cb_no_chapters->SetValue(false);
  cb_no_attachments->SetValue(false);
  cb_no_tags->SetValue(false);
  b_track_up->Enable(false);
  b_track_down->Enable(false);
}

void
tab_input::save(wxConfigBase *cfg) {
  cfg->SetPath(wxT("/input"));
  cfg->Write(wxT("number_of_files"), (int)files.size());

  unsigned int fidx;
  for (fidx = 0; fidx < files.size(); fidx++) {
    mmg_file_t *f = &files[fidx];
    wxString s;
    s.Printf(wxT("file %u"), fidx);
    cfg->SetPath(s);
    cfg->Write(wxT("file_name"),        f->file_name);
    cfg->Write(wxT("container"),        f->container);
    cfg->Write(wxT("no_chapters"),      f->no_chapters);
    cfg->Write(wxT("no_attachments"),   f->no_attachments);
    cfg->Write(wxT("no_tags"),          f->no_tags);
    cfg->Write(wxT("appending"),        f->appending);

    cfg->Write(wxT("number_of_tracks"), (int)f->tracks.size());
    unsigned int tidx;
    for (tidx = 0; tidx < f->tracks.size(); tidx++) {
      string format;

      mmg_track_ptr &t = f->tracks[tidx];
      s.Printf(wxT("track %u"), tidx);
      cfg->SetPath(s);

      s.Printf(wxT("%c"),                            t->type);
      cfg->Write(wxT("type"),                        s);
      fix_format("" LLD,                             format);
      s.Printf(wxU(format.c_str()),                  t->id);
      cfg->Write(wxT("id"),                          s);
      cfg->Write(wxT("enabled"),                     t->enabled);
      cfg->Write(wxT("content_type"),                t->ctype);
      cfg->Write(wxT("default_track_2"),             t->default_track);
      cfg->Write(wxT("aac_is_sbr"),                  t->aac_is_sbr);
      cfg->Write(wxT("language"),                    t->language);
      cfg->Write(wxT("track_name"),                  t->track_name);
      cfg->Write(wxT("cues"),                        t->cues);
      cfg->Write(wxT("delay"),                       t->delay);
      cfg->Write(wxT("stretch"),                     t->stretch);
      cfg->Write(wxT("sub_charset"),                 t->sub_charset);
      cfg->Write(wxT("tags"),                        t->tags);
      cfg->Write(wxT("timecodes"),                   t->timecodes);
      cfg->Write(wxT("display_dimensions_selected"), t->display_dimensions_selected);
      cfg->Write(wxT("aspect_ratio"),                t->aspect_ratio);
      cfg->Write(wxT("display_width"),               t->dwidth);
      cfg->Write(wxT("display_height"),              t->dheight);
      cfg->Write(wxT("fourcc"),                      t->fourcc);
      cfg->Write(wxT("fps"),                         t->fps);
      cfg->Write(wxT("nalu_size_length"),            t->nalu_size_length);
      cfg->Write(wxT("stereo_mode"),                 t->stereo_mode);
      cfg->Write(wxT("compression"),                 t->compression);
      cfg->Write(wxT("track_name_was_present"),      t->track_name_was_present);
      cfg->Write(wxT("appending"),                   t->appending);
      cfg->Write(wxT("user_defined"),                t->user_defined);
      cfg->Write(wxT("packetizer"),                  t->packetizer);

      cfg->SetPath(wxT(".."));
    }

    cfg->SetPath(wxT(".."));
  }
  cfg->Write(wxT("track_order"), create_track_order(true));
}

void
tab_input::load(wxConfigBase *cfg,
                int version) {
  dont_copy_values_now = true;

  clb_tracks->Clear();
  lb_input_files->Clear();
  set_track_mode(NULL);
  selected_file  = -1;
  selected_track = -1;
  b_remove_file->Enable(false);
  b_remove_all_files->Enable(false);
  b_append_file->Enable(false);

  files.clear();
  tracks.clear();

  cfg->SetPath(wxT("/input"));
  int num_files;
  if (!cfg->Read(wxT("number_of_files"), &num_files) || (0 > num_files)) {
    dont_copy_values_now = false;
    return;
  }

  string format;
  fix_format("%u:%lld", format);

  wxString c, track_order;
  long fidx;
  for (fidx = 0; fidx < (long)num_files; fidx++) {
    mmg_file_t fi;

    wxString s;
    s.Printf(wxT("file %u"), fidx);
    cfg->SetPath(s);

    int num_tracks;
    if (!cfg->Read(wxT("number_of_tracks"), &num_tracks) || (0 > num_tracks)) {
      cfg->SetPath(wxT(".."));
      continue;
    }

    if (!cfg->Read(wxT("file_name"), &fi.file_name)) {
      cfg->SetPath(wxT(".."));
      continue;
    }
    cfg->Read(wxT("title"),          &fi.title);
    cfg->Read(wxT("container"),      &fi.container);
    cfg->Read(wxT("no_chapters"),    &fi.no_chapters,    false);
    cfg->Read(wxT("no_attachments"), &fi.no_attachments, false);
    cfg->Read(wxT("no_tags"),        &fi.no_tags,        false);
    cfg->Read(wxT("appending"),      &fi.appending,      false);

    long tidx;
    for (tidx = 0; tidx < (long)num_tracks; tidx++) {
      mmg_track_ptr tr(new mmg_track_t);
      bool dummy = false;

      s.Printf(wxT("track %ld"), tidx);

      cfg->SetPath(s);
      wxString id;
      if (!cfg->Read(wxT("type"), &c) || (c.Length() != 1) || !cfg->Read(wxT("id"), &id)) {
        cfg->SetPath(wxT(".."));
        continue;
      }

      tr->type = c.c_str()[0];
      if (((tr->type != 'a') && (tr->type != 'v') && (tr->type != 's')) || !parse_int(wxMB(id), tr->id)) {
        cfg->SetPath(wxT(".."));
        continue;
      }

      cfg->Read(wxT("enabled"),                     &tr->enabled);
      cfg->Read(wxT("content_type"),                &tr->ctype);
      if (cfg->Read(wxT("default_track"),           &dummy)) {
        tr->default_track = dummy ? 1 : 0;
      } else
        cfg->Read(wxT("default_track_2"),           &tr->default_track,               0);
      cfg->Read(wxT("aac_is_sbr"),                  &tr->aac_is_sbr,                  false);
      cfg->Read(wxT("language"),                    &tr->language);
      cfg->Read(wxT("track_name"),                  &tr->track_name);
      cfg->Read(wxT("cues"),                        &tr->cues);
      cfg->Read(wxT("delay"),                       &tr->delay);
      cfg->Read(wxT("stretch"),                     &tr->stretch);
      cfg->Read(wxT("sub_charset"),                 &tr->sub_charset);
      cfg->Read(wxT("tags"),                        &tr->tags);
      cfg->Read(wxT("display_dimensions_selected"), &tr->display_dimensions_selected, false);
      cfg->Read(wxT("aspect_ratio"),                &tr->aspect_ratio);
      cfg->Read(wxT("display_width"),               &tr->dwidth);
      cfg->Read(wxT("display_height"),              &tr->dheight);
      cfg->Read(wxT("fourcc"),                      &tr->fourcc);
      cfg->Read(wxT("fps"),                         &tr->fps);
      cfg->Read(wxT("nalu_size_length"),            &tr->nalu_size_length,            4);
      cfg->Read(wxT("stereo_mode"),                 &tr->stereo_mode,                 0);
      cfg->Read(wxT("compression"),                 &tr->compression);
      cfg->Read(wxT("timecodes"),                   &tr->timecodes);
      cfg->Read(wxT("track_name_was_present"),      &tr->track_name_was_present,      false);
      cfg->Read(wxT("appending"),                   &tr->appending,                   false);
      cfg->Read(wxT("user_defined"),                &tr->user_defined);
      cfg->Read(wxT("packetizer"),                  &tr->packetizer);

      tr->source = files.size();
      if (track_order.Length() > 0)
        track_order += wxT(",");
      track_order += wxString::Format(wxUCS(format.c_str()), files.size(), tr->id);

      fi.tracks.push_back(tr);
      cfg->SetPath(wxT(".."));
    }

    if (!fi.tracks.empty()) {
      s = fi.file_name.BeforeLast(PSEP);
      c = fi.file_name.AfterLast(PSEP);
      lb_input_files->Append(wxString::Format(wxT("%s%s (%s)"), fi.appending ? wxT("++> ") : wxEmptyString, c.c_str(), s.c_str()));
      files.push_back(fi);
    }

    cfg->SetPath(wxT(".."));
  }

  wxString s = wxEmptyString;
  if (!cfg->Read(wxT("track_order"), &s) || (s.length() == 0))
    s = track_order;
  strip(s);
  if (s.length() > 0) {
    vector<wxString> entries = split(s, (wxString)wxT(","));
    int i;
    for (i = 0; i < entries.size(); i++) {
      vector<wxString> pair = split(entries[i], (wxString)wxT(":"));
      if (pair.size() != 2)
        wxdie(Z("The job file could not have been parsed correctly.\n"
                "Either it is invalid / damaged, or you've just found\n"
                "a bug in mmg. Please report this to the author\n"
                "Moritz Bunkus <moritz@bunkus.org>\n\n"
                "(Problem occured in tab_input::load(), #1)"));
      long tidx;
      if (!pair[0].ToLong(&fidx) || !pair[1].ToLong(&tidx) || (fidx >= files.size()))
        wxdie(Z("The job file could not have been parsed correctly.\n"
                "Either it is invalid / damaged, or you've just found\n"
                "a bug in mmg. Please report this to the author\n"
                "Moritz Bunkus <moritz@bunkus.org>\n\n"
                "(Problem occured in tab_input::load(), #2)"));
      bool found = false;
      int j;
      for (j = 0; j < files[fidx].tracks.size(); j++)
        if (files[fidx].tracks[j]->id == tidx) {
          found = true;
          tidx = j;
        }
      if (!found)
        wxdie(Z("The job file could not have been parsed correctly.\n"
                "Either it is invalid / damaged, or you've just found\n"
                "a bug in mmg. Please report this to the author\n"
                "Moritz Bunkus <moritz@bunkus.org>\n\n"
                "(Problem occured in tab_input::load(), #3)"));
      mmg_track_ptr &t = files[fidx].tracks[tidx];
      tracks.push_back(t.get());

      fix_format(Y("%s%s (ID %lld, type: %s) from %s"), format);
      wxString name  = files[fidx].file_name.AfterLast(wxT(PSEP));
      name          += wxT(" (") + files[fidx].file_name.BeforeLast(wxT(PSEP)) + wxT(")");
      wxString type  = t->type == 'a' ? Z("audio")
                     : t->type == 'v' ? Z("video")
                     : t->type == 's' ? Z("subtitles")
                     :                  Z("unknown");
      wxString label;
      label.Printf(wxU(format.c_str()), t->appending ? wxT("++> ") : wxEmptyString, t->ctype.c_str(), t->id, type.c_str(), name.c_str());
      clb_tracks->Append(label);
      clb_tracks->Check(i, t->enabled);
    }
  }
  st_tracks->Enable(!tracks.empty());
  clb_tracks->Enable(!tracks.empty());
  b_append_file->Enable(!files.empty());
  b_remove_all_files->Enable(!files.empty());

  dont_copy_values_now = false;
}

bool
tab_input::validate_settings() {
  // Appending a file to itself is not allowed.
  unsigned int tidx;
  for (tidx = 2; tidx < tracks.size(); tidx++)
    if (tracks[tidx - 1]->appending && tracks[tidx]->appending &&
        (tracks[tidx - 1]->source == tracks[tidx]->source)) {
      wxString err;

      err.Printf(Z("Appending a track from a file to another track from the same file is not allowed. "
                   "This is the case for tracks number %u and %u."),
                 tidx, tidx + 1);
      wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);

      return false;
    }

  bool tracks_selected = false;
  unsigned int fidx;
  for (fidx = 0; fidx < files.size(); fidx++) {
    mmg_file_t *f = &files[fidx];

    for (tidx = 0; tidx < f->tracks.size(); tidx++) {
      mmg_track_ptr &t = f->tracks[tidx];
      if (!t->enabled)
        continue;

      tracks_selected = true;
      string format;
      fix_format("%lld", format);
      wxString sid;
      sid.Printf(wxU(format.c_str()), t->id);

      string s = wxMB(t->delay);
      strip(s);
      int dummy_i;
      if ((s.length() > 0) && !parse_int(s, dummy_i)) {
        wxString err;
        err.Printf(Z("The delay setting for track nr. %s in file '%s' is invalid."), sid.c_str(), f->file_name.c_str());
        wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
        return false;
      }

      wxString str = t->stretch;
      strip(str);
      if (str.length() > 0) {
        wxRegEx re_stretch(wxT("^[[:digit:]]+([,\\.][[:digit:]]+)?(/[[:digit:]]+([,\\.][[:digit:]]+)?)?$"), wxRE_ICASE);
        if (!re_stretch.Matches(str)) {
          wxString err;
          err.Printf(Z("The stretch setting for track nr. %s in file '%s' is invalid."), sid.c_str(), f->file_name.c_str());
          wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
          return false;
        }
      }

      s = wxMB(t->fourcc);
      strip(s);
      if ((s.length() > 0) && (s.length() != 4)) {
        wxString err;
        err.Printf(Z("The FourCC setting for track nr. %s in file '%s' is not excatly four characters long."), sid.c_str(), f->file_name.c_str());
        wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
        return false;
      }

      str = t->fps;
      strip(str);
      if (str.length() > 0) {
        wxRegEx re_fps1(wxT("^[[:digit:]]+\\.?[[:digit:]]*$"));
        wxRegEx re_fps2(wxT("^[[:digit:]]+/[[:digit:]]+$"));
        if (!re_fps1.Matches(str) && !re_fps2.Matches(str)) {
          wxString err;
          err.Printf(Z("The FPS setting for track nr. %s in file '%s' is invalid."), sid.c_str(), f->file_name.c_str());
          wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
          return false;
        }
      } else if (   (FILE_TYPE_AVC_ES == f->container)
                 && ('v' == t->type)
                 && (t->ctype.Find(wxT("MPEG-4 part 10 ES")) >= 0)
                 && mdlg->options.warn_usage) {
        wxString message;
        message.Printf(Z("You haven't selected a number of frames per second for track %lld of file '%s'. "
                         "mkvmerge cannot determine the number of frames per second for such files itself. "
                         "Therefore you have to set this parameter yourself on the 'format specific options' page.\n\n"
                         "If you don't do this then mkvmerge will assume 25 fps.\n\n"
                         "Do you still want to continue?"),
                       t->id, f->file_name.c_str());
        if (wxMessageBox(message, Z("No FPS selected for AVC/h.264 track"), wxYES | wxNO, mdlg) == wxNO)
          return false;
      }

      s = wxMB(t->aspect_ratio);
      strip(s);
      if (s.length() > 0) {
        bool dot_present = false;
        int i            = 0;
        bool ok          = true;
        while (i < s.length()) {
          if (isdigit(s[i]) || (!dot_present && ((s[i] == '.') || (s[i] == ',')))) {
            if ((s[i] == '.') || (s[i] == ','))
              dot_present = true;
            i++;
          } else {
            ok = false;
            break;
          }
        }

        if (!ok) {
          dot_present = false;
          i           = 0;
          ok          = true;
          while (i < s.length()) {
            if (isdigit(s[i]) ||
                (!dot_present && (s[i] == '/'))) {
              if (s[i] == '/')
                dot_present = true;
              i++;
            } else {
              ok = false;
              break;
            }
          }
        }

        if (!ok) {
          wxString err;
          err.Printf(Z("The aspect ratio setting for track nr. %s in file '%s' is invalid."), sid.c_str(), f->file_name.c_str());
          wxMessageBox(err, Z("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
          return false;
        }
      }
    }
  }

  if (!tracks_selected) {
    wxMessageBox(Z("You have not yet selected any input file and/or no tracks."), wxT("mkvmerge GUI: error"), wxOK | wxCENTER | wxICON_ERROR);
    return false;
  }

  return true;
}

IMPLEMENT_CLASS(tab_input, wxPanel);
BEGIN_EVENT_TABLE(tab_input, wxPanel)
  EVT_BUTTON(ID_B_ADDFILE,          tab_input::on_add_file)
  EVT_BUTTON(ID_B_REMOVEFILE,       tab_input::on_remove_file)
  EVT_BUTTON(ID_B_REMOVE_ALL_FILES, tab_input::on_remove_all_files)
  EVT_BUTTON(ID_B_APPENDFILE,       tab_input::on_append_file)
  EVT_BUTTON(ID_B_TRACKUP,          tab_input::on_move_track_up)
  EVT_BUTTON(ID_B_TRACKDOWN,        tab_input::on_move_track_down)

  EVT_LISTBOX(ID_LB_INPUTFILES,     tab_input::on_file_selected)
  EVT_LISTBOX(ID_CLB_TRACKS,        tab_input::on_track_selected)
  EVT_CHECKLISTBOX(ID_CLB_TRACKS,   tab_input::on_track_enabled)

  EVT_CHECKBOX(ID_CB_NOCHAPTERS,    tab_input::on_nochapters_clicked)
  EVT_CHECKBOX(ID_CB_NOATTACHMENTS, tab_input::on_noattachments_clicked)
  EVT_CHECKBOX(ID_CB_NOTAGS,        tab_input::on_notags_clicked)

  EVT_TIMER(ID_T_INPUTVALUES,       tab_input::on_value_copy_timer)
END_EVENT_TABLE();
