#include "AgenteInteligente.h"

#include "GestorNivel.h"
#include "Objeto.h"
#include "Personaje.h"

#include <algorithm>
#include <cmath>

namespace {
struct PerfilIA {
    float agresividad = 1.0f;
    float rangoAtaque = 1.0f;
    float distanciaDefensa = 1.0f;
    float interesObjetos = 1.0f;
    int pausaDecision = 1;
    int frecuenciaGanchoCerca = 7;
    int frecuenciaCombo = 5;
    int retiradaPostAtaque = 10;
    float vidaRetirada = 0.22f;
};

PerfilIA perfilPara(Dificultad dificultad)
{
    if (dificultad == Dificultad::Entrenamiento) {
        return {1.0f, 1.02f, 0.92f, 0.55f, 2, 10, 7, 3, 0.14f};
    }
    if (dificultad == Dificultad::Vibranium) {
        return {1.65f, 1.38f, 1.35f, 1.55f, 1, 4, 2, 5, 0.24f};
    }
    return {1.35f, 1.18f, 1.12f, 1.05f, 1, 5, 3, 4, 0.18f};
}
}

void Percepcion::capturar(const Jugador& jugador, const Enemigo& enemigo, const GestorNivel& nivel)
{
    posicionJugador = jugador.posicion();
    posicionEnemigo = enemigo.posicion();
    distanciaJugador = (posicionJugador - posicionEnemigo).magnitud();
    accionJugador = jugador.ultimaAccion();
    vidaJugador = jugador.vida();
    vidaEnemigo = enemigo.vida();
    vidaMaximaEnemigo = enemigo.vidaMaxima();
    nivelActual = nivel.nivelActual();
    estadoJugador = jugador.estado();
    enemigoEnDanio = enemigo.estado() == EstadoPersonaje::Daniado;
    enemigoEnSuelo = enemigo.enSuelo();
    jugadorEnSuelo = jugador.enSuelo();
    objetosCercanos.clear();
    for (Objeto* objeto : nivel.objetos()) {
        if (objeto && objeto->estaActivo() && (objeto->posicion() - posicionEnemigo).magnitud() < 170.0f) {
            objetosCercanos.push_back(objeto);
        }
    }
}

MemoriaIA::MemoriaIA(int ventanaAprendizaje)
    : m_ventanaAprendizaje(ventanaAprendizaje)
{
}

void MemoriaIA::registrarAccion(TipoAccion accion)
{
    m_historialAcciones.push_back(accion);
    m_frecuenciaAtaques[accion]++;
    if (static_cast<int>(m_historialAcciones.size()) > m_ventanaAprendizaje) {
        const TipoAccion descartada = m_historialAcciones.front();
        m_historialAcciones.pop_front();
        m_frecuenciaAtaques[descartada] = std::max(0, m_frecuenciaAtaques[descartada] - 1);
    }
}

int MemoriaIA::obtenerFrecuencia(TipoAccion accion) const
{
    const auto it = m_frecuenciaAtaques.find(accion);
    return it == m_frecuenciaAtaques.end() ? 0 : it->second;
}

TipoAccion MemoriaIA::detectarPatronRepetido() const
{
    TipoAccion mejor = TipoAccion::Ninguna;
    int mayor = 0;
    for (const auto& [accion, frecuencia] : m_frecuenciaAtaques) {
        if (frecuencia > mayor) {
            mayor = frecuencia;
            mejor = accion;
        }
    }
    return mayor >= 5 ? mejor : TipoAccion::Ninguna;
}

DecisionIA Razonador::evaluar(const Percepcion& percepcion, const MemoriaIA& memoria, Dificultad dificultad) const
{
    DecisionIA decision;
    decision.destino = percepcion.posicionJugador;
    const PerfilIA perfil = perfilPara(dificultad);
    const TipoAccion patron = memoria.detectarPatronRepetido();
    const float vidaRelativa = percepcion.vidaMaximaEnemigo > 0
        ? static_cast<float>(percepcion.vidaEnemigo) / static_cast<float>(percepcion.vidaMaximaEnemigo)
        : 1.0f;
    const bool jugadorAtacando = percepcion.accionJugador == TipoAccion::GolpeFrontal || percepcion.accionJugador == TipoAccion::Gancho;
    const bool jugadorBloqueando = percepcion.estadoJugador == EstadoPersonaje::Bloqueando;
    const bool jugadorVulnerable = percepcion.estadoJugador == EstadoPersonaje::Daniado
        || percepcion.estadoJugador == EstadoPersonaje::Saltando
        || !percepcion.jugadorEnSuelo;

    if (perfil.interesObjetos > 0.5f && (vidaRelativa < 0.75f || dificultad == Dificultad::Vibranium)) {
        if (Objeto* objeto = calcularPrioridadObjeto(percepcion.objetosCercanos)) {
        decision.accion = TipoAccion::RecogerObjeto;
        decision.destino = objeto->posicion();
        decision.prioridad = 0.68f + perfil.interesObjetos * 0.18f;
        return decision;
        }
    }
    if (jugadorBloqueando && percepcion.distanciaJugador <= m_umbralAtaque * perfil.rangoAtaque * 1.18f) {
        decision.accion = TipoAccion::Gancho;
        decision.destino = percepcion.posicionJugador;
        decision.prioridad = 1.0f;
        return decision;
    }
    if (jugadorVulnerable && percepcion.distanciaJugador <= m_umbralAtaque * perfil.rangoAtaque * 1.28f) {
        decision.accion = percepcion.distanciaJugador < m_umbralAtaque * 0.78f ? TipoAccion::Gancho : TipoAccion::GolpeFrontal;
        decision.destino = percepcion.posicionJugador;
        decision.prioridad = 0.98f;
        return decision;
    }
    if (vidaRelativa < perfil.vidaRetirada) {
        const float escapeX = percepcion.posicionEnemigo.x < percepcion.posicionJugador.x ? -170.0f : 170.0f;
        const float escapeY = percepcion.nivelActual == 2
            ? (percepcion.posicionEnemigo.y < percepcion.posicionJugador.y ? -95.0f : 95.0f)
            : 0.0f;
        decision.accion = TipoAccion::Mover;
        decision.destino = {percepcion.posicionEnemigo.x + escapeX, percepcion.posicionEnemigo.y + escapeY};
        decision.prioridad = 1.0f;
        return decision;
    }
    if (jugadorAtacando && percepcion.distanciaJugador < (m_umbralDefensa + 38.0f) * perfil.distanciaDefensa) {
        const bool contraGolpe = dificultad != Dificultad::Entrenamiento && percepcion.distanciaJugador < m_umbralAtaque * 0.82f;
        const bool esquivar = !contraGolpe && (dificultad == Dificultad::Vibranium || memoria.obtenerFrecuencia(TipoAccion::GolpeFrontal) > 3);
        decision.accion = contraGolpe ? TipoAccion::Gancho : (esquivar ? TipoAccion::Esquivar : TipoAccion::Bloquear);
        decision.destino = percepcion.posicionJugador;
        decision.prioridad = 0.76f + perfil.distanciaDefensa * 0.16f;
        return decision;
    }
    if (percepcion.enemigoEnDanio && percepcion.distanciaJugador < m_umbralDefensa + 25.0f) {
        decision.accion = percepcion.nivelActual == 1 && percepcion.enemigoEnSuelo ? TipoAccion::Saltar : TipoAccion::Mover;
        const float escapeX = percepcion.posicionEnemigo.x < percepcion.posicionJugador.x ? -95.0f : 95.0f;
        decision.destino = {percepcion.posicionEnemigo.x + escapeX, percepcion.posicionEnemigo.y};
        decision.prioridad = 0.92f;
        return decision;
    }
    if (percepcion.vidaEnemigo < m_umbralVidaCritica && percepcion.distanciaJugador < m_umbralAtaque) {
        decision.accion = TipoAccion::Gancho;
        decision.prioridad = 0.85f;
        return decision;
    }
    if (patron == TipoAccion::GolpeFrontal || patron == TipoAccion::Gancho || percepcion.accionJugador == TipoAccion::GolpeFrontal) {
        decision.accion = percepcion.distanciaJugador < m_umbralAtaque * 0.9f ? TipoAccion::Gancho : TipoAccion::Mover;
        if (decision.accion == TipoAccion::Mover) {
            decision.destino = percepcion.posicionJugador;
        }
        decision.prioridad = 0.78f;
        return decision;
    }
    if (percepcion.distanciaJugador <= m_umbralAtaque * perfil.rangoAtaque) {
        const bool castigarBloqueo = memoria.obtenerFrecuencia(TipoAccion::Bloquear) > (dificultad == Dificultad::Entrenamiento ? 5 : 2);
        const bool ganchoRitmo = dificultad == Dificultad::Vibranium && memoria.obtenerFrecuencia(TipoAccion::GolpeFrontal) > 1;
        decision.accion = (castigarBloqueo || ganchoRitmo) ? TipoAccion::Gancho : TipoAccion::GolpeFrontal;
        decision.prioridad = 0.7f * perfil.agresividad;
        return decision;
    }
    decision.accion = TipoAccion::Mover;
    decision.prioridad = 0.45f;
    return decision;
}

Objeto* Razonador::calcularPrioridadObjeto(const std::vector<Objeto*>& objetos) const
{
    for (Objeto* objeto : objetos) {
        if (objeto && objeto->tipo() == TipoObjeto::BebidaEnergetica && objeto->altura() <= 45.0f) {
            return objeto;
        }
    }
    return nullptr;
}

DecisionIA AgenteInteligente::actualizar(const Jugador& jugador, Enemigo& enemigo, const GestorNivel& nivel, Dificultad dificultad)
{
    percibir(jugador, enemigo, nivel);
    razonar(dificultad);
    actuar(enemigo);
    aprender(jugador.ultimaAccion(), false);
    return m_accionSeleccionada;
}

void AgenteInteligente::percibir(const Jugador& jugador, const Enemigo& enemigo, const GestorNivel& nivel)
{
    m_percepcion.capturar(jugador, enemigo, nivel);
}

void AgenteInteligente::razonar(Dificultad dificultad)
{
    ++m_cicloDecision;
    const PerfilIA perfil = perfilPara(dificultad);
    m_accionSeleccionada = m_razonador.evaluar(m_percepcion, m_memoria, dificultad);
    const float vidaRelativa = m_percepcion.vidaMaximaEnemigo > 0
        ? static_cast<float>(m_percepcion.vidaEnemigo) / static_cast<float>(m_percepcion.vidaMaximaEnemigo)
        : 1.0f;

    if (perfil.pausaDecision > 1 && m_cicloDecision % perfil.pausaDecision != 0) {
        m_accionSeleccionada.accion = m_percepcion.distanciaJugador < 62.0f ? TipoAccion::GolpeFrontal : TipoAccion::Mover;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad *= 0.78f;
        return;
    }

    if (m_ciclosPostAtaque > 0) {
        --m_ciclosPostAtaque;
        const bool puedePresionar = m_percepcion.distanciaJugador > 76.0f || dificultad == Dificultad::Vibranium;
        const float escapeX = m_percepcion.posicionEnemigo.x < m_percepcion.posicionJugador.x
            ? (puedePresionar ? 70.0f : -95.0f)
            : (puedePresionar ? -70.0f : 95.0f);
        const float escapeY = m_percepcion.nivelActual == 2
            ? (m_percepcion.posicionEnemigo.y < m_percepcion.posicionJugador.y ? 44.0f : -44.0f)
            : 0.0f;
        m_accionSeleccionada.accion = TipoAccion::Mover;
        m_accionSeleccionada.destino = {m_percepcion.posicionEnemigo.x + escapeX, m_percepcion.posicionEnemigo.y + escapeY};
        m_accionSeleccionada.prioridad = 0.88f;
        if (m_percepcion.distanciaJugador < 70.0f * perfil.rangoAtaque && m_cicloDecision % (dificultad == Dificultad::Vibranium ? 3 : 5) == 0) {
            m_accionSeleccionada.accion = dificultad == Dificultad::Entrenamiento ? TipoAccion::GolpeFrontal : TipoAccion::Gancho;
            m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        }
    } else if (vidaRelativa < perfil.vidaRetirada && m_ciclosRetirada < (dificultad == Dificultad::Vibranium ? 44 : 55)) {
        ++m_ciclosRetirada;
        if (m_percepcion.nivelActual == 1 && m_percepcion.enemigoEnSuelo && m_cicloDecision % (dificultad == Dificultad::Vibranium ? 18 : 26) == 0) {
            m_accionSeleccionada.accion = TipoAccion::Saltar;
            m_accionSeleccionada.destino = m_percepcion.posicionJugador;
            m_accionSeleccionada.prioridad = 1.0f;
        }
    } else if (vidaRelativa >= 0.42f) {
        m_ciclosRetirada = 0;
    }

    const bool jugadorBloqueando = m_percepcion.estadoJugador == EstadoPersonaje::Bloqueando;
    const bool jugadorVulnerable = m_percepcion.estadoJugador == EstadoPersonaje::Daniado || m_percepcion.estadoJugador == EstadoPersonaje::Saltando;

    if ((jugadorBloqueando || jugadorVulnerable) && m_percepcion.distanciaJugador < 74.0f * perfil.rangoAtaque) {
        m_accionSeleccionada.accion = jugadorBloqueando ? TipoAccion::Gancho : TipoAccion::GolpeFrontal;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad = 1.0f;
        m_ciclosPostAtaque = std::max(1, perfil.retiradaPostAtaque / 2);
    } else if (m_percepcion.distanciaJugador < 48.0f * perfil.rangoAtaque && m_cicloDecision % perfil.frecuenciaGanchoCerca == 0 && vidaRelativa > 0.30f) {
        m_accionSeleccionada.accion = TipoAccion::Gancho;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad = 0.94f;
        m_ciclosPostAtaque = perfil.retiradaPostAtaque;
    } else if (m_percepcion.distanciaJugador < 68.0f * perfil.rangoAtaque && m_cicloDecision % perfil.frecuenciaCombo == 0 && vidaRelativa > 0.25f) {
        m_accionSeleccionada.accion = (dificultad != Dificultad::Entrenamiento && m_cicloDecision % (perfil.frecuenciaCombo * 2) == 0) ? TipoAccion::Gancho : TipoAccion::GolpeFrontal;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad = 0.91f;
        m_ciclosPostAtaque = perfil.retiradaPostAtaque;
    }

    if (m_accionSeleccionada.accion == TipoAccion::Mover
        && m_percepcion.nivelActual == 1
        && m_percepcion.enemigoEnSuelo
        && m_percepcion.distanciaJugador > 135.0f
        && m_cicloDecision % (dificultad == Dificultad::Vibranium ? 30 : 42) == 0) {
        m_accionSeleccionada.accion = TipoAccion::Saltar;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad = 0.72f;
    }

    if (m_accionSeleccionada.accion == TipoAccion::Bloquear) {
        ++m_ciclosDefensivos;
    } else {
        m_ciclosDefensivos = 0;
    }

    const int maxDefensa = dificultad == Dificultad::Vibranium ? 2 : dificultad == Dificultad::Normal ? 4 : 7;
    if (m_ciclosDefensivos > maxDefensa && m_percepcion.distanciaJugador < 72.0f * perfil.rangoAtaque) {
        m_accionSeleccionada.accion = TipoAccion::Gancho;
        m_accionSeleccionada.destino = m_percepcion.posicionJugador;
        m_accionSeleccionada.prioridad = 0.95f;
        m_ciclosDefensivos = 0;
        m_ciclosPostAtaque = perfil.retiradaPostAtaque;
    }
}

void AgenteInteligente::actuar(Enemigo& enemigo)
{
    enemigo.aplicarDecision(m_accionSeleccionada);
}

void AgenteInteligente::aprender(TipoAccion accionJugador, bool)
{
    m_memoria.registrarAccion(accionJugador);
}

DecisionIA AgenteInteligente::decisionActual() const
{
    return m_accionSeleccionada;
}
