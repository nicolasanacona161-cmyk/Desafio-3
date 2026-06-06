#include "Fisica.h"

#include "Objeto.h"
#include "Personaje.h"
#include "Plataforma.h"

#include <stdexcept>

Fisica::Fisica(float gravedadActual, float factorFlotacion)
    : m_gravedadActual(gravedadActual),
      m_factorFlotacion(factorFlotacion)
{
    if (gravedadActual <= 0.0f || factorFlotacion <= 0.0f) {
        throw std::invalid_argument("Fisica requiere gravedad y flotacion positivas.");
    }
}

void Fisica::setGravedad(float gravedad)
{
    if (gravedad <= 0.0f) {
        throw std::invalid_argument("La gravedad debe ser positiva.");
    }
    m_gravedadActual = gravedad;
}

void Fisica::setFactorFlotacion(float factor)
{
    if (factor <= 0.0f) {
        throw std::invalid_argument("El factor de flotacion debe ser positivo.");
    }
    m_factorFlotacion = factor;
}

void Fisica::aplicarGravedad(Personaje& personaje, float dt, float sueloY)
{
    if (personaje.enSuelo()) {
        return;
    }
    const float nuevaVelocidad = personaje.velocidadVertical() + m_gravedadActual * 90.0f * dt * m_factorFlotacion;
    Vector2D posicion = personaje.posicion();
    posicion.y += nuevaVelocidad * dt;
    if (posicion.y >= sueloY) {
        posicion.y = sueloY;
        personaje.setEnSuelo(true);
        personaje.setVelocidadVertical(0.0f);
    } else {
        personaje.setVelocidadVertical(nuevaVelocidad);
    }
    personaje.setPosicion(posicion);
}

void Fisica::actualizarCaida(Objeto& objeto, float dt)
{
    objeto.actualizar(dt);
}

void Fisica::actualizarPlataforma(Plataforma& plataforma, float tiempo)
{
    plataforma.actualizar(tiempo);
}

bool Fisica::detectarColision(const Personaje& a, const Personaje& b, float rango) const
{
    return (a.posicion() - b.posicion()).magnitud() <= rango;
}
