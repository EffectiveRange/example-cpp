

# define a static library for mocked implementation
# for testing and for x86_64 build
add_library(mockgpio STATIC src/mockgpio.cpp)

target_include_directories(mockgpio PUBLIC include)
target_compile_definitions(mockgpio PUBLIC FMT_HEADER_ONLY)
target_link_libraries(mockgpio PUBLIC  fmt::fmt)


# Try to find pigpio library
find_library(PIGPIO_LIBRARY NAMES pigpio)
if(PIGPIO_LIBRARY)
    message(STATUS "Found pigpio library: ${PIGPIO_LIBRARY}")
    add_library(pigpioimpl STATIC src/pigpio.cpp)
    
    find_path(PIGIO_INCLUDE_DIR pigpio.h REQUIRED)
    # Add pigpio as public library dependency to the gpio implementation library
    # the header path should not be visible to the clients
    target_link_libraries(pigpioimpl PUBLIC ${PIGPIO_LIBRARY}  fmt::fmt PRIVATE ${PIGIO_INCLUDE_DIR})
    
    # setup fmt lib for header only usage
    target_compile_definitions(pigpioimpl PUBLIC FMT_HEADER_ONLY)

    target_include_directories(pigpioimpl PUBLIC  include PRIVATE include_priv)

    # link our target binary with our pigpio impl library 
    target_link_libraries(er_example_cpp  rt pigpioimpl)
else()
    message(STATUS "pigpio library not found, using Mock implementation")
    target_link_libraries(er_example_cpp  mockgpio)

endif()