/**
   mkvpropedit -- utility for editing properties of existing Matroska files

   Distributed under the GPL v2
   see the file COPYING for details
   or visit https://www.gnu.org/licenses/old-licenses/gpl-2.0.html

   \file

   \author Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#include "common/common_pch.h"

#include <string>
#include <vector>

#include <ebml/EbmlBinary.h>
#include <ebml/EbmlFloat.h>
#include <ebml/EbmlSInteger.h>
#include <ebml/EbmlString.h>
#include <ebml/EbmlUInteger.h>
#include <ebml/EbmlUnicodeString.h>
#include <matroska/KaxInfo.h>
#include <matroska/KaxInfoData.h>
#include <matroska/KaxTracks.h>
#include <matroska/KaxTrackAudio.h>
#include <matroska/KaxTrackEntryData.h>
#include <matroska/KaxTrackVideo.h>

#include "common/common_pch.h"
#include "common/ebml.h"
#include "common/property_element.h"
#include "common/translation.h"

using namespace libmatroska;

std::map<uint32_t, std::vector<property_element_c> > property_element_c::s_properties;
std::map<uint32_t, std::vector<property_element_c> > property_element_c::s_composed_properties;

property_element_c::property_element_c(std::string name,
                                       EbmlCallbacks const &callbacks,
                                       translatable_string_c title,
                                       translatable_string_c description,
                                       EbmlCallbacks const &sub_master_callbacks,
                                       EbmlCallbacks const *sub_sub_master_callbacks,
                                       EbmlCallbacks const *sub_sub_sub_master_callbacks)
  : m_name{std::move(name)}
  , m_title{std::move(title)}
  , m_description{std::move(description)}
  , m_callbacks{&callbacks}
  , m_sub_master_callbacks{&sub_master_callbacks}
  , m_sub_sub_master_callbacks{sub_sub_master_callbacks}
  , m_sub_sub_sub_master_callbacks{sub_sub_sub_master_callbacks}
  , m_bit_length{128}
  , m_type{EBMLT_SKIP}
{
  derive_type();
}

property_element_c::property_element_c()
  : m_callbacks(nullptr)
  , m_sub_master_callbacks(nullptr)
  , m_type(EBMLT_SKIP)
{
}

bool
property_element_c::is_valid()
  const
{
  return !m_name.empty() && (m_callbacks) && (EBMLT_SKIP != m_type);
}

void
property_element_c::derive_type() {
  EbmlElement *e = &m_callbacks->Create();

  m_type = dynamic_cast<EbmlBinary *>(e)        ? EBMLT_BINARY
         : dynamic_cast<EbmlFloat *>(e)         ? EBMLT_FLOAT
         : dynamic_cast<EbmlSInteger *>(e)      ? EBMLT_INT
         : dynamic_cast<EbmlString *>(e)        ? EBMLT_STRING
         : dynamic_cast<EbmlUInteger *>(e)      ? EBMLT_UINT
         : dynamic_cast<EbmlUnicodeString *>(e) ? EBMLT_USTRING
         : dynamic_cast<EbmlDate *>(e)          ? EBMLT_DATE
         :                                        EBMLT_SKIP;

  if (EBMLT_SKIP == m_type)
    mxerror(fmt::format("property_element_c::derive_type(): programming error: unknown type for EBML ID {0:08x}\n", m_callbacks->GlobalId.Value));

  if ((EBMLT_UINT == m_type) && (m_name.find("flag") != std::string::npos))
    m_type = EBMLT_BOOL;

  delete e;
}

#define ELE( name, callbacks, title, description) s_properties[current_index].push_back(property_element_c(name, callbacks, title, description, *sub_master_callbacks, nullptr, nullptr))
#define ELE2(name, callbacks, title, description) s_properties[current_index].push_back(property_element_c(name, callbacks, title, description, *sub_master_callbacks, sub_sub_master_callbacks))
#define ELE3(name, callbacks, title, description) s_properties[current_index].push_back(property_element_c(name, callbacks, title, description, *sub_master_callbacks, sub_sub_master_callbacks, sub_sub_sub_master_callbacks))

void
property_element_c::init_tables() {
  EbmlCallbacks const *sub_master_callbacks = nullptr, *sub_sub_master_callbacks = nullptr, *sub_sub_sub_master_callbacks = nullptr;

  s_properties.clear();

  s_properties[KaxInfo::ClassInfos.GlobalId.Value]   = std::vector<property_element_c>();
  s_properties[KaxTracks::ClassInfos.GlobalId.Value] = std::vector<property_element_c>();
  uint32_t current_index                             = KaxInfo::ClassInfos.GlobalId.Value;

  ELE("title",                KaxTitle::ClassInfos,           YT("Title"),                        YT("The title for the whole movie."));
  ELE("date",                 KaxDateUTC::ClassInfos,         YT("Date"),                         YT("The date the file was created."));
  ELE("segment-filename",     KaxSegmentFilename::ClassInfos, YT("Segment filename"),             YT("The file name for this segment."));
  ELE("prev-filename",        KaxPrevFilename::ClassInfos,    YT("Previous filename"),            YT("An escaped filename corresponding to the previous segment."));
  ELE("next-filename",        KaxNextFilename::ClassInfos,    YT("Next filename"),                YT("An escaped filename corresponding to the next segment."));
  ELE("segment-uid",          KaxSegmentUID::ClassInfos,      YT("Segment unique ID"),            YT("A randomly generated unique ID to identify the current "
                                                                                                     "segment between many others (128 bits)."));
  ELE("prev-uid",             KaxPrevUID::ClassInfos,         YT("Previous segment's unique ID"), YT("A unique ID to identify the previous chained segment (128 bits)."));
  ELE("next-uid",             KaxNextUID::ClassInfos,         YT("Next segment's unique ID"),     YT("A unique ID to identify the next chained segment (128 bits)."));
  ELE("muxing-application",   KaxMuxingApp::ClassInfos,       YT("Multiplexing application"),     YT("The name of the application or library used for multiplexing the file."));
  ELE("writing-application",  KaxWritingApp::ClassInfos,      YT("Writing application"),          YT("The name of the application or library used for writing the file."));

  current_index = KaxTracks::ClassInfos.GlobalId.Value;

  ELE("track-number",         KaxTrackNumber::ClassInfos,          YT("Track number"),           YT("The track number as used in the Block Header."));
  ELE("track-uid",            KaxTrackUID::ClassInfos,             YT("Track UID"),              YT("A unique ID to identify the Track. This should be kept the same when making a "
                                                                                                    "direct stream copy of the Track to another file."));
  ELE("flag-default",         KaxTrackFlagDefault::ClassInfos,     YT("'Default track' flag"),   YT("Set if that track (audio, video or subs) SHOULD be used if no language found matches the "
                                                                                                    "user preference."));
  ELE("flag-enabled",         KaxTrackFlagEnabled::ClassInfos,     YT("'Track enabled' flag"),   YT("Set if the track is used."));
  ELE("flag-forced",          KaxTrackFlagForced::ClassInfos,      YT("'Forced display' flag"),  YT("Set if that track MUST be used during playback. "
                                                                                                    "There can be many forced track for a kind (audio, video or subs). "
                                                                                                    "The player should select the one whose language matches the user preference or the "
                                                                                                    "default + forced track."));
  ELE("min-cache",            KaxTrackMinCache::ClassInfos,        YT("Minimum cache"),          YT("The minimum number of frames a player should be able to cache during playback. "
                                                                                                    "If set to 0, the reference pseudo-cache system is not used."));
  ELE("max-cache",            KaxTrackMaxCache::ClassInfos,        YT("Maximum cache"),          YT("The maximum number of frames a player should be able to cache during playback. "
                                                                                                    "If set to 0, the reference pseudo-cache system is not used."));
  ELE("default-duration",     KaxTrackDefaultDuration::ClassInfos, YT("Default duration"),       YT("Number of nanoseconds (not scaled) per frame."));
  ELE("name",                 KaxTrackName::ClassInfos,            YT("Name"),                   YT("A human-readable track name."));
  ELE("language",             KaxTrackLanguage::ClassInfos,        YT("Language"),               YT("Specifies the language of the track."));
  ELE("language-ietf",        KaxLanguageIETF::ClassInfos,         YT("Language (IETF BCP 47)"), YT("Specifies the language of the track in the form of a BCP 47 language tag."));
  ELE("codec-id",             KaxCodecID::ClassInfos,              YT("Codec ID"),               YT("An ID corresponding to the codec."));
  ELE("codec-name",           KaxCodecName::ClassInfos,            YT("Codec name"),             YT("A human-readable string specifying the codec."));
  ELE("codec-delay",          KaxCodecDelay::ClassInfos,           YT("Codec-inherent delay"),   YT("Delay built into the codec during decoding in ns."));

  sub_master_callbacks = &KaxTrackVideo::ClassInfos;

  ELE("interlaced",        KaxVideoFlagInterlaced::ClassInfos,  YT("Video interlaced flag"),   YT("Set if the video is interlaced."));
  ELE("pixel-width",       KaxVideoPixelWidth::ClassInfos,      YT("Video pixel width"),       YT("Width of the encoded video frames in pixels."));
  ELE("pixel-height",      KaxVideoPixelHeight::ClassInfos,     YT("Video pixel height"),      YT("Height of the encoded video frames in pixels."));
  ELE("display-width",     KaxVideoDisplayWidth::ClassInfos,    YT("Video display width"),     YT("Width of the video frames to display."));
  ELE("display-height",    KaxVideoDisplayHeight::ClassInfos,   YT("Video display height"),    YT("Height of the video frames to display."));
  ELE("display-unit",      KaxVideoDisplayUnit::ClassInfos,     YT("Video display unit"),      YT("Type of the unit for DisplayWidth/Height (0: pixels, 1: centimeters, 2: inches, 3: aspect ratio)."));
  ELE("pixel-crop-left",   KaxVideoPixelCropLeft::ClassInfos,   YT("Video crop left"),         YT("The number of video pixels to remove on the left of the image."));
  ELE("pixel-crop-top",    KaxVideoPixelCropTop::ClassInfos,    YT("Video crop top"),          YT("The number of video pixels to remove on the top of the image."));
  ELE("pixel-crop-right",  KaxVideoPixelCropRight::ClassInfos,  YT("Video crop right"),        YT("The number of video pixels to remove on the right of the image."));
  ELE("pixel-crop-bottom", KaxVideoPixelCropBottom::ClassInfos, YT("Video crop bottom"),       YT("The number of video pixels to remove on the bottom of the image."));
  ELE("aspect-ratio-type", KaxVideoAspectRatio::ClassInfos,     YT("Video aspect ratio type"), YT("Specify the possible modifications to the aspect ratio "
                                                                                                  "(0: free resizing, 1: keep aspect ratio, 2: fixed)."));
  ELE("field-order",       KaxVideoFieldOrder::ClassInfos,      YT("Video field order"),       YT("Field order (0, 1, 2, 6, 9 or 14, see documentation)."));
  ELE("stereo-mode",       KaxVideoStereoMode::ClassInfos,      YT("Video stereo mode"),       YT("Stereo-3D video mode (0 - 14, see documentation)."));

  sub_master_callbacks         = &KaxTrackVideo::ClassInfos;
  sub_sub_master_callbacks     = &KaxVideoColour::ClassInfos;
  sub_sub_sub_master_callbacks = &KaxVideoColourMasterMeta::ClassInfos;

  ELE2("colour-matrix-coefficients",       KaxVideoColourMatrix::ClassInfos,            YT("Video: colour matrix coefficients"), YT("Sets the matrix coefficients of the video used to derive luma and chroma values "
                                                                                                                                    "from red, green and blue color primaries."));
  ELE2("colour-bits-per-channel",          KaxVideoBitsPerChannel::ClassInfos,          YT("Video: bits per colour channel"),    YT("Sets the number of coded bits for a colour channel."));
  ELE2("chroma-subsample-horizontal",      KaxVideoChromaSubsampHorz::ClassInfos,       YT("Video: pixels to remove in chroma"), YT("The amount of pixels to remove in the Cr and Cb channels for every pixel not removed horizontally."));
  ELE2("chroma-subsample-vertical",        KaxVideoChromaSubsampVert::ClassInfos,       YT("Video: pixels to remove in chroma"), YT("The amount of pixels to remove in the Cr and Cb channels for every pixel not removed vertically."));
  ELE2("cb-subsample-horizontal",          KaxVideoCbSubsampHorz::ClassInfos,           YT("Video: pixels to remove in Cb"),     YT("The amount of pixels to remove in the Cb channel for every pixel not removed horizontally. "
                                                                                                                                    "This is additive with chroma-subsample-horizontal."));
  ELE2("cb-subsample-vertical",            KaxVideoCbSubsampVert::ClassInfos,           YT("Video: pixels to remove in Cb"),     YT("The amount of pixels to remove in the Cb channel for every pixel not removed vertically. "
                                                                                                                                    "This is additive with chroma-subsample-vertical."));
  ELE2("chroma-siting-horizontal",         KaxVideoChromaSitHorz::ClassInfos,           YT("Video: horizontal chroma siting"),   YT("How chroma is sited horizontally."));
  ELE2("chroma-siting-vertical",           KaxVideoChromaSitVert::ClassInfos,           YT("Video: vertical chroma siting"),     YT("How chroma is sited vertically."));
  ELE2("colour-range",                     KaxVideoColourRange::ClassInfos,             YT("Video: colour range"),               YT("Clipping of the color ranges."));
  ELE2("colour-transfer-characteristics",  KaxVideoColourTransferCharacter::ClassInfos, YT("Video: transfer characteristics"),   YT("The colour transfer characteristics of the video."));
  ELE2("colour-primaries",                 KaxVideoColourPrimaries::ClassInfos,         YT("Video: colour primaries"),           YT("The colour primaries of the video."));
  ELE2("max-content-light",                KaxVideoColourMaxCLL::ClassInfos,            YT("Video: maximum content light"),      YT("Maximum brightness of a single pixel in candelas per square meter (cd/m²)."));
  ELE2("max-frame-light",                  KaxVideoColourMaxFALL::ClassInfos,           YT("Video: maximum frame light"),        YT("Maximum frame-average light level in candelas per square meter (cd/m²)."));

  ELE3("chromaticity-coordinates-red-x",   KaxVideoRChromaX::ClassInfos,                YT("Video: chromaticity red X"),         YT("Red X chromaticity coordinate as defined by CIE 1931."));
  ELE3("chromaticity-coordinates-red-y",   KaxVideoRChromaY::ClassInfos,                YT("Video: chromaticity red Y"),         YT("Red Y chromaticity coordinate as defined by CIE 1931."));
  ELE3("chromaticity-coordinates-green-x", KaxVideoGChromaX::ClassInfos,                YT("Video: chromaticity green X"),       YT("Green X chromaticity coordinate as defined by CIE 1931."));
  ELE3("chromaticity-coordinates-green-y", KaxVideoGChromaY::ClassInfos,                YT("Video: chromaticity green Y"),       YT("Green Y chromaticity coordinate as defined by CIE 1931."));
  ELE3("chromaticity-coordinates-blue-x",  KaxVideoBChromaX::ClassInfos,                YT("Video: chromaticity blue X"),        YT("Blue X chromaticity coordinate as defined by CIE 1931."));
  ELE3("chromaticity-coordinates-blue-y",  KaxVideoBChromaY::ClassInfos,                YT("Video: chromaticity blue Y"),        YT("Blue Y chromaticity coordinate as defined by CIE 1931."));
  ELE3("white-coordinates-x",              KaxVideoWhitePointChromaX::ClassInfos,       YT("Video: white point X"),              YT("White colour chromaticity coordinate X as defined by CIE 1931."));
  ELE3("white-coordinates-y",              KaxVideoWhitePointChromaY::ClassInfos,       YT("Video: white point Y"),              YT("White colour chromaticity coordinate Y as defined by CIE 1931."));
  ELE3("max-luminance",                    KaxVideoLuminanceMax::ClassInfos,            YT("Video: maximum luminance"),          YT("Maximum luminance in candelas per square meter (cd/m²)."));
  ELE3("min-luminance",                    KaxVideoLuminanceMin::ClassInfos,            YT("Video: minimum luminance"),          YT("Minimum luminance in candelas per square meter (cd/m²)."));

  sub_sub_master_callbacks = &KaxVideoProjection::ClassInfos;

  ELE2("projection-type",       KaxVideoProjectionType::ClassInfos,      YT("Video: projection type"),             YT("Describes the projection used for this video track (0 – 3)."));
  ELE2("projection-private",    KaxVideoProjectionPrivate::ClassInfos,   YT("Video: projection-specific data"),    YT("Private data that only applies to a specific projection."));
  ELE2("projection-pose-yaw",   KaxVideoProjectionPoseYaw::ClassInfos,   YT("Video: projection's yaw rotation"),   YT("Specifies a yaw rotation to the projection."));
  ELE2("projection-pose-pitch", KaxVideoProjectionPosePitch::ClassInfos, YT("Video: projection's pitch rotation"), YT("Specifies a pitch rotation to the projection."));
  ELE2("projection-pose-roll",  KaxVideoProjectionPoseRoll::ClassInfos,  YT("Video: projection's roll rotation"),  YT("Specifies a roll rotation to the projection."));

  sub_master_callbacks = &KaxTrackAudio::ClassInfos;

  ELE("sampling-frequency",        KaxAudioSamplingFreq::ClassInfos,       YT("Audio sampling frequency"),        YT("Sampling frequency in Hz."));
  ELE("output-sampling-frequency", KaxAudioOutputSamplingFreq::ClassInfos, YT("Audio output sampling frequency"), YT("Real output sampling frequency in Hz."));
  ELE("channels",                  KaxAudioChannels::ClassInfos,           YT("Audio channels"),                  YT("Numbers of channels in the track."));
  ELE("bit-depth",                 KaxAudioBitDepth::ClassInfos,           YT("Audio bit depth"),                 YT("Bits per sample, mostly used for PCM."));

  auto look_up = [](EbmlCallbacks const &callbacks, std::string const &name) -> property_element_c & {
    auto &props = s_properties[callbacks.GlobalId.GetValue()];
    auto itr    = std::find_if(props.begin(), props.end(), [&name](auto const &prop) { return prop.m_name == name; });
    return *itr;
  };

  look_up(KaxTracks::ClassInfos, "projection-private").m_bit_length = 0;
}

std::vector<property_element_c> &
property_element_c::get_table_for(const EbmlCallbacks &master_callbacks,
                                  const EbmlCallbacks *sub_master_callbacks,
                                  bool full_table) {
  if (s_properties.empty())
    init_tables();

  auto src_map_it = s_properties.find(master_callbacks.GlobalId.Value);
  if (s_properties.end() == src_map_it)
    mxerror(fmt::format("property_element_c::get_table_for(): programming error: no table found for EBML ID {0:08x}\n", master_callbacks.GlobalId.Value));

  if (full_table)
    return src_map_it->second;

  uint32_t element_id  = !sub_master_callbacks ? master_callbacks.GlobalId.Value : sub_master_callbacks->GlobalId.Value;
  auto composed_map_it = s_composed_properties.find(element_id);
  if (s_composed_properties.end() != composed_map_it)
    return composed_map_it->second;

  s_composed_properties[element_id]      = std::vector<property_element_c>();
  std::vector<property_element_c> &table = s_composed_properties[element_id];

  for (auto &property : src_map_it->second)
    if (!property.m_sub_master_callbacks || (sub_master_callbacks && (sub_master_callbacks->GlobalId == property.m_sub_master_callbacks->GlobalId)))
      table.push_back(property);

  return table;
}
