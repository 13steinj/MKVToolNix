#ifndef MTX_MKVTOOLNIX_GUI_UTIL_SETTINGS_H
#define MTX_MKVTOOLNIX_GUI_UTIL_SETTINGS_H

#include "common/common_pch.h"

#include <QDateTime>
#include <QDir>
#include <QString>
#include <QTabWidget>
#include <QVariant>

#include "common/translation.h"

class QSettings;
class QSplitter;

namespace mtx { namespace gui { namespace Util {

class Settings: public QObject {
  Q_OBJECT;
  Q_ENUMS(RunProgramForEvent);

public:
  enum RunProgramForEvent {
    RunNever                         = 0x00,
    RunAfterJobQueueFinishes         = 0x01,
    RunAfterJobCompletesSuccessfully = 0x02,
    RunAfterJobCompletesWithErrors   = 0x04,
  };

  Q_DECLARE_FLAGS(RunProgramForEvents, RunProgramForEvent);

  enum ProcessPriority {
    LowestPriority = 0,
    LowPriority,
    NormalPriority,
    HighPriority,
    HighestPriority,
  };

  enum ScanForPlaylistsPolicy {
    AskBeforeScanning = 0,
    AlwaysScan,
    NeverScan,
  };

  enum OutputFileNamePolicy {
    DontSetOutputFileName = 0,
    ToPreviousDirectory,
    ToFixedDirectory,
    ToParentOfFirstInputFile,
    ToSameAsFirstInputFile,
    ToRelativeOfFirstInputFile,
  };

  enum class JobRemovalPolicy {
    Never,
    IfSuccessful,
    IfWarningsFound,
    Always,
  };

  enum class ClearMergeSettingsAction {
    None,
    NewSettings,
    RemoveInputFiles,
  };

  enum class AddingAppendingFilesPolicy {
    Ask,
    Add,
    AddToNew,
    AddEachToNew,
    Append,
    AddAdditionalParts,
  };

  enum class TrackPropertiesLayout {
    HorizontalScrollArea,
    HorizontalTwoColumns,
    VerticalTabWidget,
  };

  class RunProgramConfig {
  public:
    QStringList m_commandLine;
    RunProgramForEvents m_forEvents{};

    bool isValid() const;
  };

  using RunProgramConfigPtr  = std::shared_ptr<RunProgramConfig>;
  using RunProgramConfigList = QList<RunProgramConfigPtr>;

  QString m_defaultAudioTrackLanguage, m_defaultVideoTrackLanguage, m_defaultSubtitleTrackLanguage;
  QString m_chapterNameTemplate, m_defaultChapterLanguage, m_defaultChapterCountry, m_defaultSubtitleCharset, m_defaultAdditionalMergeOptions;
  QStringList m_oftenUsedLanguages, m_oftenUsedCountries, m_oftenUsedCharacterSets;
  ProcessPriority m_priority;
  QTabWidget::TabPosition m_tabPosition;
  QDir m_lastOpenDir, m_lastOutputDir, m_lastConfigDir;
  bool m_setAudioDelayFromFileName, m_autoSetFileTitle, m_disableCompressionForAllTrackTypes, m_disableDefaultTrackForSubtitles, m_mergeAlwaysShowOutputFileControls, m_dropLastChapterFromBlurayPlaylist;
  ClearMergeSettingsAction m_clearMergeSettings;
  AddingAppendingFilesPolicy m_mergeAddingAppendingFilesPolicy;
  TrackPropertiesLayout m_mergeTrackPropertiesLayout;

  OutputFileNamePolicy m_outputFileNamePolicy;
  QDir m_relativeOutputDir, m_fixedOutputDir;
  bool m_uniqueOutputFileNames;

  ScanForPlaylistsPolicy m_scanForPlaylistsPolicy;
  unsigned int m_minimumPlaylistDuration;

  JobRemovalPolicy m_jobRemovalPolicy;
  bool m_useDefaultJobDescription, m_showOutputOfAllJobs, m_switchToJobOutputAfterStarting, m_resetJobWarningErrorCountersOnExit;

  bool m_checkForUpdates;
  QDateTime m_lastUpdateCheck;

  bool m_disableAnimations, m_showToolSelector, m_warnBeforeClosingModifiedTabs, m_warnBeforeAbortingJobs, m_warnBeforeOverwriting, m_showMoveUpDownButtons;
  QString m_uiLocale, m_uiFontFamily;
  int m_uiFontPointSize;

  bool m_enableMuxingTracksByLanguage, m_enableMuxingAllVideoTracks, m_enableMuxingAllAudioTracks, m_enableMuxingAllSubtitleTracks;
  QStringList m_enableMuxingTracksByTheseLanguages;

  QHash<QString, QList<int> > m_splitterSizes;

  QString m_mediaInfoExe;

  RunProgramConfigList m_runProgramConfigurations;

public:
  Settings();
  void load();
  void save() const;

  QString priorityAsString() const;
  QString actualMkvmergeExe() const;

  void setValue(QString const &group, QString const &key, QVariant const &value);
  QVariant value(QString const &group, QString const &key, QVariant const &defaultValue = QVariant{}) const;

  void handleSplitterSizes(QSplitter *splitter);
  void restoreSplitterSizes(QSplitter *splitter);

  QString localeToUse(QString const &requestedLocale = {}) const;

  QString lastOpenDirPath() const;
  QString lastConfigDirPath() const;

public slots:
  void storeSplitterSizes();

protected:
  void loadDefaults(QSettings &reg, QString const &guiVersion);
  void loadSplitterSizes(QSettings &reg);
  void loadRunProgramConfigurations(QSettings &reg);

  void saveDefaults(QSettings &reg) const;
  void saveSplitterSizes(QSettings &reg) const;
  void saveRunProgramConfigurations(QSettings &reg) const;

protected:
  static Settings s_settings;

  static void withGroup(QString const &group, std::function<void(QSettings &)> worker);

public:
  static Settings &get();
  static void change(std::function<void(Settings &)> worker);
  static std::unique_ptr<QSettings> registry();

  static QString exeWithPath(QString const &exe);

  static void migrateFromRegistry();
  static void convertOldSettings();

  static QString iniFileLocation();
  static QString iniFileName();
};

// extern Settings g_settings;

}}}

Q_DECLARE_OPERATORS_FOR_FLAGS(mtx::gui::Util::Settings::RunProgramForEvents);

#endif  // MTX_MKVTOOLNIX_GUI_UTIL_SETTINGS_H
