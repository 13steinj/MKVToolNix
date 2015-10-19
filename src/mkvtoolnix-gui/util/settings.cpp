#include "common/common_pch.h"

#include <QDebug>
#include <QSettings>
#include <QSplitter>
#include <QStandardPaths>

#include "common/extern_data.h"
#include "common/iso639.h"
#include "common/qt.h"
#include "common/version.h"
#include "mkvtoolnix-gui/app.h"
#include "mkvtoolnix-gui/util/file_dialog.h"
#include "mkvtoolnix-gui/util/settings.h"

namespace mtx { namespace gui { namespace Util {

Settings Settings::s_settings;

Settings::Settings() {
}

void
Settings::change(std::function<void(Settings &)> worker) {
  worker(s_settings);
  s_settings.save();
}

Settings &
Settings::get() {
  return s_settings;
}

QString
Settings::iniFileLocation() {
#if defined(SYS_WINDOWS)
  if (!App::isInstalled())
    return App::applicationDirPath();

  return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
#else
  return Q("%1/%2/%3").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)).arg(App::organizationName()).arg(App::applicationName());
#endif
}

QString
Settings::iniFileName() {
  return Q("%1/mkvtoolnix-gui.ini").arg(iniFileLocation());
}

void
Settings::migrateFromRegistry() {
#if defined(SYS_WINDOWS)
  // Migration of settings from the Windows registry into an .ini file
  // due to performance issues.

  // If this is the portable version the no settings have to be migrated.
  if (!App::isInstalled())
    return;

  // Don't do anything if such a file exists already.
  auto targetFileName = iniFileName();
  if (QFileInfo{targetFileName}.exists())
    return;

  // Copy all settings.
  QSettings target{targetFileName, QSettings::IniFormat};
  QSettings source{};

  for (auto const &key : source.allKeys())
    target.setValue(key, source.value(key));

  // Ensure the new file is written and remove the keys from the
  // registry.
  target.sync();
  source.clear();
  source.sync();

#else
  // Rename file from .conf to .ini in order to be consistent.
  auto target = QFileInfo{ iniFileName() };
  auto source = QFileInfo{ Q("%1/%2.conf").arg(target.absolutePath()).arg(target.baseName()) };

  if (source.exists())
    QFile{ source.absoluteFilePath() }.rename(target.filePath());
#endif
}

std::unique_ptr<QSettings>
Settings::registry() {
  return std::make_unique<QSettings>(iniFileName(), QSettings::IniFormat);
}

void
Settings::convertOldSettings() {
  auto reg = registry();

  // mergeAlwaysAddDroppedFiles → mergeAddingAppendingFilesPolicy
  reg->beginGroup("settings");
  auto mergeAlwaysAddDroppedFiles = reg->value("mergeAlwaysAddDroppedFiles");

  if (mergeAlwaysAddDroppedFiles.isValid())
    reg->setValue("mergeAddingAppendingFilesPolicy", static_cast<int>(AddingAppendingFilesPolicy::Add));

  reg->remove("mergeAlwaysAddDroppedFiles");
  reg->endGroup();

  // defaultTrackLanguage → defaultAudioTrackLanguage, defaultVideoTrackLanguage, defaultSubtitleTrackLanguage;
  reg->beginGroup("settings");

  auto defaultTrackLanguage = reg->value("defaultTrackLanguage");
  reg->remove("defaultTrackLanguage");

  if (defaultTrackLanguage.isValid()) {
    reg->setValue("defaultAudioTrackLanguage",    defaultTrackLanguage.toString());
    reg->setValue("defaultVideoTrackLanguage",    defaultTrackLanguage.toString());
    reg->setValue("defaultSubtitleTrackLanguage", defaultTrackLanguage.toString());
  }

  reg->endGroup();
}

void
Settings::load() {
  convertOldSettings();

  auto regPtr = registry();
  auto &reg   = *regPtr;

  reg.beginGroup("info");
  auto guiVersion                      = reg.value("guiVersion").toString();
  reg.endGroup();

  reg.beginGroup("settings");
  m_priority                           = static_cast<ProcessPriority>(reg.value("priority", static_cast<int>(NormalPriority)).toInt());
  m_tabPosition                        = static_cast<QTabWidget::TabPosition>(reg.value("tabPosition", static_cast<int>(QTabWidget::North)).toInt());
  m_lastOpenDir                        = QDir{reg.value("lastOpenDir").toString()};
  m_lastOutputDir                      = QDir{reg.value("lastOutputDir").toString()};
  m_lastConfigDir                      = QDir{reg.value("lastConfigDir").toString()};

  m_oftenUsedLanguages                 = reg.value("oftenUsedLanguages").toStringList();
  m_oftenUsedCountries                 = reg.value("oftenUsedCountries").toStringList();
  m_oftenUsedCharacterSets             = reg.value("oftenUsedCharacterSets").toStringList();

  m_scanForPlaylistsPolicy             = static_cast<ScanForPlaylistsPolicy>(reg.value("scanForPlaylistsPolicy", static_cast<int>(AskBeforeScanning)).toInt());
  m_minimumPlaylistDuration            = reg.value("minimumPlaylistDuration", 120).toUInt();

  m_setAudioDelayFromFileName          = reg.value("setAudioDelayFromFileName", true).toBool();
  m_autoSetFileTitle                   = reg.value("autoSetFileTitle",          true).toBool();
  m_clearMergeSettings                 = static_cast<ClearMergeSettingsAction>(reg.value("clearMergeSettings", static_cast<int>(ClearMergeSettingsAction::None)).toInt());
  m_disableCompressionForAllTrackTypes = reg.value("disableCompressionForAllTrackTypes", false).toBool();
  m_disableDefaultTrackForSubtitles    = reg.value("disableDefaultTrackForSubtitles",    false).toBool();
  m_mergeAlwaysShowOutputFileControls  = reg.value("mergeAlwaysShowOutputFileControls",  true).toBool();
  m_mergeUseVerticalInputLayout        = reg.value("mergeUseVerticalInputLayout",        false).toBool();
  m_mergeAddingAppendingFilesPolicy    = static_cast<AddingAppendingFilesPolicy>(reg.value("mergeAddingAppendingFilesPolicy", static_cast<int>(AddingAppendingFilesPolicy::Ask)).toInt());

  m_uniqueOutputFileNames              = reg.value("uniqueOutputFileNames",     true).toBool();
  m_outputFileNamePolicy               = static_cast<OutputFileNamePolicy>(reg.value("outputFileNamePolicy", static_cast<int>(ToSameAsFirstInputFile)).toInt());
  m_relativeOutputDir                  = QDir{reg.value("relativeOutputDir").toString()};
  m_fixedOutputDir                     = QDir{reg.value("fixedOutputDir").toString()};

  m_enableMuxingTracksByLanguage       = reg.value("enableMuxingTracksByLanguage", false).toBool();
  m_enableMuxingAllVideoTracks         = reg.value("enableMuxingAllVideoTracks", true).toBool();
  m_enableMuxingAllAudioTracks         = reg.value("enableMuxingAllAudioTracks", false).toBool();
  m_enableMuxingAllSubtitleTracks      = reg.value("enableMuxingAllSubtitleTracks", false).toBool();
  m_enableMuxingTracksByTheseLanguages = reg.value("enableMuxingTracksByTheseLanguages").toStringList();

  m_useDefaultJobDescription           = reg.value("useDefaultJobDescription",       false).toBool();
  m_showOutputOfAllJobs                = reg.value("showOutputOfAllJobs",            true).toBool();
  m_switchToJobOutputAfterStarting     = reg.value("switchToJobOutputAfterStarting", false).toBool();
  m_resetJobWarningErrorCountersOnExit = reg.value("resetJobWarningErrorCountersOnExit", false).toBool();
  m_jobRemovalPolicy                   = static_cast<JobRemovalPolicy>(reg.value("jobRemovalPolicy", static_cast<int>(JobRemovalPolicy::Never)).toInt());

  m_disableAnimations                  = reg.value("disableAnimations", false).toBool();
  m_showToolSelector                   = reg.value("showToolSelector", true).toBool();
  m_warnBeforeClosingModifiedTabs      = reg.value("warnBeforeClosingModifiedTabs", true).toBool();
  m_warnBeforeAbortingJobs             = reg.value("warnBeforeAbortingJobs", true).toBool();
  m_warnBeforeOverwriting              = reg.value("warnBeforeOverwriting",  true).toBool();
  m_showMoveUpDownButtons              = reg.value("showMoveUpDownButtons", false).toBool();

  m_chapterNameTemplate                = reg.value("chapterNameTemplate", QY("Chapter <NUM:2>")).toString();
  m_dropLastChapterFromBlurayPlaylist  = reg.value("dropLastChapterFromBlurayPlaylist", true).toBool();

  m_mediaInfoExe                       = reg.value("mediaInfoExe", Q("mediainfo-gui")).toString();

#if defined(HAVE_LIBINTL_H)
  m_uiLocale                           = reg.value("uiLocale").toString();
#endif

  reg.beginGroup("updates");
  m_checkForUpdates                    = reg.value("checkForUpdates", true).toBool();
  m_lastUpdateCheck                    = reg.value("lastUpdateCheck", QDateTime{}).toDateTime();
  reg.endGroup();               // settings.updates
  reg.endGroup();               // settings

  reg.beginGroup("defaults");
  m_defaultAudioTrackLanguage          = reg.value("defaultAudioTrackLanguage",    Q("und")).toString();
  m_defaultVideoTrackLanguage          = reg.value("defaultVideoTrackLanguage",    Q("und")).toString();
  m_defaultSubtitleTrackLanguage       = reg.value("defaultSubtitleTrackLanguage", Q("und")).toString();
  m_defaultChapterLanguage             = reg.value("defaultChapterLanguage", Q("und")).toString();
  m_defaultChapterCountry              = reg.value("defaultChapterCountry").toString();
  auto subtitleCharset                 = reg.value("defaultSubtitleCharset").toString();
  m_defaultSubtitleCharset             = guiVersion.isEmpty() && (subtitleCharset == Q("ISO-8859-15")) ? Q("") : subtitleCharset; // Fix for a bug in versions prior to 8.2.0.
  m_defaultAdditionalMergeOptions      = reg.value("defaultAdditionalMergeOptions").toString();
  reg.endGroup();               // defaults

  reg.beginGroup("splitterSizes");

  m_splitterSizes.clear();
  for (auto const &name : reg.childKeys()) {
    auto sizes = reg.value(name).toList();
    for (auto const &size : sizes)
      m_splitterSizes[name] << size.toInt();
  }

  reg.endGroup();               // splitterSizes

  if (m_oftenUsedLanguages.isEmpty())
    for (auto const &languageCode : g_popular_language_codes)
      m_oftenUsedLanguages << Q(languageCode);

  if (m_oftenUsedCountries.isEmpty())
    for (auto const &countryCode : g_popular_country_codes)
      m_oftenUsedCountries << Q(countryCode);

  if (m_oftenUsedCharacterSets.isEmpty())
    for (auto const &characterSet : g_popular_character_sets)
      m_oftenUsedCharacterSets << Q(characterSet);

  if (ToParentOfFirstInputFile == m_outputFileNamePolicy) {
    m_outputFileNamePolicy = ToRelativeOfFirstInputFile;
    m_relativeOutputDir    = Q("..");
  }
}

QString
Settings::actualMkvmergeExe()
  const {
  return exeWithPath(Q("mkvmerge"));
}

void
Settings::save()
  const {
  auto regPtr = registry();
  auto &reg   = *regPtr;

  reg.beginGroup("info");
  reg.setValue("guiVersion",                         Q(get_current_version().to_string()));
  reg.endGroup();

  reg.beginGroup("settings");
  reg.setValue("priority",                           static_cast<int>(m_priority));
  reg.setValue("tabPosition",                        static_cast<int>(m_tabPosition));
  reg.setValue("lastOpenDir",                        m_lastOpenDir.path());
  reg.setValue("lastOutputDir",                      m_lastOutputDir.path());
  reg.setValue("lastConfigDir",                      m_lastConfigDir.path());

  reg.setValue("oftenUsedLanguages",                 m_oftenUsedLanguages);
  reg.setValue("oftenUsedCountries",                 m_oftenUsedCountries);
  reg.setValue("oftenUsedCharacterSets",             m_oftenUsedCharacterSets);

  reg.setValue("scanForPlaylistsPolicy",             static_cast<int>(m_scanForPlaylistsPolicy));
  reg.setValue("minimumPlaylistDuration",            m_minimumPlaylistDuration);

  reg.setValue("setAudioDelayFromFileName",          m_setAudioDelayFromFileName);
  reg.setValue("autoSetFileTitle",                   m_autoSetFileTitle);
  reg.setValue("clearMergeSettings",                 static_cast<int>(m_clearMergeSettings));
  reg.setValue("disableCompressionForAllTrackTypes", m_disableCompressionForAllTrackTypes);
  reg.setValue("disableDefaultTrackForSubtitles",    m_disableDefaultTrackForSubtitles);
  reg.setValue("mergeAlwaysShowOutputFileControls",  m_mergeAlwaysShowOutputFileControls);
  reg.setValue("mergeUseVerticalInputLayout",        m_mergeUseVerticalInputLayout);
  reg.setValue("mergeAddingAppendingFilesPolicy",    static_cast<int>(m_mergeAddingAppendingFilesPolicy));

  reg.setValue("outputFileNamePolicy",               static_cast<int>(m_outputFileNamePolicy));
  reg.setValue("relativeOutputDir",                  m_relativeOutputDir.path());
  reg.setValue("fixedOutputDir",                     m_fixedOutputDir.path());
  reg.setValue("uniqueOutputFileNames",              m_uniqueOutputFileNames);

  reg.setValue("enableMuxingTracksByLanguage",       m_enableMuxingTracksByLanguage);
  reg.setValue("enableMuxingAllVideoTracks",         m_enableMuxingAllVideoTracks);
  reg.setValue("enableMuxingAllAudioTracks",         m_enableMuxingAllAudioTracks);
  reg.setValue("enableMuxingAllSubtitleTracks",      m_enableMuxingAllSubtitleTracks);
  reg.setValue("enableMuxingTracksByTheseLanguages", m_enableMuxingTracksByTheseLanguages);

  reg.setValue("useDefaultJobDescription",           m_useDefaultJobDescription);
  reg.setValue("showOutputOfAllJobs",                m_showOutputOfAllJobs);
  reg.setValue("switchToJobOutputAfterStarting",     m_switchToJobOutputAfterStarting);
  reg.setValue("resetJobWarningErrorCountersOnExit", m_resetJobWarningErrorCountersOnExit);
  reg.setValue("jobRemovalPolicy",                   static_cast<int>(m_jobRemovalPolicy));

  reg.setValue("disableAnimations",                  m_disableAnimations);
  reg.setValue("showToolSelector",                   m_showToolSelector);
  reg.setValue("warnBeforeClosingModifiedTabs",      m_warnBeforeClosingModifiedTabs);
  reg.setValue("warnBeforeAbortingJobs",             m_warnBeforeAbortingJobs);
  reg.setValue("warnBeforeOverwriting",              m_warnBeforeOverwriting);
  reg.setValue("showMoveUpDownButtons",              m_showMoveUpDownButtons);

  reg.setValue("chapterNameTemplate",                m_chapterNameTemplate);
  reg.setValue("dropLastChapterFromBlurayPlaylist",  m_dropLastChapterFromBlurayPlaylist);

  reg.setValue("uiLocale",                           m_uiLocale);

  reg.setValue("mediaInfoExe",                       m_mediaInfoExe);

  reg.beginGroup("updates");
  reg.setValue("checkForUpdates",                    m_checkForUpdates);
  reg.setValue("lastUpdateCheck",                    m_lastUpdateCheck);
  reg.endGroup();               // settings.updates
  reg.endGroup();               // settings

  reg.beginGroup("defaults");
  reg.setValue("defaultAudioTrackLanguage",          m_defaultAudioTrackLanguage);
  reg.setValue("defaultVideoTrackLanguage",          m_defaultVideoTrackLanguage);
  reg.setValue("defaultSubtitleTrackLanguage",       m_defaultSubtitleTrackLanguage);
  reg.setValue("defaultChapterLanguage",             m_defaultChapterLanguage);
  reg.setValue("defaultChapterCountry",              m_defaultChapterCountry);
  reg.setValue("defaultSubtitleCharset",             m_defaultSubtitleCharset);
  reg.setValue("defaultAdditionalMergeOptions",      m_defaultAdditionalMergeOptions);
  reg.endGroup();               // defaults

  reg.beginGroup("splitterSizes");
  for (auto const &name : m_splitterSizes.keys()) {
    auto sizes = QVariantList{};
    for (auto const &size : m_splitterSizes[name])
      sizes << size;
    reg.setValue(name, sizes);
  }
  reg.endGroup();               // splitterSizes
}

QString
Settings::priorityAsString()
  const {
  return LowestPriority == m_priority ? Q("lowest")
       : LowPriority    == m_priority ? Q("lower")
       : NormalPriority == m_priority ? Q("normal")
       : HighPriority   == m_priority ? Q("higher")
       :                                Q("highest");
}

QString
Settings::exeWithPath(QString const &exe) {
  auto path          = bfs::path{ to_utf8(exe) };
  auto program       = path.filename();
  auto installPath   = bfs::path{ to_utf8(App::applicationDirPath()) };
  auto potentialExes = QList<bfs::path>{} << path << (installPath / path) << (installPath / ".." / path);

#if defined(SYS_WINDOWS)
  for (auto &potentialExe : potentialExes)
    potentialExe.replace_extension(bfs::path{"exe"});

  program.replace_extension(bfs::path{"exe"});
#endif  // SYS_WINDOWS

  for (auto const &potentialExe : potentialExes)
    if (bfs::exists(potentialExe))
      return to_qs(potentialExe.string());

  auto location = QStandardPaths::findExecutable(to_qs(program.string()));
  if (!location.isEmpty())
    return location;

  return exe;
}

void
Settings::setValue(QString const &group,
                   QString const &key,
                   QVariant const &value) {
  withGroup(group, [&key, &value](QSettings &reg) {
    reg.setValue(key, value);
  });
}

QVariant
Settings::value(QString const &group,
                QString const &key,
                QVariant const &defaultValue)
  const {
  auto result = QVariant{};

  withGroup(group, [&key, &defaultValue, &result](QSettings &reg) {
    result = reg.value(key, defaultValue);
  });

  return result;
}

void
Settings::withGroup(QString const &group,
                    std::function<void(QSettings &)> worker) {
  auto reg    = registry();
  auto groups = group.split(Q("/"));

  for (auto const &subGroup : groups)
    reg->beginGroup(subGroup);

  worker(*reg);

  for (auto idx = groups.size(); idx > 0; --idx)
    reg->endGroup();
}

void
Settings::handleSplitterSizes(QSplitter *splitter) {
  restoreSplitterSizes(splitter);
  connect(splitter, &QSplitter::splitterMoved, this, &Settings::storeSplitterSizes);
}

void
Settings::restoreSplitterSizes(QSplitter *splitter) {
 auto name   = splitter->objectName();
 auto &sizes = m_splitterSizes[name];

  if (sizes.isEmpty())
    for (auto idx = 0, numWidgets = splitter->count(); idx < numWidgets; ++idx)
      sizes << 1;

  splitter->setSizes(sizes);
}

void
Settings::storeSplitterSizes() {
  auto splitter = dynamic_cast<QSplitter *>(sender());
  if (splitter)
    m_splitterSizes[ splitter->objectName() ] = splitter->sizes();
  else
    qDebug() << "storeSplitterSize() signal from non-splitter" << sender() << sender()->objectName();
}

QString
Settings::localeToUse(QString const &requestedLocale)
  const {
  auto locale = to_utf8(requestedLocale);

#if defined(HAVE_LIBINTL_H)
  translation_c::initialize_available_translations();

  if (locale.empty())
    locale = to_utf8(m_uiLocale);

  if (-1 == translation_c::look_up_translation(locale))
    locale = "";

  if (locale.empty()) {
    locale = boost::regex_replace(translation_c::get_default_ui_locale(), boost::regex{"\\..*", boost::regex::perl}, "");
    if (-1 == translation_c::look_up_translation(locale))
      locale = "";
  }
#endif

  return to_qs(locale);
}

QString
Settings::lastConfigDirPath()
  const {
  return Util::dirPath(m_lastConfigDir);
}

QString
Settings::lastOpenDirPath()
  const {
  return Util::dirPath(m_lastOpenDir);
}

}}}
