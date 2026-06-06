#pragma once

#include "Enums.h"
#include "Fisica.h"
#include "Personaje.h"
#include "Plataforma.h"
#include "SistemaCombate.h"

#include <QString>
#include <vector>

class Control;
class NivelBase;
class Objeto;

class GestorNivel {
public:
    GestorNivel();
    ~GestorNivel();

    void cargarNivel(int numero);
    void actualizar(float dt, const Control& control);
    void generarObjetoAleatorio();
    void verificarColisiones();
    void eliminarObjetosInactivos();
    bool finalizarNivel() const;
    bool tiempoAgotado() const;
    void configurarAreaJuego(float ancho, float alto);
    void setDificultad(Dificultad dificultad);
    void reiniciar();
    void cambiarNivel();

    int nivelActual() const;
    float tiempoNivel() const;
    const Jugador& jugador() const;
    const Enemigo& enemigo() const;
    Jugador& jugador();
    Enemigo& enemigo();
    const std::vector<Personaje*>& personajes() const;
    const std::vector<Objeto*>& objetos() const;
    const std::vector<Plataforma*>& plataformas() const;
    Dificultad dificultad() const;
    QString mensajeEstado() const;
    QString resultadoFinal() const;

private:
    friend class NivelUnoLunar;
    friend class NivelDosCenital;
    void limpiar();
    void configurarDificultad();
    float intervaloObjetos() const;
    void separarPersonajes();

    int m_nivelActual = 1;
    float m_tiempoNivel = 0.0f;
    float m_duracionNivel = 90.0f;
    float m_temporizadorObjetos = 0.0f;
    float m_sueloY = 430.0f;
    float m_anchoJuego = 920.0f;
    float m_altoJuego = 600.0f;
    Dificultad m_dificultad = Dificultad::Normal;
    std::vector<Personaje*> m_personajes;
    std::vector<Objeto*> m_objetos;
    std::vector<Plataforma*> m_plataformas;
    Fisica m_fisica;
    SistemaCombate m_sistemaCombate;
    QString m_mensajeEstado = "Listo";
};
