cmake_minimum_required(VERSION 3.16)

project(MarvelBoxing VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 QUIET COMPONENTS Widgets)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Widgets)
else()
    find_package(Qt5 REQUIRED COMPONENTS Widgets)
    set(QT_LIBS Qt5::Widgets)
endif()

add_executable(MarvelBoxing
    resources.qrc
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/GameWidget.h
    src/GameWidget.cpp
    src/model/AgenteInteligente.h
    src/model/AgenteInteligente.cpp
    src/model/Control.h
    src/model/Control.cpp
    src/model/Efecto.h
    src/model/Efecto.cpp
    src/model/Enums.h
    src/model/Fisica.h
    src/model/Fisica.cpp
    src/model/GestorNivel.h
    src/model/GestorNivel.cpp
    src/model/niveles/NivelBase.h
    src/model/niveles/NivelUnoLunar.h
    src/model/niveles/NivelUnoLunar.cpp
    src/model/niveles/NivelDosCenital.h
    src/model/niveles/NivelDosCenital.cpp
    src/model/Objeto.h
    src/model/Objeto.cpp
    src/model/Personaje.h
    src/model/Personaje.cpp
    src/model/Plataforma.h
    src/model/Plataforma.cpp
    src/model/SistemaCombate.h
    src/model/SistemaCombate.cpp
    src/model/Vector2D.h
)

target_include_directories(MarvelBoxing PRIVATE src src/model src/model/niveles)
target_link_libraries(MarvelBoxing PRIVATE ${QT_LIBS})
