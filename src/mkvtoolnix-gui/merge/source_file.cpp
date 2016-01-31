#include "common/common_pch.h"

#include "common/list_utils.h"
#include "common/qt.h"
#include "mkvtoolnix-gui/merge/mkvmerge_option_builder.h"
#include "mkvtoolnix-gui/merge/mux_config.h"
#include "mkvtoolnix-gui/merge/source_file.h"

namespace mtx { namespace gui { namespace Merge {

namespace {

template<typename T>
void
fixAssociationsFor(char const *group,
                   QList<std::shared_ptr<T> > &container,
                   MuxConfig::Loader &l) {
  l.settings.beginGroup(group);

  int idx = 0;
  for (auto &entry : container) {
    l.settings.beginGroup(QString::number(idx));
    entry->fixAssociations(l);
    l.settings.endGroup();
    ++idx;
  }

  l.settings.endGroup();
}

}

SourceFile::SourceFile(QString const &fileName)
  : m_fileName{fileName}
  , m_tracks{}
  , m_additionalParts{}
  , m_appendedFiles{}
  , m_type{FILE_TYPE_IS_UNKNOWN}
  , m_appended{}
  , m_additionalPart{}
  , m_isPlaylist{}
  , m_appendedTo{}
  , m_playlistDuration{}
  , m_playlistSize{}
  , m_playlistChapters{}
{

}

SourceFile::SourceFile(SourceFile const &other)
{
  *this = other;
}

SourceFile::~SourceFile() {
}

SourceFile &
SourceFile::operator =(SourceFile const &other) {
  if (this == &other)
    return *this;

  m_properties       = other.m_properties;
  m_fileName         = other.m_fileName;
  m_type             = other.m_type;
  m_appended         = other.m_appended;
  m_additionalPart   = other.m_additionalPart;
  m_isPlaylist       = other.m_isPlaylist;
  m_playlistDuration = other.m_playlistDuration;
  m_playlistSize     = other.m_playlistSize;
  m_playlistChapters = other.m_playlistChapters;
  m_playlistFiles    = other.m_playlistFiles;
  m_appendedTo       = other.m_appendedTo;

  m_tracks.clear();
  m_additionalParts.clear();
  m_appendedFiles.clear();
  m_playlistFiles.clear();

  for (auto const &track : other.m_tracks)
    m_tracks << std::make_shared<Track>(*track);

  for (auto const &file : other.m_additionalParts)
    m_additionalParts << std::make_shared<SourceFile>(*file);

  for (auto const &file : other.m_appendedFiles)
    m_appendedFiles << std::make_shared<SourceFile>(*file);

  return *this;
}

bool
SourceFile::isValid()
  const {
  return !container().isEmpty() || isAdditionalPart();
}


bool
SourceFile::isRegular()
  const {
  return !m_appended && !m_additionalPart;
}

bool
SourceFile::isAppended()
  const {
  return m_appended;
}

bool
SourceFile::isAdditionalPart()
  const {
  return m_additionalPart;
}

bool
SourceFile::isPlaylist()
  const {
  return m_isPlaylist;
}

bool
SourceFile::hasRegularTrack()
  const {
  return m_tracks.end() != brng::find_if(m_tracks, [](TrackPtr const &track) { return track->isRegular(); });
}

QString
SourceFile::container()
  const {
  return Q(file_type_t::get_name(m_type).get_translated());
}

bool
SourceFile::isTextSubtitleContainer()
  const {
  return mtx::included_in(m_type, FILE_TYPE_SRT, FILE_TYPE_SSA);
}

void
SourceFile::saveSettings(Util::ConfigFile &settings)
  const {
  MuxConfig::saveProperties(settings, m_properties);

  saveSettingsGroup("tracks",          m_tracks,          settings);
  saveSettingsGroup("additionalParts", m_additionalParts, settings);
  saveSettingsGroup("appendedFiles",   m_appendedFiles,   settings);

  settings.setValue("objectID",        reinterpret_cast<qulonglong>(this));
  settings.setValue("fileName",        m_fileName);
  settings.setValue("type",            m_type);
  settings.setValue("appended",        m_appended);
  settings.setValue("additionalPart",  m_additionalPart);
  settings.setValue("appendedTo",      reinterpret_cast<qulonglong>(m_appendedTo));

  auto playlistFiles = QStringList{};
  for (auto const &playlistFile : m_playlistFiles)
    playlistFiles << playlistFile.filePath();

  settings.setValue("isPlaylist",       m_isPlaylist);
  settings.setValue("playlistFiles",    playlistFiles);
  settings.setValue("playlistDuration", static_cast<qulonglong>(m_playlistDuration));
  settings.setValue("playlistSize",     static_cast<qulonglong>(m_playlistSize));
  settings.setValue("playlistChapters", static_cast<qulonglong>(m_playlistChapters));
}

void
SourceFile::loadSettings(MuxConfig::Loader &l) {
  auto objectID = l.settings.value("objectID").toULongLong();
  if ((0 >= objectID) || l.objectIDToSourceFile.contains(objectID))
    throw InvalidSettingsX{};

  l.objectIDToSourceFile[objectID] = this;
  m_fileName                       = l.settings.value("fileName").toString();
  m_type                           = static_cast<file_type_e>(l.settings.value("type").toInt());
  m_appended                       = l.settings.value("appended").toBool();
  m_additionalPart                 = l.settings.value("additionalPart").toBool();
  m_appendedTo                     = reinterpret_cast<SourceFile *>(l.settings.value("appendedTo").toULongLong());

  m_isPlaylist                     = l.settings.value("isPlaylist").toBool();
  auto playlistFiles               = l.settings.value("playlistFiles").toStringList();
  m_playlistDuration               = l.settings.value("playlistDuration").toULongLong();
  m_playlistSize                   = l.settings.value("playlistSize").toULongLong();
  m_playlistChapters               = l.settings.value("playlistChapters").toULongLong();

  m_playlistFiles.clear();
  for (auto const &playlistFile : playlistFiles)
    m_playlistFiles << QFileInfo{playlistFile};

  if ((FILE_TYPE_IS_UNKNOWN > m_type) || (FILE_TYPE_MAX < m_type))
    throw InvalidSettingsX{};

  MuxConfig::loadProperties(l.settings, m_properties);

  loadSettingsGroup<Track>     ("tracks",          m_tracks,          l, [](){ return std::make_shared<Track>(); });
  loadSettingsGroup<SourceFile>("additionalParts", m_additionalParts, l);
  loadSettingsGroup<SourceFile>("appendedFiles",   m_appendedFiles,   l);
}

void
SourceFile::fixAssociations(MuxConfig::Loader &l) {
  if (isRegular() || isAdditionalPart())
    m_appendedTo = nullptr;

  else {
    auto appendedToID = reinterpret_cast<qulonglong>(m_appendedTo);
    if ((0 >= appendedToID) || !l.objectIDToSourceFile.contains(appendedToID))
      throw InvalidSettingsX{};
    m_appendedTo = l.objectIDToSourceFile.value(appendedToID);
  }

  for (auto &track : m_tracks)
    track->m_file = this;

  fixAssociationsFor("tracks",          m_tracks,          l);
  fixAssociationsFor("additionalParts", m_additionalParts, l);
  fixAssociationsFor("appendedFiles",   m_appendedFiles,   l);
}

Track *
SourceFile::findNthOrLastTrackOfType(Track::Type type,
                                     int nth)
  const {
  auto nthFound   = -1;
  auto lastOfType = static_cast<Track *>(nullptr);

  for (auto const &track : m_tracks)
    if (track->m_type == type) {
      ++nthFound;
      if (nthFound == nth)
        return track.get();

      lastOfType = track.get();
    }

  return lastOfType;
}

void
SourceFile::buildMkvmergeOptions(QStringList &options)
  const {
  Q_ASSERT(!isAdditionalPart());

  auto opt = MkvmergeOptionBuilder{};

  for (auto const &track : m_tracks)
    track->buildMkvmergeOptions(opt);

  auto buildTrackIdArg = [&options,&opt](Track::Type type, QString const &enabled, QString const &disabled) {
    if (!enabled.isEmpty() && !opt.enabledTrackIds[type].isEmpty() && (static_cast<unsigned int>(opt.enabledTrackIds[type].size()) < opt.numTracksOfType[type]))
      options << enabled << opt.enabledTrackIds[type].join(Q(","));

    else if (opt.enabledTrackIds[type].isEmpty() && opt.numTracksOfType[type])
      options << disabled;
  };

  buildTrackIdArg(Track::Audio,      Q("--audio-tracks"),    Q("--no-audio"));
  buildTrackIdArg(Track::Video,      Q("--video-tracks"),    Q("--no-video"));
  buildTrackIdArg(Track::Subtitles,  Q("--subtitle-tracks"), Q("--no-subtitles"));
  buildTrackIdArg(Track::Buttons,    Q("--button-tracks"),   Q("--no-buttons"));
  buildTrackIdArg(Track::Attachment, Q("--attachments"),     Q("--no-attachments"));
  buildTrackIdArg(Track::Tags,       Q("--track-tags"),      Q("--no-track-tags"));
  buildTrackIdArg(Track::GlobalTags, Q(""),                  Q("--no-global-tags"));
  buildTrackIdArg(Track::Chapters,   Q(""),                  Q("--no-chapters"));

  options += opt.options;
  if (m_appendedTo)
    options << Q("+");
  options << Q("(") << m_fileName;
  for (auto const &additionalPart : m_additionalParts)
    options << additionalPart->m_fileName;
  options << Q(")");

  for (auto const &appendedFile : m_appendedFiles)
    appendedFile->buildMkvmergeOptions(options);
}

}}}
