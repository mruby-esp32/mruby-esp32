group = ESP32::LEDC_LOW_SPEED_MODE
channel = ESP32::LEDC_CHANNEL_0
timer = ESP32::LEDC_TIMER_0
resolution = ESP32::LEDC_TIMER_8_BIT
pin = ESP32::GPIO_NUM_4

# Configure the channel once.
ESP32::LEDC.channel_config(pin, group, timer, channel)

# Note frequencies.
C4 = 262
D4 = 294
E4 = 330

# Melody to play.
notes = [
        [E4, 1], [D4, 1], [C4, 1], [D4, 1], [E4, 1], [E4, 1], [E4, 2],
        [D4, 1], [D4, 1], [D4, 2],          [E4, 1], [E4, 1], [E4, 2],
        [E4, 1], [D4, 1], [C4, 1], [D4, 1], [E4, 1], [E4, 1], [E4, 1], [E4, 1],
        [D4, 1], [D4, 1], [E4, 1], [D4, 1], [C4, 4],
        ]
        
# Calculate length of one beat in milliseconds.
bpm = 180
beat_time = 60000.to_f / bpm

# Play the melody.
notes.each do |note|
  # Set timer frequency to 0th element of the note array.
  ESP32::LEDC.timer_config(group, timer, resolution, note[0])
  # Duty cycle to 50% for square wave.
  ESP32::LEDC.set_duty(group, channel, 128)
  
  # Wait for length of the note, 1st element.
  ESP32::System.delay(note[1] * beat_time)
  
  # Duty cycle to 0 to stop note.
  ESP32::LEDC.set_duty(group, channel, 0)
end
