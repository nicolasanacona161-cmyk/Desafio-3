#pragma once

#include "NivelBase.h"

class NivelUnoLunar : public NivelBase {
public:
    int numero() const override;
    void cargar(GestorNivel& gestor) override;
    void actualizar(GestorNivel& gestor, float dt, const Control& control) override;
    const char* nombre() const override;
};
