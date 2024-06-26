// SPDX-FileCopyrightText: 2024 Ferenc Nandor Janky <ferenj@effective-range.com>
// SPDX-FileCopyrightText: 2024 Attila Gombos <attila.gombos@effective-range.com>
// SPDX-License-Identifier: MIT

#include <fmt/format.h>
#include <igpio.hpp>
#include <mockgpio.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <signal.h>
#include <stdexcept>

namespace {
volatile sig_atomic_t s_interrupted = 0;

void catch_signals(int sig) {
  s_interrupted = 1;
  signal(sig, catch_signals);
}

} // namespace

auto IGPIO::Create() -> Ptr {
  auto gpio = MockGPIO::Create();
  return gpio;
}

std::shared_ptr<MockGPIO> MockGPIO::Create() {
  return std::make_shared<MockGPIO>();
}

void MockGPIO::set_gpio_mode(port_id_t port, Modes mode) {
  ensure_running();
  if (auto it = m_gpios.find(port); it != m_gpios.end()) {
    it->second.listener->onModeChange(it->second, mode);
    it->second.mode = mode;
  } else {
    m_gpios.emplace(port, GPIOState{port, mode});
  }
}

void MockGPIO::gpio_write(port_id_t gpio, val_t val) {
  ensure_running();
  if (auto it = m_gpios.find(gpio);
      it == m_gpios.end() || it->second.mode != Modes::OUTPUT) {
    throw std::runtime_error("Trying to write GPIO on non-output port");
  } else if (it->second.listener == nullptr) {
    std::cerr << fmt::format(
        "Writing on mocked port {} value {} with no listener\n", gpio, val);
  } else {
    it->second.listener->onWrite(it->second, val);
    it->second.val = val;
  }
}
IGPIO::val_t MockGPIO::gpio_read(port_id_t gpio) {
  ensure_running();
  if (auto it = m_gpios.find(gpio);
      it == m_gpios.end() || it->second.mode != Modes::INPUT) {
    throw std::runtime_error("Trying to read GPIO on non-input port");
  } else if (it->second.listener == nullptr) {
    std::cerr << fmt::format("Reading on mocked port {} with no listener\n",
                             gpio);
    return {};
  } else {
    return it->second.listener->onRead(it->second);
  }
}

void MockGPIO::set_pin_listener(port_id_t p, PinListener *listener) {
  if (auto it = m_gpios.find(p); it == m_gpios.end()) {
    m_gpios.emplace(p, GPIOState{p, Modes::UNDEFINED, std::nullopt, listener});
  } else {
    it->second.listener = listener;
    if (listener) {
      it->second.listener->onModeChange(it->second, it->second.mode);
    }
  }
}

void MockGPIO::delay(std::chrono::microseconds d) {
  ensure_running();
  std::for_each(m_gpios.begin(), m_gpios.end(),
                [d](auto &e) { e.second.listener->onWait(d); });
}

MockGPIO::MockGPIO() : m_handle(GPIOLibHandle::instance()) {}

GPIOLibHandle::GPIOLibHandle() {
  MockGPIO::ensure_running();
  if (fail_to_initialize) {
    throw std::runtime_error("GPIO init failed");
  }
  initialized = true;
  terminated = false;
}
GPIOLibHandle::~GPIOLibHandle() { terminate(); }

GPIOLibHandle::Ref GPIOLibHandle::handle{};

bool GPIOLibHandle::fail_to_initialize{false};
bool GPIOLibHandle::initialized{false};
bool GPIOLibHandle::terminated{false};

void GPIOLibHandle::terminate() {
  if (initialized && !terminated) {
    initialized = false;
    terminated = true;
  }
}

void register_signal_handler(int sig, sighandler_t handler) {
  if (::signal(sig, handler) == SIG_ERR) {
    std::cerr << fmt::format("Warning: can't register signal handler for {}!\n",
                             sig);
  }
}

void GPIOLibHandle::atexit_cleanup() {
  if (auto p = weak_instance().lock(); p) {
    p->terminate();
  }
}
auto GPIOLibHandle::weak_instance() -> Ref { return handle; }

auto GPIOLibHandle::instance() -> Ptr {
  MockGPIO::ensure_running();

  if (auto p = handle.lock(); p) {
    return p;
  }
  auto p = Ptr(new GPIOLibHandle{}, [](GPIOLibHandle *p) { delete p; });
  register_signal_handler(SIGINT, catch_signals);
  register_signal_handler(SIGTERM, catch_signals);
  const auto regexit = std::atexit(atexit_cleanup);
  const auto regqexit = std::at_quick_exit(atexit_cleanup);
  if (regexit || regqexit) {
    std::cerr << "Failed to register atexit cleanup function!\n";
  }
  handle = p;
  return p;
}

void MockGPIO::ensure_running() {
  // Don't throw if there's already an exception in-flight
  if (s_interrupted && std::uncaught_exceptions() == 0) {
    // reset back interrupt flag
    s_interrupted = 0;
    throw Interrupted{};
  }
}

auto MockGPIO::get_state(port_id_t p) -> std::optional<GPIOState> {

  if (auto it = m_gpios.find(p); it != m_gpios.end()) {
    return it->second;
  }
  return {};
}