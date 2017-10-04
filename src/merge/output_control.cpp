/** \brief output handling

   mkvmerge -- utility for splicing together matroska files
   from component media subtypes

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   \file

   \author Written by Moritz Bunkus <moritz@bunkus.org>.
   \author Modified by Steve Lhomme <steve.lhomme@free.fr>.
*/

#include "common/common_pch.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <cmath>
#include <iostream>
#include <typeinfo>

#include <ebml/EbmlHead.h>
#include <ebml/EbmlSubHead.h>
#include <ebml/EbmlVersion.h>
#include <ebml/EbmlVoid.h>

#include <matroska/FileKax.h>
#include <matroska/KaxAttached.h>
#include <matroska/KaxAttachments.h>
#include <matroska/KaxBlock.h>
#include <matroska/KaxChapters.h>
#include <matroska/KaxCluster.h>
#include <matroska/KaxClusterData.h>
#include <matroska/KaxCues.h>
#include <matroska/KaxCuesData.h>
#include <matroska/KaxInfo.h>
#include <matroska/KaxInfoData.h>
#include <matroska/KaxSeekHead.h>
#include <matroska/KaxSegment.h>
#include <matroska/KaxTags.h>
#include <matroska/KaxTag.h>
#include <matroska/KaxTracks.h>
#include <matroska/KaxTrackEntryData.h>
#include <matroska/KaxTrackAudio.h>
#include <matroska/KaxTrackVideo.h>
#include <matroska/KaxVersion.h>

#include "common/chapters/chapters.h"
#include "common/command_line.h"
#include "common/construct.h"
#include "common/container.h"
#include "common/date_time.h"
#include "common/debugging.h"
#include "common/ebml.h"
#include "common/fs_sys_helpers.h"
#include "common/hacks.h"
#include "common/mm_io_x.h"
#include "common/mm_write_buffer_io.h"
#include "common/strings/formatting.h"
#include "common/tags/tags.h"
#include "common/translation.h"
#include "common/unique_numbers.h"
#include "common/version.h"
#include "merge/cluster_helper.h"
#include "merge/cues.h"
#include "merge/filelist.h"
#include "merge/generic_packetizer.h"
#include "merge/generic_reader.h"
#include "merge/output_control.h"
#include "merge/webm.h"

using namespace libmatroska;
using namespace mtx::construct;

namespace libmatroska {

  class KaxMyDuration: public KaxDuration {
  public:
    KaxMyDuration(const EbmlFloat::Precision prec): KaxDuration() {
      SetPrecision(prec);
    }
  };
}

std::vector<packetizer_t> g_packetizers;
std::vector<filelist_cptr> g_files;
std::vector<attachment_cptr> g_attachments;
std::vector<track_order_t> g_track_order;
std::vector<append_spec_t> g_append_mapping;
std::unordered_map<int64_t, generic_packetizer_c *> g_packetizers_by_track_num;
family_uids_c g_segfamily_uids;

int64_t g_attachment_sizes_first                              = 0;
int64_t g_attachment_sizes_others                             = 0;

kax_info_cptr g_kax_info_chap;

// Variables set by the command line parser.
std::string g_outfile;
int64_t g_file_sizes                                          = 0;
int g_max_blocks_per_cluster                                  = 65535;
int64_t g_max_ns_per_cluster                                  = 5000000000ll;
bool g_write_cues                                             = true;
bool g_cue_writing_requested                                  = false;
generic_packetizer_c *g_video_packetizer                      = nullptr;
bool g_write_meta_seek_for_clusters                           = false;
bool g_no_lacing                                              = false;
bool g_no_linking                                             = true;
bool g_use_durations                                          = false;
bool g_no_track_statistics_tags                               = false;
bool g_write_date                                             = true;

double g_timestamp_scale                                      = TIMESTAMP_SCALE;
timestamp_scale_mode_e g_timestamp_scale_mode                 = timestamp_scale_mode_e{TIMESTAMP_SCALE_MODE_NORMAL};

float g_video_fps                                             = -1.0;
int g_default_tracks[3]                                       = { 0, 0, 0, };
int g_default_tracks_priority[3]                              = { 0, 0, 0, };

bool g_identifying                                            = false;
identification_output_format_e g_identification_output_format = identification_output_format_e::text;

std::unique_ptr<KaxSegment> g_kax_segment;
std::unique_ptr<KaxTracks> g_kax_tracks;
KaxTrackEntry *g_kax_last_entry             = nullptr;
std::unique_ptr<KaxSeekHead> g_kax_sh_main;
std::unique_ptr<KaxSeekHead> g_kax_sh_cues;
mtx::chapters::kax_cptr g_kax_chapters;

std::unique_ptr<KaxTags> g_tags_from_cue_chapters;

std::string g_chapter_file_name;
std::string g_chapter_language;
std::string g_chapter_charset;

std::string g_segmentinfo_file_name;

std::string g_segment_title;
bool g_segment_title_set                    = false;

std::string g_segment_filename, g_previous_segment_filename, g_next_segment_filename;

int64_t g_tags_size                         = 0;

int g_file_num = 1;

int g_split_max_num_files                   = 65535;
std::string g_splitting_by_chapters_arg;

append_mode_e g_append_mode                 = APPEND_MODE_FILE_BASED;
bool s_appending_files                      = false;
auto s_debug_appending                      = debugging_option_c{"append|appending"};
auto s_debug_rerender_track_headers         = debugging_option_c{"rerender|rerender_track_headers"};

std::string g_default_language              = "und";

mtx::bits::value_cptr g_seguid_link_previous;
mtx::bits::value_cptr g_seguid_link_next;
std::deque<mtx::bits::value_cptr> g_forced_seguids;

std::unique_ptr<KaxInfo> s_kax_infos;
static KaxMyDuration *s_kax_duration;

static std::unique_ptr<KaxTags> s_kax_tags;
static mtx::chapters::kax_cptr s_chapters_in_this_file;

static std::unique_ptr<KaxAttachments> s_kax_as;

static std::unique_ptr<EbmlVoid> s_kax_sh_void;
static std::unique_ptr<EbmlVoid> s_kax_chapters_void;
static int64_t s_max_chapter_size           = 0;
static std::unique_ptr<EbmlVoid> s_void_after_track_headers;

static std::vector<std::tuple<timestamp_c, std::string, std::string>> s_additional_chapter_atoms;

static mm_io_cptr s_out;

static mtx::bits::value_c s_seguid_prev(128), s_seguid_current(128), s_seguid_next(128);

static int s_display_files_done           = 0;
static int s_display_path_length          = 1;
static generic_reader_c *s_display_reader = nullptr;

static std::unique_ptr<EbmlHead> s_head;

static std::string s_muxing_app, s_writing_app;
static boost::posix_time::ptime s_writing_date;

static auto s_required_matroska_version      = 1u;
static auto s_required_matroska_read_version = 1u;

/** \brief Add a segment family UID to the list if it doesn't exist already.

  \param family This segment family element is converted to a 128 bit
    element which is added to the list of segment family UIDs.
*/
bool
family_uids_c::add_family_uid(const KaxSegmentFamily &family) {
  mtx::bits::value_c new_uid(family);

  // look for the same UID
  family_uids_c::const_iterator it;
  for (it = begin(); it != end(); it++)
    if (new_uid == *it)
      return false;

  push_back(new_uid);

  return true;
}

static int64_t
calculate_file_duration() {
  return std::llround(static_cast<double>(g_cluster_helper->get_duration()) / static_cast<double>(g_timestamp_scale));
}

/** \brief Fix the file after mkvmerge has been interrupted

   On Unix like systems mkvmerge will install a signal handler. On \c SIGUSR1
   all debug information will be dumped to \c stdout if mkvmerge has been
   compiled with \c -DDEBUG.

   On \c SIGINT mkvmerge will try to sanitize the current output file
   by writing the cues, the meta seek information and by updating the
   segment duration and the segment length.
*/
#if defined(SYS_UNIX) || defined(SYS_APPLE)
void
sighandler(int /* signum */) {
  if (!s_out)
    mxerror(Y("mkvmerge was interrupted by a SIGINT (Ctrl+C?)\n"));

  mxwarn(Y("\nmkvmerge received a SIGINT (probably because the user pressed "
           "Ctrl+C). Trying to sanitize the file. If mkvmerge hangs during "
           "this process you'll have to kill it manually.\n"));

  mxinfo(Y("The file is being fixed, part 1/4..."));
  // Render the cues.
  if (g_write_cues && g_cue_writing_requested)
    cues_c::get().write(*s_out, *g_kax_sh_main);
  mxinfo(Y(" done\n"));

  mxinfo(Y("The file is being fixed, part 2/4..."));
  // Now re-render the kax_duration and fill in the biggest timestamp
  // as the file's duration.
  s_out->save_pos(s_kax_duration->GetElementPosition());
  s_kax_duration->SetValue(calculate_file_duration());
  s_kax_duration->Render(*s_out);
  s_out->restore_pos();
  mxinfo(Y(" done\n"));

  mxinfo(Y("The file is being fixed, part 3/4..."));
  if ((g_kax_sh_main->ListSize() > 0) && !hack_engaged(ENGAGE_NO_META_SEEK)) {
    g_kax_sh_main->UpdateSize();
    if (s_kax_sh_void->ReplaceWith(*g_kax_sh_main, *s_out, true) == INVALID_FILEPOS_T)
      mxwarn(boost::format(Y("This should REALLY not have happened. The space reserved for the first meta seek element was too small. %1%\n")) % BUGMSG);
  }
  mxinfo(Y(" done\n"));

  mxinfo(Y("The file is being fixed, part 4/4..."));
  // Set the correct size for the segment.
  if (g_kax_segment->ForceSize(s_out->getFilePointer() - g_kax_segment->GetElementPosition() - g_kax_segment->HeadSize()))
    g_kax_segment->OverwriteHead(*s_out);

  mxinfo(Y(" done\n"));

  // Manually close s_out because cleanup() will discard any remaining
  // write buffer content in s_out.
  s_out->close();

  cleanup();

  mxerror(Y("mkvmerge was interrupted by a SIGINT (Ctrl+C?)\n"));
}
#endif

static generic_reader_c *
determine_display_reader() {
  if (g_video_packetizer)
    return g_video_packetizer->m_reader;

  auto winner = static_cast<filelist_t const *>(nullptr);
  for (auto &current : g_files)
    if (!current->appending && (0 != current->reader->get_num_packetizers()) && (!winner || (current->size > winner->size)))
      winner = current.get();

  if (winner)
    return winner->reader.get();

  for (auto &current : g_files)
    if (!current->appending && (!winner || (current->size > winner->size)))
      winner = current.get();

  return winner->reader.get();
}

/** \brief Selects a reader for displaying its progress information
*/
static void
display_progress(bool is_100percent = false) {
  static auto s_no_progress             = debugging_option_c{"no_progress"};
  static int64_t s_previous_progress_on = 0;
  static int s_previous_percentage      = -1;

  if (s_no_progress)
    return;

  if (is_100percent) {
    if (mtx::cli::g_gui_mode)
      mxinfo(boost::format("#GUI#progress 100%%\n"));
    else
      mxinfo(boost::format(Y("Progress: 100%%%1%")) % "\r");
    return;
  }

  if (!s_display_reader)
    s_display_reader = determine_display_reader();

  bool display_progress  = false;
  int current_percentage = (s_display_reader->get_progress() + s_display_files_done * 100) / s_display_path_length;
  int64_t current_time   = mtx::sys::get_current_time_millis();

  if (   (-1 == s_previous_percentage)
      || ((100 == current_percentage) && (100 > s_previous_percentage))
      || ((current_percentage != s_previous_percentage) && ((current_time - s_previous_progress_on) >= 500)))
    display_progress = true;

  if (!display_progress)
    return;

  // if (2 < current_percentage)
  //   exit(42);

  if (mtx::cli::g_gui_mode)
    mxinfo(boost::format("#GUI#progress %1%%%\n") % current_percentage);
  else
    mxinfo(boost::format(Y("Progress: %1%%%%2%")) % current_percentage % "\r");

  s_previous_percentage  = current_percentage;
  s_previous_progress_on = current_time;
}

/** \brief Add some tags to the list of all tags
*/
void
add_tags(KaxTag *tags) {
  if (tags->ListSize() == 0)
    return;

  if (!s_kax_tags)
    s_kax_tags = std::make_unique<KaxTags>();

  s_kax_tags->PushElement(*tags);
}

/** \brief Add an attachment

   \param attachment The attachment specification to add
   \return The attachment UID created for this attachment.
*/
int64_t
add_attachment(attachment_cptr const &attachment) {
  // If the attachment is coming from an existing file then we should
  // check if we already have another attachment stored. This can happen
  // if we're concatenating files.
  if (0 != attachment->id) {
    for (auto &ex_attachment : g_attachments)
      if ((   (ex_attachment->id == attachment->id)
           && !hack_engaged(ENGAGE_NO_VARIABLE_DATA))
          ||
          (   (ex_attachment->name             == attachment->name)
           && (ex_attachment->description      == attachment->description)
           && (ex_attachment->data->get_size() == attachment->data->get_size())
           && (ex_attachment->source_file      != attachment->source_file)
           && !attachment->source_file.empty()))
        return attachment->id;

    add_unique_number(attachment->id, UNIQUE_ATTACHMENT_IDS);

  } else
    // No ID yet. Let's assign one.
    attachment->id = create_unique_number(UNIQUE_ATTACHMENT_IDS);

  g_attachments.push_back(attachment);

  return attachment->id;
}

/** \brief Add a packetizer to the list of packetizers
*/
void
add_packetizer_globally(generic_packetizer_c *packetizer) {
  packetizer_t pack;
  pack.packetizer      = packetizer;
  pack.orig_packetizer = packetizer;
  pack.status          = FILE_STATUS_MOREDATA;
  pack.old_status      = pack.status;
  pack.file            = -1;

  int idx = 0;
  for (auto &file : g_files)
    if (file->reader.get() == packetizer->m_reader) {
      pack.file      = idx;
      pack.orig_file = pack.file;
      break;
    } else
      ++idx;

  if (-1 == pack.file)
    mxerror(boost::format(Y("filelist_t not found for generic_packetizer_c. %1%\n")) % BUGMSG);

  g_packetizers.push_back(pack);
}

static void
set_timestamp_scale() {
  bool video_present          = false;
  bool audio_present          = false;
  int64_t highest_sample_rate = 0;

  for (auto &ptzr : g_packetizers)
    if (ptzr.packetizer->get_track_type() == track_video)
      video_present = true;

    else if (ptzr.packetizer->get_track_type() == track_audio) {
      audio_present       = true;
      highest_sample_rate = std::max(static_cast<int64_t>(ptzr.packetizer->get_audio_sampling_freq()), highest_sample_rate);
    }

  bool debug = debugging_c::requested("set_timestamp_scale|timestamp_scale");
  mxdebug_if(debug,
             boost::format("timestamp_scale: %1% audio present: %2% video present: %3% highest sample rate: %4%\n")
             % (  TIMESTAMP_SCALE_MODE_NORMAL == g_timestamp_scale_mode ? "normal"
                : TIMESTAMP_SCALE_MODE_FIXED  == g_timestamp_scale_mode ? "fixed"
                : TIMESTAMP_SCALE_MODE_AUTO   == g_timestamp_scale_mode ? "auto"
                :                                                         "unknown")
             % audio_present % video_present % highest_sample_rate);

  if (   (TIMESTAMP_SCALE_MODE_FIXED != g_timestamp_scale_mode)
      && audio_present
      && (0 < highest_sample_rate)
      && (   !video_present
          || (TIMESTAMP_SCALE_MODE_AUTO == g_timestamp_scale_mode)))
    g_timestamp_scale = static_cast<int64_t>(1000000000.0 / highest_sample_rate - 1.0);

  g_max_ns_per_cluster = std::min<int64_t>(32700 * g_timestamp_scale, g_max_ns_per_cluster);
  GetChild<KaxTimecodeScale>(*s_kax_infos).SetValue(g_timestamp_scale);

  mxdebug_if(debug, boost::format("timestamp_scale: %1% max ns per cluster: %2%\n") % g_timestamp_scale % g_max_ns_per_cluster);
}

bool
set_required_matroska_version(unsigned int required_version) {
  auto previous               = s_required_matroska_version;
  s_required_matroska_version = std::max(s_required_matroska_version, required_version);
  auto version_changed        = s_required_matroska_version != previous;

  if (version_changed)
    rerender_ebml_head();

  return version_changed;
}

bool
set_required_matroska_read_version(unsigned int required_read_version) {
  auto previous                    = s_required_matroska_read_version;
  s_required_matroska_read_version = std::max(s_required_matroska_read_version, required_read_version);

  auto read_version_changed        = s_required_matroska_read_version != previous;
  auto version_changed             = set_required_matroska_version(required_read_version);

  if (read_version_changed && !version_changed)
    rerender_ebml_head();

  return read_version_changed;
}

static void
render_ebml_head(mm_io_c *out) {
  if (!hack_engaged(ENGAGE_NO_CUE_DURATION) || !hack_engaged(ENGAGE_NO_CUE_RELATIVE_POSITION))
    set_required_matroska_version(4);

  if (!hack_engaged(ENGAGE_NO_SIMPLE_BLOCKS))
    set_required_matroska_read_version(2);

  if (!s_head)
    s_head = std::make_unique<EbmlHead>();

  GetChild<EDocType           >(*s_head).SetValue(outputting_webm() ? "webm" : "matroska");
  GetChild<EDocTypeVersion    >(*s_head).SetValue(s_required_matroska_version);
  GetChild<EDocTypeReadVersion>(*s_head).SetValue(s_required_matroska_read_version);

  s_head->Render(*out, true);
}

void
rerender_ebml_head() {
  mm_io_c *out = g_cluster_helper->get_output();

  if (!out || !s_head)
    return;

  out->save_pos(s_head->GetElementPosition());
  render_ebml_head(out);
  out->restore_pos();
}

static void
generate_segment_uids() {
  if (g_cluster_helper->discarding())
    return;

  // Generate the segment UIDs.
  if (hack_engaged(ENGAGE_NO_VARIABLE_DATA)) {
    s_seguid_current.zero_content();
    s_seguid_next.zero_content();
    s_seguid_prev.zero_content();

    return;
  }

  if (1 == g_file_num) {
    if (g_forced_seguids.empty())
      s_seguid_current.generate_random();
    else {
      s_seguid_current = *g_forced_seguids.front();
      g_forced_seguids.pop_front();
    }
    s_seguid_next.generate_random();

    return;
  }

  s_seguid_prev = s_seguid_current;
  if (g_forced_seguids.empty())
    s_seguid_current = s_seguid_next;
  else {
    s_seguid_current = *g_forced_seguids.front();
    g_forced_seguids.pop_front();
  }
  s_seguid_next.generate_random();
}

/** \brief Render the basic EBML and Matroska headers

   Renders the segment information and track headers. Also reserves
   some space with EBML Void elements so that the headers can be
   overwritten safely by the rerender_headers function.
*/
static void
render_headers(mm_io_c *out) {
  try {
    render_ebml_head(out);

    s_kax_infos = std::make_unique<KaxInfo>();

    s_kax_duration = new KaxMyDuration{ !g_video_packetizer || (TIMESTAMP_SCALE_MODE_AUTO == g_timestamp_scale_mode) ? EbmlFloat::FLOAT_64 : EbmlFloat::FLOAT_32};

    s_kax_duration->SetValue(0.0);
    s_kax_infos->PushElement(*s_kax_duration);

    if (s_muxing_app.empty()) {
      auto info_data = get_default_segment_info_data("mkvmerge");
      s_muxing_app   = info_data.muxing_app;
      s_writing_app  = info_data.writing_app;
      s_writing_date = info_data.writing_date;
    }

    GetChild<KaxMuxingApp >(*s_kax_infos).SetValueUTF8(s_muxing_app);
    GetChild<KaxWritingApp>(*s_kax_infos).SetValueUTF8(s_writing_app);

    if (g_write_date)
      GetChild<KaxDateUTC>(*s_kax_infos).SetEpochDate(s_writing_date.is_not_a_date_time() ? 0 : mtx::date_time::to_time_t(s_writing_date));
    else
      DeleteChildren<KaxDateUTC>(*s_kax_infos);

    if (!g_segment_title.empty())
      GetChild<KaxTitle>(*s_kax_infos).SetValueUTF8(g_segment_title.c_str());

    bool first_file = (1 == g_file_num);

    generate_segment_uids();

    if (!outputting_webm()) {
      // Set the segment UIDs.
      GetChild<KaxSegmentUID>(*s_kax_infos).CopyBuffer(s_seguid_current.data(), 128 / 8);

      // Set the segment family
      if (!g_segfamily_uids.empty()) {
        size_t i;
        for (i = 0; i < g_segfamily_uids.size(); i++)
          AddNewChild<KaxSegmentFamily>(*s_kax_infos).CopyBuffer(g_segfamily_uids[i].data(), 128 / 8);
      }

      // Set the chaptertranslate elements
      if (g_kax_info_chap) {
        // copy the KaxChapterTranslates in the current KaxInfo
        KaxChapterTranslate *chapter_translate = FindChild<KaxChapterTranslate>(g_kax_info_chap.get());
        while (chapter_translate) {
          s_kax_infos->PushElement(*new KaxChapterTranslate(*chapter_translate));
          chapter_translate = FindNextChild(*g_kax_info_chap, *chapter_translate);
        }
      }

      if (first_file && g_seguid_link_previous)
        GetChild<KaxPrevUID>(*s_kax_infos).CopyBuffer(g_seguid_link_previous->data(), 128 / 8);

      // The next segment UID is also set in finish_file(). This is not
      // redundant! It is set here as well in order to reserve enough space
      // for the KaxInfo structure in the file. If it is removed later then
      // an EbmlVoid element will be used for the freed space.
      if (g_seguid_link_next)
        GetChild<KaxNextUID>(*s_kax_infos).CopyBuffer(g_seguid_link_next->data(), 128 / 8);

      if (!g_no_linking && g_cluster_helper->splitting()) {
        GetChild<KaxNextUID>(*s_kax_infos).CopyBuffer(s_seguid_next.data(), 128 / 8);

        if (!first_file)
          GetChild<KaxPrevUID>(*s_kax_infos).CopyBuffer(s_seguid_prev.data(), 128 / 8);
      }

      if (!g_segment_filename.empty())
        GetChild<KaxSegmentFilename>(*s_kax_infos).SetValueUTF8(g_segment_filename);

      if (!g_next_segment_filename.empty())
        GetChild<KaxNextFilename>(*s_kax_infos).SetValueUTF8(g_next_segment_filename);

      if (!g_previous_segment_filename.empty())
        GetChild<KaxPrevFilename>(*s_kax_infos).SetValueUTF8(g_previous_segment_filename);

      g_segment_filename.clear();
      g_next_segment_filename.clear();
      g_previous_segment_filename.clear();
    }

    g_kax_segment->WriteHead(*out, 8);

    // Reserve some space for the meta seek stuff.
    g_kax_sh_main = std::make_unique<KaxSeekHead>();
    s_kax_sh_void = std::make_unique<EbmlVoid>();
    s_kax_sh_void->SetSize(4096);
    s_kax_sh_void->Render(*out);

    if (g_write_meta_seek_for_clusters)
      g_kax_sh_cues = std::make_unique<KaxSeekHead>();

    if (first_file) {
      g_kax_last_entry = nullptr;

      size_t i;
      for (i = 0; i < g_track_order.size(); i++)
        if ((g_track_order[i].file_id >= 0) && (g_track_order[i].file_id < static_cast<int>(g_files.size())) && !g_files[g_track_order[i].file_id]->appending)
          g_files[g_track_order[i].file_id]->reader->set_headers_for_track(g_track_order[i].track_id);

      for (i = 0; i < g_files.size(); i++)
        if (!g_files[i]->appending)
          g_files[i]->reader->set_headers();

      set_timestamp_scale();

      for (i = 0; i < g_packetizers.size(); i++)
        if (g_packetizers[i].packetizer)
          g_packetizers[i].packetizer->fix_headers();

    } else
      set_timestamp_scale();

    s_kax_infos->Render(*out, true);
    g_kax_sh_main->IndexThis(*s_kax_infos, *g_kax_segment);

    if (!g_packetizers.empty()) {
      g_kax_tracks->UpdateSize(true);
      uint64_t full_header_size = g_kax_tracks->ElementSize(true);
      g_kax_tracks->UpdateSize(false);

      g_kax_tracks->Render(*out, false);
      g_kax_sh_main->IndexThis(*g_kax_tracks, *g_kax_segment);

      // Reserve some small amount of space for header changes by the
      // packetizers.
      s_void_after_track_headers = std::make_unique<EbmlVoid>();
      s_void_after_track_headers->SetSize(1024 + full_header_size - g_kax_tracks->ElementSize(false));
      s_void_after_track_headers->Render(*out);
    }

  } catch (...) {
    mxerror(boost::format(Y("The track headers could not be rendered correctly. %1%\n")) % BUGMSG);
  }
}

static void
adjust_cluster_seekhead_positions(uint64_t data_start_pos,
                                  uint64_t delta) {
  auto relative_data_start_pos = g_kax_segment->GetRelativePosition(data_start_pos);

  for (auto sh_child : g_kax_sh_cues->GetElementList()) {
    auto seek_entry = dynamic_cast<KaxSeek *>(sh_child);
    if (!seek_entry)
      continue;

    auto seek_position = FindChild<KaxSeekPosition>(*seek_entry);
    if (!seek_position)
      continue;

    auto old_value = seek_position->GetValue();
    if (old_value >= relative_data_start_pos)
      seek_position->SetValue(old_value + delta);
  }
}

static void
adjust_cue_and_seekhead_positions(uint64_t data_start_pos,
                                  uint64_t delta) {
  if (!delta)
    return;

  if (g_cue_writing_requested)
    cues_c::get().adjust_positions(g_kax_segment->GetRelativePosition(data_start_pos), delta);

  if (g_write_meta_seek_for_clusters)
    adjust_cluster_seekhead_positions(data_start_pos, delta);
}

static void
relocate_written_data(uint64_t data_start_pos,
                      uint64_t delta) {
  if (g_cluster_helper->discarding())
    return;

  auto rel_pos_from_end = s_out->get_size() - s_out->getFilePointer();
  auto const block_size = 1024llu * 1024;
  auto to_relocate      = s_out->get_size() - data_start_pos;
  auto relocated        = 0llu;
  auto af_buffer        = memory_c::alloc(block_size);
  auto buffer           = af_buffer->get_buffer();

  mxdebug_if(s_debug_rerender_track_headers,
             boost::format("[rerender] relocate_written_data: void pos %1% void size %2% = data_start_pos %3% s_out size %4% delta %5% to_relocate %6% rel_pos_from_end %7%\n")
             % s_void_after_track_headers->GetElementPosition() % s_void_after_track_headers->ElementSize(true) % data_start_pos % s_out->get_size() % delta % to_relocate % rel_pos_from_end);

  // Extend the file's size. Setting the file pointer to beyond the
  // end and starting to write from there won't work with most of the
  // mm_io_c-derived classes.
  s_out->save_pos(s_out->get_size());
  auto dummy_data = std::make_unique<std::string>(delta, '\0');
  s_out->write(dummy_data->c_str(), dummy_data->length());
  s_out->restore_pos();

  // Copy the data from back to front in order not to overwrite
  // existing data in case it overlaps which is likely.
  while (relocated < to_relocate) {
    auto to_copy = std::min(block_size, to_relocate - relocated);
    auto src_pos = data_start_pos + to_relocate - relocated - to_copy;
    auto dst_pos = src_pos + delta;

    mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender]   relocating %1% bytes from %2% to %3%\n") % to_copy % src_pos % dst_pos);

    s_out->setFilePointer(src_pos);
    auto num_read = s_out->read(buffer, to_copy);

    if (num_read != to_copy) {
      mxinfo(boost::format(Y("Error reading from the file '%1%'.\n")) % s_out->get_file_name());
      mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender]   relocation failed; read only %1% bytes\n") % num_read);
    }

    s_out->setFilePointer(dst_pos);
    auto num_written = s_out->write(buffer, num_read);

    if (num_written != num_read)
      mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender]   relocation failed; wrote only %1% of %2% bytes\n") % num_written % num_read);

    relocated += to_copy;
  }

  if (s_kax_as) {
    mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender]  re-writing attachments; old position %1% new %2%\n") % s_kax_as->GetElementPosition() % (s_kax_as->GetElementPosition() + delta));
    s_out->setFilePointer(s_kax_as->GetElementPosition() + delta);
    s_kax_as->Render(*s_out);
  }

  if (s_kax_chapters_void) {
    mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender]  re-writing chapter placeholder; old position %1% new %2%\n") % s_kax_chapters_void->GetElementPosition() % (s_kax_chapters_void->GetElementPosition() + delta));
    s_out->setFilePointer(s_kax_chapters_void->GetElementPosition() + delta);
    s_kax_chapters_void->Render(*s_out);
  }

  s_out->setFilePointer(rel_pos_from_end, seek_end);

  adjust_cue_and_seekhead_positions(data_start_pos, delta);
}

static void
render_void(int64_t new_size) {
  auto actual_size = new_size;

  s_void_after_track_headers = std::make_unique<EbmlVoid>();
  s_void_after_track_headers->SetSize(new_size);
  s_void_after_track_headers->UpdateSize();

  while (static_cast<int64_t>(s_void_after_track_headers->ElementSize()) > new_size)
    s_void_after_track_headers->SetSize(--actual_size);

  if (static_cast<int64_t>(s_void_after_track_headers->ElementSize()) < new_size)
    s_void_after_track_headers->SetSizeLength(new_size - actual_size - 1);

  mxdebug_if(s_debug_rerender_track_headers, boost::format("[rerender] render_void new_size %1% actual_size %2% size_length %3%\n") % new_size % actual_size % (new_size - actual_size - 1));

  s_void_after_track_headers->Render(*s_out);
}

static void
shrink_void_and_rerender_track_headers(int64_t new_void_size) {
  auto old_void_pos           = s_void_after_track_headers->GetElementPosition();
  auto projected_new_void_pos = g_kax_tracks->GetElementPosition() + g_kax_tracks->ElementSize();

  s_out->setFilePointer(g_kax_tracks->GetElementPosition());

  g_kax_tracks->Render(*s_out, false);
  render_void(new_void_size);

  s_out->setFilePointer(0, seek_end);

  mxdebug_if(s_debug_rerender_track_headers,
             boost::format("[rerender] Normal case, only shrinking void down to %1%, new position %2% projected %9% new full size %3% new end %4% s_out size %5% old void start pos %6% tracks pos %7% tracks size %8%\n")
             % new_void_size                                                                                  // 1
             % s_void_after_track_headers->GetElementPosition()                                               // 2
             % s_void_after_track_headers->ElementSize()                                                      // 3
             % (s_void_after_track_headers->GetElementPosition() + s_void_after_track_headers->ElementSize()) // 4
             % s_out->get_size()                                                                              // 5
             % old_void_pos                                                                                   // 6
             % g_kax_tracks->GetElementPosition()                                                             // 7
             % g_kax_tracks->ElementSize()                                                                    // 8
             % projected_new_void_pos);                                                                       // 9
}

/** \brief Overwrites the track headers with current values

   Can be used by packetizers that have to modify their headers
   depending on the track contents.
*/
void
rerender_track_headers() {
  g_kax_tracks->UpdateSize(false);

  auto position_before    = s_out->getFilePointer();
  auto file_size_before   = s_out->get_size();
  auto new_tracks_end_pos = g_kax_tracks->GetElementPosition() + g_kax_tracks->ElementSize();
  auto data_start_pos     = s_void_after_track_headers->GetElementPosition() + s_void_after_track_headers->ElementSize(true);
  auto data_size          = file_size_before - data_start_pos;
  auto new_void_size      = data_start_pos >= (new_tracks_end_pos + 4) ? data_start_pos - new_tracks_end_pos : 1024;

  mxdebug_if(s_debug_rerender_track_headers,
             boost::format("[rerender] track_headers: new_tracks_end_pos %1% data_start_pos %2% data_size %3% old void at %4% size %5% new_void_size %6%\n")
             % new_tracks_end_pos % data_start_pos % data_size % s_void_after_track_headers->GetElementPosition() % s_void_after_track_headers->ElementSize(true) % new_void_size);

  if (data_size  && (new_tracks_end_pos >= (data_start_pos - 3))) {
    auto delta      = 1024 + new_tracks_end_pos - data_start_pos;
    data_start_pos += delta;
    new_void_size   = 1024;

    relocate_written_data(data_start_pos - delta, delta);
  }

  shrink_void_and_rerender_track_headers(new_void_size);

  mxdebug_if(s_debug_rerender_track_headers,
             boost::format("[rerender] track_headers:   position_before %1% file_size_before %2% (diff %3%) position_after %4% file_size_after %5% (diff %6%) void now at %7% size %8%\n")
             % position_before % file_size_before % (file_size_before - position_before) % s_out->getFilePointer() % s_out->get_size() % (s_out->get_size() - s_out->getFilePointer())
             % s_void_after_track_headers->GetElementPosition() % s_void_after_track_headers->ElementSize(true));
}

/** \brief Render all attachments into the output file at the current position

   This function also makes sure that no duplicates are output. This might
   happen when appending files.
*/
static void
render_attachments(IOCallback *out) {
  s_kax_as   = std::make_unique<KaxAttachments>();
  auto kax_a = static_cast<KaxAttached *>(nullptr);

  for (auto &attachment_p : g_attachments) {
    auto attch = *attachment_p;

    if ((1 == g_file_num) || attch.to_all_files) {
      kax_a = !kax_a ? &GetChild<KaxAttached>(*s_kax_as) : &GetNextChild<KaxAttached>(*s_kax_as, *kax_a);

      if (attch.description != "")
        GetChild<KaxFileDescription>(kax_a).SetValueUTF8(attch.description);

      if (attch.mime_type != "")
        GetChild<KaxMimeType>(kax_a).SetValue(attch.mime_type);

      std::string name;
      if (attch.stored_name == "")
        name = bfs::path{attch.name}.filename().string();

      else
        name = attch.stored_name;

      GetChild<KaxFileName>(kax_a).SetValueUTF8(name);
      GetChild<KaxFileUID >(kax_a).SetValue(attch.id);

      GetChild<KaxFileData>(*kax_a).CopyBuffer(attch.data->get_buffer(), attch.data->get_size());
    }
  }

  if (s_kax_as->ListSize() != 0)
    s_kax_as->Render(*out);
  else
    // Delete the kax_as pointer so that it won't be referenced in a seek head.
    s_kax_as.reset();
}

/** \brief Check the complete append mapping mechanism

   Each entry given with '--append-to' has to be checked for validity.
   For files that aren't managed with '--append-to' default entries have
   to be created.
*/
void
check_append_mapping() {
  std::vector<int64_t>::iterator id;
  std::vector<filelist_cptr>::iterator src_file, dst_file;

  for (auto &amap : g_append_mapping) {
    // Check each mapping entry for validity.

    // 1. Is there a file with the src_file_id?
    if (g_files.size() <= amap.src_file_id)
      mxerror(boost::format(Y("There is no file with the ID '%1%'. The argument for '--append-to' was invalid.\n")) % amap.src_file_id);

    // 2. Is the "source" file in "append mode", meaning does its file name
    // start with a '+'?
    src_file = g_files.begin() + amap.src_file_id;
    if (!(*src_file)->appending)
      mxerror(boost::format(Y("The file no. %1% ('%2%') is not being appended. The argument for '--append-to' was invalid.\n")) % amap.src_file_id % (*src_file)->name);

    // 3. Is there a file with the dst_file_id?
    if (g_files.size() <= amap.dst_file_id)
      mxerror(boost::format(Y("There is no file with the ID '%1%'. The argument for '--append-to' was invalid.\n")) % amap.dst_file_id);

    // 4. G_Files cannot be appended to itself.
    if (amap.src_file_id == amap.dst_file_id)
      mxerror(Y("Files cannot be appended to themselves. The argument for '--append-to' was invalid.\n"));
  }

  // Now let's check each appended file if there are NO append to mappings
  // available (in which case we fill in default ones) or if there are fewer
  // mappings than tracks that are to be copied (which is an error).
  for (auto &src_file : g_files) {
    if (!src_file->appending)
      continue;

    size_t count = boost::count_if(g_append_mapping, [&src_file](auto const &e) { return e.src_file_id == src_file->id; });

    if ((0 < count) && (src_file-> reader->m_used_track_ids.size() > count))
      mxerror(boost::format(Y("Only partial append mappings were given for the file no. %1% ('%2%'). Either don't specify any mapping (in which case the "
                              "default mapping will be used) or specify a mapping for all tracks that are to be copied.\n")) % src_file-> id % src_file-> name);
    else if (0 == count) {
      std::string missing_mappings;

      // Default mapping.
      for (auto id : src_file-> reader->m_used_track_ids) {
        append_spec_t new_amap;

        new_amap.src_file_id  = src_file-> id;
        new_amap.src_track_id = id;
        new_amap.dst_file_id  = src_file-> id - 1;
        new_amap.dst_track_id = id;
        g_append_mapping.push_back(new_amap);

        if (!missing_mappings.empty())
          missing_mappings += ",";
        missing_mappings += (boost::format("%1%:%2%:%3%:%4%") % new_amap.src_file_id % new_amap.src_track_id % new_amap.dst_file_id % new_amap.dst_track_id).str();
      }
      mxinfo(boost::format(Y("No append mapping was given for the file no. %1% ('%2%'). A default mapping of %3% will be used instead. "
                             "Please keep that in mind if mkvmerge aborts with an error message regarding invalid '--append-to' options.\n"))
             % src_file-> id % src_file-> name % missing_mappings);
    }
  }

  // Some more checks.
  for (auto &amap : g_append_mapping) {
    src_file = g_files.begin() + amap.src_file_id;
    dst_file = g_files.begin() + amap.dst_file_id;

    // 5. Does the "source" file have a track with the src_track_id, and is
    // that track selected for copying?
    if (!mtx::includes((*src_file)->reader->m_used_track_ids, amap.src_track_id))
      mxerror(boost::format(Y("The file no. %1% ('%2%') does not contain a track with the ID %3%, or that track is not to be copied. "
                              "The argument for '--append-to' was invalid.\n")) % amap.src_file_id % (*src_file)->name % amap.src_track_id);

    // 6. Does the "destination" file have a track with the dst_track_id, and
    // that track selected for copying?
    if (!mtx::includes((*dst_file)->reader->m_used_track_ids, amap.dst_track_id))
      mxerror(boost::format(Y("The file no. %1% ('%2%') does not contain a track with the ID %3%, or that track is not to be copied. Therefore no "
                              "track can be appended to it. The argument for '--append-to' was invalid.\n")) % amap.dst_file_id % (*dst_file)->name % amap.dst_track_id);

    // 7. Is this track already mapped to somewhere else?
    for (auto &cmp_amap : g_append_mapping) {
      if (cmp_amap == amap)
        continue;

      if (   (cmp_amap.src_file_id  == amap.src_file_id)
          && (cmp_amap.src_track_id == amap.src_track_id))
        mxerror(boost::format(Y("The track %1% from file no. %2% ('%3%') is to be appended more than once. The argument for '--append-to' was invalid.\n"))
                % amap.src_track_id % amap.src_file_id % (*src_file)->name);
    }

    // 8. Is there another track that is being appended to the dst_track_id?
    for (auto &cmp_amap : g_append_mapping) {
      if (cmp_amap == amap)
        continue;

      if (   (cmp_amap.dst_file_id  == amap.dst_file_id)
          && (cmp_amap.dst_track_id == amap.dst_track_id))
        mxerror(boost::format(Y("More than one track is to be appended to the track %1% from file no. %2% ('%3%'). The argument for '--append-to' was invalid.\n"))
                % amap.dst_track_id % amap.dst_file_id % (*dst_file)->name);
    }
  }

  // Finally see if the packetizers can be connected and connect them if they
  // can.
  for (auto &amap : g_append_mapping) {
    src_file      = g_files.begin() + amap.src_file_id;
    dst_file      = g_files.begin() + amap.dst_file_id;

    auto src_ptzr = (*src_file)->reader->find_packetizer_by_id(amap.src_track_id);
    auto dst_ptzr = (*dst_file)->reader->find_packetizer_by_id(amap.dst_track_id);

    if (!src_ptzr || !dst_ptzr)
      mxerror(boost::format("(!src_ptzr || !dst_ptzr). %1%\n") % BUGMSG);

    // And now try to connect the packetizers.
    std::string error_message;
    auto result = src_ptzr->can_connect_to(dst_ptzr, error_message);
    if (CAN_CONNECT_MAYBE_CODECPRIVATE == result)
      mxwarn(boost::format(Y("The track number %1% from the file '%2%' can probably not be appended correctly to the track number %3% from the file '%4%': %5% "
                             "Please make sure that the resulting file plays correctly the whole time. "
                             "The author of this program will probably not give support for playback issues with the resulting file.\n"))
             % amap.src_track_id % g_files[amap.src_file_id]->name
             % amap.dst_track_id % g_files[amap.dst_file_id]->name
             % error_message);

    else if (CAN_CONNECT_YES != result) {
      if (error_message.empty())
        error_message = (  result == CAN_CONNECT_NO_FORMAT     ? Y("The formats do not match.")
                         : result == CAN_CONNECT_NO_PARAMETERS ? Y("The track parameters do not match.")
                         :                                       Y("The reason is unknown."));
      mxerror(boost::format(Y("The track number %1% from the file '%2%' cannot be appended to the track number %3% from the file '%4%'. %5%\n"))
              % amap.src_track_id % g_files[amap.src_file_id]->name
              % amap.dst_track_id % g_files[amap.dst_file_id]->name
              % error_message);
    }

    src_ptzr->connect(dst_ptzr);
    (*dst_file)->appended_to = true;
  }

  // Calculate the "longest path" -- meaning the maximum number of
  // concatenated files. This is needed for displaying the progress.
  for (auto amap = g_append_mapping.begin(), amap_end = g_append_mapping.end(); amap != amap_end; ++amap) {
    // Is this the first in a chain?
    auto cmp_amap = boost::find_if(g_append_mapping, [&amap](auto const &e) {
      return (*amap              != e)
          && (amap->dst_file_id  == e.src_file_id)
          && (amap->dst_track_id == e.src_track_id);
      });

    if (cmp_amap != g_append_mapping.end())
      continue;

    // Find consecutive mappings.
    auto trav_amap  = amap;
    int path_length = 2;
    do {
      for (cmp_amap = g_append_mapping.begin(); cmp_amap != amap_end; ++cmp_amap)
        if (   (trav_amap->src_file_id  == cmp_amap->dst_file_id)
            && (trav_amap->src_track_id == cmp_amap->dst_track_id)) {
          trav_amap = cmp_amap;
          path_length++;
          break;
        }
    } while (cmp_amap != amap_end);

    if (path_length > s_display_path_length)
      s_display_path_length = path_length;
  }
}

/** \brief Add chapters from the readers and calculate the max size

   The reader do not add their chapters to the global chapter pool.
   This has to be done after creating the readers. Only the chapters
   of readers that aren't appended are put into the pool right away.
   The other chapters are added when a packetizer is appended because
   the chapter timestamps have to be adjusted by the length of the file
   the packetizer is appended to.
   This function also calculates the sum of all chapter sizes so that
   enough space can be allocated at the start of each output file.
*/
void
calc_max_chapter_size() {
  // Step 1: Add all chapters from files that are not being appended.
  for (auto &file : g_files) {
    if (file->appending)
      continue;

    if (!file->reader->m_chapters)
      continue;

    if (!g_kax_chapters)
      g_kax_chapters = std::make_shared<KaxChapters>();

    mtx::chapters::move_by_edition(*g_kax_chapters, *file->reader->m_chapters);
    file->reader->m_chapters.reset();
  }

  // Step 2: Fix the mandatory elements and count the size of all chapters.
  s_max_chapter_size = 0;
  if (g_kax_chapters) {
    fix_mandatory_elements(g_kax_chapters.get());
    g_kax_chapters->UpdateSize(true);
    s_max_chapter_size += g_kax_chapters->ElementSize();
  }

  for (auto &file : g_files) {
    auto chapters = file->reader->m_chapters.get();
    if (!chapters)
      continue;

    fix_mandatory_elements(chapters);
    chapters->UpdateSize(true);
    s_max_chapter_size += chapters->ElementSize();
  }
}

void
calc_attachment_sizes() {
  // Calculate the size of all attachments for split control.
  for (auto &att : g_attachments) {
    g_attachment_sizes_first += att->data->get_size();
    if (att->to_all_files)
      g_attachment_sizes_others += att->data->get_size();
  }
}

static void
run_before_file_finished_packetizer_hooks() {
  for (auto &ptzr : g_packetizers)
    ptzr.packetizer->before_file_finished();
}

static void
run_after_file_created_packetizer_hooks() {
  for (auto &ptzr : g_packetizers)
    ptzr.packetizer->after_file_created();
}

void
create_packetizers() {
  // Create the packetizers.
  for (auto &file : g_files) {
    file->reader->m_appending = file->appending;
    file->reader->create_packetizers();

    if (!s_appending_files)
      s_appending_files = file->appending;
  }
}

void
check_track_id_validity() {
  // Check if all track IDs given on the command line are actually
  // present.
  for (auto &file : g_files) {
    file->reader->check_track_ids_and_packetizers();
    file->num_unfinished_packetizers     = file->reader->m_reader_packetizers.size();
    file->old_num_unfinished_packetizers = file->num_unfinished_packetizers;
  }
}

/** \brief Transform the output filename and insert the current file number

   Rules and search order:
   \arg %d
   \arg %[0-9]+d
   \arg . ("-%03d" will be inserted before the .)
   \arg "-%03d" will be appended
*/
std::string
create_output_name() {
  std::string s = g_outfile;
  int p2   = 0;
  // First possibility: %d
  int p    = s.find("%d");
  if (0 <= p) {
    s.replace(p, 2, to_string(g_file_num));

    return s;
  }

  // Now search for something like %02d
  p = s.find("%");
  if (0 <= p) {
    p2 = s.find("d", p + 1);
    if (0 <= p2) {
      int i;
      for (i = p + 1; i < p2; i++)
        if (!isdigit(s[i]))
          break;

      std::string format(&s.c_str()[p]);
      format.erase(p2 - p + 1);
      s.replace(p, format.size(), (boost::format(format) % g_file_num).str());

      return s;
    }
  }

  std::string buffer = (boost::format("-%|1$03d|") % g_file_num).str();

  // See if we can find a '.'.
  p = s.rfind(".");
  if (0 <= p)
    s.insert(p, buffer);
  else
    s.append(buffer);

  return s;
}

void
add_tags_from_cue_chapters() {
  if (!g_tags_from_cue_chapters || ptzrs_in_header_order.empty())
    return;

  bool found = false;
  int tuid   = 0;
  size_t i;
  for (i = 0; ptzrs_in_header_order.size() > i; ++i)
    if (ptzrs_in_header_order[i]->get_track_type() == 'v') {
      found = true;
      tuid  = ptzrs_in_header_order[i]->get_uid();
      break;
    }

  if (!found)
    for (i = 0; ptzrs_in_header_order.size() > i; ++i)
      if (ptzrs_in_header_order[i]->get_track_type() == 'a') {
        found = true;
        tuid  = ptzrs_in_header_order[i]->get_uid();
        break;
      }

  if (!found)
    tuid = ptzrs_in_header_order[0]->get_uid();

  for (auto tag : *g_tags_from_cue_chapters)
    GetChild<KaxTagTrackUID>(GetChild<KaxTagTargets>(*static_cast<KaxTag *>(tag))).SetValue(tuid);

  if (!s_kax_tags)
    s_kax_tags.swap(g_tags_from_cue_chapters);

  else {
    while (g_tags_from_cue_chapters->ListSize() > 0) {
      s_kax_tags->PushElement(*(*g_tags_from_cue_chapters)[0]);
      g_tags_from_cue_chapters->Remove(0);
    }

    g_tags_from_cue_chapters.reset();
  }
}

/** \brief Render an EbmlVoid element as a placeholder for chapters

    Chapters cannot be rendered at the start of the file until the
    file's actual length is known during \c finish_file(). However,
    the maximum size of chapters is know. So we reserve space at the
    beginning of the file for all of the chapters.
 */
static void
render_chapter_void_placeholder() {
  if ((0 >= s_max_chapter_size) && (chapter_generation_mode_e::none == g_cluster_helper->get_chapter_generation_mode()))
    return;

  auto size           = s_max_chapter_size + (chapter_generation_mode_e::none == g_cluster_helper->get_chapter_generation_mode() ? 100 : 1000);
  s_kax_chapters_void = std::make_unique<EbmlVoid>();
  s_kax_chapters_void->SetSize(size);
  s_kax_chapters_void->Render(*s_out);
}

/** \brief Prepare tag elements for rendering

    Adds missing mandatory elements to the tag structures and sorts
    them. Also determines the maximum size needed for rendering the
    tags.

    WebM compliant files support tags, but only a limited subset. All
    unsupported elements will be removed silently.
 */
static void
prepare_tags_for_rendering() {
  if (!s_kax_tags)
    return;

  if (outputting_webm())
    mtx::tags::remove_elements_unsupported_by_webm(*s_kax_tags);

  fix_mandatory_elements(s_kax_tags.get());
  sort_ebml_master(s_kax_tags.get());
  if (!s_kax_tags->CheckMandatory())
    mxerror(boost::format(Y("Some tag elements are missing (this error should not have occured - another similar error should have occured earlier). %1%\n")) % BUGMSG);

  s_kax_tags->UpdateSize();
  g_tags_size = s_kax_tags->ElementSize();
}

/** \brief Creates the next output file

   Creates a new file name depending on the split settings. Opens that
   file for writing and calls \c render_headers(). Also renders
   attachments if they exist and the chapters if no splitting is used.
*/
void
create_next_output_file() {
  auto s_debug = debugging_option_c{"splitting"};
  mxdebug_if(s_debug, boost::format("splitting: Create next destination file; splitting? %1% discarding? %2%\n") % g_cluster_helper->splitting() % g_cluster_helper->discarding());

  auto this_outfile   = g_cluster_helper->split_mode_produces_many_files() ? create_output_name() : g_outfile;
  g_kax_segment       = std::make_unique<KaxSegment>();

  // Open the output file.
  try {
    s_out = !g_cluster_helper->discarding() ? mm_write_buffer_io_c::open(this_outfile, 20 * 1024 * 1024) : mm_io_cptr{ new mm_null_io_c{this_outfile} };
  } catch (mtx::mm_io::exception &ex) {
    mxerror(boost::format(Y("The file '%1%' could not be opened for writing: %2%.\n")) % this_outfile % ex);
  }

  if (verbose && !g_cluster_helper->discarding())
    mxinfo(boost::format(Y("The file '%1%' has been opened for writing.\n")) % this_outfile);

  g_cluster_helper->set_output(s_out.get());

  render_headers(s_out.get());
  render_attachments(s_out.get());
  render_chapter_void_placeholder();
  add_tags_from_cue_chapters();
  prepare_tags_for_rendering();

  if (g_cluster_helper->discarding())
    return;

  s_chapters_in_this_file.reset();

  run_after_file_created_packetizer_hooks();

  ++g_file_num;
}

static void
add_chapters_for_current_part() {
  auto s_debug = debugging_option_c{"splitting_chapters"};

  mxdebug_if(s_debug, boost::format("Adding chapters. have_global? %1% splitting? %2%\n") % !!g_kax_chapters % g_cluster_helper->splitting());

  if (!g_cluster_helper->splitting()) {
    s_chapters_in_this_file = clone(g_kax_chapters);
    mtx::chapters::merge_entries(*s_chapters_in_this_file);
    sort_ebml_master(s_chapters_in_this_file.get());
    return;
  }

  int64_t start                   = g_cluster_helper->get_first_timestamp_in_part();
  int64_t end                     = g_cluster_helper->get_max_timestamp_in_file(); // start + g_cluster_helper->get_duration();
  int64_t offset                  = g_no_linking ? g_cluster_helper->get_first_timestamp_in_file() + g_cluster_helper->get_discarded_duration() : 0;

  auto chapters_here              = clone(g_kax_chapters);
  bool have_chapters_in_timeframe = mtx::chapters::select_in_timeframe(chapters_here.get(), start, end, offset);

  mxdebug_if(s_debug, boost::format("offset %1% start %2% end %3% have chapters in timeframe? %4% chapters in this file? %5%\n") % offset % start % end % have_chapters_in_timeframe % !!s_chapters_in_this_file);

  if (!have_chapters_in_timeframe)
    return;

  if (!s_chapters_in_this_file)
    s_chapters_in_this_file = chapters_here;
  else
    mtx::chapters::move_by_edition(*s_chapters_in_this_file, *chapters_here);

  mtx::chapters::merge_entries(*s_chapters_in_this_file);
  sort_ebml_master(s_chapters_in_this_file.get());
}

void
add_chapter_atom(timestamp_c const &start_timestamp,
                 std::string const &name,
                 std::string const &language) {
  s_additional_chapter_atoms.emplace_back(start_timestamp, name, language);
}

static void
prepare_additional_chapter_atoms_for_rendering() {
  if (s_additional_chapter_atoms.empty())
    return;

  if (!s_chapters_in_this_file)
    s_chapters_in_this_file = std::make_shared<KaxChapters>();

  auto offset   = timestamp_c::ns(g_no_linking ? g_cluster_helper->get_first_timestamp_in_file() + g_cluster_helper->get_discarded_duration() : 0);
  auto &edition = GetChild<KaxEditionEntry>(*s_chapters_in_this_file);

  if (!FindChild<KaxEditionUID>(edition))
    GetChild<KaxEditionUID>(edition).SetValue(create_unique_number(UNIQUE_EDITION_IDS));

  for (auto const &additional_chapter : s_additional_chapter_atoms) {
    auto atom = cons<KaxChapterAtom>(new KaxChapterUID,       create_unique_number(UNIQUE_CHAPTER_IDS),
                                     new KaxChapterTimeStart, (std::get<0>(additional_chapter) - offset).to_ns(),
                                     cons<KaxChapterDisplay>(new KaxChapterString,   std::get<1>(additional_chapter),
                                                             new KaxChapterLanguage, std::get<2>(additional_chapter)));
    edition.PushElement(*atom);
  }

  s_additional_chapter_atoms.clear();
}

static void
render_chapters() {
  prepare_additional_chapter_atoms_for_rendering();

  if (!s_chapters_in_this_file) {
    s_kax_chapters_void.reset();
    return;
  }

  fix_mandatory_elements(s_chapters_in_this_file.get());
  mtx::chapters::fix_country_codes(*s_chapters_in_this_file);

  if (outputting_webm())
    mtx::chapters::remove_elements_unsupported_by_webm(*s_chapters_in_this_file);

  auto replaced = false;
  if (s_kax_chapters_void) {
    auto with_defaults = !outputting_webm();
    replaced           = s_kax_chapters_void->ReplaceWith(*s_chapters_in_this_file, *s_out, true, with_defaults);
  }

  if (!replaced) {
    s_out->setFilePointer(0, seek_end);
    s_chapters_in_this_file->Render(*s_out);
  }

  s_kax_chapters_void.reset();
}

static KaxTags *
set_track_statistics_tags(KaxTags *tags) {
  if (g_no_track_statistics_tags || outputting_webm())
    return tags;

  if (!tags)
    tags = new KaxTags;

  g_cluster_helper->create_tags_for_track_statistics(*tags, s_writing_app, s_writing_date);

  return tags;
}

/** \brief Finishes and closes the current file

   Renders the data that is generated during the muxing run. The cues
   and meta seek information are rendered at the end. If splitting is
   active the chapters are stripped to those that actually lie in this
   file and rendered at the front.  The segment duration and the
   segment size are set to their actual values.
*/
void
finish_file(bool last_file,
            bool create_new_file,
            bool previously_discarding) {
  if (g_kax_chapters && !previously_discarding)
    add_chapters_for_current_part();

  if (!last_file && !create_new_file)
    return;

  run_before_file_finished_packetizer_hooks();

  bool do_output = verbose && !dynamic_cast<mm_null_io_c *>(s_out.get());
  if (do_output)
    mxinfo("\n");

  // Render the track headers a second time if the user has requested that.
  if (hack_engaged(ENGAGE_WRITE_HEADERS_TWICE)) {
    auto second_tracks = clone(g_kax_tracks);
    second_tracks->Render(*s_out);
    g_kax_sh_main->IndexThis(*second_tracks, *g_kax_segment);
  }

  // Render the cues.
  if (g_write_cues && g_cue_writing_requested) {
    if (do_output)
      mxinfo(Y("The cue entries (the index) are being written...\n"));
    cues_c::get().write(*s_out, *g_kax_sh_main);
  }

  // Now re-render the s_kax_duration and fill in the biggest timestamp
  // as the file's duration.
  s_out->save_pos(s_kax_duration->GetElementPosition());
  s_kax_duration->SetValue(calculate_file_duration());
  s_kax_duration->Render(*s_out);

  // If splitting is active and this is the last part then handle the
  // 'next segment UID'. If it was given on the command line then set it here.
  // Otherwise remove an existing one (e.g. from file linking during
  // splitting).

  s_kax_infos->UpdateSize(true);
  int64_t info_size = s_kax_infos->ElementSize();
  int changed       = 0;

  if (last_file && g_seguid_link_next) {
    GetChild<KaxNextUID>(*s_kax_infos).CopyBuffer(g_seguid_link_next->data(), 128 / 8);
    changed = 1;

  } else if (last_file || g_no_linking) {
    size_t i;
    for (i = 0; s_kax_infos->ListSize() > i; ++i)
      if (Is<KaxNextUID>((*s_kax_infos)[i])) {
        delete (*s_kax_infos)[i];
        s_kax_infos->Remove(i);
        changed = 2;
        break;
      }
  }

  if (0 != changed) {
    s_out->setFilePointer(s_kax_infos->GetElementPosition());
    s_kax_infos->UpdateSize(true);
    info_size -= s_kax_infos->ElementSize();
    s_kax_infos->Render(*s_out, true);
    if (2 == changed) {
      if (2 < info_size) {
        EbmlVoid void_after_infos;
        void_after_infos.SetSize(info_size);
        void_after_infos.UpdateSize();
        void_after_infos.SetSize(info_size - void_after_infos.HeadSize());
        void_after_infos.Render(*s_out);

      } else if (0 < info_size) {
        char zero[2] = {0, 0};
        s_out->write(zero, info_size);
      }
    }
  }
  s_out->restore_pos();

  // Render the segment info a second time if the user has requested that.
  if (hack_engaged(ENGAGE_WRITE_HEADERS_TWICE)) {
    s_kax_infos->Render(*s_out);
    g_kax_sh_main->IndexThis(*s_kax_infos, *g_kax_segment);
  }

  render_chapters();

  // Render the meta seek information with the cues
  if (g_write_meta_seek_for_clusters && (g_kax_sh_cues->ListSize() > 0) && !hack_engaged(ENGAGE_NO_META_SEEK)) {
    g_kax_sh_cues->UpdateSize();
    g_kax_sh_cues->Render(*s_out);
    g_kax_sh_main->IndexThis(*g_kax_sh_cues, *g_kax_segment);
  }

  // Set the tags for track statistics and render all tags for this
  // file.
  KaxTags *tags_here = nullptr;
  if (s_kax_tags) {
    if (!s_chapters_in_this_file) {
      KaxChapters temp_chapters;
      tags_here = mtx::tags::select_for_chapters(*s_kax_tags, temp_chapters);
    } else
      tags_here = mtx::tags::select_for_chapters(*s_kax_tags, *s_chapters_in_this_file);
  }

  tags_here = set_track_statistics_tags(tags_here);

  if (tags_here && (0 == mtx::tags::count_simple(*tags_here))) {
    delete tags_here;
    tags_here = nullptr;
  }

  if (tags_here) {
    fix_mandatory_elements(tags_here);
    tags_here->UpdateSize();
    tags_here->Render(*s_out, true);

    g_kax_sh_main->IndexThis(*tags_here, *g_kax_segment);
    delete tags_here;
  }

  if (s_chapters_in_this_file) {
    if (!hack_engaged(ENGAGE_NO_CHAPTERS_IN_META_SEEK))
      g_kax_sh_main->IndexThis(*s_chapters_in_this_file, *g_kax_segment);
    s_chapters_in_this_file.reset();
  }

  if (s_kax_as) {
    g_kax_sh_main->IndexThis(*s_kax_as, *g_kax_segment);
    s_kax_as.reset();
  }

  if ((g_kax_sh_main->ListSize() > 0) && !hack_engaged(ENGAGE_NO_META_SEEK)) {
    g_kax_sh_main->UpdateSize();
    if (s_kax_sh_void->ReplaceWith(*g_kax_sh_main, *s_out, true) == INVALID_FILEPOS_T)
      mxwarn(boost::format(Y("This should REALLY not have happened. The space reserved for the first meta seek element was too small. Size needed: %1%. %2%\n"))
             % g_kax_sh_main->ElementSize() % BUGMSG);
  }

  // Set the correct size for the segment.
  int64_t final_file_size = s_out->getFilePointer();
  if (g_kax_segment->ForceSize(final_file_size - g_kax_segment->GetElementPosition() - g_kax_segment->HeadSize()))
    g_kax_segment->OverwriteHead(*s_out);

  s_out.reset();

  g_kax_segment.reset();
  s_kax_sh_void.reset();
  g_kax_sh_main.reset();
  s_void_after_track_headers.reset();
  g_kax_sh_cues.reset();
  s_head.reset();
}

void
force_close_output_file() {
  if (!s_out)
    return;

  auto wb_out = dynamic_cast<mm_write_buffer_io_c *>(s_out.get());
  if (wb_out)
    wb_out->discard_buffer();

  s_out.reset();
}

static void establish_deferred_connections(filelist_t &file);

static void
append_chapters_for_track(filelist_t &src_file,
                          int64_t timestamp_adjustment) {
  // Append some more chapters and adjust their timestamps by the highest
  // timestamp seen in the previous file/the track that we've been searching
  // for above.
  auto chapters = src_file.reader->m_chapters.get();
  if (!chapters)
    return;

  if (!g_kax_chapters)
    g_kax_chapters = std::make_unique<KaxChapters>();
  else
    mtx::chapters::align_uids(*g_kax_chapters, *chapters);

  mtx::chapters::adjust_timestamps(*chapters, timestamp_adjustment);
  mtx::chapters::move_by_edition(*g_kax_chapters, *chapters);
  src_file.reader->m_chapters.reset();
}

/** \brief Append a packetizer to another one

   Appends a packetizer to another one. Finds the packetizer that is
   to replace the current one, informs the user about the action,
   connects the two packetizers and changes the structs to reflect
   the switch.

   \param ptzr The packetizer that is to be replaced.
   \param amap The append specification the replacement is based upon.
*/
void
append_track(packetizer_t &ptzr,
             const append_spec_t &amap,
             filelist_t *deferred_file = nullptr) {
  auto &src_file = *g_files[amap.src_file_id];
  auto &dst_file = *g_files[amap.dst_file_id];

  if (deferred_file)
    src_file.deferred_max_timestamp_seen = deferred_file->reader->m_max_timestamp_seen;

  // Find the generic_packetizer_c that we will be appending to the one
  // stored in ptzr.
  auto gptzr = brng::find_if(src_file.reader->m_reader_packetizers, [&amap](generic_packetizer_c *p) -> bool {
    return amap.src_track_id == static_cast<size_t>(p->m_ti.m_id);
  });

  if (src_file.reader->m_reader_packetizers.end() == gptzr)
    mxerror(boost::format(Y("Could not find gptzr when appending. %1%\n")) % BUGMSG);

  // If we're dealing with a subtitle track or if the appending file contains
  // chapters then we have to suck the previous file dry. See below for the
  // reason (short version: we need all max_timestamp_seen values).
  if (   !dst_file.done
      && (   (APPEND_MODE_FILE_BASED     == g_append_mode)
          || ((*gptzr)->get_track_type() == track_subtitle)
          || src_file.reader->m_chapters)) {
    dst_file.reader->read_all();
    dst_file.num_unfinished_packetizers     = 0;
    dst_file.old_num_unfinished_packetizers = 0;
    dst_file.done                           = true;
    establish_deferred_connections(dst_file);
  }

  if (   !ptzr.deferred
      && (track_subtitle == (*gptzr)->get_track_type())
      && (             0 == dst_file.reader->m_num_video_tracks)
      && (            -1 == src_file.deferred_max_timestamp_seen)
      && g_video_packetizer) {

    for (auto &file : g_files) {
      if (file->done)
        continue;

      auto vptzr = brng::find_if(file->reader->m_reader_packetizers, [](generic_packetizer_c *p) { return p->get_track_type() == track_video; });
      if (vptzr == file->reader->m_reader_packetizers.end())
        continue;

      filelist_t::deferred_connection_t new_def_con;

      ptzr.deferred    = true;
      new_def_con.amap = amap;
      new_def_con.ptzr = &ptzr;
      file->deferred_connections.push_back(new_def_con);

      return;
    }
  }

  if (s_debug_appending) {
    mxdebug(boost::format("appending: reader m_max_timestamp_seen %1% and ptzr to append is %2% ptzr appended to is %3% src_file.appending %4% src_file.appended_to %5% dst_file.appending %6% dst_file.appended_to %7%\n")
            % dst_file.reader->m_max_timestamp_seen % static_cast<void *>(*gptzr) % static_cast<void *>(ptzr.packetizer) % src_file.appending % src_file.appended_to % dst_file.appending % dst_file.appended_to);
    for (auto &rep_ptzr : dst_file.reader->m_reader_packetizers)
      mxdebug(boost::format("  ptzr @ %1% connected_to %2% max_timestamp_seen %3%\n") % static_cast<void *>(rep_ptzr) % rep_ptzr->m_connected_to % rep_ptzr->m_max_timestamp_seen);
  }

  // In rare cases (e.g. empty tracks) a whole file could be skipped
  // without having gotten a single packet through to the timecoding
  // code. Therefore the reader's m_max_timestamp_seen field would
  // still be 0. Therefore we must ensure that each packetizer from a
  // file we're trying to use m_max_timestamp_seen from has already
  // been connected fully. The very first file in a chain (meaning
  // files that are not in "appending to other file mode",
  // filelist_t.appending == false) would be OK as well.
  if (dst_file.appending) {
    std::list<generic_packetizer_c *> not_connected_ptzrs;
    for (auto &check_ptzr : dst_file.reader->m_reader_packetizers)
      if ((check_ptzr != ptzr.packetizer) && (2 != check_ptzr->m_connected_to))
        not_connected_ptzrs.push_back(check_ptzr);

    if (s_debug_appending) {
      std::string result;
      for (auto &out_ptzr : not_connected_ptzrs)
        result += (boost::format(" %1%") % static_cast<void *>(out_ptzr)).str();

      mxdebug(boost::format("appending: check for connection on dst file's packetizers; these are not connected: %1%\n") % result);
    }

    if (!not_connected_ptzrs.empty())
      return;
  }

  mxinfo(boost::format(Y("Appending track %1% from file no. %2% ('%3%') to track %4% from file no. %5% ('%6%').\n"))
         % (*gptzr)->m_ti.m_id % amap.src_file_id % (*gptzr)->m_ti.m_fname % ptzr.packetizer->m_ti.m_id % amap.dst_file_id % ptzr.packetizer->m_ti.m_fname);

  // Is the current file currently used for displaying the progress? If yes
  // then replace it with the next one.
  if (s_display_reader == dst_file.reader.get()) {
    s_display_files_done++;
    s_display_reader = src_file.reader.get();
  }

  // Also fix the ptzr structure and reset the ptzr's state to "I want more".
  generic_packetizer_c *old_packetizer = ptzr.packetizer;
  ptzr.packetizer                      = *gptzr;
  ptzr.file                            = amap.src_file_id;
  ptzr.status                          = FILE_STATUS_MOREDATA;

  // Fix the globally stored video packetizer reference so that
  // decisions based on a packet's source such as when to render a new
  // cluster continue working.
  if (old_packetizer == g_video_packetizer)
    g_video_packetizer = ptzr.packetizer;

  // If we're dealing with a subtitle track or if the appending file contains
  // chapters then we have to do some magic. During splitting timestamps are
  // offset by a certain amount. This amount is NOT the duration of the
  // previous file! That's why we cannot use
  // dst_file.reader->max_timestamp_seen. Instead we have to find the first
  // packet in the appending file because its original timestamp during the
  // split phase was the offset. If we have that we can find the corresponding
  // packetizer and use its max_timestamp_seen.
  //
  // All this only applies to gapless tracks. Good luck with other files.
  // Some files types also allow access to arbitrary tracks and packets
  // (e.g. AVI and Quicktime). Those files will not work correctly for this.
  // But then again I don't expect that people will try to concatenate such
  // files if they've been split before.
  int64_t timestamp_adjustment = dst_file.reader->m_max_timestamp_seen;
  if ((APPEND_MODE_FILE_BASED == g_append_mode)
      && (   (track_subtitle != ptzr.packetizer->get_track_type())
          || !dst_file.reader->is_simple_subtitle_container()))
    // Intentionally left empty.
    ;

  else if (ptzr.deferred && deferred_file)
    timestamp_adjustment = src_file.deferred_max_timestamp_seen;

  else if (   (track_subtitle == ptzr.packetizer->get_track_type())
           && (-1 < src_file.deferred_max_timestamp_seen))
    timestamp_adjustment = src_file.deferred_max_timestamp_seen;

  else if (   (ptzr.packetizer->get_track_type() == track_subtitle)
           || (src_file.reader->m_chapters)) {
    if (!src_file.reader->m_ptzr_first_packet)
      ptzr.status = ptzr.packetizer->read(false);

    if (src_file.reader->m_ptzr_first_packet) {
      auto cmp_amap = brng::find_if(g_append_mapping, [&amap, &src_file](append_spec_t const &m) {
        return (m.src_file_id  == amap.src_file_id)
            && (m.src_track_id == static_cast<size_t>(src_file.reader->m_ptzr_first_packet->m_ti.m_id))
            && (m.dst_file_id  == amap.dst_file_id);
      });

      if (g_append_mapping.end() != cmp_amap) {
        auto gptzr = dst_file.reader->find_packetizer_by_id(cmp_amap->dst_track_id);
        if (gptzr)
          timestamp_adjustment = gptzr->m_max_timestamp_seen;
      }
    }
  }

  if ((APPEND_MODE_FILE_BASED == g_append_mode) || (ptzr.packetizer->get_track_type() == track_subtitle)) {
    mxdebug_if(s_debug_appending, boost::format("appending: new timestamp_adjustment for append_mode == FILE_BASED or subtitle track: %1% for %2%\n") % format_timestamp(timestamp_adjustment) % ptzr.packetizer->m_ti.m_id);
    // The actual connection.
    ptzr.packetizer->connect(old_packetizer, timestamp_adjustment);

  } else {
    mxdebug_if(s_debug_appending, boost::format("appending: new timestamp_adjustment for append_mode == TRACK_BASED and NON subtitle track: %1% for %2%\n") % format_timestamp(timestamp_adjustment) % ptzr.packetizer->m_ti.m_id);
    // The actual connection.
    ptzr.packetizer->connect(old_packetizer);
  }

  append_chapters_for_track(src_file, timestamp_adjustment);

  ptzr.deferred = false;
}

/** \brief Decide if packetizers have to be appended

   Iterates over all current packetizers and decides if the next one
   should be appended now. This is the case if the current packetizer
   has finished and there is another packetizer waiting to be appended.

   \return true if at least one track has been appended to another one.
*/
bool
append_tracks_maybe() {
  bool appended_a_track = false;

  for (auto &ptzr : g_packetizers) {
    if (ptzr.deferred)
      continue;

    if (!g_files[ptzr.orig_file]->appended_to)
      continue;

    if (FILE_STATUS_DONE_AND_DRY != ptzr.status)
      continue;

    append_spec_t *amap = nullptr;
    for (auto &amap_idx : g_append_mapping)
      if ((amap_idx.dst_file_id == static_cast<size_t>(ptzr.file)) && (amap_idx.dst_track_id == static_cast<size_t>(ptzr.packetizer->m_ti.m_id))) {
        amap = &amap_idx;
        break;
      }

    if (!amap)
      continue;

    append_track(ptzr, *amap);
    appended_a_track = true;
  }

  return appended_a_track;
}

/** \brief Establish deferred packetizer connections

   In some cases (e.g. subtitle only files being appended) establishing the
   connections is deferred until a file containing a video track has
   finished, too. This is necessary because the subtitle files themselves
   are usually "shorter" than the movie they belong to. This is not the
   case if the subs are already embedded with a movie in a single file.

   This function iterates over all deferred connections and establishes
   them.

   \param file All connections that have been deferred until this file has
     finished are established.
*/
static void
establish_deferred_connections(filelist_t &file) {
  auto def_cons = file.deferred_connections;
  file.deferred_connections.clear();

  for (auto &def_con : def_cons)
    append_track(*def_con.ptzr, def_con.amap, &file);

  // \todo Select a new file that the subs will defer to.
}

static void
check_and_handle_end_of_input_after_pulling(packetizer_t &ptzr) {
  if (!ptzr.pack && (FILE_STATUS_DONE == ptzr.status))
    ptzr.status = FILE_STATUS_DONE_AND_DRY;

  // Has this packetizer changed its status from "data available" to
  // "file done" during this loop? If so then decrease the number of
  // unfinished packetizers in the corresponding file structure.
  if (   (FILE_STATUS_DONE_AND_DRY != ptzr.status)
      || (ptzr.old_status          == ptzr.status))
    return;

  auto &file = *g_files[ptzr.file];
  file.num_unfinished_packetizers--;

  // If all packetizers for a file have finished then establish the
  // deferred connections.
  if ((0 >= file.num_unfinished_packetizers) && (0 < file.old_num_unfinished_packetizers)) {
    establish_deferred_connections(file);
    file.done = true;
  }
  file.old_num_unfinished_packetizers = file.num_unfinished_packetizers;
}

static bool
force_pull_packetizers_of_fully_held_files() {
  std::unordered_map<generic_reader_c *, bool> fully_held_files;

  for (auto &ptzr : g_packetizers) {
    auto reader = ptzr.packetizer->m_reader;
    auto pos    = fully_held_files.find(reader);

    if (pos == fully_held_files.end())
      fully_held_files[reader] = true;

    if (FILE_STATUS_HOLDING != ptzr.status)
      fully_held_files[reader] = false;
  }

  auto force_pulled = false;
  for (auto &ptzr : g_packetizers)
    if (fully_held_files[ptzr.packetizer->m_reader] && !ptzr.packetizer->packet_available()) {
      ptzr.old_status = ptzr.status;
      ptzr.status     = ptzr.packetizer->read(true);
      force_pulled    = true;

      if (!ptzr.pack)
        ptzr.pack = ptzr.packetizer->get_packet();

      check_and_handle_end_of_input_after_pulling(ptzr);
    }

  return force_pulled;
}

static void
pull_packetizers_for_packets() {
  for (auto &ptzr : g_packetizers) {
    if (FILE_STATUS_HOLDING == ptzr.status)
      ptzr.status = FILE_STATUS_MOREDATA;

    ptzr.old_status = ptzr.status;

    while (   !ptzr.pack
           && (FILE_STATUS_MOREDATA == ptzr.status)
           && !ptzr.packetizer->packet_available())
      ptzr.status = ptzr.packetizer->read(false);

    if (   (FILE_STATUS_MOREDATA != ptzr.status)
        && (FILE_STATUS_MOREDATA == ptzr.old_status))
      ptzr.packetizer->force_duration_on_last_packet();

    if (!ptzr.pack)
      ptzr.pack = ptzr.packetizer->get_packet();

    check_and_handle_end_of_input_after_pulling(ptzr);
  }
}

static packetizer_t *
select_winning_packetizer() {
  packetizer_t *winner = nullptr;

  for (auto &ptzr : g_packetizers) {
    if (!ptzr.pack)
      continue;

    if (!winner || !winner->pack)
      winner = &ptzr;

    else if (ptzr.pack && (ptzr.pack->output_order_timestamp < winner->pack->output_order_timestamp))
      winner = &ptzr;
  }

  return winner;
}

static void
discard_queued_packets() {
  for (auto &ptzr : g_packetizers)
    ptzr.packetizer->discard_queued_packets();

  g_cluster_helper->discard_queued_packets();
}

/** \brief Request packets and handle the next one

   Requests packets from each packetizer, selects the packet with the
   lowest timestamp and hands it over to the cluster helper for
   rendering.  Also displays the progress.
*/
void
main_loop() {
  // Let's go!
  while (1) {
    // Step 1: Make sure a packet is available for each output
    // as long we haven't already processed the last one.
    pull_packetizers_for_packets();
    auto force_pulled = force_pull_packetizers_of_fully_held_files();

    // Step 2: Pick the packet with the lowest timestamp and
    // stuff it into the Matroska file.
    auto winner = select_winning_packetizer();

    // Append the next track if appending is wanted.
    bool appended_a_track = s_appending_files && append_tracks_maybe();

    if (winner && winner->pack) {
      packet_cptr pack = winner->pack;

      // Step 3: Add the winning packet to a cluster. Full clusters will be
      // rendered automatically.
      g_cluster_helper->add_packet(pack);

      winner->pack.reset();

      // If splitting by parts is active and the last part has been
      // processed fully then we can finish up.
      if (g_cluster_helper->is_splitting_and_processed_fully()) {
        discard_queued_packets();
        break;
      }

      // display some progress information
      if (1 <= verbose)
        display_progress();

    } else if (!appended_a_track && !force_pulled) // exit if there are no more packets
      break;
  }

  // Render all remaining packets (if there are any).
  if (g_cluster_helper && (0 < g_cluster_helper->get_packet_count()))
    g_cluster_helper->render();

  if (1 <= verbose)
    display_progress(true);
}

/** \brief Deletes the file readers and other associated objects
*/
static void
destroy_readers() {
  g_files.clear();
  g_packetizers.clear();
}

/** \brief Uninitialization

   Frees memory and shuts down the readers.
*/
void
cleanup() {
  if (s_out) {
    // If cleanup was called as a result of an exception during
    // writing due to the file system being full, the destructor would
    // normally write remaining buffered before closing the file. This
    // would lead to another exception and another error
    // message. However, the regular paths all close the file
    // manually. Therefore any buffered content remaining at this
    // point can only be due to an error having occurred. The content
    // can therefore be discarded.
    dynamic_cast<mm_write_buffer_io_c &>(*s_out).discard_buffer();
    s_out.reset();
  }

  g_cluster_helper.reset();

  destroy_readers();
  g_attachments.clear();

  s_kax_tags.reset();
  g_tags_from_cue_chapters.reset();
  g_kax_chapters.reset();
  s_kax_as.reset();
  g_kax_info_chap.reset();
  g_forced_seguids.clear();
  g_kax_tracks.reset();
  g_kax_segment.reset();
  g_kax_sh_main.reset();
  g_kax_sh_cues.reset();
  s_kax_infos.reset();
  s_head.reset();

  s_chapters_in_this_file.reset();
  s_kax_sh_void.reset();
  s_kax_chapters_void.reset();
  s_void_after_track_headers.reset();

  g_packetizers.clear();
  g_files.clear();
  g_attachments.clear();
  g_track_order.clear();
  g_append_mapping.clear();

  g_seguid_link_previous.reset();
  g_seguid_link_next.reset();
  g_forced_seguids.clear();
}
