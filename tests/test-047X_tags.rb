#!/usr/bin/ruby -w

class T_047X_tags < Test
  def description
    return "mkvextract / tags / out(XML)"
  end

  def run
    sys("../src/mkvextract tags data/mkv/complex.mkv &> #{tmp}")
    return hash_tmp
  end
end

