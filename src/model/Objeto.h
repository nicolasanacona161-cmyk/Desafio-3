#pragma once

#include "Efecto.h"
#include "Enums.h"
#include "Vector2D.h"

class Personaje;

class Objeto {
public:
    Objeto(TipoObjeto tipo, Vector2D posicion, float velocidadCaida);
    Objeto(TipoObjeto tipo, Vector2D posicion, bool paraJugador, float retardoEfecto);

    void actualizar(float dt);
    void aplicarA(Personaje& personaje);
    bool estaActivo() const;
    bool listoParaAplicar() const;
    bool tieneObjetivo() const;
    bool paraJugador() const;
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
    float m_retardoEfecto = 0.0f;
    bool m_tieneObjetivo = false;
    bool m_paraJugador = false;
    bool m_activo = true;
    Efecto m_efecto;
};
