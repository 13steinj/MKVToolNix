/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   main dialog

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/os.h"

#include <wx/wx.h>
#include <wx/clipbrd.h>
#include <wx/config.h>
#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/fileconf.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/statusbr.h>
#include <wx/statline.h>
#include <wx/strconv.h>

#include "common/chapters/chapters.h"
#include "common/command_line.h"
#include "common/common.h"
#include "common/ebml.h"
#include "common/extern_data.h"
#include "common/locale.h"
#include "common/strings/editing.h"
#include "common/strings/formatting.h"
#include "common/translation.h"
#include "common/version.h"
#include "common/wx.h"
#include "common/xml/element_mapping.h"
#include "merge/mkvmerge.h"
#include "mmg/cli_options_dlg.h"
#include "mmg/header_editor/frame.h"
#include "mmg/jobs.h"
#include "mmg/matroskalogo.xpm"
#include "mmg/mmg_dialog.h"
#include "mmg/mmg.h"
#include "mmg/mux_dialog.h"
#include "mmg/options/dialog.h"
#include "mmg/show_text_dlg.h"
#include "mmg/tabs/attachments.h"
#include "mmg/tabs/chapters.h"
#include "mmg/tabs/global.h"
#include "mmg/tabs/input.h"

mmg_dialog *mdlg;
wxString last_open_dir;
std::vector<wxString> last_settings;
std::vector<wxString> last_chapters;
std::vector<mmg_file_cptr> files;
std::vector<mmg_track_t *> tracks;
std::map<wxString, wxString> capabilities;
std::vector<job_t> jobs;

mmg_dialog::mmg_dialog()
  : wxFrame(NULL, wxID_ANY, wxEmptyString)
#if defined(SYS_WINDOWS)
  , m_taskbar_msg_received(false)
#endif
{
  wxBoxSizer *bs_main;
  wxPanel *panel;

  mdlg = this;

  load_preferences();

#if defined(__WXGTK__)
  // GTK seems to call bindtextdomain() after our call to it.
  // So lets re-initialize the UI locale in case the user has
  // selected a language in mmg's preferences that doesn't match
  // his environment variable's language.
  app->init_ui_locale();
#endif

  SetTitle(wxU(get_version_info("mkvmerge GUI")));

  log_window = new wxLogWindow(this, Z("mmg debug output"), false);
  wxLog::SetActiveTarget(log_window);

  file_menu = new wxMenu();
  file_menu->Append(ID_M_FILE_NEW, Z("&New\tCtrl-N"), Z("Start with empty settings"));
  file_menu->Append(ID_M_FILE_LOAD, Z("&Load settings\tCtrl-L"), Z("Load muxing settings from a file"));
  file_menu->Append(ID_M_FILE_SAVE, Z("&Save settings\tCtrl-S"), Z("Save muxing settings to a file"));
  file_menu->AppendSeparator();
  file_menu->Append(ID_M_FILE_SETOUTPUT, Z("Set &output file"), Z("Select the file you want to write to"));
  file_menu->AppendSeparator();
  file_menu->Append(ID_M_FILE_OPTIONS, Z("Op&tions\tCtrl-P"), Z("Change mmg's preferences and options"));
  file_menu->AppendSeparator();
  file_menu->Append(ID_M_FILE_HEADEREDITOR, Z("&Header editor\tCtrl-E"), Z("Run the header field editor"));
  file_menu->AppendSeparator();
  file_menu->Append(ID_M_FILE_EXIT, Z("&Quit\tCtrl-Q"), Z("Quit the application"));

  file_menu_sep = false;
  update_file_menu();

  wxMenu *muxing_menu = new wxMenu();
  muxing_menu->Append(ID_M_MUXING_START, Z("Sta&rt muxing (run mkvmerge)\tCtrl-R"), Z("Run mkvmerge and start the muxing process"));
  muxing_menu->AppendSeparator();
  muxing_menu->Append(ID_M_MUXING_SHOW_CMDLINE, Z("S&how the command line"), Z("Show the command line mmg creates for mkvmerge"));
  muxing_menu->Append(ID_M_MUXING_COPY_CMDLINE, Z("&Copy command line to clipboard"), Z("Copy the command line to the clipboard"));
  muxing_menu->Append(ID_M_MUXING_SAVE_CMDLINE, Z("Sa&ve command line"), Z("Save the command line to a file"));
  muxing_menu->Append(ID_M_MUXING_CREATE_OPTIONFILE, Z("Create &option file"), Z("Save the command line to an option file that can be read by mkvmerge"));
  muxing_menu->AppendSeparator();
  muxing_menu->Append(ID_M_MUXING_ADD_TO_JOBQUEUE, Z("&Add to job queue"), Z("Adds the current settings as a new job entry to the job queue"));
  muxing_menu->Append(ID_M_MUXING_MANAGE_JOBS, Z("&Manage jobs\tCtrl-J"), Z("Brings up the job queue editor"));
  muxing_menu->AppendSeparator();
  muxing_menu->Append(ID_M_MUXING_ADD_CLI_OPTIONS, Z("Add &command line options"), Z("Lets you add arbitrary options to the command line"));

  chapter_menu = new wxMenu();
  chapter_menu->Append(ID_M_CHAPTERS_NEW, Z("&New chapters"), Z("Create a new chapter file"));
  chapter_menu->Append(ID_M_CHAPTERS_LOAD, Z("&Load"), Z("Load a chapter file (simple/OGM format or XML "
                           "format)"));
  chapter_menu->Append(ID_M_CHAPTERS_SAVE, Z("&Save"), Z("Save the current chapters to a XML file"));
  chapter_menu->Append(ID_M_CHAPTERS_SAVETOKAX, Z("Save to &Matroska file"), Z("Save the current chapters to an existing Matroska file"));
  chapter_menu->Append(ID_M_CHAPTERS_SAVEAS, Z("Save &as"), Z("Save the current chapters to a file with another name"));
  chapter_menu->AppendSeparator();
  chapter_menu->Append(ID_M_CHAPTERS_VERIFY, Z("&Verify"), Z("Verify the current chapter entries to see if there are any errors"));
  chapter_menu_sep = false;
  update_chapter_menu();

  wxMenu *window_menu = new wxMenu();
  window_menu->Append(ID_M_WINDOW_INPUT, Z("&Input\tAlt-1"));
  window_menu->Append(ID_M_WINDOW_ATTACHMENTS, Z("&Attachments\tAlt-2"));
  window_menu->Append(ID_M_WINDOW_GLOBAL, Z("&Global options\tAlt-3"));
  window_menu->AppendSeparator();
  window_menu->Append(ID_M_WINDOW_CHAPTEREDITOR, Z("&Chapter editor\tAlt-4"));

  wxMenu *help_menu = new wxMenu();
  help_menu->Append(ID_M_HELP_HELP, Z("&Help\tF1"), Z("Show the guide to mkvmerge GUI"));
  help_menu->Append(ID_M_HELP_ABOUT, Z("&About"), Z("Show program information"));

  wxMenuBar *menu_bar = new wxMenuBar();
  menu_bar->Append(file_menu, Z("&File"));
  menu_bar->Append(muxing_menu, Z("&Muxing"));
  menu_bar->Append(chapter_menu, Z("&Chapter Editor"));
  menu_bar->Append(window_menu, Z("&Window"));
  menu_bar->Append(help_menu, Z("&Help"));
  SetMenuBar(menu_bar);

  status_bar = new wxStatusBar(this, -1);
  SetStatusBar(status_bar);
  status_bar_timer.SetOwner(this, ID_T_STATUSBAR);

  panel = new wxPanel(this, -1);

  bs_main = new wxBoxSizer(wxVERTICAL);
  panel->SetSizer(bs_main);
  panel->SetAutoLayout(true);

  notebook =
    new wxNotebook(panel, ID_NOTEBOOK, wxDefaultPosition, wxSize(500, 500),
                   wxNB_TOP);
  input_page = new tab_input(notebook);
  attachments_page = new tab_attachments(notebook);
  global_page = new tab_global(notebook);
  chapter_editor_page = new tab_chapters(notebook, chapter_menu);

  notebook->AddPage(input_page, Z("Input"));
  notebook->AddPage(attachments_page, Z("Attachments"));
  notebook->AddPage(global_page, Z("Global"));
  notebook->AddPage(chapter_editor_page, Z("Chapter Editor"));

  bs_main->Add(notebook, 1, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxGROW, 5);

  wxStaticBox *sb_low = new wxStaticBox(panel, -1, Z("Output filename"));
  wxStaticBoxSizer *sbs_low = new wxStaticBoxSizer(sb_low, wxHORIZONTAL);
  bs_main->Add(sbs_low, 0, wxALIGN_CENTER_HORIZONTAL | wxLEFT | wxRIGHT | wxGROW, 5);

  tc_output = new wxTextCtrl(panel, ID_TC_OUTPUT, wxEmptyString);
  sbs_low->AddSpacer(5);
  sbs_low->Add(tc_output, 1, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxGROW, 2);
  sbs_low->AddSpacer(5);

  b_browse_output = new wxButton(panel, ID_B_BROWSEOUTPUT, Z("Browse"));
  sbs_low->Add(b_browse_output, 0, wxALIGN_CENTER_VERTICAL | wxALL, 3);

  wxBoxSizer *bs_buttons = new wxBoxSizer(wxHORIZONTAL);

  b_start_muxing = new wxButton(panel, ID_B_STARTMUXING, Z("Sta&rt muxing"));
  bs_buttons->Add(b_start_muxing, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 8);

  b_copy_to_clipboard = new wxButton(panel, ID_B_COPYTOCLIPBOARD, Z("&Copy to clipboard"));
  bs_buttons->Add(b_copy_to_clipboard, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 8);

  b_add_to_jobqueue = new wxButton(panel, ID_B_ADD_TO_JOBQUEUE, Z("&Add to job queue"));
  bs_buttons->Add(b_add_to_jobqueue, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 8);

  bs_main->Add(bs_buttons, 0, wxALIGN_CENTER_HORIZONTAL);

#ifdef SYS_WINDOWS
  SetSizeHints(700, 680);
  SetSize(700, 680);
#else
  SetSizeHints(700, 660);
  SetSize(700, 660);
#endif

  log_window->Show(options.gui_debugging);
  set_on_top(options.on_top);
  query_mkvmerge_capabilities();

  muxing_in_progress = false;
  last_open_dir      = wxEmptyString;
  cmdline            = wxString::Format(wxU("\"%s\" -o \"%s\""), options.mkvmerge.c_str(), tc_output->GetValue().c_str());

  load_job_queue();

  SetIcon(wxIcon(matroskalogo_xpm));

  help = NULL;

  set_status_bar(Z("mkvmerge GUI ready"));

#if defined(SYS_WINDOWS)
  RegisterWindowMessages();
#endif
}

mmg_dialog::~mmg_dialog() {
  delete help;
}

void
mmg_dialog::on_browse_output(wxCommandEvent &evt) {
  wxFileDialog dlg(NULL, Z("Choose an output file"), last_open_dir,
                   tc_output->GetValue().AfterLast(PSEP),
                   wxString::Format(Z("Matroska A/V files (*.mka;*.mkv)|*.mkv;*.mka|%s"), ALLFILES.c_str()),
                   wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if(dlg.ShowModal() != wxID_OK)
    return;

  last_open_dir = dlg.GetDirectory();
  tc_output->SetValue(dlg.GetPath());
  remember_previous_output_directory();
}

void
mmg_dialog::set_status_bar(wxString text) {
  status_bar_timer.Stop();
  status_bar->SetStatusText(text);
  status_bar_timer.Start(4000, true);
}

void
mmg_dialog::on_clear_status_bar(wxTimerEvent &evt) {
  status_bar->SetStatusText(wxEmptyString);
}

void
mmg_dialog::on_quit(wxCommandEvent &evt) {
  Close(true);
}

void
mmg_dialog::on_file_new(wxCommandEvent &evt) {
  wxString tmp_name;

  tmp_name.Printf(wxT("%stempsettings-%d.mmg"), get_temp_dir().c_str(), (int)wxGetProcessId());
  wxFileConfig cfg(wxT("mkvmerge GUI"), wxT("Moritz Bunkus"), tmp_name);
  tc_output->SetValue(wxEmptyString);

  input_page->load(&cfg, MMG_CONFIG_FILE_VERSION_MAX);
  input_page->on_file_new(evt);
  attachments_page->load(&cfg, MMG_CONFIG_FILE_VERSION_MAX);
  global_page->load(&cfg, MMG_CONFIG_FILE_VERSION_MAX);
  notebook->SetSelection(0);

  wxRemoveFile(tmp_name);

  set_status_bar(Z("Configuration cleared."));
}

void
mmg_dialog::on_file_load(wxCommandEvent &evt) {
  wxFileDialog dlg(NULL, Z("Choose an input file"), last_open_dir, wxEmptyString, wxString::Format(Z("mkvmerge GUI settings (*.mmg)|*.mmg|%s"), ALLFILES.c_str()), wxFD_OPEN);
  if(dlg.ShowModal() != wxID_OK)
    return;

  if (!wxFileExists(dlg.GetPath()) || wxDirExists(dlg.GetPath())) {
    wxMessageBox(Z("The file does not exist."), Z("Error loading settings"), wxOK | wxCENTER | wxICON_ERROR);
    return;
  }

  last_open_dir = dlg.GetDirectory();
  load(dlg.GetPath());
}

void
mmg_dialog::load(wxString file_name,
                 bool used_for_jobs) {
  wxString s;
  int version;

  wxFileConfig cfg(wxT("mkvmerge GUI"), wxT("Moritz Bunkus"), file_name);
  cfg.SetPath(wxT("/mkvmergeGUI"));
  if (!cfg.Read(wxT("file_version"), &version) || (1 > version) || (MMG_CONFIG_FILE_VERSION_MAX < version)) {
    if (used_for_jobs)
      return;
    wxMessageBox(Z("The file does not seem to be a valid mkvmerge GUI settings file."), Z("Error loading settings"), wxOK | wxCENTER | wxICON_ERROR);
    return;
  }

  cfg.Read(wxT("output_file_name"), &s);
  tc_output->SetValue(s);
  cfg.Read(wxT("cli_options"), &cli_options, wxEmptyString);

  input_page->load(&cfg, version);
  attachments_page->load(&cfg, version);
  global_page->load(&cfg, version);

  if (!used_for_jobs) {
    set_last_settings_in_menu(file_name);
    set_status_bar(Z("Configuration loaded."));
  }
}

void
mmg_dialog::on_file_save(wxCommandEvent &evt) {
  wxFileDialog dlg(NULL, Z("Choose an output file"), last_open_dir, wxEmptyString, wxString::Format(Z("mkvmerge GUI settings (*.mmg)|*.mmg|%s"), ALLFILES.c_str()), wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if(dlg.ShowModal() != wxID_OK)
    return;

  last_open_dir = dlg.GetDirectory();
  if (wxFileExists(dlg.GetPath()))
    wxRemoveFile(dlg.GetPath());
  save(dlg.GetPath());
}

void
mmg_dialog::save(wxString file_name,
                 bool used_for_jobs) {
  wxFileConfig *cfg;

  cfg = new wxFileConfig(wxT("mkvmerge GUI"), wxT("Moritz Bunkus"), file_name);
  cfg->SetPath(wxT("/mkvmergeGUI"));
  cfg->Write(wxT("file_version"), MMG_CONFIG_FILE_VERSION_MAX);
  cfg->Write(wxT("gui_version"), wxT(VERSION));
  cfg->Write(wxT("output_file_name"), tc_output->GetValue());
  cfg->Write(wxT("cli_options"), cli_options);

  input_page->save(cfg);
  attachments_page->save(cfg);
  global_page->save(cfg);

  delete cfg;

  if (!used_for_jobs) {
    set_status_bar(Z("Configuration saved."));
    set_last_settings_in_menu(file_name);
  }
}

void
mmg_dialog::set_last_settings_in_menu(wxString name) {
  uint32_t i;
  wxConfigBase *cfg;
  wxString s;

  i = 0;
  while (i < last_settings.size()) {
    if (last_settings[i] == name)
      last_settings.erase(last_settings.begin() + i);
    else
      i++;
  }
  last_settings.insert(last_settings.begin(), name);
  while (last_settings.size() > 4)
    last_settings.pop_back();

  cfg = wxConfigBase::Get();
  cfg->SetPath(wxT("/GUI"));
  for (i = 0; i < last_settings.size(); i++) {
    s.Printf(wxT("last_settings %d"), i);
    cfg->Write(s, last_settings[i]);
  }
  cfg->Flush();

  update_file_menu();
}

void
mmg_dialog::set_last_chapters_in_menu(wxString name) {
  uint32_t i;
  wxConfigBase *cfg;
  wxString s;

  i = 0;
  while (i < last_chapters.size()) {
    if (last_chapters[i] == name)
      last_chapters.erase(last_chapters.begin() + i);
    else
      i++;
  }
  last_chapters.insert(last_chapters.begin(), name);
  while (last_chapters.size() > 4)
    last_chapters.pop_back();

  cfg = wxConfigBase::Get();
  cfg->SetPath(wxT("/GUI"));
  for (i = 0; i < last_chapters.size(); i++) {
    s.Printf(wxT("last_chapters %d"), i);
    cfg->Write(s, last_chapters[i]);
  }
  cfg->Flush();

  update_chapter_menu();
}

bool
mmg_dialog::check_before_overwriting() {
  wxFileName file_name(tc_output->GetValue());
  wxString dir, name, ext;
  wxArrayString files_in_output_dir;
  int i;

  dir = file_name.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);

  if (!global_page->cb_split->GetValue()) {
    if (wxFile::Exists(tc_output->GetValue()) &&
        (wxMessageBox(wxString::Format(Z("The output file '%s' already exists. Do you want to overwrite it?"), tc_output->GetValue().c_str()), Z("Overwrite existing file?"), wxYES_NO) != wxYES))
      return false;
    return true;
  }

  name = file_name.GetName() + wxT("-");
  ext = file_name.GetExt();

  wxDir::GetAllFiles(dir, &files_in_output_dir, wxEmptyString, wxDIR_FILES);
  for (i = 0; i < files_in_output_dir.Count(); i++) {
    wxFileName test_name(files_in_output_dir[i]);

    if (test_name.GetName().StartsWith(name) &&
        (test_name.HasExt() == file_name.HasExt()) &&
        (test_name.GetExt() == ext)) {
      if (file_name.HasExt())
        ext = wxU(".") + ext;
      if (wxMessageBox(wxString::Format(Z("Splitting is active, and at least one of the potential output files '%s%s*%s' already exists. Do you want to overwrite them?"),
                                        dir.c_str(), name.c_str(), ext.c_str()),
                       Z("Overwrite existing file(s)?"), wxYES_NO) != wxYES)
        return false;
      return true;
    }
  }

  return true;
}

bool
mmg_dialog::is_output_file_name_valid() {
#if defined(SYS_WINDOWS)
  wxString forbidden_chars(wxFileName::GetForbiddenChars());
  wxFileName output_file_name(tc_output->GetValue());
  wxString check_name(output_file_name.GetFullName());

  int i;
  for (i = 0; check_name.Length() > i; ++i)
    if (wxNOT_FOUND != forbidden_chars.Find(check_name[i]))
      return false;
#endif
  return true;
}

void
mmg_dialog::on_run(wxCommandEvent &evt) {
  if (muxing_in_progress) {
    wxMessageBox(Z("Another muxing job in still in progress. Please wait until it has finished or abort it manually before starting a new one."),
                 Z("Cannot start second muxing job"), wxOK | wxCENTER | wxICON_ERROR);
    return;
  }

  update_command_line();

  if (!is_output_file_name_valid()) {
    wxMessageBox(Z("The output file name is invalid, e.g. it might contain invalid characters like ':'."), Z("Invalid file name"), wxOK | wxCENTER | wxICON_ERROR, this);
    return;
  }

  if (!input_page->validate_settings() ||
      !attachments_page->validate_settings() ||
      !global_page->validate_settings())
    return;

  if (!chapter_editor_page->is_empty() &&
      (global_page->tc_chapters->GetValue() == wxEmptyString) &&
      (!warned_chapter_editor_not_empty || options.warn_usage)) {
    warned_chapter_editor_not_empty = true;
    if (wxMessageBox(Z("The chapter editor has been used and contains data. "
                       "However, no chapter file has been selected on the global page. "
                       "In mmg, the chapter editor is independant of the muxing process. "
                       "The chapters present in the editor will NOT be muxed into the output file. "
                       "Only the various 'save' functions from the chapter editor menu will cause the chapters to be written to the hard disk.\n\n"
                       "Do you really want to continue muxing?\n\n"
                       "Note: This warning can be deactivated on the 'settings' page. "
                       "Turn off the 'Warn about usage...' option."),
                     Z("Chapter editor is not empty"),
                     wxYES_NO | wxICON_QUESTION) != wxYES)
      return;
  }

  if (options.ask_before_overwriting && !check_before_overwriting())
    return;

  remember_previous_output_directory();

  set_on_top(false);
  muxing_in_progress = true;
  new mux_dialog(this);
}

void
mmg_dialog::muxing_has_finished(int exit_code) {
  muxing_in_progress = false;
  restore_on_top();

  if ((0 == exit_code) && options.filenew_after_successful_mux) {
    wxCommandEvent dummy;
    on_file_new(dummy);
  }
}

void
mmg_dialog::on_help(wxCommandEvent &evt) {
  display_help(notebook->GetCurrentPage() == chapter_editor_page ? HELP_ID_CHAPTER_EDITOR : HELP_ID_CONTENTS);
}

void
mmg_dialog::display_help(int id) {
  if (help == NULL) {
    wxDirDialog dlg(this, Z("Choose the location of the mkvmerge GUI help files"));
    std::vector<wxString> potential_help_paths;
    wxString help_path;
    wxConfigBase *cfg;
    bool first;

#if defined(SYS_WINDOWS)
    help_path = get_installation_dir();
    if (!help_path.IsEmpty()) {
      help_path += wxT("/doc");
      potential_help_paths.push_back(help_path);
    }

#else
    // Debian, probably others
    potential_help_paths.push_back(wxT("/usr/share/doc/mkvtoolnix"));
    potential_help_paths.push_back(wxT("/usr/share/doc/mkvtoolnix/doc"));
    potential_help_paths.push_back(wxT("/usr/share/doc/mkvtoolnix-gui"));
    // SuSE
    potential_help_paths.push_back(wxT("/usr/share/doc/packages/mkvtoolnix"));
    // Fedora Core
    potential_help_paths.push_back(wxT("/usr/share/doc/mkvtoolnix-" VERSION));
    potential_help_paths.push_back(wxT("/usr/share/doc/mkvtoolnix-gui-" VERSION));
    // (Almost the) same for /usr/local
    potential_help_paths.push_back(wxT("/usr/local/share/doc/mkvtoolnix"));
    potential_help_paths.push_back(wxT("/usr/local/share/doc/packages/mkvtoolnix"));
    potential_help_paths.push_back(wxT("/usr/local/share/doc/mkvtoolnix-" VERSION));
    potential_help_paths.push_back(wxT("/usr/local/share/doc/mkvtoolnix-gui-" VERSION));
    // New location
    potential_help_paths.push_back(wxT(MTX_PKG_DATA_DIR));
    potential_help_paths.push_back(wxT(MTX_PKG_DATA_DIR "-" VERSION));
#endif

    cfg = wxConfigBase::Get();
    cfg->SetPath(wxT("/GUI"));
    if (cfg->Read(wxT("help_path"), &help_path))
      potential_help_paths.push_back(help_path);

    potential_help_paths.push_back(wxGetCwd() + wxT("/doc"));
    potential_help_paths.push_back(wxGetCwd());

    help_path = wxEmptyString;
    foreach(const wxString &php, potential_help_paths)
      if (wxFileExists(php + wxT("/mkvmerge-gui.hhp"))) {
        help_path = php;
        break;
      }

    first = true;
    while (!wxFileExists(help_path + wxT("/mkvmerge-gui.hhp"))) {
      if (first) {
        wxMessageBox(Z("The mkvmerge GUI help file was not found. "
                       "This indicates that it has never before been opened, or that the installation path has since been changed.\n\n"
                       "Please select the location of the 'mkvmerge-gui.hhp' file."),
                     Z("Help file not found"),
                     wxOK | wxICON_INFORMATION);
        first = false;
      } else
        wxMessageBox(Z("The mkvmerge GUI help file was not found in the path you've selected. "
                       "Please try again, or abort by pressing the 'abort' button."),
                     Z("Help file not found"),
                     wxOK | wxICON_INFORMATION);

      dlg.SetPath(help_path);
      if (dlg.ShowModal() == wxID_CANCEL)
        return;
      help_path = dlg.GetPath();
      cfg->Write(wxT("help_path"), help_path);
    }
    help = new wxHtmlHelpController;
    help->AddBook(wxFileName(help_path + wxT("/mkvmerge-gui.hhp")), false);
  }

  help->Display(id);
}

void
mmg_dialog::on_about(wxCommandEvent &evt) {
  wxMessageBox(wxString::Format(Z("%s\n\n"
                                  "This GUI was written by Moritz Bunkus <moritz@bunkus.org>"
                                  "\nBased on mmg by Florian Wagner <flo.wagner@gmx.de>\n"
                                  "mkvmerge GUI is licensed under the GPL.\n"
                                  "http://www.bunkus.org/videotools/mkvtoolnix/\n"
                                  "\n"
                                  "Help is available in form of tool tips, from the\n"
                                  "'Help' menu or by pressing the 'F1' key."),
                                wxUCS(get_version_info("mkvmerge GUI", true))),
               Z("About mkvmerge's GUI"),
               wxOK | wxCENTER | wxICON_INFORMATION);
}

void
mmg_dialog::on_show_cmdline(wxCommandEvent &evt) {
  update_command_line();

  show_text_dlg dlg(this, Z("Current command line"), cmdline);
  dlg.ShowModal();
}

void
mmg_dialog::on_save_cmdline(wxCommandEvent &evt) {
  wxFile *file;
  wxString s;
  wxFileDialog dlg(NULL, Z("Choose an output file"), last_open_dir, wxEmptyString, ALLFILES, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if(dlg.ShowModal() == wxID_OK) {
    update_command_line();

    last_open_dir = dlg.GetDirectory();
    file = new wxFile(dlg.GetPath(), wxFile::write);
    s = cmdline + wxT("\n");
    file->Write(s);
    delete file;

    set_status_bar(Z("Command line saved."));
  }
}

void
mmg_dialog::on_create_optionfile(wxCommandEvent &evt) {
  const unsigned char utf8_bom[3] = {0xef, 0xbb, 0xbf};
  uint32_t i;
  std::string arg_utf8;
  wxFile *file;

  wxFileDialog dlg(NULL, Z("Choose an output file"), last_open_dir, wxEmptyString, ALLFILES, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
  if(dlg.ShowModal() == wxID_OK) {
    update_command_line();

    last_open_dir = dlg.GetDirectory();
    try {
      file = new wxFile(dlg.GetPath(), wxFile::write);
      file->Write(utf8_bom, 3);
    } catch (...) {
      wxMessageBox(Z("Could not create the specified file."), Z("File creation failed"), wxOK | wxCENTER | wxICON_ERROR);
      return;
    }
    for (i = 1; i < clargs.Count(); i++) {
      if (clargs[i].length() == 0)
        file->Write(wxT("#EMPTY#"), wxConvUTF8);
      else
        file->Write(clargs[i], wxConvUTF8);
      file->Write(wxT("\n"), wxConvUTF8);
    }
    delete file;

    set_status_bar(Z("Option file created."));
  }
}

void
mmg_dialog::on_copy_to_clipboard(wxCommandEvent &evt) {
  update_command_line();
  if (wxTheClipboard->Open()) {
    wxTheClipboard->SetData(new wxTextDataObject(cmdline));
    wxTheClipboard->Close();
    set_status_bar(Z("Command line copied to clipboard."));
  } else
    set_status_bar(Z("Could not open the clipboard."));
}

wxString &
mmg_dialog::get_command_line() {
  return cmdline;
}

wxArrayString &
mmg_dialog::get_command_line_args() {
  return clargs;
}

void
mmg_dialog::on_update_command_line(wxTimerEvent &evt) {
  update_command_line();
}

void
mmg_dialog::update_command_line() {
  unsigned int i;

  clargs.Clear();
  clargs.Add(options.mkvmerge);
  clargs.Add(wxT("--output-charset"));
  clargs.Add(wxT("UTF-8"));
  clargs.Add(wxT("-o"));
  clargs.Add(tc_output->GetValue());

#if defined(HAVE_LIBINTL_H)
  if (!app->m_ui_locale.empty()) {
    clargs.Add(wxT("--ui-language"));
    clargs.Add(wxU(app->m_ui_locale));
  }
#endif  // HAVE_LIBINTL_H

  unsigned int args_start = clargs.Count();

  if (options.priority != wxT("normal")) {
    clargs.Add(wxT("--priority"));
    clargs.Add(options.priority);
  }

  unsigned int fidx;
  for (fidx = 0; files.size() > fidx; fidx++) {
    mmg_file_cptr &f    = files[fidx];
    bool no_audio       = true;
    bool no_video       = true;
    bool no_subs        = true;
    bool no_chapters    = true;
    bool no_tags        = true;
    bool no_global_tags = true;
    wxString aids, dids, sids, tids;

    unsigned int tidx;
    for (tidx = 0; f->tracks.size() > tidx; tidx++) {
      mmg_track_cptr &t = f->tracks[tidx];
      if (!t->enabled)
        continue;

      wxString sid = wxLongLong('t' == t->type ? t->id - TRACK_ID_TAGS_BASE : t->id).ToString();

      if (t->type == wxT('a')) {
        no_audio = false;
        if (aids.length() > 0)
          aids += wxT(",");
        aids += sid;

        if (!t->appending && (t->aac_is_sbr || t->aac_is_sbr_detected)) {
          clargs.Add(wxT("--aac-is-sbr"));
          clargs.Add(wxString::Format(wxT("%s:%d"), sid.c_str(), t->aac_is_sbr ? 1 : 0));
        }

      } else if (t->type == wxT('v')) {
        no_video = false;
        if (dids.length() > 0)
          dids += wxT(",");
        dids += sid;

      } else if (t->type == wxT('s')) {
        no_subs = false;
        if (sids.length() > 0)
          sids += wxT(",");
        sids += sid;

        if ((t->sub_charset.Length() > 0) && (t->sub_charset != wxT("default"))) {
          clargs.Add(wxT("--sub-charset"));
          clargs.Add(sid + wxT(":") + t->sub_charset);
        }

      } else if (t->type == wxT('c')) {
        no_chapters = false;

        if ((t->sub_charset.Length() > 0) && (t->sub_charset != wxT("default"))) {
          clargs.Add(wxT("--chapter-charset"));
          clargs.Add(t->sub_charset);
        }

        if (extract_language_code(t->language) != wxT("und")) {
          clargs.Add(wxT("--chapter-language"));
          clargs.Add(extract_language_code(t->language));
        }

        continue;

      } else if (t->type == wxT('t')) {
        if (TRACK_ID_GLOBAL_TAGS == t->id)
          no_global_tags = false;
        else {
          no_tags        = false;
          if (tids.length() > 0)
            tids += wxT(",");
          tids += sid;
        }

        continue;
      }

      if (!t->appending && (t->language != wxT("und"))) {
        clargs.Add(wxT("--language"));
        clargs.Add(sid + wxT(":") + extract_language_code(t->language));
      }

      if (!t->appending && (t->cues != wxT("default"))) {
        clargs.Add(wxT("--cues"));
        if (t->cues == wxT("only for I frames"))
          clargs.Add(sid + wxT(":iframes"));
        else if (t->cues == wxT("for all frames"))
          clargs.Add(sid + wxT(":all"));
        else if (t->cues == wxT("none"))
          clargs.Add(sid + wxT(":none"));
      }

      if ((t->delay.Length() > 0) || (t->stretch.Length() > 0)) {
        wxString arg = sid + wxT(":");
        if (t->delay.Length() > 0)
          arg += t->delay;
        else
          arg += wxT("0");
        if (t->stretch.Length() > 0) {
          arg += wxT(",") + t->stretch;
          if (t->stretch.Find(wxT("/")) < 0)
            arg += wxT("/1");
        }
        clargs.Add(wxT("--sync"));
        clargs.Add(arg);
      }

      if (!t->appending && ((t->track_name.Length() > 0) || t->track_name_was_present)) {
        clargs.Add(wxT("--track-name"));
        clargs.Add(sid + wxT(":") + t->track_name);
      }

      if (!t->appending && (0 != t->default_track)) {
        clargs.Add(wxT("--default-track"));
        clargs.Add(sid + wxT(":") + (1 == t->default_track ? wxT("yes") : wxT("no")));
      }

      if (!t->appending) {
        clargs.Add(wxT("--forced-track"));
        clargs.Add(sid + wxT(":") + (t->forced_track ? wxT("yes") : wxT("no")));
      }

      if (!t->appending && (t->tags.Length() > 0)) {
        clargs.Add(wxT("--tags"));
        clargs.Add(sid + wxT(":") + t->tags);
      }

      if (!t->appending && !t->display_dimensions_selected && (t->aspect_ratio.Length() > 0)) {
        clargs.Add(wxT("--aspect-ratio"));
        clargs.Add(sid + wxT(":") + t->aspect_ratio);

      } else if (!t->appending && t->display_dimensions_selected && (t->dwidth.Length() > 0) && (t->dheight.Length() > 0)) {
        clargs.Add(wxT("--display-dimensions"));
        clargs.Add(sid + wxT(":") + t->dwidth + wxT("x") + t->dheight);
      }

      if (!t->appending && (t->fourcc.Length() > 0)) {
        clargs.Add(wxT("--fourcc"));
        clargs.Add(sid + wxT(":") + t->fourcc);
      }

      if (t->fps.Length() > 0) {
        clargs.Add(wxT("--default-duration"));
        clargs.Add(sid + wxT(":") + t->fps + wxT("fps"));
      }

      if (0 != t->nalu_size_length) {
        clargs.Add(wxT("--nalu-size-length"));
        clargs.Add(wxString::Format(wxT("%s:%d"), sid.c_str(), t->nalu_size_length));
      }

      if (!t->appending && (0 != t->stereo_mode)) {
        clargs.Add(wxT("--stereo-mode"));
        clargs.Add(wxString::Format(wxT("%s:%d"), sid.c_str(), t->stereo_mode - 1));
      }

      if (!t->appending && (t->compression.Length() > 0)) {
        wxString compression = t->compression;
        if (compression == wxT("none"))
          compression = wxT("none");
        clargs.Add(wxT("--compression"));
        clargs.Add(sid + wxT(":") + compression);
      }

      if (!t->appending && (t->timecodes.Length() > 0)) {
        clargs.Add(wxT("--timecodes"));
        clargs.Add(sid + wxT(":") + t->timecodes);
      }

      if (t->user_defined.Length() > 0) {
        std::vector<wxString> opts = split(t->user_defined, wxString(wxT(" ")));
        for (i = 0; opts.size() > i; i++) {
          wxString opt = strip(opts[i]);
          opt.Replace(wxT("<TID>"), sid, true);
          clargs.Add(opt);
        }
      }
    }

    if (aids.length() > 0) {
      clargs.Add(wxT("-a"));
      clargs.Add(aids);
    }
    if (dids.length() > 0) {
      clargs.Add(wxT("-d"));
      clargs.Add(dids);
    }
    if (sids.length() > 0) {
      clargs.Add(wxT("-s"));
      clargs.Add(sids);
    }
    if (tids.length() > 0) {
      clargs.Add(wxT("--track-tags"));
      clargs.Add(tids);
    }

    std::vector<mmg_attached_file_cptr>::iterator att_file = f->attached_files.begin();
    std::vector<wxString> att_file_ids;

    while (att_file != f->attached_files.end()) {
      if ((*att_file)->enabled)
        att_file_ids.push_back(wxString::Format(wxT("%ld"), (*att_file)->id));
      ++att_file;
    }

    if (!att_file_ids.empty()) {
      clargs.Add(wxT("--attachments"));
      clargs.Add(join(wxT(","), att_file_ids));

    } else if (!f->attached_files.empty())
      clargs.Add(wxT("--no-attachments"));

    if (no_video)
      clargs.Add(wxT("-D"));
    if (no_audio)
      clargs.Add(wxT("-A"));
    if (no_subs)
      clargs.Add(wxT("-S"));
    if (no_tags)
      clargs.Add(wxT("-T"));
    if (no_global_tags)
      clargs.Add(wxT("--no-global-tags"));
    if (no_chapters)
      clargs.Add(wxT("--no-chapters"));

    if (f->appending)
      clargs.Add(wxString(wxT("+")) + f->file_name);
    else
      clargs.Add(f->file_name);
  }

  wxString track_order = create_track_order(false);
  if (track_order.length() > 0) {
    clargs.Add(wxT("--track-order"));
    clargs.Add(track_order);
  }

  wxString append_mapping = create_append_mapping();
  if (append_mapping.length() > 0) {
    clargs.Add(wxT("--append-to"));
    clargs.Add(append_mapping);
  }

  for (fidx = 0; attachments.size() > fidx; fidx++) {
    mmg_attachment_cptr &a = attachments[fidx];

    clargs.Add(wxT("--attachment-mime-type"));
    clargs.Add(a->mime_type);
    if (a->description.Length() > 0) {
      clargs.Add(wxT("--attachment-description"));
      clargs.Add(no_cr(a->description));
    }
    clargs.Add(wxT("--attachment-name"));
    clargs.Add(a->stored_name);
    if (a->style == 0)
      clargs.Add(wxT("--attach-file"));
    else
      clargs.Add(wxT("--attach-file-once"));
    clargs.Add(a->file_name);
  }

  if (title_was_present || (global_page->tc_title->GetValue().Length() > 0)) {
    clargs.Add(wxT("--title"));
    clargs.Add(global_page->tc_title->GetValue());
  }

  if (global_page->cb_split->IsChecked()) {
    clargs.Add(wxT("--split"));
    if (global_page->rb_split_by_size->GetValue())
      clargs.Add(wxT("size:") + global_page->cob_split_by_size->GetValue());
    else if (global_page->rb_split_by_time->GetValue())
      clargs.Add(wxT("duration:") + global_page->cob_split_by_time->GetValue());
    else
      clargs.Add(wxT("timecodes:") + global_page->tc_split_after_timecodes->GetValue());

    if (global_page->tc_split_max_files->GetValue().Length() > 0) {
      clargs.Add(wxT("--split-max-files"));
      clargs.Add(global_page->tc_split_max_files->GetValue());
    }

    if (global_page->cb_link->IsChecked())
      clargs.Add(wxT("--link"));
  }

  if (!global_page->tc_segment_uid->GetValue().IsEmpty()) {
    clargs.Add(wxT("--segment-uid"));
    clargs.Add(global_page->tc_segment_uid->GetValue());
  }

  if (global_page->tc_previous_segment_uid->GetValue().Length() > 0) {
    clargs.Add(wxT("--link-to-previous"));
    clargs.Add(global_page->tc_previous_segment_uid->GetValue());
  }

  if (global_page->tc_next_segment_uid->GetValue().Length() > 0) {
    clargs.Add(wxT("--link-to-next"));
    clargs.Add(global_page->tc_next_segment_uid->GetValue());
  }

  if (global_page->tc_chapters->GetValue().Length() > 0) {
    if (global_page->cob_chap_language->GetValue().Length() > 0) {
      clargs.Add(wxT("--chapter-language"));
      clargs.Add(extract_language_code(global_page->cob_chap_language->GetValue()));
    }

    if (global_page->cob_chap_charset->GetValue().Length() > 0) {
      clargs.Add(wxT("--chapter-charset"));
      clargs.Add(global_page->cob_chap_charset->GetValue());
    }

    if (global_page->tc_cue_name_format->GetValue().Length() > 0) {
      clargs.Add(wxT("--cue-chapter-name-format"));
      clargs.Add(global_page->tc_cue_name_format->GetValue());
    }

    clargs.Add(wxT("--chapters"));
    clargs.Add(global_page->tc_chapters->GetValue());
  }

  if (global_page->tc_global_tags->GetValue().Length() > 0) {
    clargs.Add(wxT("--global-tags"));
    clargs.Add(global_page->tc_global_tags->GetValue());
  }

  cli_options = strip(cli_options);
  if (cli_options.length() > 0) {
    std::vector<wxString> opts = split(cli_options, wxString(wxT(" ")));
    for (i = 0; i < opts.size(); i++) {
      wxString opt = strip(opts[i]);
      if (!opt.IsEmpty())
        clargs.Add(opt);
    }
  }

  cmdline = wxT("\"") + shell_escape(options.mkvmerge, true) + wxT("\" -o \"") + shell_escape(tc_output->GetValue()) + wxT("\" ");

  for (i = args_start; i < clargs.Count(); i++)
    cmdline += wxT(" \"") + shell_escape(clargs[i]) + wxT("\"");
}

void
mmg_dialog::on_file_load_last(wxCommandEvent &evt) {
  if ((evt.GetId() < ID_M_FILE_LOADLAST1) || ((evt.GetId() - ID_M_FILE_LOADLAST1) >= last_settings.size()))
    return;

  load(last_settings[evt.GetId() - ID_M_FILE_LOADLAST1]);
}

void
mmg_dialog::on_chapters_load_last(wxCommandEvent &evt) {
  if ((evt.GetId() < ID_M_CHAPTERS_LOADLAST1) || ((evt.GetId() - ID_M_CHAPTERS_LOADLAST1) >= last_chapters.size()))
    return;

  notebook->SetSelection(4);
  chapter_editor_page->load(last_chapters[evt.GetId() - ID_M_CHAPTERS_LOADLAST1]);
}

void
mmg_dialog::update_file_menu() {
  uint32_t i;
  wxMenuItem *mi;
  wxString s;

  for (i = ID_M_FILE_LOADLAST1; i <= ID_M_FILE_LOADLAST4; i++) {
    mi = file_menu->FindItem(i);
    if (mi != NULL)
      file_menu->Destroy(mi);
  }

  if ((last_settings.size() > 0) && !file_menu_sep) {
    file_menu->AppendSeparator();
    file_menu_sep = true;
  }
  for (i = 0; i < last_settings.size(); i++) {
    s.Printf(wxT("&%u. %s"), i + 1, last_settings[i].c_str());
    file_menu->Append(ID_M_FILE_LOADLAST1 + i, s);
  }
}

void
mmg_dialog::update_chapter_menu() {
  uint32_t i;
  wxMenuItem *mi;
  wxString s;

  for (i = ID_M_CHAPTERS_LOADLAST1; i <= ID_M_CHAPTERS_LOADLAST4; i++) {
    mi = chapter_menu->FindItem(i);
    if (mi != NULL)
      chapter_menu->Destroy(mi);
  }

  if ((last_chapters.size() > 0) && !chapter_menu_sep) {
    chapter_menu->AppendSeparator();
    chapter_menu_sep = true;
  }
  for (i = 0; i < last_chapters.size(); i++) {
    s.Printf(wxT("&%u. %s"), i + 1, last_chapters[i].c_str());
    chapter_menu->Append(ID_M_CHAPTERS_LOADLAST1 + i, s);
  }
}

void
mmg_dialog::on_new_chapters(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_new_chapters(evt);
}

void
mmg_dialog::on_load_chapters(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_load_chapters(evt);
}

void
mmg_dialog::on_save_chapters(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_save_chapters(evt);
}

void
mmg_dialog::on_save_chapters_to_kax_file(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_save_chapters_to_kax_file(evt);
}

void
mmg_dialog::on_save_chapters_as(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_save_chapters_as(evt);
}

void
mmg_dialog::on_verify_chapters(wxCommandEvent &evt) {
  notebook->SetSelection(4);
  chapter_editor_page->on_verify_chapters(evt);
}

void
mmg_dialog::on_window_selected(wxCommandEvent &evt) {
  notebook->SetSelection(evt.GetId() - ID_M_WINDOW_INPUT);
}

void
mmg_dialog::set_title_maybe(const wxString &new_title) {
  if ((new_title.length() > 0) &&
      (global_page->tc_title->GetValue().length() == 0))
    global_page->tc_title->SetValue(new_title);
}

void
mmg_dialog::set_output_maybe(const wxString &new_output) {
  if (!options.autoset_output_filename || new_output.empty())
    return;

  bool has_video = false, has_audio = false;

  foreach(mmg_track_t *t, tracks) {
    if (t->is_video()) {
      has_video = true;
      break;
    } else if (t->is_audio())
      has_audio = true;
  }

  wxFileName source_file_name(tc_output->GetValue().IsEmpty() ? new_output : tc_output->GetValue());
  wxString output_dir;
  if (ODM_PREVIOUS == options.output_directory_mode)
    output_dir = previous_output_directory;
  else if (ODM_FIXED == options.output_directory_mode)
    output_dir = options.output_directory;

  if (output_dir.IsEmpty())
    output_dir = source_file_name.GetPath();

  wxFileName output_file_name;
  wxFileName dir   = wxFileName::DirName(output_dir);
  unsigned int idx = 0;
  while (true) {
    output_file_name = dir;
    output_file_name.SetName(source_file_name.GetName() + (0 == idx ? wxT("") : wxString::Format(wxT(" (%u)"), idx)));
    output_file_name.SetExt(has_video ? wxU("mkv") : has_audio ? wxU("mka") : wxU("mks"));

    if (!output_file_name.FileExists())
      break;

    ++idx;
  }

  tc_output->SetValue(output_file_name.GetFullPath());
}

void
mmg_dialog::remove_output_filename() {
  if (!options.autoset_output_filename)
    return;

  tc_output->SetValue(wxEmptyString);
}

void
mmg_dialog::on_add_to_jobqueue(wxCommandEvent &evt) {
  wxString description, line;
  job_t job;
  int i, result;
  bool ok;

  if (!input_page->validate_settings() ||
      !attachments_page->validate_settings() ||
      !global_page->validate_settings())
    return;

  line.Printf(Z("The output file '%s' already exists. Do you want to overwrite it?"), tc_output->GetValue().c_str());
  if (   options.ask_before_overwriting
      && wxFile::Exists(tc_output->GetValue())
      && (wxMessageBox(break_line(line, 60), Z("Overwrite existing file?"), wxYES_NO) != wxYES))
    return;

  description = tc_output->GetValue().AfterLast(wxT('/')).AfterLast(wxT('\\')).AfterLast(wxT('/')).BeforeLast(wxT('.'));
  ok = false;
  do {
    description = wxGetTextFromUser(Z("Please enter a description for the new job:"), Z("Job description"), description);
    if (description.length() == 0)
      return;

    if (!options.ask_before_overwriting)
      break;
    break_line(line, 60);
    ok = true;
    for (i = 0; i < jobs.size(); i++)
      if (description == *jobs[i].description) {
        ok = false;
        line.Printf(Z("A job with the description '%s' already exists. Do you really want to add another one with the same description?"), description.c_str());
        result = wxMessageBox(line, Z("Description already exists"), wxYES_NO | wxCANCEL);
        if (result == wxYES)
          ok = true;
        else if (result == wxCANCEL)
          return;
        break;
      }
  } while (!ok);

  if (!wxDirExists(wxT("jobs")))
    wxMkdir(wxT("jobs"));

  last_job_id++;
  if (last_job_id > 2000000000)
    last_job_id = 0;
  job.id = last_job_id;
  job.status = JOBS_PENDING;
  job.added_on = wxGetUTCTime();
  job.started_on = -1;
  job.finished_on = -1;
  job.description = new wxString(description);
  job.log = new wxString();
  jobs.push_back(job);

  description.Printf(wxT("/jobs/%d.mmg"), job.id);
  save(wxGetCwd() + description);

  save_job_queue();

  remember_previous_output_directory();

  if (options.filenew_after_add_to_jobqueue) {
    wxCommandEvent dummy;
    on_file_new(dummy);
  }

  set_status_bar(Z("Job added to job queue"));
}

void
mmg_dialog::on_manage_jobs(wxCommandEvent &evt) {
  set_on_top(false);
  job_dialog jdlg(this);
  restore_on_top();
}

void
mmg_dialog::load_job_queue() {
  int num, i, value;
  wxString s;
  wxConfigBase *cfg;
  job_t job;

  last_job_id = 0;

  cfg = wxConfigBase::Get();
  cfg->SetPath(wxT("/jobs"));
  cfg->Read(wxT("last_job_id"), &last_job_id);
  if (!cfg->Read(wxT("number_of_jobs"), &num))
    return;

  for (i = 0; i < jobs.size(); i++) {
    delete jobs[i].description;
    delete jobs[i].log;
  }
  jobs.clear();

  for (i = 0; i < num; i++) {
    cfg->SetPath(wxT("/jobs"));
    s.Printf(wxT("%u"), i);
    if (!cfg->HasGroup(s))
      continue;
    cfg->SetPath(s);
    cfg->Read(wxT("id"), &value);
    job.id = value;
    cfg->Read(wxT("status"), &s);
    job.status =
      s == wxT("pending") ? JOBS_PENDING :
      s == wxT("done") ? JOBS_DONE :
      s == wxT("done_warnings") ? JOBS_DONE_WARNINGS :
      s == wxT("aborted") ? JOBS_ABORTED :
      JOBS_FAILED;
    cfg->Read(wxT("added_on"), &value);
    job.added_on = value;
    cfg->Read(wxT("started_on"), &value);
    job.started_on = value;
    cfg->Read(wxT("finished_on"), &value);
    job.finished_on = value;
    cfg->Read(wxT("description"), &s);
    job.description = new wxString(s);
    cfg->Read(wxT("log"), &s);
    s.Replace(wxT(":::"), wxT("\n"));
    job.log = new wxString(s);
    jobs.push_back(job);
  }
}

void
mmg_dialog::save_job_queue() {
  wxString s;
  wxConfigBase *cfg;
  uint32_t i;
  std::vector<wxString> job_groups;
  long cookie;

  cfg = wxConfigBase::Get();
  cfg->SetPath(wxT("/jobs"));
  if (cfg->GetFirstGroup(s, cookie)) {
    do {
      job_groups.push_back(s);
    } while (cfg->GetNextGroup(s, cookie));
    for (i = 0; i < job_groups.size(); i++)
      cfg->DeleteGroup(job_groups[i]);
  }
  cfg->Write(wxT("last_job_id"), last_job_id);
  cfg->Write(wxT("number_of_jobs"), (int)jobs.size());
  for (i = 0; i < jobs.size(); i++) {
    s.Printf(wxT("/jobs/%u"), i);
    cfg->SetPath(s);
    cfg->Write(wxT("id"), jobs[i].id);
    cfg->Write(wxT("status"),
               jobs[i].status == JOBS_PENDING ? wxT("pending") :
               jobs[i].status == JOBS_DONE ? wxT("done") :
               jobs[i].status == JOBS_DONE_WARNINGS ? wxT("done_warnings") :
               jobs[i].status == JOBS_ABORTED ? wxT("aborted") :
               wxT("failed"));
    cfg->Write(wxT("added_on"), jobs[i].added_on);
    cfg->Write(wxT("started_on"), jobs[i].started_on);
    cfg->Write(wxT("finished_on"), jobs[i].finished_on);
    cfg->Write(wxT("description"), *jobs[i].description);
    s = *jobs[i].log;
    s.Replace(wxT("\n"), wxT(":::"));
    cfg->Write(wxT("log"), s);
  }
  cfg->Flush();
}

void
mmg_dialog::save_preferences() {
  wxConfigBase *cfg = (wxConfig *)wxConfigBase::Get();
  int x, y;

  cfg->SetPath(wxU("/GUI"));

  GetPosition(&x, &y);
  cfg->Write(wxU("window_position_x"),               x);
  cfg->Write(wxU("window_position_y"),               y);
  cfg->Write(wxU("warned_chapter_editor_not_empty"), warned_chapter_editor_not_empty);
  cfg->Write(wxU("previous_output_directory"),       previous_output_directory);

  cfg->Write(wxU("mkvmerge_executable"),             options.mkvmerge);
  cfg->Write(wxU("process_priority"),                options.priority);
  cfg->Write(wxU("autoset_output_filename"),         options.autoset_output_filename);
  cfg->Write(wxU("output_directory_mode"),           (long)options.output_directory_mode);
  cfg->Write(wxU("output_directory"),                options.output_directory);
  cfg->Write(wxU("ask_before_overwriting"),          options.ask_before_overwriting);
  cfg->Write(wxU("filenew_after_add_to_jobqueue"),   options.filenew_after_add_to_jobqueue);
  cfg->Write(wxU("filenew_after_successful_mux"),    options.filenew_after_successful_mux);
  cfg->Write(wxU("on_top"),                          options.on_top);
  cfg->Write(wxU("warn_usage"),                      options.warn_usage);
  cfg->Write(wxU("gui_debugging"),                   options.gui_debugging);
  cfg->Write(wxU("set_delay_from_filename"),         options.set_delay_from_filename);
  cfg->Write(wxU("popular_languages"),               join(wxU(" "), options.popular_languages));

  cfg->Flush();
}

void
mmg_dialog::load_preferences() {
  wxConfigBase *cfg = wxConfigBase::Get();
  wxString s;
  int window_pos_x, window_pos_y;

  cfg->SetPath(wxT("/GUI"));

  if (   cfg->Read(wxT("window_position_x"), &window_pos_x)
      && cfg->Read(wxT("window_position_y"), &window_pos_y)
      && (0 < window_pos_x) && (0xffff > window_pos_x)
      && (0 < window_pos_y) && (0xffff > window_pos_y))
    Move(window_pos_x, window_pos_y);

  cfg->Read(wxU("warned_chapterditor_not_empty"), &warned_chapter_editor_not_empty, false);
  cfg->Read(wxU("previous_output_directory"),     &previous_output_directory, wxU(""));

  cfg->Read(wxU("mkvmerge_executable"),           &options.mkvmerge, wxU("mkvmerge"));
  cfg->Read(wxU("process_priority"),              &options.priority, wxU("normal"));
  cfg->Read(wxU("autoset_output_filename"),       &options.autoset_output_filename, true);
  cfg->Read(wxU("output_directory_mode"),         (long *)&options.output_directory_mode, ODM_FROM_FIRST_INPUT_FILE);
  cfg->Read(wxU("output_directory"),              &options.output_directory, wxU(""));
  cfg->Read(wxU("ask_before_overwriting"),        &options.ask_before_overwriting, true);
  cfg->Read(wxU("filenew_after_add_to_jobqueue"), &options.filenew_after_add_to_jobqueue, false);
  cfg->Read(wxU("filenew_after_successful_mux"),  &options.filenew_after_successful_mux, false);
  cfg->Read(wxU("on_top"),                        &options.on_top, false);
  cfg->Read(wxU("warn_usage"),                    &options.warn_usage, true);
  cfg->Read(wxU("gui_debugging"),                 &options.gui_debugging, false);
  cfg->Read(wxU("set_delay_from_filename"),       &options.set_delay_from_filename, true);
  cfg->Read(wxU("popular_languages"),             &s, wxEmptyString);

  options.init_popular_languages(s);
  options.validate();

#if defined(SYS_WINDOWS)
  // Check whether or not the mkvmerge executable path is still set to
  // the default value "mkvmerge". If it is try getting the
  // installation path from the registry and use that for a more
  // precise location for mkvmerge.exe. Fall back to the old default
  // value "mkvmerge" if all else fails.
  if (options.mkvmerge == wxT("mkvmerge"))
    options.mkvmerge.Empty();

  if (options.mkvmerge.IsEmpty()) {
    options.mkvmerge = get_installation_dir();
    if (!options.mkvmerge.IsEmpty())
      options.mkvmerge += wxT("\\mkvmerge.exe");
  }

  if (options.mkvmerge.IsEmpty())
    options.mkvmerge = wxT("mkvmerge");
#endif  // SYS_WINDOWS
}

void
mmg_dialog::remember_previous_output_directory() {
  wxFileName filename(tc_output->GetValue());
  wxString path = filename.GetPath();

  if (!path.IsEmpty())
    previous_output_directory = path;
}

void
mmg_dialog::on_add_cli_options(wxCommandEvent &evt) {
  cli_options_dlg dlg(this);

  if (dlg.go(cli_options))
    update_command_line();
}

void
mmg_dialog::on_close(wxCloseEvent &evt) {
  save_preferences();
  Destroy();
}

void
mmg_dialog::set_on_top(bool on_top) {
  long style;

  style = GetWindowStyleFlag();
  if (on_top)
    style |= wxSTAY_ON_TOP;
  else
    style &= ~wxSTAY_ON_TOP;
  SetWindowStyleFlag(style);
}

void
mmg_dialog::restore_on_top() {
  set_on_top(options.on_top);
}

void
mmg_dialog::on_file_options(wxCommandEvent &evt) {
  options_dialog dlg(this, options);

  if (dlg.ShowModal() != wxID_OK)
    return;

  log_window->Show(options.gui_debugging);
  set_on_top(options.on_top);
  query_mkvmerge_capabilities();
}

void
mmg_dialog::on_run_header_editor(wxCommandEvent &evt) {
  header_editor_frame_c *window  = new header_editor_frame_c(this);

  window->Show();
}

void
mmg_dialog::query_mkvmerge_capabilities() {
  wxString tmp;
  wxArrayString output;
  std::vector<wxString> parts;
  int result, i;

  wxLogMessage(Z("Querying mkvmerge's capabilities"));
  tmp = wxT("\"") + options.mkvmerge + wxT("\" --capabilities");
#if defined(SYS_WINDOWS)
  result = wxExecute(tmp, output);
#else
  // Workaround for buggy behaviour of some wxWindows/GTK combinations.
  wxProcess *process;
  wxInputStream *out;
  int c;
  std::string tmps;

  process = new wxProcess(this, 1);
  process->Redirect();
  result = wxExecute(tmp, wxEXEC_ASYNC, process);
  if (result == 0)
    return;

  out = process->GetInputStream();
  tmps = "";
  c = 0;
  while (1) {
    if (!out->Eof()) {
      c = out->GetC();
      if (c == '\n') {
        output.Add(wxU(tmps));
        tmps = "";
      } else if (c < 0)
        break;
      else if (c != '\r')
        tmps += c;
    } else
      break;
  }
  if (tmps.length() > 0)
    output.Add(wxU(tmps));
  result = 0;
#endif

  if (result == 0) {
    capabilities.clear();
    for (i = 0; i < output.Count(); i++) {
      tmp = output[i];
      strip(tmp);
      wxLogMessage(wxT("Capability: %s"), tmp.c_str());
      parts = split(tmp, wxU("="), 2);
      if (parts.size() == 1)
        capabilities[parts[0]] = wxT("true");
      else
        capabilities[parts[0]] = parts[1];
    }
  }
}

IMPLEMENT_CLASS(mmg_dialog, wxFrame);
BEGIN_EVENT_TABLE(mmg_dialog, wxFrame)
  EVT_BUTTON(ID_B_BROWSEOUTPUT,           mmg_dialog::on_browse_output)
  EVT_BUTTON(ID_B_STARTMUXING,            mmg_dialog::on_run)
  EVT_BUTTON(ID_B_COPYTOCLIPBOARD,        mmg_dialog::on_copy_to_clipboard)
  EVT_BUTTON(ID_B_ADD_TO_JOBQUEUE,        mmg_dialog::on_add_to_jobqueue)
  EVT_TIMER(ID_T_UPDATECMDLINE,           mmg_dialog::on_update_command_line)
  EVT_TIMER(ID_T_STATUSBAR,               mmg_dialog::on_clear_status_bar)
  EVT_MENU(ID_M_FILE_EXIT,                mmg_dialog::on_quit)
  EVT_MENU(ID_M_FILE_NEW,                 mmg_dialog::on_file_new)
  EVT_MENU(ID_M_FILE_LOAD,                mmg_dialog::on_file_load)
  EVT_MENU(ID_M_FILE_SAVE,                mmg_dialog::on_file_save)
  EVT_MENU(ID_M_FILE_OPTIONS,             mmg_dialog::on_file_options)
  EVT_MENU(ID_M_FILE_HEADEREDITOR,        mmg_dialog::on_run_header_editor)
  EVT_MENU(ID_M_FILE_SETOUTPUT,           mmg_dialog::on_browse_output)
  EVT_MENU(ID_M_MUXING_START,             mmg_dialog::on_run)
  EVT_MENU(ID_M_MUXING_SHOW_CMDLINE,      mmg_dialog::on_show_cmdline)
  EVT_MENU(ID_M_MUXING_COPY_CMDLINE,      mmg_dialog::on_copy_to_clipboard)
  EVT_MENU(ID_M_MUXING_SAVE_CMDLINE,      mmg_dialog::on_save_cmdline)
  EVT_MENU(ID_M_MUXING_CREATE_OPTIONFILE, mmg_dialog::on_create_optionfile)
  EVT_MENU(ID_M_MUXING_ADD_TO_JOBQUEUE,   mmg_dialog::on_add_to_jobqueue)
  EVT_MENU(ID_M_MUXING_MANAGE_JOBS,       mmg_dialog::on_manage_jobs)
  EVT_MENU(ID_M_MUXING_ADD_CLI_OPTIONS,   mmg_dialog::on_add_cli_options)
  EVT_MENU(ID_M_HELP_HELP,                mmg_dialog::on_help)
  EVT_MENU(ID_M_HELP_ABOUT,               mmg_dialog::on_about)
  EVT_MENU(ID_M_FILE_LOADLAST1,           mmg_dialog::on_file_load_last)
  EVT_MENU(ID_M_FILE_LOADLAST2,           mmg_dialog::on_file_load_last)
  EVT_MENU(ID_M_FILE_LOADLAST3,           mmg_dialog::on_file_load_last)
  EVT_MENU(ID_M_FILE_LOADLAST4,           mmg_dialog::on_file_load_last)
  EVT_MENU(ID_M_CHAPTERS_NEW,             mmg_dialog::on_new_chapters)
  EVT_MENU(ID_M_CHAPTERS_LOAD,            mmg_dialog::on_load_chapters)
  EVT_MENU(ID_M_CHAPTERS_SAVE,            mmg_dialog::on_save_chapters)
  EVT_MENU(ID_M_CHAPTERS_SAVEAS,          mmg_dialog::on_save_chapters_as)
  EVT_MENU(ID_M_CHAPTERS_SAVETOKAX,       mmg_dialog::on_save_chapters_to_kax_file)
  EVT_MENU(ID_M_CHAPTERS_VERIFY,          mmg_dialog::on_verify_chapters)
  EVT_MENU(ID_M_CHAPTERS_LOADLAST1,       mmg_dialog::on_chapters_load_last)
  EVT_MENU(ID_M_CHAPTERS_LOADLAST2,       mmg_dialog::on_chapters_load_last)
  EVT_MENU(ID_M_CHAPTERS_LOADLAST3,       mmg_dialog::on_chapters_load_last)
  EVT_MENU(ID_M_CHAPTERS_LOADLAST4,       mmg_dialog::on_chapters_load_last)
  EVT_MENU(ID_M_WINDOW_INPUT,             mmg_dialog::on_window_selected)
  EVT_MENU(ID_M_WINDOW_ATTACHMENTS,       mmg_dialog::on_window_selected)
  EVT_MENU(ID_M_WINDOW_GLOBAL,            mmg_dialog::on_window_selected)
  EVT_MENU(ID_M_WINDOW_CHAPTEREDITOR,     mmg_dialog::on_window_selected)
  EVT_CLOSE(mmg_dialog::on_close)
END_EVENT_TABLE();
