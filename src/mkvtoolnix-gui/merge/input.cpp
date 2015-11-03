#include "common/common_pch.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QString>
#include <QTimer>

#include "common/extern_data.h"
#include "common/iso639.h"
#include "common/logger.h"
#include "common/qt.h"
#include "common/stereo_mode.h"
#include "mkvtoolnix-gui/app.h"
#include "mkvtoolnix-gui/forms/merge/tab.h"
#include "mkvtoolnix-gui/main_window/main_window.h"
#include "mkvtoolnix-gui/main_window/select_character_set_dialog.h"
#include "mkvtoolnix-gui/merge/adding_appending_files_dialog.h"
#include "mkvtoolnix-gui/merge/executable_location_dialog.h"
#include "mkvtoolnix-gui/merge/tab.h"
#include "mkvtoolnix-gui/merge/tool.h"
#include "mkvtoolnix-gui/merge/playlist_scanner.h"
#include "mkvtoolnix-gui/util/file_identifier.h"
#include "mkvtoolnix-gui/util/file_dialog.h"
#include "mkvtoolnix-gui/util/file_type_filter.h"
#include "mkvtoolnix-gui/util/header_view_manager.h"
#include "mkvtoolnix-gui/util/message_box.h"
#include "mkvtoolnix-gui/util/model.h"
#include "mkvtoolnix-gui/util/settings.h"
#include "mkvtoolnix-gui/util/widget.h"

namespace mtx { namespace gui { namespace Merge {

using namespace mtx::gui;

void
Tab::setupControlLists() {
  m_typeIndependantControls << ui->muxThisLabel << ui->muxThis << ui->miscellaneousBox << ui->additionalTrackOptionsLabel << ui->additionalTrackOptions;

  m_audioControls << ui->trackNameLabel << ui->trackName << ui->trackLanguageLabel << ui->trackLanguage << ui->defaultTrackFlagLabel << ui->defaultTrackFlag << ui->forcedTrackFlagLabel << ui->forcedTrackFlag
                  << ui->compressionLabel << ui->compression << ui->trackTagsLabel << ui->trackTags << ui->browseTrackTags << ui->timecodesAndDefaultDurationBox
                  << ui->delayLabel << ui->delay << ui->stretchByLabel << ui->stretchBy << ui->timecodesLabel << ui->timecodes << ui->browseTimecodes << ui->audioPropertiesBox << ui->aacIsSBR << ui->cuesLabel << ui->cues
                  << ui->propertiesLabel << ui->generalOptionsBox << ui->reduceToAudioCore;

  m_videoControls << ui->trackNameLabel << ui->trackName << ui->trackLanguageLabel << ui->trackLanguage << ui->defaultTrackFlagLabel << ui->defaultTrackFlag
                  << ui->forcedTrackFlagLabel << ui->forcedTrackFlag << ui->compressionLabel << ui->compression << ui->trackTagsLabel << ui->trackTags << ui->browseTrackTags << ui->timecodesAndDefaultDurationBox
                  << ui->delayLabel << ui->delay << ui->stretchByLabel << ui->stretchBy << ui->defaultDurationLabel << ui->defaultDuration << ui->timecodesLabel << ui->timecodes << ui->browseTimecodes
                  << ui->videoPropertiesBox << ui->setAspectRatio << ui->aspectRatio << ui->setDisplayWidthHeight << ui->displayWidth << ui->displayDimensionsXLabel << ui->displayHeight << ui->stereoscopyLabel
                  << ui->stereoscopy << ui->naluSizeLengthLabel << ui->naluSizeLength << ui->croppingLabel << ui->cropping << ui->cuesLabel << ui->cues
                  << ui->propertiesLabel << ui->generalOptionsBox << ui->fixBitstreamTimingInfo;

  m_subtitleControls << ui->trackNameLabel << ui->trackName << ui->trackLanguageLabel << ui->trackLanguage << ui->defaultTrackFlagLabel << ui->defaultTrackFlag
                     << ui->forcedTrackFlagLabel << ui->forcedTrackFlag << ui->compressionLabel << ui->compression << ui->trackTagsLabel << ui->trackTags << ui->browseTrackTags << ui->timecodesAndDefaultDurationBox
                     << ui->delayLabel << ui->delay << ui->stretchByLabel << ui->stretchBy << ui->timecodesLabel << ui->timecodes << ui->browseTimecodes
                     << ui->subtitleAndChapterPropertiesBox << ui->characterSetLabel << ui->subtitleCharacterSet << ui->cuesLabel << ui->cues
                     << ui->propertiesLabel << ui->generalOptionsBox;

  m_chapterControls << ui->subtitleAndChapterPropertiesBox << ui->characterSetLabel << ui->subtitleCharacterSet << ui->propertiesLabel << ui->generalOptionsBox;

  m_allInputControls << ui->muxThisLabel << ui->muxThis << ui->trackNameLabel << ui->trackName << ui->trackLanguageLabel << ui->trackLanguage << ui->defaultTrackFlagLabel << ui->defaultTrackFlag
                     << ui->forcedTrackFlagLabel << ui->forcedTrackFlag << ui->compressionLabel << ui->compression << ui->trackTagsLabel << ui->trackTags << ui->browseTrackTags << ui->timecodesAndDefaultDurationBox
                     << ui->delayLabel << ui->delay << ui->stretchByLabel << ui->stretchBy << ui->defaultDurationLabel << ui->defaultDuration << ui->timecodesLabel << ui->timecodes << ui->browseTimecodes
                     << ui->videoPropertiesBox << ui->setAspectRatio << ui->aspectRatio << ui->setDisplayWidthHeight << ui->displayWidth << ui->displayDimensionsXLabel << ui->displayHeight << ui->stereoscopyLabel
                     << ui->stereoscopy << ui->croppingLabel << ui->cropping << ui->audioPropertiesBox << ui->aacIsSBR << ui->subtitleAndChapterPropertiesBox << ui->characterSetLabel << ui->subtitleCharacterSet
                     << ui->miscellaneousBox << ui->cuesLabel << ui->cues << ui->additionalTrackOptionsLabel << ui->additionalTrackOptions
                     << ui->propertiesLabel << ui->generalOptionsBox << ui->fixBitstreamTimingInfo << ui->reduceToAudioCore << ui->naluSizeLengthLabel << ui->naluSizeLength;

  m_comboBoxControls << ui->muxThis << ui->trackLanguage << ui->defaultTrackFlag << ui->forcedTrackFlag << ui->compression << ui->cues << ui->stereoscopy << ui->naluSizeLength << ui->aacIsSBR << ui->subtitleCharacterSet;

  m_notIfAppendingControls << ui->trackLanguageLabel   << ui->trackLanguage           << ui->trackNameLabel              << ui->trackName        << ui->defaultTrackFlagLabel << ui->defaultTrackFlag
                           << ui->forcedTrackFlagLabel << ui->forcedTrackFlag         << ui->compressionLabel            << ui->compression      << ui->trackTagsLabel        << ui->trackTags         << ui->browseTrackTags
                           << ui->defaultDurationLabel << ui->defaultDuration         << ui->fixBitstreamTimingInfo      << ui->setAspectRatio   << ui->setDisplayWidthHeight << ui->aspectRatio
                           << ui->displayWidth         << ui->displayDimensionsXLabel << ui->displayHeight               << ui->stereoscopyLabel << ui->stereoscopy
                           << ui->naluSizeLengthLabel  << ui->naluSizeLength          << ui->croppingLabel               << ui->cropping         << ui->aacIsSBR              << ui->characterSetLabel << ui->subtitleCharacterSet
                           << ui->cuesLabel            << ui->cues                    << ui->additionalTrackOptionsLabel << ui->additionalTrackOptions;
}

void
Tab::setupMoveUpDownButtons() {
  auto show    = Util::Settings::get().m_showMoveUpDownButtons;
  auto widgets = QList<QWidget *>{} << ui->moveFilesButtons << ui->moveTracksButtons << ui->moveAttachmentsButtons;

  for (auto const &widget : widgets)
    widget->setVisible(show);
}

void
Tab::setupInputLayout() {
  if (Util::Settings::get().m_mergeUseVerticalInputLayout)
    setupVerticalInputLayout();
  else
    setupHorizontalInputLayout();
}

void
Tab::setupHorizontalInputLayout() {
  if (ui->wProperties->isVisible())
    return;

  ui->twProperties->hide();
  ui->wProperties->show();

  auto layout  = qobject_cast<QBoxLayout *>(ui->scrollAreaWidgetContents->layout());

  Q_ASSERT(!!layout);

  ui->generalOptionsBox              ->setParent(ui->scrollAreaWidgetContents);
  ui->timecodesAndDefaultDurationBox ->setParent(ui->scrollAreaWidgetContents);
  ui->videoPropertiesBox             ->setParent(ui->scrollAreaWidgetContents);
  ui->audioPropertiesBox             ->setParent(ui->scrollAreaWidgetContents);
  ui->subtitleAndChapterPropertiesBox->setParent(ui->scrollAreaWidgetContents);
  ui->miscellaneousBox               ->setParent(ui->scrollAreaWidgetContents);

  layout->insertWidget(0, ui->generalOptionsBox);
  layout->insertWidget(1, ui->timecodesAndDefaultDurationBox);
  layout->insertWidget(2, ui->videoPropertiesBox);
  layout->insertWidget(3, ui->audioPropertiesBox);
  layout->insertWidget(4, ui->subtitleAndChapterPropertiesBox);
  layout->insertWidget(5, ui->miscellaneousBox);
}

void
Tab::setupVerticalInputLayout() {
  if (ui->twProperties->isVisible())
    return;

  ui->twProperties->show();
  ui->wProperties->hide();

  auto moveTo = [this](QWidget *page, int position, QWidget *widget) {
    auto layout = qobject_cast<QBoxLayout *>(page->layout());
    Q_ASSERT(!!layout);

    widget->setParent(page);
    layout->insertWidget(position, widget);
  };

  moveTo(ui->generalOptionsPage,                 0, ui->generalOptionsBox);
  moveTo(ui->timecodesAndDefaultDurationPage,    0, ui->timecodesAndDefaultDurationBox);
  moveTo(ui->videoPropertiesPage,                0, ui->videoPropertiesBox);
  moveTo(ui->audioSubtitleChapterPropertiesPage, 0, ui->audioPropertiesBox);
  moveTo(ui->audioSubtitleChapterPropertiesPage, 1, ui->subtitleAndChapterPropertiesBox);
  moveTo(ui->miscellaneousPage,                  0, ui->miscellaneousBox);
}

void
Tab::setupInputControls() {
  auto &cfg = Util::Settings::get();

  ui->twProperties->hide();

  setupControlLists();
  setupMoveUpDownButtons();
  setupInputLayout();

  ui->files->setModel(m_filesModel);
  ui->tracks->setModel(m_tracksModel);
  ui->tracks->enterActivatesAllSelected(true);

  cfg.handleSplitterSizes(ui->mergeInputSplitter);
  cfg.handleSplitterSizes(ui->mergeFilesTracksSplitter);

  // Track & chapter language
  ui->trackLanguage->setup();
  ui->chapterLanguage->setup(true);

  // Track & chapter character set
  ui->subtitleCharacterSet->setup(true);
  ui->chapterCharacterSet->setup(true);

  ui->muxThis->addItem(QString{}, true);
  ui->muxThis->addItem(QString{}, false);

  // Stereoscopy
  ui->stereoscopy->addItem(Q(""), 0);
  for (auto idx = 0u, end = stereo_mode_c::max_index(); idx <= end; ++idx)
    ui->stereoscopy->addItem(QString{}, idx + 1);

  // NALU size length
  for (auto idx = 0; idx < 3; ++idx)
    ui->naluSizeLength->addItem(QString{}, idx * 2);

  for (auto idx = 0; idx < 3; ++idx)
    ui->defaultTrackFlag->addItem(QString{}, idx);

  // Originally the "forced track" flag's options where ordered "off,
  // on"; now they're ordered "yes, no" for consistency with other
  // flags, requiring the values to be reversed.
  ui->forcedTrackFlag->addItem(QString{}, 1);
  ui->forcedTrackFlag->addItem(QString{}, 0);
  ui->forcedTrackFlag->setCurrentIndex(1);

  for (auto idx = 0; idx < 3; ++idx)
    ui->compression->addItem(QString{}, idx);

  for (auto idx = 0; idx < 4; ++idx)
    ui->cues->addItem(QString{}, idx);

  for (auto idx = 0; idx < 3; ++idx)
    ui->aacIsSBR->addItem(QString{}, idx);

  for (auto const &control : m_comboBoxControls) {
    control->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    Util::fixComboBoxViewWidth(*control);
  }

  // "files" context menu
  m_filesMenu->addAction(m_addFilesAction);
  m_filesMenu->addAction(m_appendFilesAction);
  m_filesMenu->addAction(m_addAdditionalPartsAction);
  m_filesMenu->addSeparator();
  m_filesMenu->addAction(m_removeFilesAction);
  m_filesMenu->addAction(m_removeAllFilesAction);
  m_filesMenu->addSeparator();
  m_filesMenu->addAction(m_openFilesInMediaInfoAction);

  // "tracks" context menu
  m_tracksMenu->addAction(m_selectAllTracksAction);
  m_tracksMenu->addMenu(m_selectTracksOfTypeMenu);
  m_tracksMenu->addAction(m_enableAllTracksAction);
  m_tracksMenu->addAction(m_disableAllTracksAction);
  m_tracksMenu->addSeparator();
  m_tracksMenu->addAction(m_openTracksInMediaInfoAction);

  m_selectTracksOfTypeMenu->addAction(m_selectAllVideoTracksAction);
  m_selectTracksOfTypeMenu->addAction(m_selectAllAudioTracksAction);
  m_selectTracksOfTypeMenu->addAction(m_selectAllSubtitlesTracksAction);

  // Connect signals & slots.
  auto mw = MainWindow::get();

  connect(ui->aacIsSBR,                     static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onAacIsSBRChanged);
  connect(ui->addFiles,                     &QPushButton::clicked,                                                                            this,                     &Tab::onAddFiles);
  connect(ui->addToJobQueue,                &QPushButton::clicked,                                                                            this,                     &Tab::onAddToJobQueue);
  connect(ui->additionalTrackOptions,       &QLineEdit::textChanged,                                                                          this,                     &Tab::onAdditionalTrackOptionsChanged);
  connect(ui->aspectRatio,                  static_cast<void (QComboBox::*)(QString const &)>(&QComboBox::currentIndexChanged),               this,                     &Tab::onAspectRatioChanged);
  connect(ui->aspectRatio,                  &QComboBox::editTextChanged,                                                                      this,                     &Tab::onAspectRatioChanged);
  connect(ui->browseTimecodes,              &QPushButton::clicked,                                                                            this,                     &Tab::onBrowseTimecodes);
  connect(ui->browseTrackTags,              &QPushButton::clicked,                                                                            this,                     &Tab::onBrowseTrackTags);
  connect(ui->compression,                  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onCompressionChanged);
  connect(ui->cropping,                     &QLineEdit::textChanged,                                                                          this,                     &Tab::onCroppingChanged);
  connect(ui->cues,                         static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onCuesChanged);
  connect(ui->defaultDuration,              static_cast<void (QComboBox::*)(QString const &)>(&QComboBox::currentIndexChanged),               this,                     &Tab::onDefaultDurationChanged);
  connect(ui->defaultDuration,              &QComboBox::editTextChanged,                                                                      this,                     &Tab::onDefaultDurationChanged);
  connect(ui->defaultTrackFlag,             static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onDefaultTrackFlagChanged);
  connect(ui->delay,                        &QLineEdit::textChanged,                                                                          this,                     &Tab::onDelayChanged);
  connect(ui->displayHeight,                &QLineEdit::textChanged,                                                                          this,                     &Tab::onDisplayHeightChanged);
  connect(ui->displayWidth,                 &QLineEdit::textChanged,                                                                          this,                     &Tab::onDisplayWidthChanged);
  connect(ui->editAdditionalOptions,        &QPushButton::clicked,                                                                            this,                     &Tab::onEditAdditionalOptions);
  connect(ui->files,                        &Util::BasicTreeView::ctrlDownPressed,                                                            this,                     &Tab::onMoveFilesDown);
  connect(ui->files,                        &Util::BasicTreeView::ctrlUpPressed,                                                              this,                     &Tab::onMoveFilesUp);
  connect(ui->files,                        &Util::BasicTreeView::customContextMenuRequested,                                                 this,                     &Tab::showFilesContextMenu);
  connect(ui->files,                        &Util::BasicTreeView::deletePressed,                                                              this,                     &Tab::onRemoveFiles);
  connect(ui->files,                        &Util::BasicTreeView::filesDropped,                                                               this,                     &Tab::addOrAppendDroppedFiles);
  connect(ui->files->selectionModel(),      &QItemSelectionModel::selectionChanged,                                                           m_filesModel,             &SourceFileModel::updateSelectionStatus);
  connect(ui->files->selectionModel(),      &QItemSelectionModel::selectionChanged,                                                           this,                     &Tab::enableMoveFilesButtons);
  connect(ui->fixBitstreamTimingInfo,       &QCheckBox::toggled,                                                                              this,                     &Tab::onFixBitstreamTimingInfoChanged);
  connect(ui->forcedTrackFlag,              static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onForcedTrackFlagChanged);
  connect(ui->moveFilesDown,                &QPushButton::clicked,                                                                            this,                     &Tab::onMoveFilesDown);
  connect(ui->moveFilesUp,                  &QPushButton::clicked,                                                                            this,                     &Tab::onMoveFilesUp);
  connect(ui->moveTracksDown,               &QPushButton::clicked,                                                                            this,                     &Tab::onMoveTracksDown);
  connect(ui->moveTracksUp,                 &QPushButton::clicked,                                                                            this,                     &Tab::onMoveTracksUp);
  connect(ui->muxThis,                      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onMuxThisChanged);
  connect(ui->naluSizeLength,               static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onNaluSizeLengthChanged);
  connect(ui->reduceToAudioCore,            &QCheckBox::toggled,                                                                              this,                     &Tab::onReduceAudioToCoreChanged);
  connect(ui->setAspectRatio,               &QPushButton::clicked,                                                                            this,                     &Tab::onSetAspectRatio);
  connect(ui->setDisplayWidthHeight,        &QPushButton::clicked,                                                                            this,                     &Tab::onSetDisplayDimensions);
  connect(ui->startMuxing,                  &QPushButton::clicked,                                                                            this,                     &Tab::onStartMuxing);
  connect(ui->stereoscopy,                  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onStereoscopyChanged);
  connect(ui->stretchBy,                    &QLineEdit::textChanged,                                                                          this,                     &Tab::onStretchByChanged);
  connect(ui->subtitleCharacterSet,         static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),                           this,                     &Tab::onSubtitleCharacterSetChanged);
  connect(ui->subtitleCharacterSetPreview,  &QPushButton::clicked,                                                                            this,                     &Tab::onPreviewSubtitleCharacterSet);
  connect(ui->timecodes,                    &QLineEdit::textChanged,                                                                          this,                     &Tab::onTimecodesChanged);
  connect(ui->trackLanguage,                static_cast<void (Util::LanguageComboBox::*)(int)>(&Util::LanguageComboBox::currentIndexChanged), this,                     &Tab::onTrackLanguageChanged);
  connect(ui->trackName,                    &QLineEdit::textChanged,                                                                          this,                     &Tab::onTrackNameChanged);
  connect(ui->trackTags,                    &QLineEdit::textChanged,                                                                          this,                     &Tab::onTrackTagsChanged);
  connect(ui->tracks,                       &QTreeView::doubleClicked,                                                                        this,                     &Tab::toggleMuxThisForSelectedTracks);
  connect(ui->tracks,                       &Util::BasicTreeView::allSelectedActivated,                                                       this,                     &Tab::toggleMuxThisForSelectedTracks);
  connect(ui->tracks,                       &Util::BasicTreeView::ctrlDownPressed,                                                            this,                     &Tab::onMoveTracksDown);
  connect(ui->tracks,                       &Util::BasicTreeView::ctrlUpPressed,                                                              this,                     &Tab::onMoveTracksUp);
  connect(ui->tracks,                       &Util::BasicTreeView::customContextMenuRequested,                                                 this,                     &Tab::showTracksContextMenu);
  connect(ui->tracks->selectionModel(),     &QItemSelectionModel::selectionChanged,                                                           m_tracksModel,            &TrackModel::updateSelectionStatus);
  connect(ui->tracks->selectionModel(),     &QItemSelectionModel::selectionChanged,                                                           this,                     &Tab::onTrackSelectionChanged);

  connect(m_addFilesAction,                 &QAction::triggered,                                                                              this,                     &Tab::onAddFiles);
  connect(m_appendFilesAction,              &QAction::triggered,                                                                              this,                     &Tab::onAppendFiles);
  connect(m_addAdditionalPartsAction,       &QAction::triggered,                                                                              this,                     &Tab::onAddAdditionalParts);
  connect(m_removeFilesAction,              &QAction::triggered,                                                                              this,                     &Tab::onRemoveFiles);
  connect(m_removeAllFilesAction,           &QAction::triggered,                                                                              this,                     &Tab::onRemoveAllFiles);
  connect(m_openFilesInMediaInfoAction,     &QAction::triggered,                                                                              this,                     &Tab::onOpenFilesInMediaInfo);
  connect(m_openTracksInMediaInfoAction,    &QAction::triggered,                                                                              this,                     &Tab::onOpenTracksInMediaInfo);

  connect(m_selectAllTracksAction,          &QAction::triggered,                                                                              this,                     &Tab::selectAllTracks);
  connect(m_selectAllVideoTracksAction,     &QAction::triggered,                                                                              this,                     &Tab::selectAllVideoTracks);
  connect(m_selectAllAudioTracksAction,     &QAction::triggered,                                                                              this,                     &Tab::selectAllAudioTracks);
  connect(m_selectAllSubtitlesTracksAction, &QAction::triggered,                                                                              this,                     &Tab::selectAllSubtitlesTracks);
  connect(m_enableAllTracksAction,          &QAction::triggered,                                                                              this,                     &Tab::enableAllTracks);
  connect(m_disableAllTracksAction,         &QAction::triggered,                                                                              this,                     &Tab::disableAllTracks);

  connect(m_filesModel,                     &SourceFileModel::rowsInserted,                                                                   this,                     &Tab::onFileRowsInserted);
  connect(m_tracksModel,                    &TrackModel::rowsInserted,                                                                        this,                     &Tab::onTrackRowsInserted);
  connect(m_tracksModel,                    &TrackModel::itemChanged,                                                                         this,                     &Tab::onTrackItemChanged);

  connect(mw,                               &MainWindow::preferencesChanged,                                                                  this,                     &Tab::setupMoveUpDownButtons);
  connect(mw,                               &MainWindow::preferencesChanged,                                                                  this,                     &Tab::setupInputLayout);
  connect(mw,                               &MainWindow::preferencesChanged,                                                                  ui->trackLanguage,        &Util::ComboBoxBase::reInitialize);
  connect(mw,                               &MainWindow::preferencesChanged,                                                                  ui->chapterLanguage,      &Util::ComboBoxBase::reInitialize);
  connect(mw,                               &MainWindow::preferencesChanged,                                                                  ui->subtitleCharacterSet, &Util::ComboBoxBase::reInitialize);
  connect(mw,                               &MainWindow::preferencesChanged,                                                                  ui->chapterCharacterSet,  &Util::ComboBoxBase::reInitialize);

  enableMoveFilesButtons();
  onTrackSelectionChanged();

  Util::HeaderViewManager::create(*ui->files,  "Merge::Files");
  Util::HeaderViewManager::create(*ui->tracks, "Merge::Tracks");
}

void
Tab::setupInputToolTips() {
  Util::setToolTip(ui->files,     QY("Right-click to add, append and remove files"));
  Util::setToolTip(ui->tracks,    QY("Right-click for actions for all tracks"));

  Util::setToolTip(ui->muxThis,   QY("If set to 'no' then the selected tracks will not be copied to the output file."));
  Util::setToolTip(ui->trackName, QY("A name for this track that players can display helping the user chose the right track to play, e.g. \"director's comments\"."));
  Util::setToolTip(ui->trackLanguage,
                   Q("%1 %2")
                   .arg(QY("The language for this track that players can use for automatic track selection and display for the user."))
                   .arg(QY("Select one of the ISO639-2 language codes.")));
  Util::setToolTip(ui->defaultTrackFlag,
                   Q("%1 %2 %3")
                   .arg(QY("Make this track the default track for its type (audio, video, subtitles)."))
                   .arg(QY("Players should prefer tracks with the default track flag set."))
                   .arg(QY("If set to 'determine automatically' then mkvmerge will choose one track of each type to have this flag set based on the information in the source files and the order of the tracks.")));
  Util::setToolTip(ui->forcedTrackFlag,
                   Q("%1 %2")
                   .arg(QY("Mark this track as 'forced'."))
                   .arg(QY("Players must play this track.")));
  Util::setToolTip(ui->compression,
                   Q("%1 %2 %3")
                   .arg(QY("Sets the lossless compression algorithm to be used for this track."))
                   .arg(QY("If set to 'determine automatically' then mkvmerge will decide whether or not to compress and which algorithm to use based on the track type."))
                   .arg(QY("Currently only certain subtitle formats are compressed with the zlib algorithm.")));
  Util::setToolTip(ui->delay,
                   Q("%1 %2 %3")
                   .arg(QY("Delay this track's timestamps by a couple of ms."))
                   .arg(QY("The value can be negative, but keep in mind that any frame whose timestamp is negative after this calculation is dropped."))
                   .arg(QY("This works with all track types.")));
  Util::setToolTip(ui->stretchBy,
                   Q("<p>%1 %2</p><p>%3</p>")
                   .arg(QY("Multiply this track's timestamps with a factor."))
                   .arg(QY("The value can be given either as a floating point number (e.g. 12.345) or a fraction of integer values (e.g. 123/456)."))
                   .arg(QY("This works well for video and subtitle tracks but should not be used with audio tracks.")));
  Util::setToolTip(ui->defaultDuration,
                   Q("%1 %2")
                   .arg(QY("Forces the default duration or number of frames per second for a track."))
                   .arg(QY("The value can be given either as a floating point number (e.g. 12.345) or a fraction of integer values (e.g. 123/456).")));
  Util::setToolTip(ui->fixBitstreamTimingInfo,
                   Q("%1 %2 %3")
                   .arg(QY("Normally mkvmerge does not change the timing information (frame/field rate) stored in the video bitstream."))
                   .arg(QY("With this option that information is adjusted to match the container's timing information."))
                   .arg(QY("The source for the container's timing information be various things: a value given on the command line with the '--default-duration' option, "
                           "the source container or the video bitstream.")));
  Util::setToolTip(ui->aspectRatio,
                   Q("<p>%1 %2 %3</p><p>%4</p>")
                   .arg(QY("The Matroska container format can store the display width/height for a video track."))
                   .arg(QY("This option tells mkvmerge the display aspect ratio to use when it calculates the display width/height."))
                   .arg(QY("Note that many players don't use the display width/height values directly but only use the ratio given by these values when setting the initial window size."))
                   .arg(QY("The value can be given either as a floating point number (e.g. 12.345) or a fraction of integer values (e.g. 123/456).")));
  Util::setToolTip(ui->displayWidth,
                   Q("<p>%1 %2</p><p>%3</p>")
                   .arg(QY("The Matroska container format can store the display width/height for a video track."))
                   .arg(QY("This parameter is the display width in pixels."))
                   .arg(QY("Note that many players don't use the display width/height values directly but only use the ratio given by these values when setting the initial window size.")));
  Util::setToolTip(ui->displayHeight,
                   Q("<p>%1 %2</p><p>%3</p>")
                   .arg(QY("The Matroska container format can store the display width/height for a video track."))
                   .arg(QY("This parameter is the display height in pixels."))
                   .arg(QY("Note that many players don't use the display width/height values directly but only use the ratio given by these values when setting the initial window size.")));
  Util::setToolTip(ui->cropping,
                   Q("<p>%1 %2</p><p>%3 %4</p><p>%5</p>")
                   .arg(QY("Sets the cropping parameters which tell a player to omit a certain number of pixels on the four sides during playback."))
                   .arg(QY("This must be comma-separated list of four numbers for the cropping to be used at the left, top, right and bottom, e.g. '0,20,0,20'."))
                   .arg(QY("Note that the video content is not modified by this option."))
                   .arg(QY("The values are only stored in the track headers."))
                   .arg(QY("Note also that there are not a lot of players that support the cropping parameters.")));
  Util::setToolTip(ui->stereoscopy,
                   Q("%1 %2")
                   .arg(QY("Sets the stereo mode of the video track to this value."))
                   .arg(QY("If left empty then the track's original stereo mode will be kept or, if it didn't have one, none will be set at all.")));
  Util::setToolTip(ui->naluSizeLength,
                   Q("<p>%1 %2 %3</p><p>%4</p>")
                   .arg(QY("Forces the NALU size length to a certain number of bytes."))
                   .arg(QY("It defaults to 4 bytes, but there are files which do not contain a frame or slice that is bigger than 65535 bytes."))
                   .arg(QY("For such files you can use this parameter and decrease the size to 2."))
                   .arg(QY("This parameter is only effective for AVC/h.264 and HEVC/h.265 elementary streams read from AVC/h.264 ES or HEVC/h.265 ES files, AVIs or Matroska files created with '--engage allow_avc_in_vwf_mode'.")));
  Util::setToolTip(ui->aacIsSBR,
                   Q("%1 %2 %3")
                   .arg(QY("This track contains SBR AAC/HE-AAC/AAC+ data."))
                   .arg(QY("Only needed for AAC input files as SBR AAC cannot be detected automatically for these files."))
                   .arg(QY("Not needed for AAC tracks read from other container formats like MP4 or Matroska files.")));
  Util::setToolTip(ui->reduceToAudioCore,
                   Q("%1 %2")
                   .arg(QY("Drops the lossless extensions from an audio track and keeps only its lossy core."))
                   .arg(QY("This only works with DTS audio tracks.")));
  Util::setToolTip(ui->subtitleCharacterSet,
                   Q("<p>%1 %2</p><p><ol><li>%3</li><li>%4</li></p>")
                   .arg(QY("Selects the character set a subtitle file or chapter information was written with."))
                   .arg(QY("Only needed in certain situations:"))
                   .arg(QY("for subtitle files that do not use a byte order marker (BOM) and that are not encoded in the system's current character set (%1)").arg(Q(g_cc_local_utf8->get_charset())))
                   .arg(QY("for files with chapter information (e.g. OGM, MP4) for which mkvmerge does not detect the encoding correctly")));
  Util::setToolTip(ui->cues,
                   Q("%1 %2")
                   .arg(QY("Selects for which blocks mkvmerge will produce index entries ( = cue entries)."))
                   .arg(QY("\"Determine automatically\" is a good choice for almost all situations.")));
  Util::setToolTip(ui->additionalTrackOptions,
                   Q("%1 %2 %3")
                   .arg(QY("Free-form edit field for user defined options for this track."))
                   .arg(QY("What you input here is added after all the other options the GUI adds so that you could overwrite any of the GUI's options for this track."))
                   .arg(QY("All occurences of the string \"<TID>\" will be replaced by the track's track ID.")));
}

void
Tab::onFileRowsInserted(QModelIndex const &parentIdx,
                        int,
                        int) {
  if (parentIdx.isValid())
    ui->files->setExpanded(parentIdx, true);
}

void
Tab::onTrackRowsInserted(QModelIndex const &parentIdx,
                         int,
                         int) {
  if (parentIdx.isValid())
    ui->tracks->setExpanded(parentIdx, true);
}

void
Tab::onTrackSelectionChanged() {
  Util::enableWidgets(m_allInputControls, false);
  ui->moveTracksUp->setEnabled(false);
  ui->moveTracksDown->setEnabled(false);
  ui->subtitleCharacterSetPreview->setEnabled(false);

  auto selection = ui->tracks->selectionModel()->selection();
  auto numRows   = Util::numSelectedRows(selection);
  if (!numRows)
    return;

  ui->moveTracksUp->setEnabled(true);
  ui->moveTracksDown->setEnabled(true);

  if (1 < numRows) {
    setInputControlValues(nullptr);
    Util::enableWidgets(m_allInputControls, true);
    return;
  }

  Util::enableWidgets(m_typeIndependantControls, true);

  auto idxs = selection[0].indexes();
  if (idxs.isEmpty() || !idxs[0].isValid())
    return;

  auto track = m_tracksModel->fromIndex(idxs[0]);
  if (!track)
    return;

  setInputControlValues(track);

  if (track->isAudio())
    Util::enableWidgets(m_audioControls, true);

  else if (track->isVideo())
    Util::enableWidgets(m_videoControls, true);

  else if (track->isSubtitles()) {
    Util::enableWidgets(m_subtitleControls, true);
    if (track->m_file->m_type == FILE_TYPE_MATROSKA)
      Util::enableWidgets(QList<QWidget *>{} << ui->characterSetLabel << ui->subtitleCharacterSet, false);

    else if (track->m_file->isTextSubtitleContainer())
      ui->subtitleCharacterSetPreview->setEnabled(true);

  } else if (track->isChapters())
    Util::enableWidgets(m_chapterControls, true);

  if (track->isAppended())
    Util::enableWidgets(m_notIfAppendingControls, false);
}

void
Tab::addOrRemoveEmptyComboBoxItem(bool add) {
  for (auto &comboBox : m_comboBoxControls)
    if (add && comboBox->itemData(0).isValid())
      comboBox->insertItem(0, QY("<do not change>"));
    else if (!add && !comboBox->itemData(0).isValid())
      comboBox->removeItem(0);
}

void
Tab::clearInputControlValues() {
  for (auto comboBox : m_comboBoxControls)
    comboBox->setCurrentIndex(0);

  for (auto control : std::vector<QLineEdit *>{ui->trackName, ui->trackTags, ui->delay, ui->stretchBy, ui->timecodes, ui->displayWidth, ui->displayHeight, ui->cropping, ui->additionalTrackOptions})
    control->setText(Q(""));

  ui->defaultDuration->setEditText(Q(""));
  ui->aspectRatio->setEditText(Q(""));

  ui->setAspectRatio->setChecked(false);
  ui->setDisplayWidthHeight->setChecked(false);
}

void
Tab::setInputControlValues(Track *track) {
  m_currentlySettingInputControlValues = true;

  addOrRemoveEmptyComboBoxItem(!track);

  if (!track) {
    clearInputControlValues();
    m_currentlySettingInputControlValues = false;
    return;
  }

  Util::setComboBoxIndexIf(ui->muxThis,          [&](QString const &, QVariant const &data) { return data.isValid() && (data.toBool() == track->m_muxThis);          });
  Util::setComboBoxIndexIf(ui->defaultTrackFlag, [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_defaultTrackFlag); });
  Util::setComboBoxIndexIf(ui->forcedTrackFlag,  [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_forcedTrackFlag);  });
  Util::setComboBoxIndexIf(ui->compression,      [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_compression);      });
  Util::setComboBoxIndexIf(ui->cues,             [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_cues);             });
  Util::setComboBoxIndexIf(ui->stereoscopy,      [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_stereoscopy);      });
  Util::setComboBoxIndexIf(ui->naluSizeLength,   [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_naluSizeLength);   });
  Util::setComboBoxIndexIf(ui->aacIsSBR,         [&](QString const &, QVariant const &data) { return data.isValid() && (data.toUInt() == track->m_aacIsSBR);         });

  ui->trackLanguage->setCurrentByData(track->m_language);
  ui->subtitleCharacterSet->setCurrentByData(track->m_characterSet);

  ui->trackName->setText(                track->m_name);
  ui->trackTags->setText(                track->m_tags);
  ui->delay->setText(                    track->m_delay);
  ui->stretchBy->setText(                track->m_stretchBy);
  ui->timecodes->setText(                track->m_timecodes);
  ui->displayWidth->setText(             track->m_displayWidth);
  ui->displayHeight->setText(            track->m_displayHeight);
  ui->cropping->setText(                 track->m_cropping);
  ui->additionalTrackOptions->setText(   track->m_additionalOptions);
  ui->defaultDuration->setEditText(      track->m_defaultDuration);
  ui->aspectRatio->setEditText(          track->m_aspectRatio);

  ui->setAspectRatio->setChecked(        track->m_setAspectRatio);
  ui->setDisplayWidthHeight->setChecked(!track->m_setAspectRatio);
  ui->fixBitstreamTimingInfo->setChecked(track->m_fixBitstreamTimingInfo);
  ui->reduceToAudioCore->setChecked(     track->m_reduceAudioToCore);

  m_currentlySettingInputControlValues = false;
}

QList<SourceFile *>
Tab::selectedSourceFiles()
  const {
  auto sourceFiles = QList<SourceFile *>{};
  Util::withSelectedIndexes(ui->files, [&sourceFiles, this](QModelIndex const &idx) {
      auto sourceFile = m_filesModel->fromIndex(idx);
      if (sourceFile)
        sourceFiles << sourceFile.get();
  });

  return sourceFiles;
}

QList<Track *>
Tab::selectedTracks()
  const {
  auto tracks = QList<Track *>{};
  Util::withSelectedIndexes(ui->tracks, [&tracks, this](QModelIndex const &idx) {
      auto track = m_tracksModel->fromIndex(idx);
      if (track)
        tracks << track;
    });

  return tracks;
}

void
Tab::withSelectedTracks(std::function<void(Track *)> code,
                        bool notIfAppending,
                        QWidget *widget) {
  if (m_currentlySettingInputControlValues)
    return;

  auto tracks = selectedTracks();
  if (tracks.isEmpty())
    return;

  if (!widget)
    widget = static_cast<QWidget *>(QObject::sender());

  bool withAudio     = m_audioControls.contains(widget);
  bool withVideo     = m_videoControls.contains(widget);
  bool withSubtitles = m_subtitleControls.contains(widget);
  bool withChapters  = m_chapterControls.contains(widget);
  bool withAll       = m_typeIndependantControls.contains(widget);

  for (auto &track : tracks) {
    if (track->m_appendedTo && notIfAppending)
      continue;

    if (   withAll
        || (track->isAudio()     && withAudio)
        || (track->isVideo()     && withVideo)
        || (track->isSubtitles() && withSubtitles)
        || (track->isChapters()  && withChapters)) {
      code(track);
      m_tracksModel->trackUpdated(track);
    }
  }
}

void
Tab::onTrackNameChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_name = newValue; }, true);
}

void
Tab::onTrackItemChanged(QStandardItem *item) {
  if (!item)
    return;

  auto idx = m_tracksModel->indexFromItem(item);
  if (idx.column())
    return;

  auto track = m_tracksModel->fromIndex(idx);
  if (!track)
    return;

  auto newMuxThis = item->checkState() == Qt::Checked;
  if (newMuxThis == track->m_muxThis)
    return;

  track->m_muxThis = newMuxThis;
  m_tracksModel->trackUpdated(track);

  auto selected = selectedTracks();
  if ((1 == selected.count()) && selected.contains(track))
    Util::setComboBoxIndexIf(ui->muxThis, [newMuxThis](QString const &, QVariant const &data) { return data.isValid() && (data.toBool() == newMuxThis); });

  setOutputFileNameMaybe();

  m_tracksModel->updateEffectiveDefaultTrackFlags();
}

void
Tab::onMuxThisChanged(int selected) {
  auto data = ui->muxThis->itemData(selected);
  if (!data.isValid())
    return;
  auto muxThis = data.toBool();

  withSelectedTracks([&](Track *track) { track->m_muxThis = muxThis; });

  setOutputFileNameMaybe();

  m_tracksModel->updateEffectiveDefaultTrackFlags();
}

void
Tab::toggleMuxThisForSelectedTracks() {
  auto allEnabled     = true;
  auto tracksSelected = false;

  withSelectedTracks([&allEnabled, &tracksSelected](Track *track) {
    tracksSelected = true;

    if (!track->m_muxThis)
      allEnabled = false;
  }, false, ui->muxThis);

  if (!tracksSelected) {
    m_tracksModel->updateEffectiveDefaultTrackFlags();
    return;
  }

  auto newEnabled = !allEnabled;

  withSelectedTracks([newEnabled](Track *track) { track->m_muxThis = newEnabled; }, false, ui->muxThis);

  Util::setComboBoxIndexIf(ui->muxThis, [&](QString const &, QVariant const &data) { return data.isValid() && (data.toBool() == newEnabled); });

  m_tracksModel->updateEffectiveDefaultTrackFlags();
}

void
Tab::onTrackLanguageChanged(int newValue) {
  auto code = ui->trackLanguage->itemData(newValue).toString();
  if (code.isEmpty())
    return;

  withSelectedTracks([&](Track *track) { track->m_language = code; }, true);
}

void
Tab::onDefaultTrackFlagChanged(int newValue) {
  auto data = ui->defaultTrackFlag->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) {
    track->m_defaultTrackFlag = newValue;
    if (1 == newValue)
      ensureOneDefaultFlagOnly(track);
  }, true);

  m_tracksModel->updateEffectiveDefaultTrackFlags();
}

void
Tab::onForcedTrackFlagChanged(int newValue) {
  auto data = ui->forcedTrackFlag->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) { track->m_forcedTrackFlag = newValue; }, true);
}

void
Tab::onCompressionChanged(int newValue) {
  auto data = ui->compression->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  Track::Compression compression;
  if (3 > newValue)
    compression = 0 == newValue ? Track::CompDefault
                : 1 == newValue ? Track::CompNone
                :                 Track::CompZlib;

  withSelectedTracks([&](Track *track) { track->m_compression = compression; }, true);
}

void
Tab::onTrackTagsChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_tags = newValue; }, true);
}

void
Tab::onDelayChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_delay = newValue; });
}

void
Tab::onStretchByChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_stretchBy = newValue; });
}

void
Tab::onDefaultDurationChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_defaultDuration = newValue; }, true);
}

void
Tab::onTimecodesChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_timecodes = newValue; });
}

void
Tab::onBrowseTimecodes() {
  auto fileName = getOpenFileName(QY("Select timecode file"), QY("Text files") + Q(" (*.txt)"), ui->timecodes);
  if (!fileName.isEmpty())
    withSelectedTracks([&](Track *track) { track->m_timecodes = fileName; });
}

void
Tab::onFixBitstreamTimingInfoChanged(bool newValue) {
  withSelectedTracks([&](Track *track) { track->m_fixBitstreamTimingInfo = newValue; }, true);
}

void
Tab::onBrowseTrackTags() {
  auto fileName = getOpenFileName(QY("Select tags file"), QY("XML tag files") + Q(" (*.xml)"), ui->trackTags);
  if (!fileName.isEmpty())
    withSelectedTracks([&](Track *track) { track->m_tags = fileName; }, true);
}

void
Tab::onSetAspectRatio() {
  withSelectedTracks([&](Track *track) { track->m_setAspectRatio = true; }, true);
}

void
Tab::onSetDisplayDimensions() {
  withSelectedTracks([&](Track *track) { track->m_setAspectRatio = false; }, true);
}

void
Tab::onAspectRatioChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_aspectRatio = newValue; }, true);
}

void
Tab::onDisplayWidthChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_displayWidth = newValue; }, true);
}

void
Tab::onDisplayHeightChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_displayHeight = newValue; }, true);
}

void
Tab::onStereoscopyChanged(int newValue) {
  auto data = ui->stereoscopy->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) { track->m_stereoscopy = newValue; }, true);
}

void
Tab::onNaluSizeLengthChanged(int newValue) {
  auto data = ui->naluSizeLength->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) { track->m_naluSizeLength = newValue; }, true);
}

void
Tab::onCroppingChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_cropping = newValue; }, true);
}

void
Tab::onAacIsSBRChanged(int newValue) {
  auto data = ui->aacIsSBR->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) { track->m_aacIsSBR = newValue; }, true);
}

void
Tab::onReduceAudioToCoreChanged(bool newValue) {
  withSelectedTracks([&](Track *track) { track->m_reduceAudioToCore = newValue; });
}

void
Tab::onSubtitleCharacterSetChanged(int newValue) {
  auto characterSet = ui->subtitleCharacterSet->itemData(newValue).toString();
  if (characterSet.isEmpty())
    return;

  withSelectedTracks([&](Track *track) {
    if (track->m_file->m_type != FILE_TYPE_MATROSKA)
      track->m_characterSet = characterSet;
  }, true);
}

void
Tab::onCuesChanged(int newValue) {
  auto data = ui->cues->itemData(newValue);
  if (!data.isValid())
    return;
  newValue = data.toInt();

  withSelectedTracks([&](Track *track) { track->m_cues = newValue; }, true);
}

void
Tab::onAdditionalTrackOptionsChanged(QString newValue) {
  withSelectedTracks([&](Track *track) { track->m_additionalOptions = newValue; }, true);
}

void
Tab::onAddFiles() {
  addOrAppendFiles(false);
}

void
Tab::onAppendFiles() {
  addOrAppendFiles(true);
}

void
Tab::addOrAppendFiles(bool append) {
  auto fileNames = selectFilesToAdd(append ? QY("Append media files") : QY("Add media files"));
  if (!fileNames.isEmpty())
    addOrAppendFiles(append, fileNames, selectedSourceFile());
}

void
Tab::addFiles(QStringList const &fileNames) {
  addOrAppendFiles(false, fileNames, QModelIndex{});
}

QStringList
Tab::handleDroppedSpecialFiles(QStringList const &fileNames) {
  auto toIdentify = QStringList{};

  auto simpleChaptersRE = boost::regex{"^CHAPTER\\d{2}=.*CHAPTER\\d{2}NAME=",   boost::regex::perl | boost::regex::mod_s};
  auto xmlChaptersRE    = boost::regex{"<\\?xml[^>]+version.*\\?>.*<Chapters>", boost::regex::perl | boost::regex::mod_s};
  auto xmlTagsRE        = boost::regex{"<\\?xml[^>]+version.*\\?>.*<Tags>",     boost::regex::perl | boost::regex::mod_s};
  auto xmlSegmentInfoRE = boost::regex{"<\\?xml[^>]+version.*\\?>.*<Info>",     boost::regex::perl | boost::regex::mod_s};

  for (auto const &fileName : fileNames) {
    QFile file{fileName};
    if (!file.open(QIODevice::ReadOnly)) {
      toIdentify << fileName;
      continue;
    }

    auto content = std::string{ file.read(1024).data() };
    if (boost::regex_search(content, simpleChaptersRE) || boost::regex_search(content, xmlChaptersRE)) {
      Util::MessageBox::warning(this)
        ->title(QY("Adding chapter files"))
        .text(Q("%1 %2 %3 %4")
              .arg(QY("The file '%1' contains chapters.").arg(fileName))
              .arg(QY("These aren't treated like other input files in MKVToolNix."))
              .arg(QY("Instead such a file must be set via the 'chapter file' option on the 'output' tab."))
              .arg(QY("The GUI will enter the dropped file's file name into that control replacing any file name which might have been set earlier.")))
        .onlyOnce(Q("mergeChaptersDropped"))
        .exec();

      m_config.m_chapters = fileName;
      ui->chapters->setText(fileName);

      continue;
    }

    if (boost::regex_search(content, xmlTagsRE)) {
      Util::MessageBox::warning(this)
        ->title(QY("Adding tag files"))
        .text(Q("%1 %2 %3 %4")
              .arg(QY("The file '%1' contains tags.").arg(fileName))
              .arg(QY("These aren't treated like other input files in MKVToolNix."))
              .arg(QY("Instead such a file must be set via the 'global tags' option on the 'output' tab."))
              .arg(QY("The GUI will enter the dropped file's file name into that control replacing any file name which might have been set earlier.")))
        .onlyOnce(Q("mergeTagsDropped"))
        .exec();

      m_config.m_globalTags = fileName;
      ui->globalTags->setText(fileName);

      continue;
    }

    if (boost::regex_search(content, xmlSegmentInfoRE)) {
      Util::MessageBox::warning(this)
        ->title(QY("Adding segment info files"))
        .text(Q("%1 %2 %3 %4")
              .arg(QY("The file '%1' contains segment information.").arg(fileName))
              .arg(QY("These aren't treated like other input files in MKVToolNix."))
              .arg(QY("Instead such a file must be set via the 'segment info' option on the 'output' tab."))
              .arg(QY("The GUI will enter the dropped file's file name into that control replacing any file name which might have been set earlier.")))
        .onlyOnce(Q("mergeSegmentInfoDropped"))
        .exec();

      m_config.m_segmentInfo = fileName;
      ui->segmentInfo->setText(fileName);

      continue;
    }

    toIdentify << fileName;
  }

  return toIdentify;
}

void
Tab::addOrAppendFiles(bool append,
                      QStringList const &fileNames,
                      QModelIndex const &sourceFileIdx) {
  if (!fileNames.isEmpty())
    Util::Settings::get().m_lastOpenDir = QFileInfo{fileNames.last()}.path();

  auto toIdentify = handleDroppedSpecialFiles(fileNames);

  QList<SourceFilePtr> identifiedFiles;
  for (auto &fileName : toIdentify) {
    Util::FileIdentifier identifier{ this, fileName };
    if (identifier.identify())
      identifiedFiles << identifier.file();
  }

  if (!append)
    identifiedFiles = PlaylistScanner{this}.checkAddingPlaylists(identifiedFiles);

  if (identifiedFiles.isEmpty())
    return;

  if (m_debugTrackModel) {
    log_it(boost::format("### BEFORE adding/appending ###\n"));
    m_config.debugDumpFileList();
    m_config.debugDumpTrackList();
  }

  setDefaultsFromSettingsForAddedFiles(identifiedFiles);

  if (m_config.m_firstInputFileName.isEmpty())
    m_config.m_firstInputFileName = identifiedFiles[0]->m_fileName;

  m_filesModel->addOrAppendFilesAndTracks(sourceFileIdx, identifiedFiles, append);

  if (m_debugTrackModel) {
    log_it(boost::format("### AFTER adding/appending ###\n"));
    m_config.debugDumpFileList();
    m_config.debugDumpTrackList();
  }

  reinitFilesTracksControls();

  setTitleMaybe(identifiedFiles);
  setOutputFileNameMaybe();
}

void
Tab::setDefaultsFromSettingsForAddedFiles(QList<SourceFilePtr> const &files) {
  auto &cfg = Util::Settings::get();

  auto defaultFlagSet = QHash<Track::Type, bool>{};
  for (auto const &track : m_config.m_tracks)
    if (track->m_defaultTrackFlag == 1)
      defaultFlagSet[track->m_type] = true;

  for (auto const &file : files)
    for (auto const &track : file->m_tracks) {
      if (cfg.m_disableCompressionForAllTrackTypes)
        track->m_compression = Track::CompNone;

      if (cfg.m_disableDefaultTrackForSubtitles && track->isSubtitles())
        track->m_defaultTrackFlag = 2;

      else if (track->m_defaultTrackFlagWasSet && !defaultFlagSet[track->m_type]) {
        track->m_defaultTrackFlag     = 1;
        defaultFlagSet[track->m_type] = true;
      }
    }
}

QStringList
Tab::selectFilesToAdd(QString const &title) {
  return Util::getOpenFileNames(this, title, Util::Settings::get().lastOpenDirPath(), Util::FileTypeFilter::get().join(Q(";;")), nullptr, QFileDialog::HideNameFilterDetails);
}

void
Tab::onAddAdditionalParts() {
  auto currentIdx = selectedSourceFile();
  auto sourceFile = m_filesModel->fromIndex(currentIdx);
  if (sourceFile && !sourceFile->m_tracks.size()) {
    Util::MessageBox::critical(this)->title(QY("Unable to append files")).text(QY("You cannot add additional parts to files that don't contain tracks.")).exec();
    return;
  }

  m_filesModel->addAdditionalParts(currentIdx, selectFilesToAdd(QY("Add media files as additional parts")));
}

void
Tab::onRemoveFiles() {
  auto selectedFiles = selectedSourceFiles();
  if (selectedFiles.isEmpty())
    return;

  m_filesModel->removeFiles(selectedFiles);

  reinitFilesTracksControls();

  if (!m_filesModel->rowCount()) {
    m_config.m_firstInputFileName.clear();
    clearDestinationMaybe();
    clearTitleMaybe();
  }
}

void
Tab::onRemoveAllFiles() {
  if (m_config.m_files.isEmpty())
    return;

  m_filesModel->removeRows(0, m_filesModel->rowCount());
  m_tracksModel->removeRows(0, m_tracksModel->rowCount());
  m_config.m_files.clear();
  m_config.m_tracks.clear();
  m_config.m_firstInputFileName.clear();

  reinitFilesTracksControls();
  clearDestinationMaybe();
  clearTitleMaybe();
}

void
Tab::reinitFilesTracksControls() {
  resizeFilesColumnsToContents();
  resizeTracksColumnsToContents();
  onTrackSelectionChanged();
}

void
Tab::resizeFilesColumnsToContents()
  const {
  Util::resizeViewColumnsToContents(ui->files);
}

void
Tab::resizeTracksColumnsToContents()
  const {
  Util::resizeViewColumnsToContents(ui->tracks);
}

void
Tab::enableMoveFilesButtons() {
  auto hasSelected = !ui->files->selectionModel()->selection().isEmpty();

  ui->moveFilesUp->setEnabled(hasSelected);
  ui->moveFilesDown->setEnabled(hasSelected);
}

void
Tab::enableFilesActions() {
  int numSelected      = ui->files->selectionModel()->selection().size();
  bool hasRegularTrack = false;
  if (1 == numSelected)
    hasRegularTrack = m_config.m_files.end() != brng::find_if(m_config.m_files, [](SourceFilePtr const &file) { return file->hasRegularTrack(); });

  m_addFilesAction->setEnabled(true);
  m_appendFilesAction->setEnabled((1 == numSelected) && hasRegularTrack);
  m_addAdditionalPartsAction->setEnabled(1 == numSelected);
  m_removeFilesAction->setEnabled(0 < numSelected);
  m_removeAllFilesAction->setEnabled(!m_config.m_files.isEmpty());
  m_openFilesInMediaInfoAction->setEnabled(0 < numSelected);
}

void
Tab::enableTracksActions() {
  int numSelected = ui->tracks->selectionModel()->selection().size();
  bool hasTracks  = !!m_tracksModel->rowCount();

  m_selectAllTracksAction->setEnabled(hasTracks);
  m_selectTracksOfTypeMenu->setEnabled(hasTracks);
  m_enableAllTracksAction->setEnabled(hasTracks);
  m_disableAllTracksAction->setEnabled(hasTracks);

  m_selectAllVideoTracksAction->setEnabled(hasTracks);
  m_selectAllAudioTracksAction->setEnabled(hasTracks);
  m_selectAllSubtitlesTracksAction->setEnabled(hasTracks);

  m_openTracksInMediaInfoAction->setEnabled(0 < numSelected);
}

void
Tab::retranslateInputUI() {
  m_filesModel->retranslateUi();
  m_tracksModel->retranslateUi();

  resizeFilesColumnsToContents();
  resizeTracksColumnsToContents();

  m_addFilesAction->setText(QY("&Add files"));
  m_appendFilesAction->setText(QY("A&ppend files"));
  m_addAdditionalPartsAction->setText(QY("Add files as a&dditional parts"));
  m_removeFilesAction->setText(QY("&Remove files"));
  m_removeAllFilesAction->setText(QY("Remove a&ll files"));
  m_openFilesInMediaInfoAction->setText(QY("Open in &MediaInfo"));

  m_selectAllTracksAction->setText(QY("&Select all tracks"));
  m_selectTracksOfTypeMenu->setTitle(QY("Select all tracks of specific &type"));
  m_enableAllTracksAction->setText(QY("&Enable all tracks"));
  m_disableAllTracksAction->setText(QY("&Disable all tracks"));
  m_openTracksInMediaInfoAction->setText(QY("Open in &MediaInfo"));

  m_selectAllVideoTracksAction->setText(QY("&Video"));
  m_selectAllAudioTracksAction->setText(QY("&Audio"));
  m_selectAllSubtitlesTracksAction->setText(QY("&Subtitles"));

  for (auto idx = 0u, end = stereo_mode_c::max_index(); idx <= end; ++idx)
    ui->stereoscopy->setItemText(idx + 1, QString{"%1 (%2; %3)"}.arg(to_qs(stereo_mode_c::translate(idx))).arg(idx).arg(to_qs(stereo_mode_c::s_modes[idx])));

  Util::fixComboBoxViewWidth(*ui->stereoscopy);

  for (auto &comboBox : m_comboBoxControls)
    if (comboBox->count() && !comboBox->itemData(0).isValid())
      comboBox->setItemText(0, QY("<do not change>"));

  Util::setComboBoxTexts(ui->muxThis,          QStringList{}                                  << QY("yes")                  << QY("no"));
  Util::setComboBoxTexts(ui->naluSizeLength,   QStringList{} << QY("don't change")            << QY("force 2 bytes")        << QY("force 4 bytes"));
  Util::setComboBoxTexts(ui->defaultTrackFlag, QStringList{} << QY("determine automatically") << QY("yes")                  << QY("no"));
  Util::setComboBoxTexts(ui->forcedTrackFlag,  QStringList{}                                  << QY("yes")                  << QY("no"));
  Util::setComboBoxTexts(ui->compression,      QStringList{} << QY("determine automatically") << QY("no extra compression") << Q("zlib"));
  Util::setComboBoxTexts(ui->cues,             QStringList{} << QY("determine automatically") << QY("only for I frames")    << QY("for all frames") << QY("no cues"));
  Util::setComboBoxTexts(ui->aacIsSBR,         QStringList{} << QY("determine automatically") << QY("yes")                  << QY("no"));

  setupInputToolTips();
}

QModelIndex
Tab::selectedSourceFile()
  const {
  auto idx = ui->files->selectionModel()->currentIndex();
  return m_filesModel->index(idx.row(), 0, idx.parent());
}

void
Tab::setTitleMaybe(QList<SourceFilePtr> const &files) {
  for (auto const &file : files) {
    setTitleMaybe(file->m_properties["title"]);

    if (FILE_TYPE_OGM != file->m_type)
      continue;

    for (auto const &track : file->m_tracks)
      if (track->isVideo() && !track->m_name.isEmpty()) {
        setTitleMaybe(track->m_name);
        break;
      }
  }
}

void
Tab::setTitleMaybe(QString const &title) {
  if (!Util::Settings::get().m_autoSetFileTitle || title.isEmpty() || !m_config.m_title.isEmpty())
    return;

  ui->title->setText(title);
  m_config.m_title = title;
}

QString
Tab::suggestOutputFileNameExtension()
  const {
  auto hasTracks = false, hasVideo = false, hasAudio = false, hasStereoscopy = false;

  for (auto const &t : m_config.m_tracks) {
    if (!t->m_muxThis)
      continue;

    hasTracks = true;

    if (t->isVideo()) {
      hasVideo = true;
      if (t->m_stereoscopy >= 2)
        hasStereoscopy = true;

    } else if (t->isAudio())
      hasAudio = true;
  }

  return m_config.m_webmMode ? "webm"
       : hasStereoscopy      ? "mk3d"
       : hasVideo            ? "mkv"
       : hasAudio            ? "mka"
       : hasTracks           ? "mks"
       :                       "mkv";
}

void
Tab::setOutputFileNameMaybe() {
  auto &settings = Util::Settings::get();
  auto policy    = settings.m_outputFileNamePolicy;

  if ((Util::Settings::DontSetOutputFileName == policy) || m_config.m_firstInputFileName.isEmpty())
    return;

  auto currentOutput = ui->output->text();
  QDir outputDir;

  // Don't override custom changes to the output file name.
  if (   !currentOutput.isEmpty()
      && !m_config.m_destinationAuto.isEmpty()
      && (currentOutput != m_config.m_destinationAuto))
    return;

  if (Util::Settings::ToPreviousDirectory == policy)
    outputDir = settings.m_lastOutputDir;

  else if (Util::Settings::ToFixedDirectory == policy)
    outputDir = settings.m_fixedOutputDir;

  else if (Util::Settings::ToSameAsFirstInputFile == policy)
    outputDir = QFileInfo{m_config.m_firstInputFileName}.absoluteDir();

  else if (Util::Settings::ToRelativeOfFirstInputFile == policy)
    outputDir = QDir{ QFileInfo{m_config.m_firstInputFileName}.absoluteDir().path() + Q("/") + settings.m_relativeOutputDir.path() };

  else
    Q_ASSERT_X(false, "setOutputFileNameMaybe", "Untested output file name policy");

  auto baseName = QFileInfo{ m_config.m_firstInputFileName }.completeBaseName();
  auto idx      = 0;

  while (true) {
    auto suffix          = suggestOutputFileNameExtension();
    auto currentBaseName = QString{"%1%2.%3"}.arg(baseName).arg(idx ? QString{" (%1)"}.arg(idx) : "").arg(suffix);
    auto outputFileName  = QFileInfo{outputDir, currentBaseName};

    if (!settings.m_uniqueOutputFileNames || !outputFileName.exists()) {
      m_config.m_destinationAuto = outputFileName.absoluteFilePath();

      ui->output->setText(m_config.m_destinationAuto);
      setDestination(m_config.m_destinationAuto);

      break;
    }

    ++idx;
  }
}

void
Tab::addOrAppendDroppedFiles(QStringList const &fileNames) {
  if (fileNames.isEmpty())
    return;

  auto noFilesAdded = m_config.m_files.isEmpty();

  if ((fileNames.count() == 1) && noFilesAdded) {
    addOrAppendFiles(false, fileNames, QModelIndex{});
    return;
  }

  auto &settings = Util::Settings::get();

  auto decision  = settings.m_mergeAddingAppendingFilesPolicy;
  auto fileIdx   = QModelIndex{};

  if (Util::Settings::AddingAppendingFilesPolicy::Ask == decision) {
    AddingAppendingFilesDialog dlg{this, m_config.m_files};
    if (!dlg.exec())
      return;

    decision = dlg.decision();
    fileIdx  = m_filesModel->index(dlg.fileIndex(), 0);

    if (dlg.alwaysUseThisDecision()) {
      settings.m_mergeAddingAppendingFilesPolicy = decision;
      settings.save();
    }
  }

  if (Util::Settings::AddingAppendingFilesPolicy::AddAdditionalParts == decision)
    m_filesModel->addAdditionalParts(fileIdx, fileNames);

  else if (Util::Settings::AddingAppendingFilesPolicy::AddToNew == decision)
    MainWindow::mergeTool()->addMultipleFilesToNewSettings(fileNames, false);

  else if (Util::Settings::AddingAppendingFilesPolicy::AddEachToNew == decision) {
    auto toAdd = fileNames;

    if (noFilesAdded)
      addOrAppendFiles(false, QStringList{} << toAdd.takeFirst(), QModelIndex{});

    if (!toAdd.isEmpty())
      MainWindow::mergeTool()->addMultipleFilesToNewSettings(toAdd, true);

  } else
    addOrAppendFiles(Util::Settings::AddingAppendingFilesPolicy::Append == decision, fileNames, fileIdx);
}

void
Tab::addOrAppendDroppedFilesDelayed() {
  addOrAppendDroppedFiles(m_filesToAddDelayed);
  m_filesToAddDelayed.clear();
}

void
Tab::addFilesToBeAddedOrAppendedDelayed(QStringList const &fileNames) {
  m_filesToAddDelayed += fileNames;
  QTimer::singleShot(0, this, SLOT(addOrAppendDroppedFilesDelayed()));
}

void
Tab::selectAllTracksOfType(boost::optional<Track::Type> type) {
  auto numRows = m_tracksModel->rowCount();
  if (!numRows)
    return;

  auto numColumns = m_tracksModel->columnCount() - 1;
  auto selection  = QItemSelection{};

  for (auto row = 0; row < numRows; ++row) {
    auto &track      = *m_config.m_tracks[row];
    auto numAppended = track.m_appendedTracks.count();

    if (!type || (track.m_type == *type))
      selection.select(m_tracksModel->index(row, 0), m_tracksModel->index(row, numColumns - 1));

    if (!numAppended)
      continue;

    auto rowIdx = m_tracksModel->index(row, 0);

    for (auto appendedRow = 0; appendedRow < numAppended; ++appendedRow) {
      auto &appendedTrack = *track.m_appendedTracks[appendedRow];
      if (!type || (appendedTrack.m_type == *type))
        selection.select(m_tracksModel->index(appendedRow, 0, rowIdx), m_tracksModel->index(appendedRow, numColumns - 1, rowIdx));
    }
  }

  ui->tracks->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void
Tab::selectAllTracks() {
  selectAllTracksOfType({});
}

void
Tab::selectAllVideoTracks() {
  selectAllTracksOfType(Track::Video);
}

void
Tab::selectAllAudioTracks() {
  selectAllTracksOfType(Track::Audio);
}

void
Tab::selectAllSubtitlesTracks() {
  selectAllTracksOfType(Track::Subtitles);
}

void
Tab::selectTracks(QList<Track *> const &tracks) {
  auto numColumns = m_tracksModel->columnCount() - 1;
  auto selection  = QItemSelection{};

  for (auto const &track : tracks) {
    auto idx = m_tracksModel->indexFromTrack(track);
    selection.select(idx.sibling(idx.row(), 0), idx.sibling(idx.row(), numColumns));
  }

  ui->tracks->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void
Tab::selectSourceFiles(QList<SourceFile *> const &files) {
  auto numColumns = m_filesModel->columnCount() - 1;
  auto selection  = QItemSelection{};

  for (auto const &file : files) {
    auto idx = m_filesModel->indexFromSourceFile(file);
    selection.select(idx.sibling(idx.row(), 0), idx.sibling(idx.row(), numColumns));
  }

  ui->files->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void
Tab::enableAllTracks() {
  enableDisableAllTracks(true);
}

void
Tab::disableAllTracks() {
  enableDisableAllTracks(false);
}

void
Tab::enableDisableAllTracks(bool enable) {
  for (auto row = 0, numRows = m_tracksModel->rowCount(); row < numRows; ++row) {
    auto track       = m_tracksModel->fromIndex(m_tracksModel->index(row, 0));
    track->m_muxThis = enable;
    m_tracksModel->trackUpdated(track);

    for (auto const &appendedTrack : track->m_appendedTracks) {
      appendedTrack->m_muxThis = enable;
      m_tracksModel->trackUpdated(appendedTrack);
    }
  }

  auto base = ui->muxThis->itemData(0).isValid() ? 0 : 1;
  ui->muxThis->setCurrentIndex(base + (enable ? 0 : 1));
}

void
Tab::ensureOneDefaultFlagOnly(Track *thisOneHasIt) {
  auto selection = selectedTracks();
  for (auto const &track : m_config.m_tracks)
    if (   (track->m_defaultTrackFlag == 1)
        && (track->m_type             == thisOneHasIt->m_type)
        && (track                     != thisOneHasIt)) {
      track->m_defaultTrackFlag = 0;
      if ((selection.count() == 1) && (selection[0] == track))
        ui->defaultTrackFlag->setCurrentIndex(0);
    }

  m_tracksModel->updateEffectiveDefaultTrackFlags();
}

void
Tab::moveSourceFilesUpOrDown(bool up) {
  auto files = selectedSourceFiles();

  m_filesModel->moveSourceFilesUpOrDown(files, up);

  for (auto const &file : files)
    if (file->isRegular())
      ui->files->setExpanded(m_filesModel->indexFromSourceFile(file), true);

  selectSourceFiles(files);
}

void
Tab::onMoveFilesUp() {
  moveSourceFilesUpOrDown(true);
}

void
Tab::onMoveFilesDown() {
  moveSourceFilesUpOrDown(false);
}

void
Tab::moveTracksUpOrDown(bool up) {
  auto tracks = selectedTracks();

  m_tracksModel->moveTracksUpOrDown(tracks, up);

  for (auto const &track : tracks)
    if (track->isRegular() && !track->m_appendedTo)
      ui->tracks->setExpanded(m_tracksModel->indexFromTrack(track), true);

  selectTracks(tracks);
}

void
Tab::onMoveTracksUp() {
  moveTracksUpOrDown(true);
}

void
Tab::onMoveTracksDown() {
  moveTracksUpOrDown(false);
}

void
Tab::showFilesContextMenu(QPoint const &pos) {
  enableFilesActions();
  m_filesMenu->exec(ui->files->viewport()->mapToGlobal(pos));
}

void
Tab::showTracksContextMenu(QPoint const &pos) {
  enableTracksActions();
  m_tracksMenu->exec(ui->tracks->viewport()->mapToGlobal(pos));
}

void
Tab::onOpenFilesInMediaInfo() {
  auto fileNames = QStringList{};
  for (auto const &sourceFile : selectedSourceFiles())
    fileNames << sourceFile->m_fileName;

  openFilesInMediaInfo(fileNames);
}

void
Tab::onOpenTracksInMediaInfo() {
  auto fileNames = QStringList{};
  for (auto const &track : selectedTracks()) {
    auto const &fileName = track->m_file->m_fileName;
    if (!fileNames.contains(fileName))
      fileNames << fileName;
  }

  openFilesInMediaInfo(fileNames);
}

QString
Tab::mediaInfoLocation() {
  auto &cfg = Util::Settings::get();
  auto exe  = cfg.m_mediaInfoExe.isEmpty() ? Q("mediainfo-gui") : cfg.m_mediaInfoExe;
  exe       = Util::Settings::exeWithPath(exe);

  if (!exe.isEmpty() && QFileInfo{exe}.exists())
    return exe;

  exe = Util::Settings::exeWithPath(Q("mediainfo"));

#if defined(SYS_WINDOWS)
  exe = QSettings{Q("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\MediaInfo.exe"), QSettings::NativeFormat}.value("Default").toString();
  if (!exe.isEmpty() && QFileInfo{exe}.exists())
    return exe;
#endif

  ExecutableLocationDialog dlg{this};
  auto result = dlg
    .setInfo(QY("Executable not found"),
             Q("<p>%1 %2 %3</p><p>%4</p>")
             .arg(QY("This function requires the application %1.").arg("MediaInfo"))
             .arg(QY("Its installation location could not be determined automatically."))
             .arg(QY("Please select its location below."))
             .arg(QY("You can download the application from the following URL:")))
    .setURL(Q("https://mediaarea.net/en/MediaInfo"))
    .exec();

  if (QDialog::Rejected == result)
    return {};

  exe = dlg.executable();
  if (exe.isEmpty() || !QFileInfo{exe}.exists())
    return {};

  cfg.m_mediaInfoExe = exe;

  return exe;
}

void
Tab::openFilesInMediaInfo(QStringList const &fileNames) {
  if (fileNames.isEmpty())
    return;

  auto exe = mediaInfoLocation();
  if (!exe.isEmpty())
    QProcess::startDetached(exe, fileNames);
}

void
Tab::onPreviewSubtitleCharacterSet() {
  auto selection = selectedTracks();
  auto track     = selection.count() ? selection[0] : nullptr;

  if ((selection.count() != 1) || !track->m_file->isTextSubtitleContainer())
    return;

  auto dlg = new SelectCharacterSetDialog{this, track->m_file->m_fileName, track->m_characterSet};
  dlg->setUserData(reinterpret_cast<qulonglong>(track));

  connect(dlg, &SelectCharacterSetDialog::characterSetSelected, this, &Tab::setSubtitleCharacterSet);

  dlg->show();
}

void
Tab::setSubtitleCharacterSet(QString const &characterSet) {
  auto dlg = qobject_cast<SelectCharacterSetDialog *>(QObject::sender());
  if (!dlg)
    return;

  auto track = reinterpret_cast<Track *>(dlg->userData().toULongLong());

  if (!m_config.m_tracks.contains(track))
    return;

  track->m_characterSet = characterSet;
  auto selection        = selectedTracks();

  if ((selection.count() == 1) && (selection[0] == track))
    Util::setComboBoxTextByData(ui->subtitleCharacterSet, characterSet);
}

}}}
