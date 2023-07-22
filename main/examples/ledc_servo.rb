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

# This is for a 180 degree MG995 motor. Values will differ for other sweep angles,
# or continuous rotation servos / ESCs. Values may vary between individual motors.
#
# Make sure to connect your servo motor to a separate power source!
#
5.times do
  # Send 500us pulses for 2 seconds. Should map to 0 degrees.
  ESP32::LEDC.set_duty(group, channel, microseconds_to_duty(500))
  ESP32::System.delay(2000)
  
  # Send 2500us pulses for 2 seconds. Should map to 180 degrees.
  ESP32::LEDC.set_duty(group, channel, microseconds_to_duty(2500))
  ESP32::System.delay(2000)
end

# Turn it off.
ESP32::LEDC.set_duty(group, channel, 0)
