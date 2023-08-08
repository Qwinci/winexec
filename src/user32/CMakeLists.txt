Library(user32)
Links(user32 printlib)

find_package(SDL2 REQUIRED)

target_link_libraries(user32 PRIVATE SDL2::SDL2)
target_link_libraries(user32_w32 PRIVATE SDL2::SDL2)