#include "MainWindow.h"

#include <QApplication>
#include <QEasingCurve>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QList>
#include <QMediaPlayer>
#include <QPair>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QUrl>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#else
#include <QMediaContent>
#endif
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_game(new GameWidget(this)),
      m_estado(new QLabel("Escoge personaje y nivel para empezar", this)),
      m_paginas(new QStackedWidget(this))
{
    m_paginas->addWidget(crearIntro());
    m_paginas->addWidget(crearMenu());
    m_paginas->addWidget(crearPantallaJuego());
    setCentralWidget(m_paginas);
    setWindowTitle("Marvel Boxing - Momento III");
    resize(980, 680);
    connect(m_game, &GameWidget::estadoCambiado, m_estado, &QLabel::setText);
    connect(m_game, &GameWidget::volverMenuSolicitado, this, [this]() {
        m_paginas->setCurrentIndex(1);
        ajustarVolumenMenu(false);
        if (m_menuPaginas) {
            m_menuPaginas->setCurrentIndex(0);
        }
        m_estado->setText("Escoge personaje y nivel para empezar");
    });
    iniciarMusicaMenu();
    connect(m_paginas, &QStackedWidget::currentChanged, this, [this](int pagina) {
        ajustarVolumenMenu(pagina == 2);
    });
    QTimer::singleShot(7000, this, [this]() {
        if (m_paginas->currentIndex() == 0) {
            m_paginas->setCurrentIndex(1);
        }
    });
}

QWidget* MainWindow::crearIntro()
{
    auto* pantalla = new QWidget(this);
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(0, 0, 0, 0);

    auto* logo = new QLabel(pantalla);
    logo->setAlignment(Qt::AlignCenter);
    logo->setPixmap(QPixmap(":/branding/logo_nicmac.png"));
    logo->setScaledContents(true);
    logo->setMinimumSize(1, 1);
    logo->setStyleSheet("background: #05080d;");
    raiz->addWidget(logo);

    auto* efecto = new QGraphicsOpacityEffect(logo);
    efecto->setOpacity(0.0);
    logo->setGraphicsEffect(efecto);

    auto* apertura = new QPropertyAnimation(efecto, "opacity", logo);
    apertura->setDuration(2200);
    apertura->setStartValue(0.0);
    apertura->setEndValue(1.0);
    apertura->setEasingCurve(QEasingCurve::OutCubic);
    apertura->start(QAbstractAnimation::DeleteWhenStopped);

    auto* salida = new QPropertyAnimation(efecto, "opacity", logo);
    salida->setDuration(1300);
    salida->setStartValue(1.0);
    salida->setEndValue(0.0);
    salida->setEasingCurve(QEasingCurve::InOutCubic);
    QTimer::singleShot(5600, logo, [salida]() {
        salida->start(QAbstractAnimation::DeleteWhenStopped);
    });

    pantalla->setStyleSheet("background: #05080d;");
    return pantalla;
}

QWidget* MainWindow::crearMenu()
{
    auto* pantalla = new QWidget(this);
    m_menuPaginas = new QStackedWidget(pantalla);

    m_menuPaginas->addWidget(crearPortadaMenu(pantalla));
    m_menuPaginas->addWidget(crearSelectorPersonajes(pantalla));
    m_menuPaginas->addWidget(crearSelectorNivel(pantalla));
    m_menuPaginas->addWidget(crearSelectorDificultad(pantalla));

    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(0, 0, 0, 0);
    raiz->addWidget(m_menuPaginas);

    pantalla->setStyleSheet(
        "QWidget#menuFondo {"
        "  border-image: url(:/fondos/menu.png) 0 0 0 0 stretch stretch;"
        "  background: #050910;"
        "}"
        "QLabel { color: #f4f7fb; }"
        "QLabel#marca { color: #bfe9ff; font-size: 42px; font-weight: 800; letter-spacing: 0px; }"
        "QLabel#marcaMenu { color: #ffffff; font-size: 68px; font-weight: 900; letter-spacing: 0px; }"
        "QLabel#submarca { color: #f2f5f9; font-size: 18px; font-weight: 600; }"
        "QLabel#encabezado { color: #f4f7fb; font-size: 22px; font-weight: 800; }"
        "QPushButton { background: rgba(6, 11, 18, 185); color: #f4f7fb; border: 1px solid #52687c; border-radius: 5px; padding: 11px 18px; font-size: 17px; font-weight: 800; }"
        "QPushButton:hover { border-color: #41c9ff; background: rgba(14, 34, 52, 220); }"
        "QPushButton#botonPrimario { border: 2px solid #25c8ff; color: #ffffff; background: rgba(9, 34, 54, 220); }"
        "QPushButton#botonMenuPrincipal { min-width: 450px; min-height: 76px; font-size: 24px; border: 2px solid #25c8ff; background: rgba(6, 18, 31, 210); }"
        "QPushButton#botonActivoAzul { border: 2px solid #29c4ff; background: rgba(12, 66, 105, 230); }"
        "QPushButton#botonActivoRojo { border: 2px solid #ff4141; background: rgba(99, 18, 23, 230); }"
        "QWidget#selectorFondo {"
        "  border-image: url(:/fondos/menu.png) 0 0 0 0 stretch stretch;"
        "  background: #050509;"
        "}"
        "QFrame#panelJugadorArcade { border: none; background: transparent; }"
        "QFrame#panelRivalArcade { border: none; background: transparent; }"
        "QFrame#globoSelector { border: 2px solid rgba(135, 226, 255, 180); border-radius: 150px; background: qradialgradient(cx:0.5, cy:0.5, radius:0.72, stop:0 rgba(60, 146, 198, 80), stop:0.48 rgba(22, 45, 76, 110), stop:1 rgba(80, 18, 54, 165)); }"
        "QLabel#nombreSelectorGrande { color: #ffffff; font-size: 22px; font-weight: 900; }"
        "QLabel#ladoSelectorAzul { color: #7ae7ff; font-size: 18px; font-weight: 900; }"
        "QLabel#ladoSelectorRojo { color: #ffd166; font-size: 18px; font-weight: 900; }"
        "QLabel#personajeGrandeAzul { border: none; background: transparent; }"
        "QLabel#personajeGrandeRojo { border: none; background: transparent; }"
        "QPushButton#tilePersonajeArcade { min-width: 82px; min-height: 82px; max-width: 82px; max-height: 82px; border: none; padding: 0px; background: transparent; }"
        "QPushButton#tilePersonajeArcade:hover { border: none; background: transparent; }"
        "QPushButton#tilePersonajeAzul { min-width: 82px; min-height: 82px; max-width: 82px; max-height: 82px; border: none; padding: 0px; background: transparent; }"
        "QPushButton#tilePersonajeRojo { min-width: 82px; min-height: 82px; max-width: 82px; max-height: 82px; border: none; padding: 0px; background: transparent; }"
        "QPushButton#tileNivel { min-height: 74px; text-align: left; }"
        "QPushButton#tileDificultad { min-height: 70px; text-align: left; }");
    actualizarMenuVisual();
    return pantalla;
}

QWidget* MainWindow::crearPortadaMenu(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("menuFondo");
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(80, 60, 80, 60);
    raiz->setSpacing(18);

    auto* marca = new QLabel("NICMAC", pantalla);
    marca->setObjectName("marcaMenu");
    marca->setAlignment(Qt::AlignCenter);
    auto* subtitulo = new QLabel("FIGHT ARENA", pantalla);
    subtitulo->setObjectName("submarca");
    subtitulo->setAlignment(Qt::AlignCenter);

    auto crearBoton = [pantalla](const QString& texto) {
        auto* boton = new QPushButton(texto, pantalla);
        boton->setObjectName("botonMenuPrincipal");
        return boton;
    };

    auto* jugar = crearBoton("JUGAR");
    auto* salir = crearBoton("SALIR JUEGO");

    raiz->addStretch(2);
    raiz->addWidget(marca);
    raiz->addWidget(subtitulo);
    raiz->addSpacing(36);
    raiz->addWidget(jugar, 0, Qt::AlignCenter);
    raiz->addSpacing(10);
    raiz->addWidget(salir, 0, Qt::AlignCenter);
    raiz->addStretch(3);

    connect(jugar, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(2); });
    connect(salir, &QPushButton::clicked, qApp, &QApplication::quit);
    return pantalla;
}

QWidget* MainWindow::crearSelectorPersonajes(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("selectorFondo");
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(28, 18, 28, 22);
    raiz->setSpacing(8);

    auto* titulo = new QLabel("CHARACTER SELECT", pantalla);
    titulo->setObjectName("marca");
    titulo->setAlignment(Qt::AlignCenter);
    raiz->addWidget(titulo);

    auto* cuerpo = new QHBoxLayout;
    cuerpo->setSpacing(12);

    auto crearPanel = [pantalla](const QString& titulo, const QString& frameName, const QString& tituloName, QLabel** retrato, QLabel** nombre) {
        auto* frame = new QFrame(pantalla);
        frame->setObjectName(frameName);
        frame->setMinimumWidth(250);
        frame->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        auto* layout = new QVBoxLayout(frame);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(8);
        auto* labelTitulo = new QLabel(titulo, frame);
        labelTitulo->setObjectName(tituloName);
        labelTitulo->setAlignment(Qt::AlignCenter);
        *retrato = new QLabel(frame);
        (*retrato)->setObjectName(frameName == QString("panelJugadorArcade") ? "personajeGrandeAzul" : "personajeGrandeRojo");
        (*retrato)->setAlignment(Qt::AlignCenter);
        (*retrato)->setMinimumSize(260, 390);
        (*retrato)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        *nombre = new QLabel(frame);
        (*nombre)->setAlignment(Qt::AlignCenter);
        (*nombre)->setObjectName("nombreSelectorGrande");
        layout->addWidget(labelTitulo);
        layout->addWidget(*retrato, 1);
        layout->addWidget(*nombre);
        return frame;
    };

    cuerpo->addWidget(crearPanel("PLAYER 1", "panelJugadorArcade", "ladoSelectorAzul", &m_retratoJugadorSelector, &m_nombreJugadorSelector), 1);

    auto* centro = new QVBoxLayout;
    centro->setSpacing(8);
    auto* vs = new QLabel("VERSUS", pantalla);
    vs->setObjectName("submarca");
    vs->setAlignment(Qt::AlignCenter);
    centro->addWidget(vs);

    auto* globo = new QFrame(pantalla);
    globo->setObjectName("globoSelector");
    globo->setMinimumSize(300, 300);
    globo->setMaximumSize(320, 320);
    auto* grid = new QGridLayout(globo);
    grid->setContentsMargins(34, 36, 34, 34);
    grid->setHorizontalSpacing(6);
    grid->setVerticalSpacing(6);
    const QStringList personajes = {"Iron Man", "Spider-Man", "Thor", "Snoopy", "Mau", "Yeng"};
    const QList<QPair<int, int>> posiciones = {
        {0, 0}, {0, 1}, {0, 2},
        {1, 0}, {1, 1}, {1, 2}
    };
    for (int i = 0; i < personajes.size(); ++i) {
        const QString nombre = personajes.at(i);
        auto* boton = new QPushButton(globo);
        boton->setObjectName("tilePersonajeArcade");
        boton->setToolTip(nombre.toUpper());
        boton->setIcon(QIcon(imagenEleccionPersonaje(nombre, QSize(110, 110))));
        boton->setIconSize(QSize(82, 82));
        m_botonesPersonaje.insert(nombre, boton);
        grid->addWidget(boton, posiciones.at(i).first, posiciones.at(i).second);
        connect(boton, &QPushButton::clicked, this, [this, nombre]() { seleccionarPersonaje(nombre); });
    }
    centro->addWidget(globo, 1, Qt::AlignCenter);

    auto* filaActivo = new QHBoxLayout;
    m_botonJugadorActivo = new QPushButton("PLAYER 1", pantalla);
    m_botonRivalActivo = new QPushButton("RIVAL", pantalla);
    filaActivo->addStretch();
    filaActivo->addWidget(m_botonJugadorActivo);
    filaActivo->addWidget(m_botonRivalActivo);
    filaActivo->addStretch();
    centro->addLayout(filaActivo);

    cuerpo->addLayout(centro, 2);
    cuerpo->addWidget(crearPanel("RIVAL", "panelRivalArcade", "ladoSelectorRojo", &m_retratoRivalSelector, &m_nombreRivalSelector), 1);
    raiz->addLayout(cuerpo, 1);

    auto* acciones = new QHBoxLayout;
    auto* atras = new QPushButton("ATRAS", pantalla);
    auto* confirmar = new QPushButton("CONFIRMAR", pantalla);
    confirmar->setObjectName("botonPrimario");
    acciones->addWidget(atras);
    acciones->addStretch();
    acciones->addWidget(confirmar);
    raiz->addLayout(acciones);

    connect(m_botonJugadorActivo, &QPushButton::clicked, this, [this]() {
        m_editandoJugador = true;
        actualizarMenuVisual();
    });
    connect(m_botonRivalActivo, &QPushButton::clicked, this, [this]() {
        m_editandoJugador = false;
        actualizarMenuVisual();
    });
    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(2); });
    connect(confirmar, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(3); });
    return pantalla;
}

QWidget* MainWindow::crearSelectorNivel(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("menuFondo");
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(70, 70, 70, 70);
    raiz->setSpacing(18);

    auto* titulo = new QLabel("ELEGIR NIVEL", pantalla);
    titulo->setObjectName("encabezado");
    titulo->setAlignment(Qt::AlignCenter);
    raiz->addWidget(titulo);

    auto agregarNivel = [this, pantalla, raiz](int nivel, const QString& texto) {
        auto* boton = new QPushButton(texto, pantalla);
        boton->setObjectName("tileNivel");
        m_botonesNivel.insert(nivel, boton);
        raiz->addWidget(boton);
        connect(boton, &QPushButton::clicked, this, [this, nivel]() {
            m_nivelSeleccionado = nivel;
            actualizarMenuVisual();
        });
    };
    agregarNivel(1, "NIVEL 1  |  LUNA Y PLATAFORMAS");
    agregarNivel(2, "NIVEL 2  |  ARENA CENITAL");
    raiz->addStretch();

    auto* acciones = new QHBoxLayout;
    auto* atras = new QPushButton("ATRAS", pantalla);
    auto* continuar = new QPushButton("CONTINUAR", pantalla);
    continuar->setObjectName("botonPrimario");
    acciones->addWidget(atras);
    acciones->addStretch();
    acciones->addWidget(continuar);
    raiz->addLayout(acciones);

    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(0); });
    connect(continuar, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(1); });
    return pantalla;
}

QWidget* MainWindow::crearSelectorDificultad(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("menuFondo");
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(70, 70, 70, 70);
    raiz->setSpacing(18);

    auto* titulo = new QLabel("DIFICULTAD", pantalla);
    titulo->setObjectName("encabezado");
    titulo->setAlignment(Qt::AlignCenter);
    raiz->addWidget(titulo);

    auto agregarDificultad = [this, pantalla, raiz](Dificultad dificultad, const QString& texto) {
        auto* boton = new QPushButton(texto, pantalla);
        boton->setObjectName("tileDificultad");
        m_botonesDificultad.insert(dificultad, boton);
        raiz->addWidget(boton);
        connect(boton, &QPushButton::clicked, this, [this, dificultad]() {
            m_dificultadSeleccionada = dificultad;
            actualizarMenuVisual();
        });
    };
    agregarDificultad(Dificultad::Entrenamiento, "FACIL  |  IA mas lenta, menos combos y menos castigo");
    agregarDificultad(Dificultad::Normal, "DIFICIL  |  IA agresiva, bloquea y contraataca");
    agregarDificultad(Dificultad::Vibranium, "MUY DIFICIL  |  IA tactica, usa objetos, presiona y se retira");
    raiz->addStretch();

    auto* acciones = new QHBoxLayout;
    auto* atras = new QPushButton("ATRAS", pantalla);
    auto* jugar = new QPushButton("A JUGAR", pantalla);
    jugar->setObjectName("botonPrimario");
    acciones->addWidget(atras);
    acciones->addStretch();
    acciones->addWidget(jugar);
    raiz->addLayout(acciones);

    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(1); });
    connect(jugar, &QPushButton::clicked, this, &MainWindow::iniciarDesdeMenu);
    return pantalla;
}

QPixmap MainWindow::retratoPersonaje(const QString& nombre, const QSize& tamano) const
{
    QString ruta = ":/menu_portraits/snoopy.png";
    if (nombre.contains("Iron", Qt::CaseInsensitive)) {
        ruta = ":/menu_portraits/iron.png";
    } else if (nombre.contains("Spider", Qt::CaseInsensitive)) {
        ruta = ":/menu_portraits/spiderman.png";
    } else if (nombre.contains("Thor", Qt::CaseInsensitive)) {
        ruta = ":/menu_portraits/thor.png";
    } else if (nombre.contains("Mau", Qt::CaseInsensitive)) {
        ruta = ":/menu_portraits/mau.png";
    } else if (nombre.contains("Yeng", Qt::CaseInsensitive)) {
        ruta = ":/menu_portraits/yeng.png";
    }

    const QPixmap retrato(ruta);
    if (retrato.isNull()) {
        return {};
    }
    return retrato.scaled(tamano, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap MainWindow::imagenEleccionPersonaje(const QString& nombre, const QSize& tamano) const
{
    QString ruta = ":/eleccion/snoopy.png";
    if (nombre.contains("Iron", Qt::CaseInsensitive)) {
        ruta = ":/eleccion/iron.png";
    } else if (nombre.contains("Spider", Qt::CaseInsensitive)) {
        ruta = ":/eleccion/spiderman.png";
    } else if (nombre.contains("Thor", Qt::CaseInsensitive)) {
        ruta = ":/eleccion/thor.png";
    } else if (nombre.contains("Mau", Qt::CaseInsensitive)) {
        ruta = ":/eleccion/mau.png";
    } else if (nombre.contains("Yeng", Qt::CaseInsensitive)) {
        ruta = ":/eleccion/yeng.png";
    }

    const QPixmap imagen(ruta);
    if (imagen.isNull()) {
        return retratoPersonaje(nombre, tamano);
    }
    return imagen.scaled(tamano, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void MainWindow::actualizarMenuVisual()
{
    const bool spidermanDisponible = m_nivelSeleccionado == 2;
    if (!spidermanDisponible && m_personajeJugador.contains("Spider", Qt::CaseInsensitive)) {
        m_personajeJugador = "Mau";
    }
    if (!spidermanDisponible && m_personajeRival.contains("Spider", Qt::CaseInsensitive)) {
        m_personajeRival = "Yeng";
    }

    if (m_retratoJugadorSelector) {
        m_retratoJugadorSelector->setPixmap(imagenEleccionPersonaje(m_personajeJugador, QSize(330, 430)));
    }
    if (m_retratoRivalSelector) {
        m_retratoRivalSelector->setPixmap(imagenEleccionPersonaje(m_personajeRival, QSize(330, 430)));
    }
    if (m_nombreJugadorSelector) {
        m_nombreJugadorSelector->setText(m_personajeJugador.toUpper());
    }
    if (m_nombreRivalSelector) {
        m_nombreRivalSelector->setText(m_personajeRival.toUpper());
    }
    if (m_botonJugadorActivo) {
        m_botonJugadorActivo->setObjectName(m_editandoJugador ? "botonActivoAzul" : "");
        m_botonJugadorActivo->style()->unpolish(m_botonJugadorActivo);
        m_botonJugadorActivo->style()->polish(m_botonJugadorActivo);
    }
    if (m_botonRivalActivo) {
        m_botonRivalActivo->setObjectName(!m_editandoJugador ? "botonActivoRojo" : "");
        m_botonRivalActivo->style()->unpolish(m_botonRivalActivo);
        m_botonRivalActivo->style()->polish(m_botonRivalActivo);
    }
    for (auto it = m_botonesPersonaje.begin(); it != m_botonesPersonaje.end(); ++it) {
        const bool esSpiderman = it.key().contains("Spider", Qt::CaseInsensitive);
        it.value()->setVisible(!esSpiderman || spidermanDisponible);
        it.value()->setEnabled(!esSpiderman || spidermanDisponible);

        QString objectName = "tilePersonajeArcade";
        if (it.key() == m_personajeJugador) {
            objectName = "tilePersonajeAzul";
        } else if (it.key() == m_personajeRival) {
            objectName = "tilePersonajeRojo";
        }
        it.value()->setObjectName(objectName);
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
    }
    for (auto it = m_botonesNivel.begin(); it != m_botonesNivel.end(); ++it) {
        it.value()->setObjectName(it.key() == m_nivelSeleccionado ? "botonPrimario" : "tileNivel");
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
    }
    for (auto it = m_botonesDificultad.begin(); it != m_botonesDificultad.end(); ++it) {
        it.value()->setObjectName(it.key() == m_dificultadSeleccionada ? "botonPrimario" : "tileDificultad");
        it.value()->style()->unpolish(it.value());
        it.value()->style()->polish(it.value());
    }
}

void MainWindow::seleccionarPersonaje(const QString& nombre)
{
    if (m_editandoJugador) {
        m_personajeJugador = nombre;
    } else {
        m_personajeRival = nombre;
    }
    actualizarMenuVisual();
}

void MainWindow::iniciarDesdeMenu()
{
    m_game->setDificultad(m_dificultadSeleccionada);
    m_game->iniciarPartida(m_nivelSeleccionado, m_personajeJugador, m_personajeRival);
    m_paginas->setCurrentIndex(2);
    ajustarVolumenMenu(true);
    m_game->setFocus();
}

void MainWindow::iniciarMusicaMenu()
{
    m_musicaMenu = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_salidaAudioMenu = new QAudioOutput(this);
    m_salidaAudioMenu->setVolume(0.65);
    m_musicaMenu->setAudioOutput(m_salidaAudioMenu);
    m_musicaMenu->setSource(QUrl("qrc:/sonidos/menu.mp3"));
    m_musicaMenu->setLoops(QMediaPlayer::Infinite);
#else
    m_musicaMenu->setMedia(QMediaContent(QUrl("qrc:/sonidos/menu.mp3")));
    m_musicaMenu->setVolume(65);
    connect(m_musicaMenu, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            m_musicaMenu->setPosition(0);
            m_musicaMenu->play();
        }
    });
#endif
    m_musicaMenu->play();
}

void MainWindow::ajustarVolumenMenu(bool enJuego)
{
    if (!m_musicaMenu) {
        return;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (m_salidaAudioMenu) {
        m_salidaAudioMenu->setVolume(enJuego ? 0.22 : 0.65);
    }
#else
    m_musicaMenu->setVolume(enJuego ? 22 : 65);
#endif
}

QWidget* MainWindow::crearPantallaJuego()
{
    auto* pantalla = new QWidget(this);
    auto* layout = new QVBoxLayout(pantalla);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_game, 1);

    return pantalla;
}
