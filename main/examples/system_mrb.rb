ver = ESP32::System.sdk_version
puts "SDK Version: #{ver}"

time = ESP32::System.time
puts "Time: #{time}"

rtc_time = ESP32::System.rtc_time
puts "RTC Time: #{rtc_time}"

mac = ESP32::System.mac
puts "MAC address: #{mac}"

mem = ESP32::System.available_memory / 1000
puts "Memory free: #{mem}K"

puts "Delaying 10 seconds"
ESP32::System.delay(10 * 1000)

#ESP32::System.restart()

puts "Deep sleeping for 10 seconds"
ESP32::System.deep_sleep_for(10 * 1000000)
