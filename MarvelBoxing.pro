QT += widgets
CONFIG += c++17

TARGET = MarvelBoxing
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/GameWidget.cpp \
    src/model/AgenteInteligente.cpp \
    src/model/Control.cpp \
    src/model/Efecto.cpp \
    src/model/Fisica.cpp \
    src/model/GestorNivel.cpp \
    src/model/niveles/NivelUnoLunar.cpp \
    src/model/niveles/NivelDosCenital.cpp \
    src/model/Objeto.cpp \
    src/model/Personaje.cpp \
    src/model/Plataforma.cpp \
    src/model/SistemaCombate.cpp

HEADERS += \
    src/MainWindow.h \
    src/GameWidget.h \
    src/model/AgenteInteligente.h \
    src/model/Control.h \
    src/model/Efecto.h \
    src/model/Enums.h \
    src/model/Fisica.h \
    src/model/GestorNivel.h \
    src/model/niveles/NivelBase.h \
    src/model/niveles/NivelUnoLunar.h \
    src/model/niveles/NivelDosCenital.h \
    src/model/Objeto.h \
    src/model/Personaje.h \
    src/model/Plataforma.h \
    src/model/SistemaCombate.h \
    src/model/Vector2D.h

RESOURCES += resources.qrc

INCLUDEPATH += src src/model src/model/niveles
