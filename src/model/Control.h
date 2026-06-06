#pragma once

#include "Enums.h"
#include "Vector2D.h"

#include <QKeyEvent>
#include <map>

class Control {
public:
    void presionar(int tecla);
    void soltar(int tecla);
    bool estaPresionada(int tecla) const;
    Vector2D obtenerDireccion(bool topDown) const;
    TipoAccion obtenerAccion(bool topDown) const;

private:
    std::map<int, bool> m_teclasPresionadas;
};
