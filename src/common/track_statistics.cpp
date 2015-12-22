/*
   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include "common/date_time.h"
#include "common/strings/formatting.h"
#include "common/tags/tags.h"
#include "common/track_statistics.h"

void
track_statistics_c::create_tags(KaxTags &tags,
                                std::string const &writing_app,
                                boost::posix_time::ptime const &writing_date)
  const {
  auto writing_date_str = !writing_date.is_not_a_date_time() ? mtx::date_time::to_string(writing_date, "%Y-%m-%d %H:%M:%S") : "1970-01-01 00:00:00";
  auto bps              = get_bits_per_second();
  auto duration         = get_duration();

  mtx::tags::remove_simple_tags_for<KaxTagTrackUID>(tags, m_track_uid, "BPS");
  mtx::tags::remove_simple_tags_for<KaxTagTrackUID>(tags, m_track_uid, "DURATION");
  mtx::tags::remove_simple_tags_for<KaxTagTrackUID>(tags, m_track_uid, "NUMBER_OF_FRAMES");
  mtx::tags::remove_simple_tags_for<KaxTagTrackUID>(tags, m_track_uid, "NUMBER_OF_BYTES");

  auto tag = mtx::tags::find_tag_for<KaxTagTrackUID>(tags, m_track_uid, mtx::tags::Movie, true);

  mtx::tags::set_target_type(*tag, mtx::tags::Movie, "MOVIE");

  mtx::tags::set_simple(*tag, "BPS",              ::to_string(bps ? *bps : 0));
  mtx::tags::set_simple(*tag, "DURATION",         format_timestamp(duration ? *duration : 0));
  mtx::tags::set_simple(*tag, "NUMBER_OF_FRAMES", ::to_string(m_num_frames));
  mtx::tags::set_simple(*tag, "NUMBER_OF_BYTES",  ::to_string(m_num_bytes));

  mtx::tags::set_simple(*tag, "_STATISTICS_WRITING_APP",      writing_app);
  mtx::tags::set_simple(*tag, "_STATISTICS_WRITING_DATE_UTC", writing_date_str);
  mtx::tags::set_simple(*tag, "_STATISTICS_TAGS",             "BPS DURATION NUMBER_OF_FRAMES NUMBER_OF_BYTES");

}
