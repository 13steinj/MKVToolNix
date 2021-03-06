#pragma once

#include "common/common_pch.h"

#include <QString>
#include <QHash>

#include "mkvtoolnix-gui/merge/enums.h"

namespace mtx::gui::Merge {

struct MkvmergeOptionBuilder {
  QStringList options;
  QHash<TrackType, unsigned int> numTracksOfType;
  QHash<TrackType, QStringList> enabledTrackIds;

  MkvmergeOptionBuilder() {}
};

}
