# Get the number of previous boots from a file.
boot_count = nil
File.open('/storage/boot_count.txt', 'a+') { |f| boot_count = f.gets }

# Increment it.
boot_count = 0 unless boot_count
boot_count = boot_count.to_i
boot_count += 1

# Write new count back to file.
File.open('/storage/boot_count.txt', 'w') { |f| f.puts(boot_count) }

# Display it.
puts "Boot count: #{boot_count}"

# If first boot, write then append to /storage/test.txt.
if boot_count == 1
  string1 = "testing "
  string2 = "1,2,3..."
  puts "Writing to /storage/test.txt: \"#{string1}#{string2}\""
  
  File.open('/storage/test.txt', 'w') { |f| f.write(string1) }
  
  # WARNING: littlefs append mode starts with the file pointer at 0 (start of file).
  File.open('/storage/test.txt', 'a') do |f|
    # Move file pointer to the end with #seek.
    f.seek(0, File::SEEK_END)
    f.write(string2)
  end
end

# Read from /storage/test.txt on every boot.
print "Read from /storage/test.txt: "
File.open('/storage/test.txt', 'r') { |f| f.each_line { |l| puts l } }
