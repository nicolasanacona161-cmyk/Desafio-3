#pragma once

class Objeto;
class Personaje;
class Plataforma;

class Fisica {
public:
    explicit Fisica(float gravedadActual = 1.62f, float factorFlotacion = 0.85f);

    void setGravedad(float gravedad);
    void setFactorFlotacion(float factor);
    void aplicarGravedad(Personaje& personaje, float dt, float sueloY);
    void actualizarCaida(Objeto& objeto, float dt);
    void actualizarPlataforma(Plataforma& plataforma, float tiempo);
    bool detectarColision(const Personaje& a, const Personaje& b, float rango) const;

private:
    float m_gravedadActual;
    float m_factorFlotacion;
};
