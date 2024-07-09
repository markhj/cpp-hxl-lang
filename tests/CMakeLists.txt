include_directories(../vendor/cpp-bbunit/include)

add_executable(tests tests.cpp)

target_link_libraries(tests PRIVATE cpp_hxl_lang)
