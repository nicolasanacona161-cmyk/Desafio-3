#include "NivelUnoLunar.h"

#include "Control.h"
#include "GestorNivel.h"

#include <cmath>

namespace {
void ajustarPlataformas(Personaje& personaje, const std::vector<Plataforma*>& plataformas, float sueloY)
{
    constexpr float medioAncho = 66.0f;
    constexpr float margenVertical = 28.0f;
    bool sobrePlataforma = false;
    Vector2D posicion = personaje.posicion();

    for (const Plataforma* plataforma : plataformas) {
        if (!plataforma || !plataforma->activa()) {
            continue;
        }
        const Vector2D plataformaPos = plataforma->obtenerPosicion();
        const float superficieY = plataformaPos.y - 8.0f;
        const bool dentroX = std::abs(posicion.x - plataformaPos.x) <= medioAncho;
        const bool cercaY = posicion.y >= superficieY - margenVertical && posicion.y <= superficieY + margenVertical;

        if (dentroX && cercaY && personaje.velocidadVertical() >= -5.0f) {
            posicion.y = superficieY;
            personaje.setPosicion(posicion);
            personaje.setEnSuelo(true);
            personaje.setVelocidadVertical(0.0f);
            sobrePlataforma = true;
            break;
        }
    }

    if (!sobrePlataforma && personaje.enSuelo() && personaje.posicion().y < sueloY - 1.0f) {
        personaje.setEnSuelo(false);
        personaje.setVelocidadVertical(18.0f);
    }
}
}

int NivelUnoLunar::numero() const
{
    return 1;
}

const char* NivelUnoLunar::nombre() const
{
    return "Nivel 1: Combate lunar con gravedad arcade";
}

void NivelUnoLunar::cargar(GestorNivel& gestor)
{
    gestor.m_nivelActual = 1;
    gestor.m_tiempoNivel = 0.0f;
    gestor.m_temporizadorObjetos = 0.0f;
    gestor.m_sueloY = std::max(500.0f, gestor.m_altoJuego * 0.78f);
    gestor.m_fisica.setGravedad(2.35f);
    gestor.m_fisica.setFactorFlotacion(0.88f);
    gestor.m_personajes.push_back(new Jugador("Iron Man", {145.0f, gestor.m_sueloY}));
    gestor.m_personajes.push_back(new Enemigo("Spider-Man IA", {780.0f, gestor.m_sueloY}));
    gestor.m_plataformas.push_back(new Plataforma({90.0f, 392.0f}, 24.0f, 2.8f));
    gestor.m_plataformas.push_back(new Plataforma({220.0f, 330.0f}, 30.0f, 3.4f));
    gestor.m_plataformas.push_back(new Plataforma({gestor.m_anchoJuego - 220.0f, 330.0f}, 34.0f, 3.7f));
    gestor.m_plataformas.push_back(new Plataforma({gestor.m_anchoJuego - 90.0f, 392.0f}, 26.0f, 3.0f));
    gestor.m_mensajeEstado = nombre();
    gestor.configurarDificultad();
}

void NivelUnoLunar::actualizar(GestorNivel& gestor, float dt, const Control& control)
{
    const float sueloAnterior = gestor.m_sueloY;
    gestor.m_sueloY = std::max(500.0f, gestor.m_altoJuego * 0.78f);
    for (Personaje* personaje : gestor.m_personajes) {
        if (personaje->enSuelo() && std::abs(personaje->posicion().y - sueloAnterior) < 4.0f) {
            Vector2D posicion = personaje->posicion();
            posicion.y = gestor.m_sueloY;
            personaje->setPosicion(posicion);
        }
    }

    gestor.jugador().procesarEntrada(control, false);
    gestor.jugador().mover(control.obtenerDireccion(false), dt);
    gestor.enemigo().tomarDecision(gestor.jugador(), gestor, gestor.m_dificultad);
    gestor.enemigo().ejecutarDecision(dt);

    for (Plataforma* plataforma : gestor.m_plataformas) {
        gestor.m_fisica.actualizarPlataforma(*plataforma, gestor.m_tiempoNivel);
    }

    gestor.m_fisica.aplicarGravedad(gestor.jugador(), dt, gestor.m_sueloY);
    gestor.m_fisica.aplicarGravedad(gestor.enemigo(), dt, gestor.m_sueloY);
    ajustarPlataformas(gestor.jugador(), gestor.m_plataformas, gestor.m_sueloY);
    ajustarPlataformas(gestor.enemigo(), gestor.m_plataformas, gestor.m_sueloY);
    gestor.separarPersonajes();
    gestor.verificarColisiones();

    for (Personaje* personaje : gestor.m_personajes) {
        personaje->actualizar(dt);
        personaje->limitarA(0.0f, gestor.m_anchoJuego, 0.0f, gestor.m_sueloY);
    }
}
