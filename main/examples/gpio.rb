led = ESP32::GPIO::GPIO_NUM_25
ESP32::GPIO::pinMode(led, ESP32::GPIO::OUTPUT)

loop {
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::HIGH)
  ESP32::System.delay(1000)
  ESP32::GPIO::digitalWrite(led, ESP32::GPIO::LOW)
  ESP32::System.delay(1000)
}
