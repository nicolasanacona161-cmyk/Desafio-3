#pragma once

#include "Enums.h"

class Control;
class GestorNivel;

class NivelBase {
public:
    virtual ~NivelBase() = default;
    virtual int numero() const = 0;
    virtual void cargar(GestorNivel& gestor) = 0;
    virtual void actualizar(GestorNivel& gestor, float dt, const Control& control) = 0;
    virtual const char* nombre() const = 0;
};
