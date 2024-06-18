// SPDX-FileCopyrightText: 2024 Ferenc Nandor Janky <ferenj@effective-range.com>
// SPDX-FileCopyrightText: 2024 Attila Gombos <attila.gombos@effective-range.com>
// SPDX-License-Identifier: MIT

#include <iostream>

#include <chrono>
#include <thread>

#include <igpio.hpp>

template <typename F> struct [[nodiscard]] Finally {
  F f;
  ~Finally() { f(); }
};

int main() {
  auto gpio = IGPIO::Create();
  gpio->set_gpio_mode(16, IGPIO::Modes::OUTPUT);
  Finally restore_gpio{[&] { gpio->set_gpio_mode(16, IGPIO::Modes::INPUT); }};
  gpio->gpio_write(16, 0);
  gpio->gpio_write(16, 1);
  gpio->delay(std::chrono::seconds{5});
  gpio->gpio_write(16, 0);
}