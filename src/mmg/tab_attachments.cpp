/*
   mkvmerge GUI -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   "attachments" tab

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "wx/wxprec.h"

#include "wx/wx.h"
#include "wx/dnd.h"
#include "wx/filename.h"
#include "wx/listctrl.h"
#include "wx/notebook.h"
#include "wx/statline.h"

#include "common.h"
#include "extern_data.h"
#include "mmg.h"
#include "tab_attachments.h"

vector<mmg_attachment_cptr> attachments;
vector<mmg_attached_file_cptr> attached_files;

class attachments_drop_target_c: public wxFileDropTarget {
private:
  tab_attachments *owner;
public:
  attachments_drop_target_c(tab_attachments *n_owner):
    owner(n_owner) {};
  virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &dropped_files) {
    int i;

    for (i = 0; i < dropped_files.Count(); i++)
      owner->add_attachment(dropped_files[i]);

    return true;
  }
};

tab_attachments::tab_attachments(wxWindow *parent):
  wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL) {

  // Create all elements
  wxStaticBox *sb_attached_files = new wxStaticBox(this, wxID_STATIC, Z("Attached files"));
  clb_attached_files             = new wxCheckListBox(this, ID_CLB_ATTACHED_FILES);

  wxStaticBox *sb_attachments    = new wxStaticBox(this, wxID_STATIC, Z("Attachments"));
  lb_attachments                 = new wxListBox(this, ID_LB_ATTACHMENTS);

  b_add_attachment               = new wxButton(this, ID_B_ADDATTACHMENT, Z("add"));
  b_remove_attachment            = new wxButton(this, ID_B_REMOVEATTACHMENT, Z("remove"));
  b_remove_attachment->Enable(false);

  wxStaticLine *sl_options = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);

  st_name = new wxStaticText(this, wxID_STATIC, Z("Name:"));
  tc_name = new wxTextCtrl(this, ID_TC_ATTACHMENTNAME);
  tc_name->SetToolTip(TIP("This is the name that will be stored in the output file for this attachment. "
                          "It defaults to the file name of the original file but can be changed."));
  tc_name->SetSizeHints(0, -1);

  st_description = new wxStaticText(this, wxID_STATIC, Z("Description:"));
  tc_description = new wxTextCtrl(this, ID_TC_DESCRIPTION);
  tc_description->SetSizeHints(0, -1);

  st_mimetype    = new wxStaticText(this, wxID_STATIC, Z("MIME type:"));
  cob_mimetype   = new wxMTX_COMBOBOX_TYPE(this, ID_CB_MIMETYPE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN);
  cob_mimetype->SetToolTip(TIP("MIME type for this track. Select one of the pre-defined MIME types or enter one yourself."));
  cob_mimetype->Append(wxEmptyString);
  int i;
  for (i = 0; mime_types[i].name != NULL; i++)
    cob_mimetype->Append(wxU(mime_types[i].name));
  cob_mimetype->SetSizeHints(0, -1);

  st_style  = new wxStaticText(this, wxID_STATIC, Z("Attachment style:"));

  cob_style = new wxMTX_COMBOBOX_TYPE(this, ID_CB_ATTACHMENTSTYLE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY | wxCB_DROPDOWN);
  cob_style->Append(Z("To all files"));
  cob_style->Append(Z("Only to the first"));
  cob_style->SetToolTip(TIP("If splitting is a file can be attached either to all files created or only to the first file. Has no effect if no splitting is used."));
  cob_style->SetSizeHints(0, -1);

  // Create the layout.
  wxStaticBoxSizer *siz_box_attached_files = new wxStaticBoxSizer(sb_attached_files, wxVERTICAL);
  siz_box_attached_files->Add(clb_attached_files, 1, wxGROW | wxALL, 5);

  wxBoxSizer *siz_line = new wxBoxSizer(wxHORIZONTAL);
  siz_line->Add(lb_attachments, 1, wxGROW | wxALL, 5);

  wxBoxSizer *siz_buttons = new wxBoxSizer(wxVERTICAL);
  siz_buttons->Add(b_add_attachment, 0, wxALL, 5);
  siz_buttons->Add(b_remove_attachment, 0, wxALL, 5);
  siz_buttons->Add(5, 5, 0, wxGROW | wxALL, 5);

  siz_line->Add(siz_buttons, 0, wxGROW, 0);

  wxStaticBoxSizer *siz_box_attachments = new wxStaticBoxSizer(sb_attachments, wxVERTICAL);
  siz_box_attachments->Add(siz_line, 1, wxGROW, 0);

  siz_box_attachments->Add(sl_options, 0, wxGROW | wxALL, 5);

  wxFlexGridSizer *siz_fg = new wxFlexGridSizer(4, 5, 5);
  siz_fg->AddGrowableCol(1);
  siz_fg->AddGrowableCol(3);

  siz_fg->Add(st_name,        0, wxALIGN_CENTER_VERTICAL,          0);
  siz_fg->Add(tc_name,        1, wxALIGN_CENTER_VERTICAL | wxGROW, 0);
  siz_fg->Add(st_description, 0, wxALIGN_CENTER_VERTICAL,          0);
  siz_fg->Add(tc_description, 1, wxALIGN_CENTER_VERTICAL | wxGROW, 0);
  siz_fg->Add(st_mimetype,    0, wxALIGN_CENTER_VERTICAL,          0);
  siz_fg->Add(cob_mimetype,   1, wxALIGN_CENTER_VERTICAL | wxGROW, 0);
  siz_fg->Add(st_style,       0, wxALIGN_CENTER_VERTICAL,          0);
  siz_fg->Add(cob_style,      1, wxALIGN_CENTER_VERTICAL | wxGROW, 0);

  siz_box_attachments->Add(siz_fg, 0, wxALL | wxGROW, 5);

  wxBoxSizer *siz_all = new wxBoxSizer(wxVERTICAL);
  siz_all->Add(siz_box_attached_files, 1, wxALL | wxGROW, 5);
  siz_all->Add(siz_box_attachments,    1, wxALL | wxGROW, 5);

  SetSizer(siz_all);

  enable(false);
  selected_attachment = -1;

  t_get_entries.SetOwner(this, ID_T_ATTACHMENTVALUES);
  t_get_entries.Start(333);

  SetDropTarget(new attachments_drop_target_c(this));
}

void
tab_attachments::enable(bool e) {
  st_name->Enable(e);
  tc_name->Enable(e);
  st_description->Enable(e);
  tc_description->Enable(e);
  st_mimetype->Enable(e);
  cob_mimetype->Enable(e);
  st_style->Enable(e);
  cob_style->Enable(e);
}

void
tab_attachments::on_add_attachment(wxCommandEvent &evt) {
  wxFileDialog dlg(NULL, Z("Choose an attachment file"), last_open_dir, wxEmptyString, ALLFILES, wxFD_OPEN | wxFD_MULTIPLE);

  if(dlg.ShowModal() == wxID_OK) {
    wxArrayString selected_files;
    int i;

    last_open_dir = dlg.GetDirectory();
    dlg.GetPaths(selected_files);
    for (i = 0; i < selected_files.Count(); i++)
      add_attachment(selected_files[i]);
  }
}

void
tab_attachments::add_attachment(const wxString &file_name) {
  mmg_attachment_cptr attch = mmg_attachment_cptr(new mmg_attachment_t);

  attch->file_name  = file_name;
  wxString name     = file_name.AfterLast(wxT(PSEP));
  wxString ext      = name.AfterLast(wxT('.'));
  name             += wxString(wxT(" (")) + file_name.BeforeLast(wxT(PSEP)) + wxT(")");
  lb_attachments->Append(name);

  if (ext.Length() > 0)
    attch->mime_type = wxU(guess_mime_type(wxMB(file_name), true).c_str());
  attch->style       = 0;
  attch->stored_name = derive_stored_name_from_file_name(attch->file_name);

  attachments.push_back(attch);
}

void
tab_attachments::on_remove_attachment(wxCommandEvent &evt) {
  if (selected_attachment == -1)
    return;

  attachments.erase(attachments.begin() + selected_attachment);
  lb_attachments->Delete(selected_attachment);
  enable(false);
  b_remove_attachment->Enable(false);
  selected_attachment = -1;
}

void
tab_attachments::on_attachment_selected(wxCommandEvent &evt) {
  int new_sel;

  selected_attachment = -1;
  new_sel = lb_attachments->GetSelection();
  if (0 > new_sel)
    return;

  mmg_attachment_cptr &a = attachments[new_sel];
  tc_name->SetValue(a->stored_name);
  tc_description->SetValue(a->description);
  cob_mimetype->SetValue(a->mime_type);
  cob_style->SetSelection(a->style);
  enable(true);
  selected_attachment = new_sel;
  b_remove_attachment->Enable(true);
}

void
tab_attachments::on_name_changed(wxCommandEvent &evt) {
  if (selected_attachment == -1)
    return;

  attachments[selected_attachment]->stored_name = tc_name->GetValue();
}

void
tab_attachments::on_description_changed(wxCommandEvent &evt) {
  if (selected_attachment == -1)
    return;

  attachments[selected_attachment]->description = tc_description->GetValue();
}

void
tab_attachments::on_mimetype_changed(wxTimerEvent &evt) {
  if (selected_attachment == -1)
    return;

  attachments[selected_attachment]->mime_type = cob_mimetype->GetValue();
}

void
tab_attachments::on_style_changed(wxCommandEvent &evt) {
  if (selected_attachment == -1)
    return;

  attachments[selected_attachment]->style = cob_style->GetStringSelection().Find(wxT("Only")) >= 0 ? 1 : 0;
}

void
tab_attachments::save(wxConfigBase *cfg) {
  uint32_t i, j;
  wxString s;

  cfg->SetPath(wxT("/attachments"));
  cfg->Write(wxT("number_of_attachments"), (int)attachments.size());
  for (i = 0; i < attachments.size(); i++) {
    mmg_attachment_cptr &a = attachments[i];
    s.Printf(wxT("attachment %u"), i);
    cfg->SetPath(s);
    cfg->Write(wxT("stored_name"), a->stored_name);
    cfg->Write(wxT("file_name"), a->file_name);
    s = wxEmptyString;
    for (j = 0; j < a->description.Length(); j++)
      if (a->description[j] == wxT('\n'))
        s += wxT("!\\N!");
      else
        s += a->description[j];
    cfg->Write(wxT("description"), s);
    cfg->Write(wxT("mime_type"), a->mime_type);
    cfg->Write(wxT("style"), a->style);

    cfg->SetPath(wxT(".."));
  }
}

wxString
tab_attachments::derive_stored_name_from_file_name(const wxString &file_name) {
  return file_name.AfterLast(wxT('/')).AfterLast(wxT('\\'));
}

void
tab_attachments::load(wxConfigBase *cfg,
                      int version) {
  int num, i;

  enable(false);
  selected_attachment = -1;
  lb_attachments->Clear();
  b_remove_attachment->Enable(false);
  attachments.clear();
  clb_attached_files->Clear();

  cfg->SetPath(wxT("/attachments"));
  if (!cfg->Read(wxT("number_of_attachments"), &num) || (num < 0))
    return;

  for (i = 0; i < (uint32_t)num; i++) {
    mmg_attachment_cptr a = mmg_attachment_cptr(new mmg_attachment_t);
    wxString s, c;
    int pos;

    s.Printf(wxT("attachment %d"), i);
    cfg->SetPath(s);
    cfg->Read(wxT("file_name"), &a->file_name);
    if (!cfg->Read(wxT("stored_name"), &a->stored_name) ||
        (a->stored_name == wxEmptyString))
      a->stored_name = derive_stored_name_from_file_name(a->file_name);
    cfg->Read(wxT("description"), &s);
    cfg->Read(wxT("mime_type"), &a->mime_type);
    cfg->Read(wxT("style"), &a->style);
    if ((a->style != 0) && (a->style != 1))
      a->style = 0;
    pos = s.Find(wxT("!\\N!"));
    while (pos >= 0) {
      c = s.Mid(0, pos);
      s.Remove(0, pos + 4);
      a->description += c + wxT("\n");
      pos = s.Find(wxT("!\\N!"));
    }
    a->description += s;

    s = a->file_name.BeforeLast(PSEP);
    c = a->file_name.AfterLast(PSEP);
    lb_attachments->Append(c + wxT(" (") + s + wxT(")"));
    attachments.push_back(a);

    cfg->SetPath(wxT(".."));
  }

  unsigned int fidx;
  for (fidx = 0; files.size() > fidx; ++fidx) {
    mmg_file_cptr &f = files[fidx];

    unsigned int aidx;
    for (aidx = 0; f->attached_files.size() > aidx; ++aidx)
      add_attached_file(f->attached_files[aidx]);
  }
}

bool
tab_attachments::validate_settings() {
  unsigned int i;
  for (i = 0; i < attachments.size(); i++) {
    mmg_attachment_cptr &a = attachments[i];
    if (a->mime_type.Length() == 0) {
      wxMessageBox(wxString::Format(Z("No MIME type has been selected for the attachment '%s'."), a->file_name.c_str()), Z("Missing input"), wxOK | wxCENTER | wxICON_ERROR);
      return false;
    }
  }

  return true;
}

void
tab_attachments::add_attached_file(mmg_attached_file_cptr &a,
                                   bool update_column_widths) {
  wxFileName file_name(a->source->file_name);
  clb_attached_files->Append(wxString::Format(Z("%s (MIME type %s, size %ld) from %s (%s)"),
                                              a->name.c_str(), a->mime_type.c_str(), a->size, file_name.GetFullName().c_str(), file_name.GetPath().c_str()));
  clb_attached_files->Check(m_attached_files.size(), a->enabled);
  m_attached_files.push_back(a);
}

void
tab_attachments::remove_attached_files_for(mmg_file_cptr &f) {
  int i;
  for (i = m_attached_files.size() - 1; 0 <= i; --i) {
    if (m_attached_files[i]->source == f.get()) {
      clb_attached_files->Delete(i);
      m_attached_files.erase(m_attached_files.begin() + i);
    }
  }
}

void
tab_attachments::on_attached_file_enabled(wxCommandEvent &evt) {
  int idx = evt.GetSelection();
  if ((0 <= idx) && (m_attached_files.size() > idx))
    m_attached_files[idx]->enabled = clb_attached_files->IsChecked(idx);
}

IMPLEMENT_CLASS(tab_attachments, wxPanel);
BEGIN_EVENT_TABLE(tab_attachments, wxPanel)
  EVT_BUTTON(ID_B_ADDATTACHMENT,          tab_attachments::on_add_attachment)
  EVT_BUTTON(ID_B_REMOVEATTACHMENT,       tab_attachments::on_remove_attachment)
  EVT_LISTBOX(ID_LB_ATTACHMENTS,          tab_attachments::on_attachment_selected)
  EVT_TEXT(ID_TC_ATTACHMENTNAME,          tab_attachments::on_name_changed)
  EVT_TEXT(ID_TC_DESCRIPTION,             tab_attachments::on_description_changed)
  EVT_TIMER(ID_T_ATTACHMENTVALUES,        tab_attachments::on_mimetype_changed)
  EVT_COMBOBOX(ID_CB_ATTACHMENTSTYLE,     tab_attachments::on_style_changed)
  EVT_CHECKLISTBOX(ID_CLB_ATTACHED_FILES, tab_attachments::on_attached_file_enabled)
END_EVENT_TABLE();
