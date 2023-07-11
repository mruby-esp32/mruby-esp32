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

# PERSISTENCE DEMO
persistence_file = "/storage/test.txt"

# If first boot, write then append to the test file.
if boot_count == 1
  string1 = "testing "
  string2 = "1,2,3..."
  
  puts "Writing to #{persistence_file}: \"#{string1}#{string2}\""
  
  File.open(persistence_file, 'w') { |f| f.write(string1) }
  File.open(persistence_file, 'a') { |f| f.write(string2) }
end

# Read the file back on every boot.
print "Read from #{persistence_file}: "
File.open(persistence_file, 'r') { |f| f.each_line { |l| puts l } }

# OVERWRITE DEMO
overwrite_file = '/storage/overwrite.txt'

File.open(overwrite_file, 'w') { |f| f.puts "12345678" }
File.open(overwrite_file, 'w') { |f| f.puts "1234" }

puts "#{overwrite_file} should contain: 1234"
print "#{overwrite_file} contains: "
File.open(overwrite_file, 'r') { |f| f.each_line { |l| puts l } }
