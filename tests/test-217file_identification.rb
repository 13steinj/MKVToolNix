#!/usr/bin/ruby -w

class T_217file_identification < Test
  def description
    return "mkvmerge / file identification / in(*)"
  end

  def run
    checksum = ""
    [ "data/avi/v.avi",
      "data/bugs/From_Nero_AVC_Muxer.mp4",
      "data/mkv/complex.mkv",
      "data/mkv/vobsubs.mks",
      "data/mp4/test_2000_inloop.mp4",
      "data/mp4/test_mp2.mp4",
      "data/ogg/v.flac.ogg",
      "data/ogg/v.ogg",
      "data/rm/rv3.rm",
      "data/rm/rv4.rm",
      "data/simple/misdetected_as_mp2.ac3",
      "data/simple/misdetected_as_mpeges.ac3",
      "data/simple/v.aac",
      "data/simple/v.ac3",
      "data/simple/v.flac",
      "data/simple/v.mp3",
      "data/simple/v.wav",
      "data/textsubs/fe.ssa",
      "data/textsubs/vde.srt",
      "data/vobsub/ally1-short.sub",
      "data/wp/with-correction.wv",
      "data/wp/without-correction.wv"
    ].each do |file|
      checksum << "-" if ("" != checksum)
      sys("../src/mkvmerge --identify-verbose #{file} > #{tmp}", 0)
      checksum << hash_tmp
    end

    return checksum
  end
end

