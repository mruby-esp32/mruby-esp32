# Stack sizes may need to be increased.
# See: https://github.com/mruby-esp32/mruby-socket/blob/0.4/README.md
#
puts "Getting ready to start Wi-Fi"

wifi = ESP32::WiFi.new

wifi.on_connected do |ip|
  puts "Wi-Fi Connected: #{ip} (#{Socket.gethostname})"
  soc = TCPSocket.open("www.kame.net", 80)
  msg = "HEAD / HTTP/1.1\r\nHost: www.kame.net\r\nConnection: close\r\n\r\n"
  msg.split("\r\n").each do |e|
    puts ">>> #{e}"
  end
  soc.send(msg, 0)
  puts "--------------------------------------------------------------------------------"
  loop do
      buf = soc.recv(128, 0)
      break if buf.length == 0
      print buf
  end
  puts ""
  puts "--------------------------------------------------------------------------------"
end

wifi.on_disconnected do
  puts "Wi-Fi Disconnected"
end

puts "Connecting to Wi-Fi"
wifi.connect('SSID', 'PASSWORD')

#
# Loop forever otherwise the script ends
#
while true do
  ESP32::System.delay(1000)
end
