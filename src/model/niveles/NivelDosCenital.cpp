#include "NivelDosCenital.h"

#include "AgenteInteligente.h"
#include "Control.h"
#include "GestorNivel.h"
#include "Objeto.h"

#include <algorithm>
#include <unordered_map>

namespace {
struct LimitesRing {
    float izquierda;
    float derecha;
    float arriba;
    float abajo;
};

struct ContactoPared {
    bool izquierda = false;
    bool derecha = false;
    bool arriba = false;
    bool abajo = false;
};

LimitesRing limitesRing(float ancho, float alto)
{
    return {
        ancho * 0.18f,
        ancho * 0.82f,
        std::max(78.0f, alto * 0.17f),
        alto * 0.88f
    };
}

void rebotarEnRing(Personaje& personaje, float ancho, float alto)
{
    const LimitesRing limites = limitesRing(ancho, alto);
    Vector2D posicion = personaje.posicion();
    const Vector2D direccion = personaje.direccion();
    constexpr float rebote = 18.0f;
    static std::unordered_map<const Personaje*, ContactoPared> contactos;
    ContactoPared& contacto = contactos[&personaje];

    if (posicion.x < limites.izquierda) {
        posicion.x = contacto.izquierda ? limites.izquierda : std::min(limites.izquierda + rebote, limites.derecha);
        contacto.izquierda = true;
    } else if (posicion.x > limites.derecha) {
        posicion.x = contacto.derecha ? limites.derecha : std::max(limites.derecha - rebote, limites.izquierda);
        contacto.derecha = true;
    }

    if (posicion.y < limites.arriba) {
        posicion.y = contacto.arriba ? limites.arriba : std::min(limites.arriba + rebote, limites.abajo);
        contacto.arriba = true;
    } else if (posicion.y > limites.abajo) {
        posicion.y = contacto.abajo ? limites.abajo : std::max(limites.abajo - rebote, limites.arriba);
        contacto.abajo = true;
    }

    if (direccion.x > 0.05f) {
        contacto.izquierda = false;
    } else if (direccion.x < -0.05f) {
        contacto.derecha = false;
    }

    if (direccion.y > 0.05f) {
        contacto.arriba = false;
    } else if (direccion.y < -0.05f) {
        contacto.abajo = false;
    }

    personaje.setPosicion(posicion);
}

DecisionIA corregirDestinoIACenital(const DecisionIA& decision, const Enemigo& enemigo, float ancho, float alto)
{
    if (!decision.esValida()) {
        return decision;
    }

    const LimitesRing limites = limitesRing(ancho, alto);
    constexpr float margenInterior = 58.0f;
    const float izquierda = limites.izquierda + margenInterior;
    const float derecha = limites.derecha - margenInterior;
    const float arriba = limites.arriba + margenInterior;
    const float abajo = limites.abajo - margenInterior;

    if (decision.accion != TipoAccion::Mover && decision.accion != TipoAccion::RecogerObjeto) {
        return decision;
    }

    DecisionIA corregida = decision;
    corregida.destino.x = std::clamp(decision.destino.x, izquierda, derecha);
    corregida.destino.y = std::clamp(decision.destino.y, arriba, abajo);

    const bool fueraX = decision.destino.x < izquierda || decision.destino.x > derecha;
    const bool fueraY = decision.destino.y < arriba || decision.destino.y > abajo;
    if (fueraX && fueraY) {
        const Vector2D posicion = enemigo.posicion();
        const Vector2D desplazamiento = decision.destino - posicion;
        if (std::abs(desplazamiento.x) >= std::abs(desplazamiento.y)) {
            corregida.destino.y = std::clamp(posicion.y, arriba, abajo);
        } else {
            corregida.destino.x = std::clamp(posicion.x, izquierda, derecha);
        }
    }
    return corregida;
}
}

int NivelDosCenital::numero() const
{
    return 2;
}

const char* NivelDosCenital::nombre() const
{
    return "Nivel 2: Arena cenital con agente inteligente";
}

void NivelDosCenital::cargar(GestorNivel& gestor)
{
    gestor.m_nivelActual = 2;
    gestor.m_tiempoNivel = 0.0f;
    gestor.m_temporizadorObjetos = 1.5f;
    gestor.m_sueloY = 300.0f;
    gestor.m_fisica.setGravedad(9.81f);
    gestor.m_fisica.setFactorFlotacion(1.0f);
    gestor.m_personajes.push_back(new Jugador("Iron Man", {210.0f, 325.0f}));
    gestor.m_personajes.push_back(new Enemigo("Spider-Man IA", {710.0f, 325.0f}));
    gestor.m_mensajeEstado = nombre();
    gestor.configurarDificultad();
}

void NivelDosCenital::actualizar(GestorNivel& gestor, float dt, const Control& control)
{
    gestor.jugador().procesarEntrada(control, true);
    gestor.jugador().mover(control.obtenerDireccion(true), dt);
    gestor.enemigo().tomarDecision(gestor.jugador(), gestor, gestor.m_dificultad);
    gestor.enemigo().aplicarDecision(corregirDestinoIACenital(gestor.enemigo().decisionActual(),
                                                               gestor.enemigo(),
                                                               gestor.m_anchoJuego,
                                                               gestor.m_altoJuego));
    gestor.enemigo().ejecutarDecision(dt);
    rebotarEnRing(gestor.jugador(), gestor.m_anchoJuego, gestor.m_altoJuego);
    rebotarEnRing(gestor.enemigo(), gestor.m_anchoJuego, gestor.m_altoJuego);
    gestor.jugador().mirarHacia(gestor.enemigo().posicion());
    gestor.enemigo().mirarHacia(gestor.jugador().posicion());

    gestor.m_temporizadorObjetos -= dt;
    if (gestor.m_temporizadorObjetos <= 0.0f) {
        gestor.generarObjetoAleatorio();
        gestor.m_temporizadorObjetos = gestor.intervaloObjetos();
    }

    for (Objeto* objeto : gestor.m_objetos) {
        gestor.m_fisica.actualizarCaida(*objeto, dt);
    }
    gestor.separarPersonajes();
    rebotarEnRing(gestor.jugador(), gestor.m_anchoJuego, gestor.m_altoJuego);
    rebotarEnRing(gestor.enemigo(), gestor.m_anchoJuego, gestor.m_altoJuego);
    gestor.verificarColisiones();
    gestor.eliminarObjetosInactivos();

    for (Personaje* personaje : gestor.m_personajes) {
        personaje->actualizar(dt);
        rebotarEnRing(*personaje, gestor.m_anchoJuego, gestor.m_altoJuego);
    }
}
