#pragma once

#include "Enums.h"
#include "Vector2D.h"

#include <deque>
#include <map>
#include <vector>

class Enemigo;
class GestorNivel;
class Jugador;
class Objeto;
class Personaje;

struct Percepcion {
    Vector2D posicionJugador;
    Vector2D posicionEnemigo;
    float distanciaJugador = 0.0f;
    TipoAccion accionJugador = TipoAccion::Ninguna;
    int vidaJugador = 100;
    int vidaEnemigo = 100;
    int vidaMaximaEnemigo = 100;
    int nivelActual = 1;
    EstadoPersonaje estadoJugador = EstadoPersonaje::Quieto;
    bool enemigoEnDanio = false;
    bool enemigoEnSuelo = true;
    bool jugadorEnSuelo = true;
    std::vector<Objeto*> objetosCercanos;

    void capturar(const Jugador& jugador, const Enemigo& enemigo, const GestorNivel& nivel);
};

struct DecisionIA {
    TipoAccion accion = TipoAccion::Ninguna;
    Vector2D destino;
    Personaje* objetivo = nullptr;
    float prioridad = 0.0f;

    bool esValida() const { return accion != TipoAccion::Ninguna; }
};

class MemoriaIA {
public:
    explicit MemoriaIA(int ventanaAprendizaje = 16);
    void registrarAccion(TipoAccion accion);
    int obtenerFrecuencia(TipoAccion accion) const;
    TipoAccion detectarPatronRepetido() const;

private:
    std::deque<TipoAccion> m_historialAcciones;
    std::map<TipoAccion, int> m_frecuenciaAtaques;
    int m_ventanaAprendizaje;
};

class Razonador {
public:
    DecisionIA evaluar(const Percepcion& percepcion, const MemoriaIA& memoria, Dificultad dificultad) const;
    Objeto* calcularPrioridadObjeto(const std::vector<Objeto*>& objetos) const;

private:
    float m_umbralAtaque = 78.0f;
    float m_umbralDefensa = 70.0f;
    int m_umbralVidaCritica = 25;
};

class AgenteInteligente {
public:
    DecisionIA actualizar(const Jugador& jugador, Enemigo& enemigo, const GestorNivel& nivel, Dificultad dificultad);
    void percibir(const Jugador& jugador, const Enemigo& enemigo, const GestorNivel& nivel);
    void razonar(Dificultad dificultad);
    void actuar(Enemigo& enemigo);
    void aprender(TipoAccion accionJugador, bool exitoJugador);
    DecisionIA decisionActual() const;

private:
    Percepcion m_percepcion;
    Razonador m_razonador;
    MemoriaIA m_memoria;
    DecisionIA m_accionSeleccionada;
    int m_ciclosDefensivos = 0;
    int m_cicloDecision = 0;
    int m_ciclosRetirada = 0;
    int m_ciclosPostAtaque = 0;
};
