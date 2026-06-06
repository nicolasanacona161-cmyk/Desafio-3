#include "GestorNivel.h"

#include "Control.h"
#include "Objeto.h"
#include "niveles/NivelDosCenital.h"
#include "niveles/NivelUnoLunar.h"

#include <QRandomGenerator>

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
int probabilidadBoostJugador(Dificultad dificultad)
{
    if (dificultad == Dificultad::Entrenamiento) {
        return 90;
    }
    if (dificultad == Dificultad::Vibranium) {
        return 15;
    }
    return 40;
}

int probabilidadBoostIA(Dificultad dificultad)
{
    return dificultad == Dificultad::Vibranium ? 75 : 50;
}
}

GestorNivel::GestorNivel()
{
    m_dificultad = Dificultad::Normal;
    cargarNivel(1);
}

GestorNivel::~GestorNivel()
{
    limpiar();
}

void GestorNivel::limpiar()
{
    for (Personaje* personaje : m_personajes) {
        delete personaje;
    }
    for (Objeto* objeto : m_objetos) {
        delete objeto;
    }
    for (Plataforma* plataforma : m_plataformas) {
        delete plataforma;
    }
    m_personajes.clear();
    m_objetos.clear();
    m_plataformas.clear();
}

void GestorNivel::cargarNivel(int numero)
{
    if (numero < 1 || numero > 2) {
        throw std::out_of_range("Marvel Boxing solo define los niveles 1 y 2.");
    }
    limpiar();
    if (numero == 1) {
        NivelUnoLunar nivel;
        nivel.cargar(*this);
    } else {
        NivelDosCenital nivel;
        nivel.cargar(*this);
    }
}

void GestorNivel::configurarDificultad()
{
    m_sistemaCombate = SistemaCombate(68.0f, 0.28f);
}

void GestorNivel::actualizar(float dt, const Control& control)
{
    if (m_personajes.size() < 2) {
        throw std::runtime_error("El nivel necesita jugador y enemigo.");
    }
    m_tiempoNivel += dt;
    if (finalizarNivel()) {
        return;
    }
    if (m_nivelActual == 1) {
        NivelUnoLunar nivel;
        nivel.actualizar(*this, dt, control);
    } else {
        NivelDosCenital nivel;
        nivel.actualizar(*this, dt, control);
    }
}

float GestorNivel::intervaloObjetos() const
{
    const float base = 2.2f;
    return std::max(0.85f, base - m_tiempoNivel * 0.015f);
}

void GestorNivel::generarObjetoAleatorio()
{
    if (m_personajes.size() < 2) {
        return;
    }

    const bool haciaJugador = QRandomGenerator::global()->bounded(100) < 50;
    Personaje& objetivo = haciaJugador
        ? static_cast<Personaje&>(jugador())
        : static_cast<Personaje&>(enemigo());
    const int probabilidadBoost = haciaJugador ? probabilidadBoostJugador(m_dificultad) : probabilidadBoostIA(m_dificultad);
    const bool boost = QRandomGenerator::global()->bounded(100) < probabilidadBoost;
    const TipoObjeto tipo = boost ? TipoObjeto::BebidaEnergetica : TipoObjeto::Tomate;

    const QString receptor = haciaJugador ? "Jugador" : "IA";
    const QString efecto = tipo == TipoObjeto::Tomate ? "SLOW" : "BOOST";
    m_objetos.push_back(new Objeto(tipo, objetivo.posicion(), haciaJugador, 0.35f));
    m_mensajeEstado = QString("%1 recibe %2").arg(receptor, efecto);
}

void GestorNivel::verificarColisiones()
{
    if (m_nivelActual == 2) {
        auto procesarAtaqueCenital = [&](Personaje& atacante, Personaje& objetivo) {
            if (atacante.estado() != EstadoPersonaje::Atacando || !atacante.consumirGolpePendiente()) {
                return;
            }
            const bool ataqueJugador = &atacante == &jugador();
            const Vector2D haciaObjetivo = objetivo.posicion() - atacante.posicion();
            const float distanciaCuerpo = haciaObjetivo.magnitud();
            Vector2D centroAtaque = atacante.posicion();
            if (distanciaCuerpo > 0.001f) {
                const float ayudaPuno = ataqueJugador
                    ? (atacante.ultimaAccion() == TipoAccion::Gancho ? 38.0f : 30.0f)
                    : 18.0f;
                centroAtaque += haciaObjetivo.normalizar() * ayudaPuno;
            }
            const float distancia = (centroAtaque - objetivo.posicion()).magnitud();
            const float rango = atacante.ultimaAccion() == TipoAccion::Gancho
                ? (ataqueJugador ? 120.0f : 108.0f)
                : (ataqueJugador ? 96.0f : 86.0f);
            if (distancia <= rango) {
                objetivo.recibirDanio(m_sistemaCombate.calcularDanio(atacante, objetivo));
                objetivo.aplicarRetrocesoDesde(atacante.posicion(), ataqueJugador ? 12.0f : 10.0f);
                if (ataqueJugador) {
                    jugador().agregarCarga(atacante.ultimaAccion() == TipoAccion::Gancho ? 14.0f : 9.0f);
                } else {
                    enemigo().agregarCarga(atacante.ultimaAccion() == TipoAccion::Gancho ? 12.0f : 8.0f);
                }
                m_mensajeEstado = atacante.nombre() + " conecto golpe cenital";
            } else {
                m_mensajeEstado = atacante.nombre() + " fallo por distancia";
            }
        };
        procesarAtaqueCenital(jugador(), enemigo());
        procesarAtaqueCenital(enemigo(), jugador());
    } else {
    if (jugador().estado() == EstadoPersonaje::Atacando && jugador().consumirGolpePendiente()) {
        if (m_sistemaCombate.procesarAtaque(jugador(), enemigo())) {
            enemigo().aplicarRetrocesoDesde(jugador().posicion(), jugador().ultimaAccion() == TipoAccion::Gancho ? 14.0f : 10.0f);
            jugador().agregarCarga(jugador().ultimaAccion() == TipoAccion::Gancho ? 14.0f : 9.0f);
        }
    }
    if (enemigo().estado() == EstadoPersonaje::Atacando && enemigo().consumirGolpePendiente()) {
        if (m_sistemaCombate.procesarAtaque(enemigo(), jugador())) {
            jugador().aplicarRetrocesoDesde(enemigo().posicion(), enemigo().ultimaAccion() == TipoAccion::Gancho ? 12.0f : 9.0f);
            enemigo().agregarCarga(enemigo().ultimaAccion() == TipoAccion::Gancho ? 12.0f : 8.0f);
        }
    }
    }
    for (Objeto* objeto : m_objetos) {
        if (!objeto || !objeto->estaActivo() || !objeto->listoParaAplicar()) {
            continue;
        }
        if (objeto->tieneObjetivo()) {
            const bool efectoJugador = objeto->paraJugador();
            Personaje& objetivo = efectoJugador
                ? static_cast<Personaje&>(jugador())
                : static_cast<Personaje&>(enemigo());
            m_mensajeEstado = efectoJugador
                ? (objeto->tipo() == TipoObjeto::Tomate ? "Jugador en SLOW: menos velocidad y golpe" : "Jugador en BOOST: mas velocidad y golpe")
                : (objeto->tipo() == TipoObjeto::Tomate ? "IA en SLOW: menos velocidad y golpe" : "IA en BOOST: mas velocidad y golpe");
            objeto->aplicarA(objetivo);
            if (efectoJugador) {
                jugador().agregarCarga(objeto->tipo() == TipoObjeto::BebidaEnergetica ? 18.0f : 8.0f);
            }
            continue;
        }
        if ((objeto->posicion() - jugador().posicion()).magnitud() <= 76.0f) {
            m_mensajeEstado = objeto->tipo() == TipoObjeto::Tomate ? "Jugador en SLOW: menos velocidad y golpe" : "Jugador en BOOST: mas velocidad y golpe";
            objeto->aplicarA(jugador());
            jugador().agregarCarga(objeto->tipo() == TipoObjeto::BebidaEnergetica ? 18.0f : 8.0f);
        } else if ((objeto->posicion() - enemigo().posicion()).magnitud() <= 76.0f) {
            m_mensajeEstado = objeto->tipo() == TipoObjeto::Tomate ? "IA en SLOW: menos velocidad y golpe" : "IA en BOOST: mas velocidad y golpe";
            objeto->aplicarA(enemigo());
        }
    }
}

void GestorNivel::separarPersonajes()
{
    if (m_personajes.size() < 2) {
        return;
    }
    Vector2D delta = enemigo().posicion() - jugador().posicion();
    float distancia = delta.magnitud();
    constexpr float distanciaMinima = 64.0f;
    if (distancia <= 0.001f) {
        delta = {1.0f, 0.0f};
        distancia = 1.0f;
    }
    if (distancia >= distanciaMinima) {
        return;
    }
    const Vector2D direccion = delta.normalizar();
    const float empuje = (distanciaMinima - distancia) * 0.5f;
    jugador().setPosicion(jugador().posicion() - direccion * empuje);
    enemigo().setPosicion(enemigo().posicion() + direccion * empuje);
}

void GestorNivel::eliminarObjetosInactivos()
{
    auto it = std::remove_if(m_objetos.begin(), m_objetos.end(), [](Objeto* objeto) {
        const bool eliminar = objeto == nullptr || !objeto->estaActivo();
        if (eliminar) {
            delete objeto;
        }
        return eliminar;
    });
    m_objetos.erase(it, m_objetos.end());
}

bool GestorNivel::finalizarNivel() const
{
    return !jugador().estaVivo() || !enemigo().estaVivo() || tiempoAgotado();
}

bool GestorNivel::tiempoAgotado() const
{
    return m_tiempoNivel >= m_duracionNivel;
}

void GestorNivel::configurarAreaJuego(float ancho, float alto)
{
    m_anchoJuego = std::max(320.0f, ancho);
    m_altoJuego = std::max(320.0f, alto);
}

void GestorNivel::setDificultad(Dificultad dificultad)
{
    m_dificultad = dificultad;
    configurarDificultad();
}

void GestorNivel::reiniciar()
{
    cargarNivel(m_nivelActual);
}

void GestorNivel::cambiarNivel()
{
    cargarNivel(m_nivelActual == 1 ? 2 : 1);
}

int GestorNivel::nivelActual() const { return m_nivelActual; }
float GestorNivel::tiempoNivel() const { return m_tiempoNivel; }
const Jugador& GestorNivel::jugador() const { return *static_cast<Jugador*>(m_personajes.at(0)); }
const Enemigo& GestorNivel::enemigo() const { return *static_cast<Enemigo*>(m_personajes.at(1)); }
Jugador& GestorNivel::jugador() { return *static_cast<Jugador*>(m_personajes.at(0)); }
Enemigo& GestorNivel::enemigo() { return *static_cast<Enemigo*>(m_personajes.at(1)); }
const std::vector<Personaje*>& GestorNivel::personajes() const { return m_personajes; }
const std::vector<Objeto*>& GestorNivel::objetos() const { return m_objetos; }
const std::vector<Plataforma*>& GestorNivel::plataformas() const { return m_plataformas; }
Dificultad GestorNivel::dificultad() const { return m_dificultad; }
QString GestorNivel::mensajeEstado() const { return m_mensajeEstado; }

QString GestorNivel::resultadoFinal() const
{
    if (!finalizarNivel()) {
        return {};
    }
    if (tiempoAgotado() && jugador().estaVivo() && enemigo().estaVivo()) {
        if (jugador().vida() == enemigo().vida()) {
            return "EMPATE";
        }
        return jugador().vida() > enemigo().vida() ? "GANASTE" : "PERDISTE";
    }
    return jugador().estaVivo() ? "GANASTE" : "PERDISTE";
}
