#pragma once

#include <QString>

class Personaje;

class Efecto {
public:
    Efecto() = default;
    Efecto(QString nombre, float factorVelocidad, float factorAtaque, float duracion);

    void aplicar(Personaje& personaje) const;
    void actualizar(float dt);
    bool finalizado() const;
    QString nombre() const;
    float factorVelocidad() const;
    float factorAtaque() const;
    float duracion() const;

private:
    QString m_nombre = "Sin efecto";
    float m_factorVelocidad = 1.0f;
    float m_factorAtaque = 1.0f;
    float m_duracion = 0.0f;
    float m_tiempoRestante = 0.0f;
};
