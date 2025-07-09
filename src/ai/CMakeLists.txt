# src/ai/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ai_lib)

file(GLOB AI_SOURCES "*.cpp")

# 1) Objekt-Bibliothek: alle AIs drin, linker droppt nichts
add_library(ai_objs OBJECT ${AI_SOURCES})
target_include_directories(ai_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ai_objs PUBLIC core)

# 2) (Optional) Für Leute, die ai_lib gewohnt sind, eine STATIC
# Bibliothek aus genau diesen Objekten erzeugen
add_library(ai_lib STATIC $<TARGET_OBJECTS:ai_objs>)
target_include_directories(ai_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# core transitiv mitverlinken
target_link_libraries(ai_lib PUBLIC core)
