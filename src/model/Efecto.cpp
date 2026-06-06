#include "Efecto.h"

#include "Personaje.h"

#include <utility>

Efecto::Efecto(QString nombre, float factorVelocidad, float factorAtaque, float duracion)
    : m_nombre(std::move(nombre)),
      m_factorVelocidad(factorVelocidad),
      m_factorAtaque(factorAtaque),
      m_duracion(duracion),
      m_tiempoRestante(duracion)
{
}

void Efecto::aplicar(Personaje& personaje) const
{
    personaje.aplicarModificadores(m_factorVelocidad, m_factorAtaque, m_duracion);
}

void Efecto::actualizar(float dt)
{
    m_tiempoRestante -= dt;
}

bool Efecto::finalizado() const
{
    return m_tiempoRestante <= 0.0f;
}

QString Efecto::nombre() const
{
    return m_nombre;
}

float Efecto::factorVelocidad() const
{
    return m_factorVelocidad;
}

float Efecto::factorAtaque() const
{
    return m_factorAtaque;
}

float Efecto::duracion() const
{
    return m_duracion;
}
