adc1 = ADC.new(ADC::CHANNEL_0, unit: ADC::UNIT_1)
adc2 = ADC.new(ADC::CHANNEL_3, unit: ADC::UNIT_1)

loop do
  value = adc1.read_raw
  puts "ADC1: #{value}"

  value = adc2.read_raw
  puts "ADC2: #{value}"

  ESP32::System.delay(1000)
end
