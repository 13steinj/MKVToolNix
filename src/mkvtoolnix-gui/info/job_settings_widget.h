#pragma once

#include "common/common_pch.h"

#include <QString>
#include <QWidget>

namespace mtx::gui::Info {

namespace Ui {
class JobSettingsWidget;
}

class JobSettings;
class JobSettingsWidgetPrivate;
class JobSettingsWidget : public QWidget {
  Q_OBJECT

protected:
  MTX_DECLARE_PRIVATE(JobSettingsWidgetPrivate)

  std::unique_ptr<JobSettingsWidgetPrivate> const p_ptr;

  explicit JobSettingsWidget(JobSettingsWidgetPrivate &p);

public:
  explicit JobSettingsWidget(QWidget *parent);
  ~JobSettingsWidget();

  JobSettings settings();
  void setSettings(JobSettings const &jobSettings);
  void setFileNameVisible(bool visible);

Q_SIGNALS:
  void fileNameChanged(QString const &fileName);

public Q_SLOTS:
  void enableControlsAccordingToMode();
  void browseFileName();
  void emitFileNameChangeSignal(QString const &fileName);
};

}
