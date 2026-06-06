#pragma once

class Personaje;

class SistemaCombate {
public:
    SistemaCombate(float rangoGolpe = 86.0f, float multiplicadorBloqueo = 0.35f);

    bool procesarAtaque(Personaje& atacante, Personaje& objetivo);
    bool hayImpacto(const Personaje& atacante, const Personaje& objetivo) const;
    int calcularDanio(const Personaje& atacante, const Personaje& objetivo) const;
    float rangoGolpe() const;

private:
    float m_rangoGolpe;
    float m_multiplicadorBloqueo;
    float m_tiempoRecuperacion = 0.25f;
};
