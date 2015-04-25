#ifndef MTX_MKVTOOLNIX_GUI_CHAPTER_EDITOR_TAB_H
#define MTX_MKVTOOLNIX_GUI_CHAPTER_EDITOR_TAB_H

#include "common/common_pch.h"

#include <QDateTime>
#include <QModelIndex>

#include "common/qt_kax_analyzer.h"
#include "mkvtoolnix-gui/chapter_editor/chapter_model.h"

class QAction;
class QItemSelection;

using ChaptersPtr = std::shared_ptr<KaxChapters>;

namespace libebml {
class EbmlBinary;
};

namespace mtx { namespace gui { namespace ChapterEditor {

namespace Ui {
class Tab;
}

class ChapterModel;
class NameModel;

class Tab : public QWidget {
  Q_OBJECT;

protected:
  using ValidationResult = std::pair<bool, QString>;

protected:
  // UI stuff:
  std::unique_ptr<Ui::Tab> ui;

  QString m_fileName;
  std::unique_ptr<QtKaxAnalyzer> m_analyzer;
  QDateTime m_fileModificationTime;

  ChapterModel *m_chapterModel;
  NameModel *m_nameModel;

  QAction *m_expandAllAction, *m_collapseAllAction, *m_addEditionBeforeAction, *m_addEditionAfterAction, *m_addChapterBeforeAction, *m_addChapterAfterAction, *m_addSubChapterAction, *m_removeElementAction;
  QAction *m_duplicateAction, *m_massModificationAction;
  QList<QWidget *> m_nameWidgets;

  bool m_ignoreChapterSelectionChanges{};

public:
  explicit Tab(QWidget *parent, QString const &fileName = QString{});
  ~Tab();

  virtual void retranslateUi();
  virtual QString const &fileName() const;
  virtual QString title() const;
  virtual bool hasChapters() const;

signals:
  void removeThisTab();
  void titleChanged();
  void numberOfEntriesChanged();

public slots:
  virtual void newFile();
  virtual void load();
  virtual void save();
  virtual void saveAsXml();
  virtual void saveToMatroska();
  virtual void expandAll();
  virtual void collapseAll();
  virtual void addEditionBefore();
  virtual void addEditionAfter();
  virtual void addChapterBefore();
  virtual void addChapterAfter();
  virtual void addSubChapter();
  virtual void removeElement();
  virtual void duplicateElement();
  virtual void massModify();

  virtual void chapterSelectionChanged(QItemSelection const &selected, QItemSelection const &deselected);
  virtual void expandInsertedElements(QModelIndex const &parentIdx, int start, int end);

  virtual void nameSelectionChanged(QItemSelection const &selected, QItemSelection const &deselected);
  virtual void chapterNameEdited(QString const &text);
  virtual void chapterNameLanguageChanged(int index);
  virtual void chapterNameCountryChanged(int index);
  virtual void addChapterName();
  virtual void removeChapterName();

  virtual void showChapterContextMenu(QPoint const &pos);

protected:
  void setupUi();
  void resetData();
  void expandCollapseAll(bool expand, QModelIndex const &parentIdx = {});

  ChaptersPtr loadFromChapterFile();
  ChaptersPtr loadFromMatroskaFile();

  void resizeChapterColumnsToContents() const;
  void resizeNameColumnsToContents() const;

  bool copyControlsToStorage();
  bool copyControlsToStorage(QModelIndex const &idx);
  ValidationResult copyControlsToStorageImpl(QModelIndex const &idx);
  ValidationResult copyChapterControlsToStorage(ChapterPtr const &chapter);
  ValidationResult copyEditionControlsToStorage(EditionPtr const &edition);

  bool setControlsFromStorage();
  bool setControlsFromStorage(QModelIndex const &idx);
  bool setChapterControlsFromStorage(ChapterPtr const &chapter);
  bool setEditionControlsFromStorage(EditionPtr const &edition);

  bool setNameControlsFromStorage(QModelIndex const &idx);
  void enableNameWidgets(bool enable);

  void withSelectedName(std::function<void(QModelIndex const &, KaxChapterDisplay &)> const &worker);

  void selectChapterRow(QModelIndex const &idx, bool ignoreSelectionChanges);
  bool handleChapterDeselection(QItemSelection const &deselected);

  void addEdition(bool before);
  void addChapter(bool before);

  ChapterPtr createEmptyChapter(int64_t startTime);

  void saveAsImpl(bool requireNewFileName, std::function<bool(bool, QString &)> const &worker);
  void saveAsXmlImpl(bool requireNewFileName);
  void saveToMatroskaImpl(bool requireNewFileName);
  void updateFileNameDisplay();

  void shiftTimecodes(QStandardItem *item, int64_t delta);
  void constrictTimecodes(QStandardItem *item, boost::optional<uint64_t> const &constrictStart, boost::optional<uint64_t> const &constrictEnd);
  std::pair<boost::optional<uint64_t>, boost::optional<uint64_t>> expandTimecodes(QStandardItem *item);
  void setLanguages(QStandardItem *item, QString const &language);
  void setCountries(QStandardItem *item, QString const &country);

protected:
  static QString formatEbmlBinary(EbmlBinary *binary);
};

}}}

#endif // MTX_MKVTOOLNIX_GUI_CHAPTER_EDITOR_TAB_H
