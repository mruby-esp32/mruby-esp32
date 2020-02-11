MRuby::Build.new do |conf|
  toolchain :gcc

  [conf.cc, conf.objc, conf.asm].each do |cc|
    cc.command = 'gcc'
    cc.flags = [%w(-g -std=gnu99 -O3 -Wall -Werror-implicit-function-declaration -Wdeclaration-after-statement -Wwrite-strings)]
  end

  [conf.cxx].each do |cxx|
    cxx.command = 'g++'
    cxx.flags = [%w(-g -O3 -Wall -Werror-implicit-function-declaration -std=gnu++11 -fno-rtti)]
  end

  conf.linker do |linker|
    linker.command = 'gcc'
    linker.flags = [%w()]
    linker.libraries = %w(m)
    linker.library_paths = []
  end

  conf.archiver do |archiver|
    archiver.command = "ar"
  end

  conf.gembox 'default'
end

MRuby::CrossBuild.new('esp32') do |conf|
  toolchain :gcc

  conf.cc do |cc|
    cc.include_paths << ENV["COMPONENT_INCLUDES"].split(' ')
    cc.include_paths << ENV["COMPONENT_EXTRA_INCLUDES"]&.split(' ')
    cc.flags << '-Wno-maybe-uninitialized'
    #cc.flags << '-g3 -O0'
    cc.flags.collect! { |x|
      if x.kind_of?(Array) and x.size == 1
        x = x.first
      end
      x.gsub('-MP', '') 
    }

    cc.defines << %w(MRB_HEAP_PAGE_SIZE=64)
    cc.defines << %w(MRB_USE_IV_SEGLIST)
    cc.defines << %w(KHASH_DEFAULT_SIZE=8)
    cc.defines << %w(MRB_STR_BUF_MIN_SIZE=20)
    cc.defines << %w(FMRB_TARGET_BUILD)
    cc.defines << %w(MRB_METHOD_T_STRUCT)

    cc.defines << %w(ESP_PLATFORM)
  end

  conf.cxx do |cxx|
    cxx.include_paths = conf.cc.include_paths.dup

    cxx.flags.collect! { |x| x.gsub('-MP', '') }
    cxx.flags << '-fpermissive'
    #cxx.flags << '-g3 -O0'

    cxx.defines = conf.cc.defines.dup
  end

  conf.bins = []
  conf.build_mrbtest_lib_only
  conf.enable_cxx_exception
  #conf.enable_debug
  
  conf.gem :core => "mruby-compiler"
  conf.gem :core => "mruby-print"
  #conf.gem :core => "mruby-fiber"
  conf.gem :core => "mruby-struct"
  conf.gem :core => "mruby-metaprog"
  conf.gem :core => "mruby-math"
  conf.gem :core => "mruby-random"
  #conf.gem :core => "mruby-kernel-ext"
  conf.gem :core => "mruby-toplevel-ext"

  #conf.gem :github => "mruby-esp32/mruby-esp32-wifi"
  conf.gem :github => "kishima/mruby-esp32-system"
  conf.gem :github => "kishima/mruby-esp32-narya", checksum_hash: 'dc9976352c4adb458a6e374166dd4b723a251f14', branch: 'master'
end
