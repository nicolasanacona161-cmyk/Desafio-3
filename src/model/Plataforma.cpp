#include "Plataforma.h"

#include <cmath>
#include <stdexcept>

namespace {
constexpr float Pi = 3.1415926535f;
}

Plataforma::Plataforma(Vector2D posicion, float amplitud, float periodo)
    : m_posicion(posicion),
      m_yBase(posicion.y),
      m_amplitud(amplitud),
      m_periodo(periodo)
{
    if (periodo <= 0.0f) {
        throw std::invalid_argument("El periodo de plataforma debe ser positivo.");
    }
}

void Plataforma::actualizar(float tiempo)
{
    m_posicion.y = m_yBase + m_amplitud * std::sin(2.0f * Pi * tiempo / m_periodo);
}

Vector2D Plataforma::obtenerPosicion() const { return m_posicion; }
bool Plataforma::activa() const { return m_activa; }
