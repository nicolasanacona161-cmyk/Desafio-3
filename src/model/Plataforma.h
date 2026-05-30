#pragma once

#include "Vector2D.h"

class Plataforma {
public:
    Plataforma(Vector2D posicion, float amplitud, float periodo);

    void actualizar(float tiempo);
    Vector2D obtenerPosicion() const;
    bool activa() const;

private:
    Vector2D m_posicion;
    float m_yBase;
    float m_amplitud;
    float m_periodo;
    bool m_activa = true;
};
