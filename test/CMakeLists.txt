
add_executable(gpio_test test.cpp)

#use mock implementation for tests

target_link_libraries(gpio_test PRIVATE mockgpio Catch2::Catch2WithMain)