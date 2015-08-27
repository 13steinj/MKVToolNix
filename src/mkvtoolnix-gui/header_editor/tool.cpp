#include "common/common_pch.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

#include "common/qt.h"
#include "mkvtoolnix-gui/app.h"
#include "mkvtoolnix-gui/forms/header_editor/tool.h"
#include "mkvtoolnix-gui/forms/main_window/main_window.h"
#include "mkvtoolnix-gui/header_editor/tab.h"
#include "mkvtoolnix-gui/header_editor/tool.h"
#include "mkvtoolnix-gui/main_window/main_window.h"
#include "mkvtoolnix-gui/merge/mux_config.h"
#include "mkvtoolnix-gui/util/message_box.h"
#include "mkvtoolnix-gui/util/settings.h"
#include "mkvtoolnix-gui/util/widget.h"

namespace mtx { namespace gui { namespace HeaderEditor {

using namespace mtx::gui;

Tool::Tool(QWidget *parent,
           QMenu *headerEditorMenu)
  : ToolBase{parent}
  , ui{new Ui::Tool}
  , m_headerEditorMenu{headerEditorMenu}
  , m_filesDDHandler{Util::FilesDragDropHandler::Mode::Remember}
{
  // Setup UI controls.
  ui->setupUi(this);

  setupActions();
  setupTabPositions();

  showHeaderEditorsWidget();
}

Tool::~Tool() {
}

void
Tool::setupActions() {
  auto mw   = MainWindow::get();
  auto mwUi = MainWindow::getUi();

  connect(mwUi->actionHeaderEditorOpen,     &QAction::triggered,             this, &Tool::selectFileToOpen);
  connect(mwUi->actionHeaderEditorSave,     &QAction::triggered,             this, &Tool::save);
  connect(mwUi->actionHeaderEditorValidate, &QAction::triggered,             this, &Tool::validate);
  connect(mwUi->actionHeaderEditorReload,   &QAction::triggered,             this, &Tool::reload);
  connect(mwUi->actionHeaderEditorClose,    &QAction::triggered,             this, &Tool::closeCurrentTab);

  connect(ui->openFileButton,               &QPushButton::clicked,           this, &Tool::selectFileToOpen);

  connect(mw,                               &MainWindow::preferencesChanged, this, &Tool::setupTabPositions);

  connect(App::instance(),                  &App::editingHeadersRequested,   this, &Tool::openFilesFromCommandLine);
}

void
Tool::showHeaderEditorsWidget() {
  auto hasTabs = !!ui->editors->count();
  auto mwUi    = MainWindow::getUi();

  ui->stack->setCurrentWidget(hasTabs ? ui->editorsPage : ui->noFilesPage);

  mwUi->actionHeaderEditorSave->setEnabled(hasTabs);
  mwUi->actionHeaderEditorReload->setEnabled(hasTabs);
  mwUi->actionHeaderEditorValidate->setEnabled(hasTabs);
  mwUi->actionHeaderEditorClose->setEnabled(hasTabs);
}

void
Tool::toolShown() {
  MainWindow::get()->showTheseMenusOnly({ m_headerEditorMenu });
  showHeaderEditorsWidget();
}

void
Tool::retranslateUi() {
  ui->retranslateUi(this);
  for (auto idx = 0, numTabs = ui->editors->count(); idx < numTabs; ++idx) {
    static_cast<Tab *>(ui->editors->widget(idx))->retranslateUi();
    auto button = Util::tabWidgetCloseTabButton(*ui->editors, idx);
    if (button)
      button->setToolTip(App::translate("CloseButton", "Close Tab"));
  }
}

void
Tool::dragEnterEvent(QDragEnterEvent *event) {
  m_filesDDHandler.handle(event, false);
}

void
Tool::dropEvent(QDropEvent *event) {
  if (m_filesDDHandler.handle(event, true))
    openFiles(m_filesDDHandler.fileNames());
}

void
Tool::openFiles(QStringList const &fileNames) {
  for (auto const &file : fileNames)
    openFile(file);
}

void
Tool::openFilesFromCommandLine(QStringList const &fileNames) {
  MainWindow::get()->switchToTool(this);
  openFiles(fileNames);
}

void
Tool::openFile(QString const &fileName) {
  auto &settings         = Util::Settings::get();
  settings.m_lastOpenDir = QFileInfo{fileName}.path();
  settings.save();

  auto tab = new Tab{this, fileName};

  connect(tab, &Tab::removeThisTab, this, &Tool::closeSendingTab);
  ui->editors->addTab(tab, tab->title());

  showHeaderEditorsWidget();

  ui->editors->setCurrentIndex(ui->editors->count() - 1);

  tab->load();
}

void
Tool::selectFileToOpen() {
  auto fileNames = QFileDialog::getOpenFileNames(this, QY("Open files in header editor"), Util::Settings::get().m_lastOpenDir.path(),
                                                QY("Matroska and WebM files") + Q(" (*.mkv *.mka *.mks *.mk3d *.webm);;") + QY("All files") + Q(" (*)"));
  if (fileNames.isEmpty())
    return;

  MainWindow::get()->setStatusBarMessage(QNY("Opening %1 file in the header editor…", "Opening %1 files in the header editor…", fileNames.count()).arg(fileNames.count()));

  for (auto const &fileName : fileNames)
    openFile(fileName);
}

void
Tool::save() {
  auto tab = currentTab();
  if (tab)
    tab->save();
}

void
Tool::reload() {
  auto tab = currentTab();
  if (!tab)
    return;

  if (Util::Settings::get().m_warnBeforeClosingModifiedTabs && tab->hasBeenModified()) {
    auto answer = Util::MessageBox::question(this, QY("File has been modified"), QY("The file »%1« has been modified. Do you really want to reload it? All changes will be lost.").arg(QFileInfo{tab->fileName()}.fileName()),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
      return;
  }

  tab->load();
}

void
Tool::validate() {
  auto tab = currentTab();
  if (tab)
    tab->validate();
}

bool
Tool::closeTab(int index) {
  if ((0  > index) || (ui->editors->count() <= index))
    return false;

  auto tab = static_cast<Tab *>(ui->editors->widget(index));

  if (Util::Settings::get().m_warnBeforeClosingModifiedTabs && tab->hasBeenModified()) {
    MainWindow::get()->switchToTool(this);
    ui->editors->setCurrentIndex(index);
    auto answer = Util::MessageBox::question(this, QY("File has been modified"), QY("The file »%1« has been modified. Do you really want to close? All changes will be lost.").arg(QFileInfo{tab->fileName()}.fileName()),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
      return false;
  }

  ui->editors->removeTab(index);
  delete tab;

  showHeaderEditorsWidget();

  return true;
}

void
Tool::closeCurrentTab() {
  closeTab(ui->editors->currentIndex());
}

void
Tool::closeSendingTab() {
  auto toRemove = static_cast<Tab *>(sender());

  for (auto idx = 0, count = ui->editors->count(); idx < count; ++idx) {
    auto tab = static_cast<Tab *>(ui->editors->widget(idx));
    if (tab == toRemove) {
      closeTab(idx);
      return;
    }
  }
}

bool
Tool::closeAllTabs() {
  for (auto index = ui->editors->count(); index > 0; --index)
    if (!closeTab(index - 1))
      return false;

  return true;
}

Tab *
Tool::currentTab() {
  return static_cast<Tab *>(ui->editors->widget(ui->editors->currentIndex()));
}

void
Tool::setupTabPositions() {
  ui->editors->setTabPosition(Util::Settings::get().m_tabPosition);
}

}}}
