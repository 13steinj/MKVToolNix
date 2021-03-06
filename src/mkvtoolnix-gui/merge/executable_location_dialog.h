#pragma once

#include "common/common_pch.h"

#include <QDialog>

#include "mkvtoolnix-gui/merge/source_file.h"
#include "mkvtoolnix-gui/util/settings.h"

namespace mtx::gui::Merge {

namespace Ui {
class ExecutableLocationDialog;
}

class ExecutableLocationDialog : public QDialog {
  Q_OBJECT
protected:
  std::unique_ptr<Ui::ExecutableLocationDialog> m_ui;

public:
  explicit ExecutableLocationDialog(QWidget *parent, QString const &executable = {});
  ~ExecutableLocationDialog();

  ExecutableLocationDialog &setInfo(QString const &title, QString const &text);
  ExecutableLocationDialog &setURL(QString const &url);

  QString executable() const;

public Q_SLOTS:
  void browse();
};

}
