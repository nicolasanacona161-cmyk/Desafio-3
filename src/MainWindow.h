#pragma once

#include "GameWidget.h"

#include <QMainWindow>
#include <QMap>

class QLabel;
class QAudioOutput;
class QComboBox;
class QMediaPlayer;
class QPushButton;
class QStackedWidget;
class QWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QWidget* crearIntro();
    QWidget* crearMenu();
    QWidget* crearPortadaMenu(QWidget* parent);
    QWidget* crearSelectorPersonajes(QWidget* parent);
    QWidget* crearSelectorNivel(QWidget* parent);
    QWidget* crearSelectorDificultad(QWidget* parent);
    QWidget* crearPantallaJuego();
    QPixmap retratoPersonaje(const QString& nombre, const QSize& tamano) const;
    QPixmap imagenEleccionPersonaje(const QString& nombre, const QSize& tamano) const;
    void actualizarMenuVisual();
    void seleccionarPersonaje(const QString& nombre);
    void iniciarDesdeMenu();
    void iniciarMusicaMenu();
    void ajustarVolumenMenu(bool enJuego);

    GameWidget* m_game = nullptr;
    QLabel* m_estado = nullptr;
    QStackedWidget* m_paginas = nullptr;
    QStackedWidget* m_menuPaginas = nullptr;
    QLabel* m_retratoJugadorMenu = nullptr;
    QLabel* m_retratoRivalMenu = nullptr;
    QLabel* m_retratoJugadorSelector = nullptr;
    QLabel* m_retratoRivalSelector = nullptr;
    QLabel* m_nombreJugadorSelector = nullptr;
    QLabel* m_nombreRivalSelector = nullptr;
    QPushButton* m_botonJugadorActivo = nullptr;
    QPushButton* m_botonRivalActivo = nullptr;
    QMap<QString, QPushButton*> m_botonesPersonaje;
    QMap<int, QPushButton*> m_botonesNivel;
    QMap<Dificultad, QPushButton*> m_botonesDificultad;
    QString m_personajeJugador = "Mau";
    QString m_personajeRival = "Yeng";
    bool m_editandoJugador = true;
    int m_nivelSeleccionado = 1;
    Dificultad m_dificultadSeleccionada = Dificultad::Normal;
    QMediaPlayer* m_musicaMenu = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QAudioOutput* m_salidaAudioMenu = nullptr;
#endif
};
