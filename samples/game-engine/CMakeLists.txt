
include_directories(src)

add_executable(game-engine src/main.cpp
        src/engine.cpp
        src/the-game.cpp)

target_link_libraries(game-engine PRIVATE cpp_hxl_lang)

file(COPY data/scene.hxl DESTINATION ${CMAKE_BINARY_DIR}/samples/game-engine)
