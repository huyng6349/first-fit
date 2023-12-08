cmake_minimum_required (VERSION 3.20)

#
# Fetch dependencies
#

include(FetchContent)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
  GIT_TAG bfadcb71658e8622211591d69616eeb9a3bfc90e
)
FetchContent_MakeAvailable(ftxui)

#
# Configure project
#

project(first-fit-demo
  VERSION 1.0.0
  LANGUAGES CXX
)

add_executable(first-fit-demo src/main.cxx)
target_include_directories(first-fit-demo PRIVATE src)

target_compile_features(first-fit-demo
  PRIVATE cxx_std_23
)

if (MSVC)
  target_compile_options(first-fit-demo PRIVATE /W4 /WX)
else()
  target_compile_options(first-fit-demo PRIVATE -Wall -Wextra -Werror)
endif()

target_link_libraries(first-fit-demo
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
