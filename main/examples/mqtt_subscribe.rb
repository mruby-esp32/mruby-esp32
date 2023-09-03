TOPIC = "mruby-esp32-mqtt"

print 'Connecting to wifi...'
wifi = ESP32::WiFi.new
wifi.connect('SSID', 'password')
puts "Connected"

mqtt = ESP32::MQTT::Client.new('test.mosquitto.org', 1883)
mqtt.connect
mqtt.subscribe(TOPIC) do |message|
  puts "Topic: #{topic}, message: #{message}"
end

# Message receive is event driven, so main loop can do whatever.
loop do
  ESP32::System.delay(1000)
end

# Send a message using another MQTT client, or a site like:
# https://testclient-cloud.mqtt.cool/
