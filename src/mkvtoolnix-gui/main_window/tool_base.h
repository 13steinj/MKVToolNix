#ifndef MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_TOOL_BASE_H
#define MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_TOOL_BASE_H

#include "common/common_pch.h"

#include <QWidget>

namespace mtx { namespace gui {

class ToolBase : public QWidget {
  Q_OBJECT;

public:
  explicit ToolBase(QWidget *parent) : QWidget{parent} {}
  virtual ~ToolBase() {};

  virtual void setupUi() = 0;
  virtual void setupActions() = 0;

public slots:
  virtual void toolShown() = 0;
};

}}

#endif // MTX_MKVTOOLNIX_GUI_MAIN_WINDOW_TOOL_BASE_H
