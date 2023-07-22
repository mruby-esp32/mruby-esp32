# Blink built-in LED. Change pin number as needed.
led = ESP32::GPIO_NUM_2
ESP32::GPIO::pinMode(led, ESP32::GPIO::OUTPUT)

loop {
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::HIGH)
  ESP32::System.delay(1000)
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::LOW)
  ESP32::System.delay(1000)
}
