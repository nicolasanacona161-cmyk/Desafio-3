#include "Control.h"

#include <Qt>

void Control::presionar(int tecla)
{
    m_teclasPresionadas[tecla] = true;
}

void Control::soltar(int tecla)
{
    m_teclasPresionadas[tecla] = false;
}

bool Control::estaPresionada(int tecla) const
{
    const auto it = m_teclasPresionadas.find(tecla);
    return it != m_teclasPresionadas.end() && it->second;
}

Vector2D Control::obtenerDireccion(bool topDown) const
{
    Vector2D direccion;
    if (estaPresionada(Qt::Key_A)) {
        direccion.x -= 1.0f;
    }
    if (estaPresionada(Qt::Key_D)) {
        direccion.x += 1.0f;
    }
    if (topDown && estaPresionada(Qt::Key_W)) {
        direccion.y -= 1.0f;
    }
    if (topDown && estaPresionada(Qt::Key_S)) {
        direccion.y += 1.0f;
    }
    return direccion;
}

TipoAccion Control::obtenerAccion(bool topDown) const
{
    if (estaPresionada(Qt::Key_J)) {
        return TipoAccion::GolpeFrontal;
    }
    if (estaPresionada(Qt::Key_K)) {
        return TipoAccion::Gancho;
    }
    if (estaPresionada(Qt::Key_L)) {
        return TipoAccion::Bloquear;
    }
    if (!topDown && estaPresionada(Qt::Key_S)) {
        return TipoAccion::Bloquear;
    }
    return TipoAccion::Ninguna;
}
