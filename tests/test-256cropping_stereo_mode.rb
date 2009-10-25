#!/usr/bin/ruby -w

class T_256cropping_stereo_mode < Test
  def description
    return "mkvmerge / Handling of cropping & stereo mode"
  end

  def get_info(file_name)
    command = "mkvinfo --ui-language en_US \"#{file_name}\" | " +
      "grep -E -i 'crop|stereo' | " +
      "sed -e 's/Stereo mode: /: S/' -e 's/.*: //' -e 's/ .*//' | " +
      "sort -n | " +
      "tr '\n' '-' | " +
      "sed -e 's/-$//'"
    return `#{command}`.chomp
  end

  def run
    result = ""

    merge "--cropping 3:1,2,3,4 --stereo-mode 3:left -A data/mp4/rain_800.mp4"
    [ "", "--cropping 1:5,6,7,8 --stereo-mode 1:right" ].each_with_index do |args, idx|
      merge "#{tmp}#{idx}", "#{args} #{tmp}"
      result += "#{idx}[" + get_info("#{tmp}#{idx}") + "]"
    end

    return result
  end
end

