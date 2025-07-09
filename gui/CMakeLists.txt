# gui/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(guard_and_towers)


find_package(SFML 2.5 COMPONENTS graphics window system audio network REQUIRED)

include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# Kopiere beim Konfigurieren den gesamten assets-Ordner in den Build-Output
file(COPY
  ${CMAKE_CURRENT_SOURCE_DIR}/assets
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
 
# ui und pages bauen
add_subdirectory(ui)
add_subdirectory(pages)

# unser Executable
add_executable(guard_and_towers main.cpp)

target_include_directories(guard_and_towers PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/ui
  ${CMAKE_CURRENT_SOURCE_DIR}/pages
  ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(guard_and_towers PRIVATE
  core
  gui_ui
  gui_pages
  -Wl,--whole-archive ai_lib -Wl,--no-whole-archive
  sfml-graphics
  sfml-window
  sfml-system
  sfml-audio
  sfml-network
  nlohmann_json::nlohmann_json
)
