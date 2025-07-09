# gui/ui/CMakeLists.txt

add_library(gui_ui
  TextField.cpp
  Button.cpp
  ComboBox.cpp
)

target_include_directories(gui_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# SFML‑Components werden transitiv über guard_and_towers gelinkt
