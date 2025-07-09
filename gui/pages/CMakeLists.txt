# gui/pages/CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(gui_pages)

set(PAGES_SRCS
  StartPage.cpp
  GamePage_ctor.cpp
  GamePage_events.cpp
  GamePage_render.cpp
  PvEConfigPage.cpp
  AIVsAIConfigPage.cpp
  NetworkConnector.cpp
  NetworkConfigPage.cpp
)

add_library(gui_pages ${PAGES_SRCS})

target_include_directories(gui_pages PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gui_pages PUBLIC
  core
  gui_ui
  ai_lib                # jetzt PUBLIC, damit der Executable auch ai_lib sieht
  sfml-network 
)
