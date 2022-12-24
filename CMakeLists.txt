cmake_minimum_required(VERSION 3.14)

project(QRmk LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)


if(MSVC)
    add_compile_options(/W3 /WX /Zc:lambda)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

#add_subdirectory(${CMAKE_SOURCE_DIR}/src)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../qstm/src/cmake/src.cmake")
  include(${CMAKE_CURRENT_SOURCE_DIR}/../qstm/src/cmake/src.cmake)
endif()

include(src/cmake/src.cmake)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    add_library(QRmk STATIC ${QRmk_SRCS})
else()
    add_library(QRmk SHARED ${QRmk_SRCS})
endif()


target_link_libraries(QRmk PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(QRmk PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(QRmk PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_definitions(QRmk PRIVATE QRmk_LIBRARY)

target_link_libraries(
    QRmk # cmake-format: sortable
)

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../qstm/src/src.cmake")
  target_include_directories(QRmk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../qstm/src)
endif()

install(TARGETS QRmk
    DESTINATION ${HOME}/qtreforce.sdk/lib
)
