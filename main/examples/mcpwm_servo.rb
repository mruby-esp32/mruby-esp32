WAIT_MS = 1000
mcpwm = MCPWM.new(0, freq: 50)

def pulse_width_us(angle)
  Rational((angle - (-90)) * (2500 - 500), (90 - (-90))).to_i + 500;
end

loop do
  [0, 30, 60, 90, 60, 30, 0, -30, -60, -90, -60, -30].each do |angle|
    puts "Angle=#{angle}"
    mcpwm.pulse_width_us(pulse_width_us(angle))
    ESP32::System.delay(WAIT_MS)
  end
end
