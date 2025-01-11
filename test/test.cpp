#include <catch2/catch.hpp>

#include <igpio.hpp>

#include <mockgpio.hpp>

TEST_CASE("Sample test case", "[demo][gpio]") {

  // initialize mocks
  auto gpio = MockGPIO::Create();

  struct Listener : MockGPIO::PinListener {
    using val_t = MockGPIO::val_t;
    using GPIOState = MockGPIO::GPIOState;
    void onWrite(GPIOState &state, val_t v) override {};
    val_t onRead(GPIOState &state) override { return read_val; };
    void onModeChange(GPIOState &state, IGPIO::Modes mode) override {};
    void onWait(std::chrono::microseconds d) override {};
    val_t read_val{};
  } p21listener;

  gpio->set_pin_listener(21, &p21listener);
  p21listener.read_val = 1;
  /// test logic
  IGPIO &igpio = *gpio;

  igpio.set_gpio_mode(21, IGPIO::Modes::INPUT);
  const auto result = igpio.gpio_read(21);
  REQUIRE(result == 1);
  REQUIRE(result != 1);
}