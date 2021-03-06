#pragma once

#include "common/common_pch.h"

#include <QString>
#include <QWidget>

class QStandardItem;

namespace mtx::gui::Info {

namespace Ui {
class Tab;
}

class Model;
class TabPrivate;
class Tab : public QWidget {
  Q_OBJECT

protected:
  MTX_DECLARE_PRIVATE(TabPrivate)

  std::unique_ptr<TabPrivate> const p_ptr;

  explicit Tab(TabPrivate &p);

public:
  explicit Tab(QWidget *parent);
  ~Tab();

  QString title() const;
  void load(QString const &fileName);
  void save();

Q_SIGNALS:
  void removeThisTab();
  void titleChanged();

public Q_SLOTS:
  void retranslateUi();
  void showError(const QString &message);
  void expandImportantElements();
  void readLevel1Element(QModelIndex const &idx);
  void showElementHexDumpInViewer();
  void showContextMenu(QPoint const &pos);

protected:
  void setItemsFromElement(QList<QStandardItem *> &items, libebml::EbmlElement &element);
};

}
