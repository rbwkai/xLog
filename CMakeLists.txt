cmake_minimum_required(VERSION 3.24)
project(xlog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(SQLite3 REQUIRED)

add_library(xlog_lib STATIC
    src/engine/xp.cpp
    src/engine/scoring.cpp
    src/engine/calibration.cpp
    src/engine/task_engine.cpp
    src/db/db.cpp
    src/ui/widgets.cpp
    src/ui/screens/prompt_screen.cpp
    src/ui/screens/today_screen.cpp
    src/ui/screens/profile_screen.cpp
    src/ui/screens/bored_screen.cpp
    src/ui/screens/quick_screen.cpp
    src/ui/screens/why_screen.cpp
    src/ui/screens/quote_screen.cpp
    src/cli/fuzzy.cpp
    src/cli/cli.cpp
)

target_include_directories(xlog_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(xlog_lib PUBLIC cxx_std_20)
target_link_libraries(xlog_lib PUBLIC SQLite3::SQLite3)

add_executable(xlog src/main.cpp)
target_link_libraries(xlog PRIVATE xlog_lib)
