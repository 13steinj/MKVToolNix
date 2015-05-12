def create_source_tarball suffix = ""
  require "tmpdir"

  tarball = "#{Dir.pwd}/../mkvtoolnix-#{c(:PACKAGE_VERSION)}#{suffix}.tar.xz"
  fail "#{tarball} does already exist" if FileTest.exists?(tarball)

  Dir.mktmpdir do |dir|
    clone_dir = "#{dir}/mkvtoolnix-#{c(:PACKAGE_VERSION)}"
    commands  = [
      "git clone \"#{Dir.pwd}\" \"#{clone_dir}\"",
      "cd #{clone_dir}",
      "./autogen.sh",
      "git submodule init",
      "git submodule update",
      "rm -rf .git",
      "cd ..",
      "tar cJf \"#{tarball}\" mkvtoolnix-#{c(:PACKAGE_VERSION)}",
    ]
    system commands.join(" && ")
    puts tarball
  end
end
