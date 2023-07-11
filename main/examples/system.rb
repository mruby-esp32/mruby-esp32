puts "RUBY_VERSION: #{RUBY_VERSION}"
puts "RUBY_ENGINE: #{RUBY_ENGINE}"
puts "RUBY_ENGINE_VERSION: #{RUBY_ENGINE_VERSION}"
puts "MRUBY_VERSION: #{MRUBY_VERSION}"
puts "MRUBY_RELEASE_NO: #{MRUBY_RELEASE_NO}"
puts "MRUBY_RELEASE_DATE: #{MRUBY_RELEASE_DATE}"
puts "MRUBY_DESCRIPTION: #{MRUBY_DESCRIPTION}"
puts "MRUBY_COPYRIGHT: #{MRUBY_COPYRIGHT}"
puts
ver = ESP32::System.sdk_version
puts "ESP32 SDK Version: #{ver}"
mem = ESP32::System.available_memory / 1000
puts "Available Memory: #{mem}K"
sec = ESP32::Timer.get_time / 10000000
puts "Uptime: #{sec}s"
puts
puts "Delaying 10 seconds..."
ESP32::System.delay(10 * 1000)
sec = ESP32::Timer.get_time / 10000000
puts "Uptime: #{sec}s"
puts
puts "Deep sleeping for 10 seconds..."
ESP32::System.deep_sleep_for(10 * 1000000)
