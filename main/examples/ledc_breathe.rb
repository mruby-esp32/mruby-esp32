group = ESP32::LEDC_LOW_SPEED_MODE
channel = ESP32::LEDC_CHANNEL_0
timer = ESP32::LEDC_TIMER_0
resolution = ESP32::LEDC_TIMER_8_BIT
frequency = 1000
pin = ESP32::GPIO_NUM_2 # Built in LED on original ESP32 devkit.

ESP32::LEDC.timer_config(group, timer, resolution, frequency)
ESP32::LEDC.channel_config(pin, group, timer, channel)

# Fade the LED up and down.
loop do
  i = 0
  while (i < 256) do
    ESP32::LEDC.set_duty(group, channel, i)
    i += 1
    ESP32::System.delay(10)
  end
  
  i=255
  while (i > -1) do
    ESP32::LEDC.set_duty(group, channel, i)
    i -= 1
    ESP32::System.delay(10)
  end
end
