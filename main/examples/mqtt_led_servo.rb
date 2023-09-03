#
# Servo Setup
#
group = ESP32::LEDC_LOW_SPEED_MODE
channel = ESP32::LEDC_CHANNEL_0
timer = ESP32::LEDC_TIMER_0
resolution = ESP32::LEDC_TIMER_14_BIT
pin = ESP32::GPIO_NUM_4
frequency = 50

# Configure the channel and timer.
ESP32::LEDC.channel_config(pin, group, timer, channel)
ESP32::LEDC.timer_config(group, timer, resolution, frequency)

# Using 14-bit PWM @ 50Hz, 0-16383 maps from 0 to 20 milliseconds.
# Calculate how many microseconds each LSB of duty cycle represents.
US_PER_BIT = (1000000.0 / frequency) / (2 ** resolution)

# Convert from microseconds to duty cycle.
def microseconds_to_duty(us)
  (us / US_PER_BIT).round
end

#
# WiFi Setup
#
print 'Connecting to wifi...'
wifi = ESP32::WiFi.new
wifi.connect('SSID', 'password')
puts "Connected"

#
# MQTT Setup
#
mqtt = ESP32::MQTT::Client.new('test.mosquitto.org', 1883)
mqtt.connect
mqtt.subscribe("mruby-esp32-mqtt") do |message|
  puts "Servo mircoseconds: #{message}"
  # Send microsecond values to move the servo.
  ESP32::LEDC.set_duty(group, channel, microseconds_to_duty(message.to_i))
end

#
# Blink LED in main loop.
#
led = ESP32::GPIO_NUM_2
ESP32::GPIO::pinMode(led, ESP32::GPIO::OUTPUT)
loop do
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::HIGH)
  ESP32::System.delay(1000)
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::LOW)
  ESP32::System.delay(1000)
end

# Send a message using another MQTT client, or a site like:
# https://testclient-cloud.mqtt.cool/
