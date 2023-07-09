# Wait a bit for startup to complete.
ESP32::System.delay(500)

string1 = "testing "
string2 = "1,2,3..."

puts "Writing to /storage/test.txt: \"#{string1}#{string2}\""
File.open('/storage/test.txt', 'w') { |f| f.write(string1) }
File.open('/storage/test.txt', 'a') { |f| f.write(string2) }

print "Read from /storage/text.txt: "
File.open('/storage/test.txt', 'r') { |f| f.each_line { |l| puts l } }
