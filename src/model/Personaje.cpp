#include "Personaje.h"

#include "AgenteInteligente.h"
#include "Control.h"
#include "Efecto.h"
#include "GestorNivel.h"

#include <QFont>
#include <QPen>
#include <Qt>
#include <algorithm>
#include <stdexcept>
#include <utility>

Personaje::Personaje(QString nombre, int vida, float velocidadBase, Vector2D posicion)
    : m_nombre(std::move(nombre)),
      m_vida(vida),
      m_vidaMaxima(vida),
      m_velocidadBase(velocidadBase),
      m_velocidadActual(velocidadBase),
      m_posicion(posicion)
{
    if (vida <= 0 || velocidadBase <= 0.0f) {
        throw std::invalid_argument("Personaje requiere vida y velocidad positivas.");
    }
}

void Personaje::actualizar(float dt)
{
    if (m_cooldownGolpe > 0.0f) {
        m_cooldownGolpe = std::max(0.0f, m_cooldownGolpe - dt);
    }
    if (m_tiempoAnimacionAtaque > 0.0f) {
        m_tiempoAnimacionAtaque = std::max(0.0f, m_tiempoAnimacionAtaque - dt);
    }
    if (m_tiempoBloqueo > 0.0f) {
        m_tiempoBloqueo = std::max(0.0f, m_tiempoBloqueo - dt);
    }
    if (m_tiempoDanio > 0.0f) {
        m_tiempoDanio = std::max(0.0f, m_tiempoDanio - dt);
    }
    if (m_tiempoEfecto > 0.0f) {
        m_tiempoEfecto -= dt;
        if (m_tiempoEfecto <= 0.0f) {
            m_velocidadActual = m_velocidadBase;
            m_factorAtaque = 1.0f;
        }
    }
    if ((m_estado == EstadoPersonaje::Atacando && m_tiempoAnimacionAtaque <= 0.0f)
        || (m_estado == EstadoPersonaje::Bloqueando && m_tiempoBloqueo <= 0.0f)
        || m_estado == EstadoPersonaje::Esquivando
        || (m_estado == EstadoPersonaje::Daniado && m_tiempoDanio <= 0.0f)) {
        m_estado = EstadoPersonaje::Quieto;
    }
}

void Personaje::mover(const Vector2D& direccion, float dt)
{
    if (!m_activo) {
        return;
    }
    m_direccion = direccion.normalizar();
    if (m_direccion.x > 0.05f) {
        m_mirandoDerecha = true;
    } else if (m_direccion.x < -0.05f) {
        m_mirandoDerecha = false;
    }
    m_posicion += m_direccion * (m_velocidadActual * dt);
    if (m_direccion.magnitud() > 0.0f
        && m_estado != EstadoPersonaje::Saltando
        && m_estado != EstadoPersonaje::Atacando
        && m_estado != EstadoPersonaje::Bloqueando
        && m_estado != EstadoPersonaje::Daniado
        && m_estado != EstadoPersonaje::Derrotado) {
        m_estado = EstadoPersonaje::Caminando;
        m_ultimaAccion = TipoAccion::Mover;
    }
}

void Personaje::atacar(TipoAccion tipo)
{
    if (!m_activo || (tipo != TipoAccion::GolpeFrontal && tipo != TipoAccion::Gancho)) {
        return;
    }
    if (m_cooldownGolpe > 0.0f) {
        return;
    }
    m_estado = EstadoPersonaje::Atacando;
    m_ultimaAccion = tipo;
    m_golpePendiente = true;
    m_tiempoAnimacionAtaque = tipo == TipoAccion::Gancho ? 0.26f : 0.20f;
    m_cooldownGolpe = tipo == TipoAccion::Gancho ? 0.48f : 0.38f;
}

void Personaje::bloquear()
{
    if (!m_activo) {
        return;
    }
    m_estado = EstadoPersonaje::Bloqueando;
    m_ultimaAccion = TipoAccion::Bloquear;
    m_tiempoBloqueo = 0.14f;
}

void Personaje::recibirDanio(int cantidad)
{
    if (cantidad < 0) {
        throw std::invalid_argument("El daño no puede ser negativo.");
    }
    if (!m_activo) {
        return;
    }
    m_vida = std::max(0, m_vida - cantidad);
    m_estado = m_vida == 0 ? EstadoPersonaje::Derrotado : EstadoPersonaje::Daniado;
    m_tiempoDanio = m_vida == 0 ? 0.0f : 0.24f;
    m_activo = m_vida > 0;
}

void Personaje::aplicarRetrocesoDesde(const Vector2D& origen, float fuerza)
{
    if (!m_activo || fuerza <= 0.0f) {
        return;
    }
    Vector2D direccion = (m_posicion - origen).normalizar();
    if (direccion.magnitud() <= 0.001f) {
        direccion = m_mirandoDerecha ? Vector2D{-1.0f, 0.0f} : Vector2D{1.0f, 0.0f};
    }
    m_posicion += direccion * fuerza;
    m_velocidadVertical = std::min(m_velocidadVertical, -fuerza * 0.18f);
}

bool Personaje::consumirGolpePendiente()
{
    if (!m_golpePendiente) {
        return false;
    }
    m_golpePendiente = false;
    return true;
}

void Personaje::aplicarEfecto(const Efecto& efecto)
{
    efecto.aplicar(*this);
}

void Personaje::aplicarModificadores(float factorVelocidad, float factorAtaque, float duracion)
{
    if (factorVelocidad <= 0.0f || factorAtaque <= 0.0f || duracion < 0.0f) {
        throw std::invalid_argument("Efecto invalido: factores positivos y duracion no negativa.");
    }
    m_velocidadActual = m_velocidadBase * factorVelocidad;
    m_factorAtaque = factorAtaque;
    m_tiempoEfecto = duracion;
}

bool Personaje::estaVivo() const { return m_activo; }

void Personaje::saltar(float impulso)
{
    if (m_enSuelo && m_activo) {
        m_velocidadVertical = -impulso;
        m_enSuelo = false;
        m_estado = EstadoPersonaje::Saltando;
        m_ultimaAccion = TipoAccion::Saltar;
    }
}

void Personaje::setPosicion(const Vector2D& posicion) { m_posicion = posicion; }
void Personaje::setNombre(const QString& nombre) { m_nombre = nombre; }
void Personaje::setDireccion(const Vector2D& direccion)
{
    m_direccion = direccion.normalizar();
    if (m_direccion.x > 0.05f) {
        m_mirandoDerecha = true;
    } else if (m_direccion.x < -0.05f) {
        m_mirandoDerecha = false;
    }
}

void Personaje::mirarHacia(const Vector2D& objetivo)
{
    const float dx = objetivo.x - m_posicion.x;
    if (dx > 0.05f) {
        m_mirandoDerecha = true;
    } else if (dx < -0.05f) {
        m_mirandoDerecha = false;
    }
}
void Personaje::setVelocidadVertical(float velocidad) { m_velocidadVertical = velocidad; }
void Personaje::setEnSuelo(bool enSuelo) { m_enSuelo = enSuelo; }

void Personaje::limitarA(float minX, float maxX, float minY, float maxY)
{
    m_posicion.x = std::clamp(m_posicion.x, minX, maxX);
    m_posicion.y = std::clamp(m_posicion.y, minY, maxY);
}

void Personaje::resetear(Vector2D posicion)
{
    m_posicion = posicion;
    m_vida = m_vidaMaxima;
    m_velocidadActual = m_velocidadBase;
    m_velocidadVertical = 0.0f;
    m_factorAtaque = 1.0f;
    m_tiempoEfecto = 0.0f;
    m_estado = EstadoPersonaje::Quieto;
    m_ultimaAccion = TipoAccion::Ninguna;
    m_cooldownGolpe = 0.0f;
    m_tiempoAnimacionAtaque = 0.0f;
    m_tiempoBloqueo = 0.0f;
    m_tiempoDanio = 0.0f;
    m_golpePendiente = false;
    m_activo = true;
    m_enSuelo = true;
    m_mirandoDerecha = true;
}

QString Personaje::nombre() const { return m_nombre; }
int Personaje::vida() const { return m_vida; }
int Personaje::vidaMaxima() const { return m_vidaMaxima; }
float Personaje::velocidadBase() const { return m_velocidadBase; }
float Personaje::velocidadActual() const { return m_velocidadActual; }
float Personaje::velocidadVertical() const { return m_velocidadVertical; }
float Personaje::factorAtaque() const { return m_factorAtaque; }
Vector2D Personaje::posicion() const { return m_posicion; }
Vector2D Personaje::direccion() const { return m_direccion; }
EstadoPersonaje Personaje::estado() const { return m_estado; }
TipoAccion Personaje::ultimaAccion() const { return m_ultimaAccion; }
int Personaje::danioGolpe() const { return m_danioGolpe; }
bool Personaje::enSuelo() const { return m_enSuelo; }
bool Personaje::activo() const { return m_activo; }
bool Personaje::mirandoDerecha() const { return m_mirandoDerecha; }
float Personaje::tiempoDanio() const { return m_tiempoDanio; }

Jugador::Jugador(QString nombre, Vector2D posicion)
    : Personaje(std::move(nombre), 300, 210.0f, posicion)
{
    m_danioGolpe = 5;
    actualizarTipoSuper();
}

void Jugador::procesarEntrada(const Control& input, bool topDown)
{
    m_accionActual = input.obtenerAccion(topDown);
    if (m_accionActual == TipoAccion::GolpeFrontal || m_accionActual == TipoAccion::Gancho) {
        atacar(m_accionActual);
        m_comboActual++;
    } else if (m_accionActual == TipoAccion::Bloquear) {
        bloquear();
        m_comboActual = 0;
    }

    if (!topDown && input.estaPresionada(Qt::Key_W)) {
        saltar(285.0f);
    }
}

void Jugador::actualizar(float dt)
{
    Personaje::actualizar(dt);
    agregarCarga(dt * 1.25f);
    actualizarSuper();
}

void Jugador::mover(const Vector2D& direccion, float dt)
{
    if (m_superActivo && superEsVeloz()) {
        m_estelaSuper.push_back(m_posicion);
        if (m_estelaSuper.size() > 8) {
            m_estelaSuper.erase(m_estelaSuper.begin());
        }
    }
    Personaje::mover(direccion, dt);
}

void Jugador::agregarCarga(float cantidad)
{
    if (cantidad <= 0.0f || m_superActivo) {
        return;
    }
    m_superCarga = std::min(100.0f, m_superCarga + cantidad);
}

bool Jugador::activarSuper()
{
    if (m_superCarga < 100.0f || m_superActivo) {
        return false;
    }
    actualizarTipoSuper();
    m_superCarga = 0.0f;
    m_superActivo = true;
    m_timerSuper = 180;
    m_timerTextoSuper = 60;
    m_estelaSuper.clear();
    if (superEsVeloz()) {
        aplicarModificadores(3.0f, 1.25f, 3.0f);
    } else if (superEsAgil()) {
        aplicarModificadores(1.65f, 1.75f, 3.0f);
        m_velocidadVertical = 0.0f;
        m_enSuelo = true;
    } else {
        aplicarModificadores(0.92f, 2.15f, 3.0f);
    }
    return true;
}

void Jugador::actualizarSuper()
{
    if (m_timerParpadeo > 0) {
        --m_timerParpadeo;
    } else {
        m_barParpadea = !m_barParpadea;
        m_timerParpadeo = 18;
    }
    if (m_timerTextoSuper > 0) {
        --m_timerTextoSuper;
    }
    if (!m_superActivo) {
        return;
    }
    if (superEsAgil()) {
        m_velocidadVertical = 0.0f;
        m_enSuelo = true;
    }
    if (m_timerSuper > 0) {
        --m_timerSuper;
    }
    if (m_timerSuper <= 0) {
        m_superActivo = false;
        m_estelaSuper.clear();
        if (m_tiempoEfecto <= 0.0f) {
            m_velocidadActual = m_velocidadBase;
            m_factorAtaque = 1.0f;
        }
    }
}

void Jugador::dibujarBarraCarga(QPainter& painter, int x, int y, int ancho, int alto) const
{
    const QRect fondo(x, y, ancho, alto);
    painter.setPen(QPen(QColor(245, 245, 245), 1));
    painter.setBrush(QColor(34, 34, 38));
    painter.drawRoundedRect(fondo, 4, 4);

    QRect relleno = fondo.adjusted(2, 2, -2, -2);
    relleno.setWidth(static_cast<int>(relleno.width() * (m_superCarga / 100.0f)));
    const bool llenoVisible = m_superCarga >= 100.0f && m_barParpadea;
    painter.setPen(Qt::NoPen);
    painter.setBrush(llenoVisible ? QColor(255, 255, 180) : QColor(242, 184, 42));
    painter.drawRoundedRect(relleno, 3, 3);

    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Segoe UI", 8, QFont::Black));
    painter.drawText(fondo, Qt::AlignCenter, m_superCarga >= 100.0f ? "MAX" : QString("SUPER %1%").arg(static_cast<int>(m_superCarga)));
}

void Jugador::dibujarEfectoSuper(QPainter& painter, const QRect& viewport) const
{
    if (!m_superActivo && m_timerTextoSuper <= 0) {
        return;
    }
    if (m_superActivo) {
        const QColor capa = superEsFuerte() ? QColor(255, 198, 34, 48)
            : superEsAgil() ? QColor(100, 210, 255, 42)
            : QColor(255, 65, 75, 42);
        painter.fillRect(viewport, capa);
    }
    if (m_timerTextoSuper > 0) {
        painter.setFont(QFont("Segoe UI", 48, QFont::Black));
        painter.setPen(QColor(255, 232, 92));
        painter.drawText(viewport.adjusted(0, -90, 0, 0), Qt::AlignCenter, "¡SUPER!");
    }
}

void Jugador::actualizarTipoSuper()
{
    if (m_nombre.contains("Snoopy", Qt::CaseInsensitive)) {
        m_tipoSuper = TipoSuper::Veloz;
    } else if (m_nombre.contains("Mau", Qt::CaseInsensitive) || m_nombre.contains("Spider", Qt::CaseInsensitive)) {
        m_tipoSuper = TipoSuper::Agil;
    } else {
        m_tipoSuper = TipoSuper::Fuerte;
    }
}

TipoAccion Jugador::obtenerAccionActual() const
{
    return m_accionActual;
}

int Jugador::comboActual() const
{
    return m_comboActual;
}

float Jugador::superCarga() const { return m_superCarga; }
bool Jugador::superActivo() const { return m_superActivo; }
int Jugador::timerSuper() const { return m_timerSuper; }
int Jugador::timerTextoSuper() const { return m_timerTextoSuper; }
bool Jugador::superEsVeloz() const { return m_tipoSuper == TipoSuper::Veloz; }
bool Jugador::superEsFuerte() const { return m_tipoSuper == TipoSuper::Fuerte; }
bool Jugador::superEsAgil() const { return m_tipoSuper == TipoSuper::Agil; }
const std::vector<Vector2D>& Jugador::estelaSuper() const { return m_estelaSuper; }

Enemigo::Enemigo(QString nombre, Vector2D posicion)
    : Personaje(std::move(nombre), 300, 185.0f, posicion),
      m_agente(new AgenteInteligente),
      m_decisionActual(new DecisionIA)
{
    m_danioGolpe = 5;
}

Enemigo::~Enemigo()
{
    delete m_agente;
    delete m_decisionActual;
}

void Enemigo::tomarDecision(const Jugador& jugador, const GestorNivel& nivel, Dificultad dificultad)
{
    if (m_tiempoEfecto <= 0.0f) {
        const float velocidadIA = dificultad == Dificultad::Vibranium ? 1.35f
            : dificultad == Dificultad::Normal ? 1.20f
            : 1.08f;
        m_velocidadActual = m_velocidadBase * velocidadIA;
    }
    m_danioGolpe = dificultad == Dificultad::Vibranium ? 10
                 : (dificultad == Dificultad::Normal ? 7 : 5);
    *m_decisionActual = m_agente->actualizar(jugador, *this, nivel, dificultad);
}

void Enemigo::actualizar(float dt)
{
    Personaje::actualizar(dt);
    agregarCarga(dt * 0.95f);
    actualizarSuper();
}

void Enemigo::ejecutarDecision(float dt)
{
    if (!m_decisionActual || !m_decisionActual->esValida()) {
        return;
    }
    if (m_decisionActual->accion == TipoAccion::Mover || m_decisionActual->accion == TipoAccion::RecogerObjeto) {
        mover((m_decisionActual->destino - m_posicion).normalizar(), dt);
    } else if (m_decisionActual->accion == TipoAccion::GolpeFrontal || m_decisionActual->accion == TipoAccion::Gancho) {
        mirarHacia(m_decisionActual->destino);
        atacar(m_decisionActual->accion);
    } else if (m_decisionActual->accion == TipoAccion::Bloquear) {
        bloquear();
    } else if (m_decisionActual->accion == TipoAccion::Saltar) {
        mirarHacia(m_decisionActual->destino);
        saltar(255.0f);
    } else if (m_decisionActual->accion == TipoAccion::Esquivar) {
        Vector2D escape = (m_posicion - m_decisionActual->destino).normalizar();
        if (escape.magnitud() <= 0.0f) {
            escape = {1.0f, 0.0f};
        }
        mover({escape.x, escape.y + 0.35f}, dt * 2.2f);
        m_estado = EstadoPersonaje::Esquivando;
    }
}

void Enemigo::aplicarDecision(const DecisionIA& decision)
{
    if (m_decisionActual) {
        *m_decisionActual = decision;
    }
}

DecisionIA Enemigo::decisionActual() const
{
    return m_decisionActual ? *m_decisionActual : DecisionIA{};
}

void Enemigo::agregarCarga(float cantidad)
{
    if (cantidad <= 0.0f || m_superActivo) {
        return;
    }
    m_superCarga = std::min(100.0f, m_superCarga + cantidad);
}

bool Enemigo::activarSuper()
{
    if (m_superCarga < 100.0f || m_superActivo) {
        return false;
    }
    actualizarTipoSuper();
    m_superCarga = 0.0f;
    m_superActivo = true;
    m_timerSuper = 180;
    if (superEsVeloz()) {
        aplicarModificadores(2.65f, 1.2f, 3.0f);
    } else if (superEsAgil()) {
        aplicarModificadores(1.45f, 1.6f, 3.0f);
    } else {
        aplicarModificadores(0.95f, 1.95f, 3.0f);
    }
    return true;
}

void Enemigo::actualizarSuper()
{
    if (!m_superActivo) {
        return;
    }
    if (m_timerSuper > 0) {
        --m_timerSuper;
    }
    if (m_timerSuper <= 0) {
        m_superActivo = false;
        if (m_tiempoEfecto <= 0.0f) {
            m_velocidadActual = m_velocidadBase;
            m_factorAtaque = 1.0f;
        }
    }
}

void Enemigo::actualizarTipoSuper()
{
    if (m_nombre.contains("Snoopy", Qt::CaseInsensitive)) {
        m_tipoSuper = TipoSuper::Veloz;
    } else if (m_nombre.contains("Mau", Qt::CaseInsensitive) || m_nombre.contains("Spider", Qt::CaseInsensitive)) {
        m_tipoSuper = TipoSuper::Agil;
    } else {
        m_tipoSuper = TipoSuper::Fuerte;
    }
}

bool Enemigo::superActivo() const { return m_superActivo; }
float Enemigo::superCarga() const { return m_superCarga; }
int Enemigo::timerSuper() const { return m_timerSuper; }
bool Enemigo::superEsVeloz() const { return m_tipoSuper == TipoSuper::Veloz; }
bool Enemigo::superEsFuerte() const { return m_tipoSuper == TipoSuper::Fuerte; }
bool Enemigo::superEsAgil() const { return m_tipoSuper == TipoSuper::Agil; }
