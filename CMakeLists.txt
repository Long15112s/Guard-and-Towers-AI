cmake_minimum_required(VERSION 3.10)
project(GuardAndTowersAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 2.5 COMPONENTS graphics window system network REQUIRED)

# --- core library ------------------------------------------
add_library(core
  src/Board.cpp
  src/MoveGenerator.cpp
  src/FEN.cpp
)
target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- AI-Library (Strategy-Pattern) -----------------------
add_subdirectory(src/ai)   # baut ai_lib

# --- benchmarks & tests & console app ---------------------
add_executable(bench_movegen bench/bench_movegen.cpp)
target_link_libraries(bench_movegen PRIVATE core)
add_custom_target(benchmark DEPENDS bench_movegen COMMAND bench_movegen)



include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)
enable_testing()


add_executable(bench_MMAB bench/bench_MMAB.cpp)
target_link_libraries(bench_MMAB PRIVATE core ai_lib)

add_executable(bench_PVS_TT bench/bench_PVS_TT.cpp)
target_link_libraries(bench_PVS_TT PRIVATE core ai_lib)

add_executable(test_movegen tests/test_board.cpp)
target_link_libraries(test_movegen PRIVATE core gtest_main)
include(GoogleTest)
gtest_discover_tests(test_movegen)


# --- Perft-Utility --------------------------------------
add_executable(perft tests/perft.cpp)
target_link_libraries(perft PRIVATE core)

add_executable(wag src/console_main.cpp)
target_link_libraries(wag PRIVATE core)

# --- Konsolen-App: console_game ---------------------------
add_executable(console_game
  src/console_game.cpp
)

target_include_directories(console_game PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
)

# Linke core und binde ai_lib komplett ein, damit alle REGISTER_AI-Initialisierer gezogen werden
target_link_libraries(console_game
  PRIVATE
    core
    -Wl,--whole-archive
    ai_lib
    -Wl,--no-whole-archive
)

# --- GUI Subdirectory -------------------------------------
add_subdirectory(gui)








# lade nlohmann_json
include(FetchContent)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2        # oder aktueller Release-Tag
)
FetchContent_MakeAvailable(json)

# --- Netzwerk-Client für den Gameserver-Anschluss ------------
add_executable(network_client
  src/NetworkClient.cpp
)
target_link_libraries(network_client
  PRIVATE
    core        # deine Board-/MoveGenerator-Bibliothek
    ai_lib      # deine AI-Bibliothek (MinimaxAI, AlphaBetaAI, …)
    sfml-network
    nlohmann_json::nlohmann_json  # JSON-Parser
)
target_include_directories(network_client PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ai
)
