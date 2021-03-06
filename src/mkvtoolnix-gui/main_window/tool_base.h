#pragma once

#include "common/common_pch.h"

#include <QWidget>

namespace mtx::gui {

class ToolBase : public QWidget {
  Q_OBJECT

public:
  explicit ToolBase(QWidget *parent) : QWidget{parent} {}
  virtual ~ToolBase() {};

  virtual void setupUi() = 0;
  virtual void setupActions() = 0;
  virtual std::pair<QString, QString> nextPreviousWindowActionTexts() const { return {}; };

public Q_SLOTS:
  virtual void toolShown() = 0;
};

}
