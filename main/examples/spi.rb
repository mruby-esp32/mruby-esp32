# use MAX7219 to drive 8x8 LED matrix

spi = SPI.new(frequency: 10_000_000)
spi.write([0x0c, 0x01])
spi.write([0x09, 0x00])
spi.write([0x0a, 0x0f])
spi.write([0x0b, 0x07])

loop do
  [
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
  ].each_with_index do |out, i|
    spi.write([i + 1, out])
  end
  ESP32::System.delay(1000)

  [
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111,
    0b11111111
  ].each_with_index do |out, i|
    spi.write([i + 1, out])
  end
  ESP32::System.delay(1000)
end
