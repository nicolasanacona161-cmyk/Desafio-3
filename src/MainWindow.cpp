#include "MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QEasingCurve>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSizePolicy>
#include <QStackedWidget>
#include <QTimer>
#include <QUrl>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
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
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #050910, stop:0.42 #081827, stop:0.58 #150c12, stop:1 #08070b);"
        "}"
        "QLabel { color: #f4f7fb; }"
        "QLabel#marca { color: #bfe9ff; font-size: 42px; font-weight: 800; letter-spacing: 0px; }"
        "QLabel#submarca { color: #f2f5f9; font-size: 18px; font-weight: 600; }"
        "QLabel#encabezado { color: #f4f7fb; font-size: 22px; font-weight: 800; }"
        "QLabel#nombreRojo { color: #ff655e; font-size: 14px; font-weight: 800; }"
        "QLabel#nombreAzul { color: #5ecbff; font-size: 14px; font-weight: 800; }"
        "QLabel#portraitAzul { border: 2px solid #14bfff; background: rgba(7, 17, 31, 190); border-radius: 4px; }"
        "QLabel#portraitRojo { border: 2px solid #ff3333; background: rgba(31, 8, 12, 190); border-radius: 4px; }"
        "QPushButton { background: rgba(6, 11, 18, 185); color: #f4f7fb; border: 1px solid #52687c; border-radius: 5px; padding: 11px 18px; font-size: 17px; font-weight: 800; }"
        "QPushButton:hover { border-color: #41c9ff; background: rgba(14, 34, 52, 220); }"
        "QPushButton#botonPrimario { border: 2px solid #25c8ff; color: #ffffff; background: rgba(9, 34, 54, 220); }"
        "QPushButton#botonActivoAzul { border: 2px solid #29c4ff; background: rgba(12, 66, 105, 230); }"
        "QPushButton#botonActivoRojo { border: 2px solid #ff4141; background: rgba(99, 18, 23, 230); }"
        "QPushButton#tilePersonaje { min-width: 72px; min-height: 72px; padding: 6px; font-size: 11px; }"
        "QPushButton#tileNivel { min-height: 74px; text-align: left; }"
        "QPushButton#tileDificultad { min-height: 70px; text-align: left; }");
    actualizarMenuVisual();
    return pantalla;
}

QWidget* MainWindow::crearPortadaMenu(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("menuFondo");
    auto* raiz = new QHBoxLayout(pantalla);
    raiz->setContentsMargins(56, 48, 56, 48);
    raiz->setSpacing(34);

    auto crearRetratoGrande = [pantalla](const QString& objectName) {
        auto* label = new QLabel(pantalla);
        label->setObjectName(objectName);
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumSize(230, 330);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        return label;
    };

    m_retratoJugadorMenu = crearRetratoGrande("portraitAzul");
    m_retratoRivalMenu = crearRetratoGrande("portraitRojo");

    auto* centro = new QVBoxLayout;
    centro->setSpacing(14);
    auto* marca = new QLabel("NICMAC", pantalla);
    marca->setObjectName("marca");
    marca->setAlignment(Qt::AlignCenter);
    auto* subtitulo = new QLabel("FIGHT ARENA", pantalla);
    subtitulo->setObjectName("submarca");
    subtitulo->setAlignment(Qt::AlignCenter);

    auto crearBoton = [pantalla](const QString& texto, const QString& objectName = QString()) {
        auto* boton = new QPushButton(texto, pantalla);
        if (!objectName.isEmpty()) {
            boton->setObjectName(objectName);
        }
        boton->setMinimumWidth(340);
        return boton;
    };

    auto* versus = crearBoton("VERSUS", "botonPrimario");
    auto* personajes = crearBoton("ELEGIR PERSONAJE");
    auto* nivel = crearBoton("ELEGIR NIVEL");
    auto* dificultad = crearBoton("DIFICULTAD");
    auto* salir = crearBoton("SALIR JUEGO");

    centro->addStretch();
    centro->addWidget(marca);
    centro->addWidget(subtitulo);
    centro->addSpacing(20);
    centro->addWidget(versus);
    centro->addWidget(personajes);
    centro->addWidget(nivel);
    centro->addWidget(dificultad);
    centro->addWidget(salir);
    centro->addStretch();

    raiz->addWidget(m_retratoJugadorMenu, 1);
    raiz->addLayout(centro, 0);
    raiz->addWidget(m_retratoRivalMenu, 1);

    connect(versus, &QPushButton::clicked, this, &MainWindow::iniciarDesdeMenu);
    connect(personajes, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(1); });
    connect(nivel, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(2); });
    connect(dificultad, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(3); });
    connect(salir, &QPushButton::clicked, qApp, &QApplication::quit);
    return pantalla;
}

QWidget* MainWindow::crearSelectorPersonajes(QWidget* parent)
{
    auto* pantalla = new QWidget(parent);
    pantalla->setObjectName("menuFondo");
    auto* raiz = new QVBoxLayout(pantalla);
    raiz->setContentsMargins(46, 32, 46, 32);
    raiz->setSpacing(14);

    auto* titulo = new QLabel("ELEGIR PERSONAJE", pantalla);
    titulo->setObjectName("encabezado");
    titulo->setAlignment(Qt::AlignCenter);
    raiz->addWidget(titulo);

    auto* versus = new QHBoxLayout;
    versus->setSpacing(26);
    auto crearPanel = [pantalla](const QString& titulo, const QString& objectName, QLabel** retrato, QLabel** nombre) {
        auto* layout = new QVBoxLayout;
        auto* labelTitulo = new QLabel(titulo, pantalla);
        labelTitulo->setObjectName(objectName == QString("portraitAzul") ? "nombreAzul" : "nombreRojo");
        labelTitulo->setAlignment(Qt::AlignCenter);
        *retrato = new QLabel(pantalla);
        (*retrato)->setObjectName(objectName);
        (*retrato)->setAlignment(Qt::AlignCenter);
        (*retrato)->setMinimumSize(220, 170);
        *nombre = new QLabel(pantalla);
        (*nombre)->setAlignment(Qt::AlignCenter);
        (*nombre)->setObjectName(objectName == QString("portraitAzul") ? "nombreAzul" : "nombreRojo");
        layout->addWidget(labelTitulo);
        layout->addWidget(*retrato);
        layout->addWidget(*nombre);
        return layout;
    };

    versus->addLayout(crearPanel("JUGADOR 1", "portraitAzul", &m_retratoJugadorSelector, &m_nombreJugadorSelector), 1);
    auto* vs = new QLabel("VS", pantalla);
    vs->setObjectName("marca");
    vs->setAlignment(Qt::AlignCenter);
    versus->addWidget(vs, 0);
    versus->addLayout(crearPanel("RIVAL", "portraitRojo", &m_retratoRivalSelector, &m_nombreRivalSelector), 1);
    raiz->addLayout(versus);

    auto* filaActivo = new QHBoxLayout;
    m_botonJugadorActivo = new QPushButton("EDITAR JUGADOR 1", pantalla);
    m_botonRivalActivo = new QPushButton("EDITAR RIVAL", pantalla);
    filaActivo->addStretch();
    filaActivo->addWidget(m_botonJugadorActivo);
    filaActivo->addWidget(m_botonRivalActivo);
    filaActivo->addStretch();
    raiz->addLayout(filaActivo);

    auto* grid = new QGridLayout;
    grid->setSpacing(10);
    const QStringList personajes = {"Iron Man", "Spider-Man", "Thor", "Snoopy", "Mau", "Yeng"};
    for (int i = 0; i < personajes.size(); ++i) {
        const QString nombre = personajes.at(i);
        auto* boton = new QPushButton(nombre, pantalla);
        boton->setObjectName("tilePersonaje");
        boton->setIcon(QIcon(retratoPersonaje(nombre, QSize(88, 72))));
        boton->setIconSize(QSize(58, 58));
        m_botonesPersonaje.insert(nombre, boton);
        grid->addWidget(boton, i / 3, i % 3);
        connect(boton, &QPushButton::clicked, this, [this, nombre]() { seleccionarPersonaje(nombre); });
    }
    raiz->addLayout(grid);

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
    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(0); });
    connect(confirmar, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(0); });
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
    auto* jugar = new QPushButton("VERSUS", pantalla);
    jugar->setObjectName("botonPrimario");
    acciones->addWidget(atras);
    acciones->addStretch();
    acciones->addWidget(jugar);
    raiz->addLayout(acciones);

    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(0); });
    connect(jugar, &QPushButton::clicked, this, &MainWindow::iniciarDesdeMenu);
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
    auto* jugar = new QPushButton("VERSUS", pantalla);
    jugar->setObjectName("botonPrimario");
    acciones->addWidget(atras);
    acciones->addStretch();
    acciones->addWidget(jugar);
    raiz->addLayout(acciones);

    connect(atras, &QPushButton::clicked, this, [this]() { m_menuPaginas->setCurrentIndex(0); });
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

void MainWindow::actualizarMenuVisual()
{
    const QPixmap jugadorGrande = retratoPersonaje(m_personajeJugador, QSize(340, 420));
    const QPixmap rivalGrande = retratoPersonaje(m_personajeRival, QSize(340, 420));
    if (m_retratoJugadorMenu) {
        m_retratoJugadorMenu->setPixmap(jugadorGrande);
    }
    if (m_retratoRivalMenu) {
        m_retratoRivalMenu->setPixmap(rivalGrande);
    }
    if (m_retratoJugadorSelector) {
        m_retratoJugadorSelector->setPixmap(retratoPersonaje(m_personajeJugador, QSize(220, 160)));
    }
    if (m_retratoRivalSelector) {
        m_retratoRivalSelector->setPixmap(retratoPersonaje(m_personajeRival, QSize(220, 160)));
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
    m_musicaMenu->setMedia(QUrl("qrc:/sonidos/menu.mp3"));
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
    auto* barra = new QHBoxLayout;
    auto* reiniciar = new QPushButton("Reiniciar partida", pantalla);
    auto* menu = new QPushButton("Volver al menu", pantalla);

    barra->addWidget(reiniciar);
    barra->addWidget(menu);
    barra->addStretch();
    barra->addWidget(m_estado);

    layout->addLayout(barra);
    layout->addWidget(m_game, 1);

    connect(reiniciar, &QPushButton::clicked, this, [this]() {
        m_game->reiniciar();
        m_game->setFocus();
    });
    connect(menu, &QPushButton::clicked, this, [this]() {
        m_paginas->setCurrentIndex(1);
        ajustarVolumenMenu(false);
        if (m_menuPaginas) {
            m_menuPaginas->setCurrentIndex(0);
        }
        m_estado->setText("Escoge personaje y nivel para empezar");
    });

    return pantalla;
}
