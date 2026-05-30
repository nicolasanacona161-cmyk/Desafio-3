#pragma once

#include "Enums.h"
#include "Vector2D.h"

#include <QPainter>
#include <QString>
#include <vector>

class AgenteInteligente;
class Control;
struct DecisionIA;
class Efecto;
class GestorNivel;

class Personaje {
public:
    Personaje(QString nombre, int vida, float velocidadBase, Vector2D posicion);
    virtual ~Personaje() = default;

    virtual void actualizar(float dt);
    virtual void mover(const Vector2D& direccion, float dt);
    virtual void atacar(TipoAccion tipo);
    void bloquear();
    void recibirDanio(int cantidad);
    void aplicarRetrocesoDesde(const Vector2D& origen, float fuerza);
    bool consumirGolpePendiente();
    void aplicarEfecto(const Efecto& efecto);
    void aplicarModificadores(float factorVelocidad, float factorAtaque, float duracion);
    bool estaVivo() const;
    void saltar(float impulso);
    void setPosicion(const Vector2D& posicion);
    void setNombre(const QString& nombre);
    void setDireccion(const Vector2D& direccion);
    void mirarHacia(const Vector2D& objetivo);
    void setVelocidadVertical(float velocidad);
    void setEnSuelo(bool enSuelo);
    void limitarA(float minX, float maxX, float minY, float maxY);
    void resetear(Vector2D posicion);

    QString nombre() const;
    int vida() const;
    int vidaMaxima() const;
    float velocidadBase() const;
    float velocidadActual() const;
    float velocidadVertical() const;
    float factorAtaque() const;
    Vector2D posicion() const;
    Vector2D direccion() const;
    EstadoPersonaje estado() const;
    TipoAccion ultimaAccion() const;
    int danioGolpe() const;
    bool enSuelo() const;
    bool activo() const;
    bool mirandoDerecha() const;
    float tiempoDanio() const;

protected:
    QString m_nombre;
    int m_vida = 100;
    int m_vidaMaxima = 100;
    float m_velocidadBase = 180.0f;
    float m_velocidadActual = 180.0f;
    float m_velocidadVertical = 0.0f;
    float m_factorAtaque = 1.0f;
    float m_tiempoEfecto = 0.0f;
    Vector2D m_posicion;
    Vector2D m_direccion;
    EstadoPersonaje m_estado = EstadoPersonaje::Quieto;
    TipoAccion m_ultimaAccion = TipoAccion::Ninguna;
    int m_danioGolpe = 12;
    float m_cooldownGolpe = 0.0f;
    float m_tiempoAnimacionAtaque = 0.0f;
    float m_tiempoBloqueo = 0.0f;
    float m_tiempoDanio = 0.0f;
    bool m_golpePendiente = false;
    bool m_activo = true;
    bool m_enSuelo = true;
    bool m_mirandoDerecha = true;
};

class Jugador : public Personaje {
public:
    Jugador(QString nombre, Vector2D posicion);

    void procesarEntrada(const Control& input, bool topDown);
    void actualizar(float dt) override;
    void mover(const Vector2D& direccion, float dt) override;
    void agregarCarga(float cantidad);
    bool activarSuper();
    void actualizarSuper();
    void dibujarBarraCarga(QPainter& painter, int x, int y, int ancho, int alto) const;
    void dibujarEfectoSuper(QPainter& painter, const QRect& viewport) const;
    TipoAccion obtenerAccionActual() const;
    int comboActual() const;
    float superCarga() const;
    bool superActivo() const;
    int timerSuper() const;
    int timerTextoSuper() const;
    bool superEsVeloz() const;
    bool superEsFuerte() const;
    bool superEsAgil() const;
    const std::vector<Vector2D>& estelaSuper() const;

private:
    void actualizarTipoSuper();

    TipoAccion m_accionActual = TipoAccion::Ninguna;
    int m_comboActual = 0;
    float m_superCarga = 0.0f;
    bool m_superActivo = false;
    int m_timerSuper = 0;
    bool m_barParpadea = false;
    int m_timerParpadeo = 0;
    int m_timerTextoSuper = 0;
    enum class TipoSuper { Veloz, Fuerte, Agil };
    TipoSuper m_tipoSuper = TipoSuper::Fuerte;
    std::vector<Vector2D> m_estelaSuper;
};

class Enemigo : public Personaje {
public:
    Enemigo(QString nombre, Vector2D posicion);
    ~Enemigo() override;

    void tomarDecision(const Jugador& jugador, const GestorNivel& nivel, Dificultad dificultad);
    void actualizar(float dt) override;
    void ejecutarDecision(float dt);
    void aplicarDecision(const DecisionIA& decision);
    DecisionIA decisionActual() const;
    void agregarCarga(float cantidad);
    bool activarSuper();
    bool superActivo() const;
    float superCarga() const;
    int timerSuper() const;
    bool superEsVeloz() const;
    bool superEsFuerte() const;
    bool superEsAgil() const;

private:
    void actualizarSuper();
    void actualizarTipoSuper();

    AgenteInteligente* m_agente = nullptr;
    DecisionIA* m_decisionActual = nullptr;
    float m_superCarga = 0.0f;
    bool m_superActivo = false;
    int m_timerSuper = 0;
    enum class TipoSuper { Veloz, Fuerte, Agil };
    TipoSuper m_tipoSuper = TipoSuper::Fuerte;
};
