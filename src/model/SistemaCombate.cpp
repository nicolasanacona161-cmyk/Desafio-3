#include "SistemaCombate.h"

#include "Enums.h"
#include "Personaje.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

SistemaCombate::SistemaCombate(float rangoGolpe, float multiplicadorBloqueo)
    : m_rangoGolpe(rangoGolpe),
      m_multiplicadorBloqueo(multiplicadorBloqueo)
{
    if (rangoGolpe <= 0.0f || multiplicadorBloqueo < 0.0f) {
        throw std::invalid_argument("SistemaCombate tiene parametros invalidos.");
    }
}

bool SistemaCombate::procesarAtaque(Personaje& atacante, Personaje& objetivo)
{
    if (!hayImpacto(atacante, objetivo)) {
        return false;
    }
    objetivo.recibirDanio(calcularDanio(atacante, objetivo));
    return true;
}

bool SistemaCombate::hayImpacto(const Personaje& atacante, const Personaje& objetivo) const
{
    if (!atacante.estaVivo() || !objetivo.estaVivo()) {
        return false;
    }
    if (atacante.estado() != EstadoPersonaje::Atacando) {
        return false;
    }
    const float distancia = (atacante.posicion() - objetivo.posicion()).magnitud();
    const float rango = atacante.ultimaAccion() == TipoAccion::Gancho ? m_rangoGolpe * 0.92f : m_rangoGolpe;
    const float deltaX = objetivo.posicion().x - atacante.posicion().x;
    const bool objetivoAlFrente = atacante.mirandoDerecha() ? deltaX >= -12.0f : deltaX <= 12.0f;
    return objetivoAlFrente && distancia <= rango;
}

int SistemaCombate::calcularDanio(const Personaje& atacante, const Personaje& objetivo) const
{
    float baseDanio = atacante.danioGolpe();
    float danio = atacante.ultimaAccion() == TipoAccion::Gancho ? baseDanio * 1.6f : baseDanio;
    danio *= atacante.factorAtaque();
    if (objetivo.estado() == EstadoPersonaje::Bloqueando) {
        danio *= m_multiplicadorBloqueo;
    }
    return std::max(1, static_cast<int>(std::round(danio)));
}

float SistemaCombate::rangoGolpe() const
{
    return m_rangoGolpe;
}
