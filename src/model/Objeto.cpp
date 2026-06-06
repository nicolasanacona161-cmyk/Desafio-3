#include "Objeto.h"

#include "Personaje.h"

#include <stdexcept>

Objeto::Objeto(TipoObjeto tipo, Vector2D posicion, float velocidadCaida)
    : m_tipo(tipo),
      m_posicion(posicion),
      m_velocidadCaida(velocidadCaida),
      m_efecto(tipo == TipoObjeto::Tomate
                   ? Efecto("Ralentizacion por tomate", 0.45f, 0.75f, 6.0f)
                   : Efecto("Bebida energetica", 1.55f, 1.45f, 7.0f))
{
    if (velocidadCaida <= 0.0f) {
        throw std::invalid_argument("La velocidad de caida del objeto debe ser positiva.");
    }
}

Objeto::Objeto(TipoObjeto tipo, Vector2D posicion, bool paraJugador, float retardoEfecto)
    : Objeto(tipo, posicion, 1.0f)
{
    if (retardoEfecto < 0.0f) {
        throw std::invalid_argument("El retardo del efecto no puede ser negativo.");
    }
    m_altura = 0.0f;
    m_retardoEfecto = retardoEfecto;
    m_tieneObjetivo = true;
    m_paraJugador = paraJugador;
}

void Objeto::actualizar(float dt)
{
    if (!m_activo) {
        return;
    }
    if (m_retardoEfecto > 0.0f) {
        m_retardoEfecto -= dt;
    }
    if (!m_tieneObjetivo) {
        m_altura -= m_velocidadCaida * dt;
        if (m_altura <= 0.0f) {
            m_altura = 0.0f;
        }
    }
}

void Objeto::aplicarA(Personaje& personaje)
{
    if (!m_activo || !listoParaAplicar()) {
        return;
    }
    personaje.aplicarEfecto(m_efecto);
    m_activo = false;
}

bool Objeto::estaActivo() const { return m_activo; }
bool Objeto::listoParaAplicar() const { return m_retardoEfecto <= 0.0f && m_altura <= 22.0f; }
bool Objeto::tieneObjetivo() const { return m_tieneObjetivo; }
bool Objeto::paraJugador() const { return m_paraJugador; }
TipoObjeto Objeto::tipo() const { return m_tipo; }
Vector2D Objeto::posicion() const { return m_posicion; }
float Objeto::altura() const { return m_altura; }
Efecto Objeto::efecto() const { return m_efecto; }
void Objeto::desactivar() { m_activo = false; }
