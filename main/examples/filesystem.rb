# Wait a bit for startup to complete.
ESP32::System.delay(1000)

string1 = "testing "
string2 = "1,2,3..."

puts "Writing to /spiffs/test.txt: \"#{string1}#{string2}\""
File.open('/spiffs/test.txt', 'w') { |f| f.write(string1) }
File.open('/spiffs/test.txt', 'a') { |f| f.write(string2) }

print "Read from /spiffs/text.txt: "
File.open('/spiffs/test.txt', 'r') { |f| f.each_line { |l| puts l } }
