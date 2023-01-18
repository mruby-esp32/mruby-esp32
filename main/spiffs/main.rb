puts "Getting ready to start mqtt"

ca = IO.read('/spiffs/rootca.pem')
cert = IO.read('/spiffs/certificate.pem.crt')
key = IO.read('/spiffs/private.pem.key')

wifi = ESP32::WiFi.new
wifi.on_connected do |ip|
  puts "Connected: #{ip}"

  mqtt = ESP32::MQTT::Client.new("a3mc7jmuxoiaun-ats.iot.ap-northeast-1.amazonaws.com", 8883)
  mqtt.ssl = true
  mqtt.ca = ca
  mqtt.cert = cert
  mqtt.key = key
  mqtt.connect

  mqtt.subscribe("mruby-esp32-mqtt")
  while true
    topic, message = mqtt.get
    puts topic
    puts message
    break if message == 'exit'
  end
  mqtt.unsubscribe("mruby-esp32-mqtt")

  mqtt.publish("mruby-esp32-mqtt", '{ "hello": "world!" }')

  mqtt.disconnect
rescue Exception => e
  puts e.message
end

wifi.on_disconnected do
  puts "Disconnected"
end

puts "Connecting to wifi"
wifi.connect('oxFC3TVcE8330UOy', 'okazakiyuheii')

#
# Loop forever otherwise the script ends
#
while true do
  mem = ESP32::System.available_memory() / 1000
  puts "Free heap: #{mem}K"
  ESP32::System.delay(10000)
end
