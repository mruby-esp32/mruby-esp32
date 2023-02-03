wifi = ESP32::WiFi.new

wifi.on_connected do |ip|
  puts "Connected: #{ip}"

  mqtt = ESP32::MQTT::Client.new('test.mosquitto.org', 1883)
  mqtt.connect
  mqtt.publish("mruby-esp32-mqtt", '{ "hello": "world." }')
  mqtt.disconnect
end

wifi.on_disconnected do
  puts 'Disconnected'
end

puts 'Connecting to wifi'
wifi.connect('SSID', 'password')

#
# Loop forever otherwise the script ends
#
while true do
  mem = ESP32::System.available_memory() / 1000
  puts "Free heap: #{mem}K"
  ESP32::System.delay(10000)
end
