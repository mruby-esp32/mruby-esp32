ver = ESP32::System.sdk_version
puts "SDK Version: #{ver}"

mem = ESP32::System.available_memory / 1000
puts "Memory free: #{mem}K"

puts "Delaying 10 seconds"
ESP32::System.delay(10 * 1000)

#ESP32::System.restart()

puts "Deep sleeping for 10 seconds"
ESP32::System.deep_sleep_for(10 * 1000000)
