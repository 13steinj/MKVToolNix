class Target
  def initialize(target)
    @target       = target
    @aliases      = []
    @sources      = []
    @objects      = []
    @libraries    = []
    @dependencies = []
    @only_if      = true
    @debug        = {}
    @desc         = nil
    @namespace    = nil
  end

  def debug(category)
    @debug[category] = !@debug[category]
    self
  end

  def description(description)
    @desc = description
    self
  end

  def only_if(condition)
    @only_if = condition
    self
  end

  def end_if
    only_if true
  end

  def extract_options(list)
    options = list.empty? || !list.last.is_a?(Hash) ? {} : list.pop
    return list, options
  end

  def aliases(*list)
    @aliases += list.compact
    self
  end

  def sources(*list)
    list, options = extract_options list

    if @debug[:sources]
      puts "Target::sources: only_if #{@only_if}; list & options:"
      pp list
      pp options
    end

    return self if !@only_if || (options.include?(:if) && !options[:if])

    list           = list.collect { |e| e.respond_to?(:to_a) ? e.to_a : e }.flatten
    file_mode      = (options[:type] || :file) == :file
    new_sources    = list.collect { |entry| file_mode ? (entry.respond_to?(:to_ia) ? entry.to_a : entry) : FileList["#{entry}/*.c", "#{entry}/*.cpp"].to_a }.flatten
    new_objects    = new_sources.collect { |file| file.ext('o') }
    @sources      += new_sources
    @objects      += new_objects
    @dependencies += new_objects
    self
  end

  def dependencies(*list)
    @dependencies += list.select { |entry| !entry.blank? } if @only_if
    self
  end

  def libraries(*list)
    list, options = extract_options list

    return self if !@only_if || (options.include?(:if) && !options[:if])

    @dependencies += list.collect do |entry|
      case entry
      when :mtxcommon  then "src/common/libmtxcommon." + c(:LIBMTXCOMMONEXT)
      when :mtxinput   then "src/input/libmtxinput.a"
      when :mtxoutput  then "src/output/libmtxoutput.a"
      when :avi        then "lib/avilib-0.6.10/libavi.a"
      when :rmff       then "lib/librmff/librmff.a"
      when :mpegparser then "src/mpegparser/libmpegparser.a"
      else                  nil
      end
    end.compact

    @libraries += list.collect do |entry|
      case entry
      when nil               then nil
      when :magic            then c(:MAGIC_LIBS)
      when :flac             then c(:FLAC_LIBS)
      when :compression      then c(:COMPRESSION_LIBRARIES)
      when :iconv            then c(:ICONV_LIBS)
      when :intl             then c(:LIBINTL_LIBS)
      when :boost_regex      then c(:BOOST_REGEX_LIB)
      when :boost_filesystem then c(:BOOST_FILESYSTEM_LIB)
      when :boost_system     then c(:BOOST_SYSTEM_LIB)
      when :qt               then c(:QT_LIBS)
      when :wxwidgets        then c(:WXWIDGETS_LIBS)
      when String            then entry
      else                        "-l#{entry}"
      end
    end.compact

    self
  end

  def dump
    %w{aliases sources objects dependencies libraries}.each do |type|
      puts "@#{type}:"
      pp instance_variable_get("@#{type}")
    end
    self
  end

  def create
    definition = Proc.new do
      @aliases.each_with_index do |name, idx|
        desc @desc if (0 == idx) && !@desc.empty?
        task name => @target
      end
    end

    if @namespace
      namespace @namespace, &definition
    else
      definition.call
    end

    create_specific

    self
  end
end
