#pragma once

#include "Efecto.h"
#include "Enums.h"
#include "Vector2D.h"

class Personaje;

class Objeto {
public:
    Objeto(TipoObjeto tipo, Vector2D posicion, float velocidadCaida);

    void actualizar(float dt);
    void aplicarA(Personaje& personaje);
    bool estaActivo() const;
    TipoObjeto tipo() const;
    Vector2D posicion() const;
    float altura() const;
    Efecto efecto() const;
    void desactivar();

private:
    TipoObjeto m_tipo;
    Vector2D m_posicion;
    float m_altura = 220.0f;
    float m_velocidadCaida;
    bool m_activo = true;
    Efecto m_efecto;
};
