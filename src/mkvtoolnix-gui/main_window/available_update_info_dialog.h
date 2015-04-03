#ifndef MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_AVAILABLE_UPDATE_INFO_DIALOG_H
#define MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_AVAILABLE_UPDATE_INFO_DIALOG_H

#include "common/common_pch.h"

#if defined(HAVE_CURL_EASY_H)

# include <QDialog>

# include "common/xml/xml.h"
# include "mkvtoolnix-gui/main_window/update_check_thread.h"

namespace mtx { namespace gui {

namespace Ui {
class AvailableUpdateInfoDialog;
}

class AvailableUpdateInfoDialog : public QDialog {
  Q_OBJECT;

protected:
  // UI stuff:
  std::unique_ptr<Ui::AvailableUpdateInfoDialog> ui;
  std::shared_ptr<pugi::xml_document> m_releasesInfo;
  QString m_downloadURL;

public:
  explicit AvailableUpdateInfoDialog(QWidget *parent);
  ~AvailableUpdateInfoDialog();

  void setChangeLogContent(QString const &content);

public slots:
  void setReleaseInformation(std::shared_ptr<pugi::xml_document> releasesInfo);
  void updateCheckFinished(UpdateCheckStatus status, mtx_release_version_t releaseVersion);
  void visitDownloadLocation();
};

}}

#endif  // HAVE_CURL_EASY_H
#endif  // MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_AVAILABLE_UPDATE_INFO_DIALOG_H
