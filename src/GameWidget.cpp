#include "GameWidget.h"

#include "Objeto.h"

#include <QApplication>
#include <QBitmap>
#include <QFont>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMediaPlayer>
#include <QPainter>
#include <QPen>
#include <QUrl>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#else
#include <QMediaContent>
#endif
#include <algorithm>
#include <cmath>
#include <vector>
#include <stdexcept>

namespace {
constexpr int DuracionCombateSegundos = 90;

void aplicarMascaraColor(QPixmap& pixmap, const QColor& color)
{
    if (!pixmap.isNull()) {
        pixmap.setMask(pixmap.createMaskFromColor(color, Qt::MaskInColor));
    }
}
}

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent),
      m_ironMan(crearSpritesIronMan()),
      m_spiderman(crearSpritesSpiderman()),
      m_spidermanNivel2(crearSpritesSpidermanNivel2()),
      m_thor(crearSpritesThor()),
      m_snoopy(crearSpritesSnoopy()),
      m_mau(crearSpritesMau()),
      m_yeng(crearSpritesYeng()),
      m_fondoNivel1(":/fondos/nivel1.png"),
      m_fondoNivel2(":/fondos/nivel2.png")
{
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setMinimumSize(920, 600);
    m_timer.setInterval(16);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &GameWidget::tick);
    m_reloj.start();
    configurarSonidos();

    m_timer.start();
}

void GameWidget::SpriteAnimator::configurar(const QPixmap& hoja, const std::vector<QRect>& frames, int ticks)
{
    spriteSheet = hoja;
    m_frames = frames;
    cantidadFrames = static_cast<int>(m_frames.size());
    frameActual = 0;
    m_tickActual = 0;
    ticksPorFrame = std::max(1, ticks);
    if (!m_frames.empty()) {
        anchoFrame = m_frames.front().width();
        altoFrame = m_frames.front().height();
    }
}

void GameWidget::SpriteAnimator::tick()
{
    if (cantidadFrames <= 1) {
        return;
    }
    ++m_tickActual;
    if (m_tickActual >= ticksPorFrame) {
        m_tickActual = 0;
        frameActual = (frameActual + 1) % cantidadFrames;
    }
}

void GameWidget::SpriteAnimator::reset()
{
    frameActual = 0;
    m_tickActual = 0;
}

QPixmap GameWidget::SpriteAnimator::getFrame() const
{
    if (spriteSheet.isNull() || m_frames.empty()) {
        return {};
    }
    const QRect source = sourceActual();
    return spriteSheet.copy(source.x(), source.y(), source.width(), source.height());
}

QRect GameWidget::SpriteAnimator::sourceActual() const
{
    if (m_frames.empty()) {
        return {};
    }
    return m_frames.at(std::clamp(frameActual, 0, cantidadFrames - 1));
}

void GameWidget::escalarFrames(SpriteAnimado& sprite, QSizeF tamanoBase) const
{
    if (sprite.hoja.isNull() || tamanoBase.width() <= 0.0 || tamanoBase.height() <= 0.0) {
        return;
    }
    const qreal escalaX = static_cast<qreal>(sprite.hoja.width()) / tamanoBase.width();
    const qreal escalaY = static_cast<qreal>(sprite.hoja.height()) / tamanoBase.height();
    if (qFuzzyCompare(escalaX, 1.0) && qFuzzyCompare(escalaY, 1.0)) {
        return;
    }
    for (auto it = sprite.frames.begin(); it != sprite.frames.end(); ++it) {
        for (QRect& frame : it.value()) {
            frame = QRect(qRound(frame.x() * escalaX),
                          qRound(frame.y() * escalaY),
                          qRound(frame.width() * escalaX),
                          qRound(frame.height() * escalaY));
        }
    }
}

void GameWidget::prepararAnimadores(SpriteAnimado& sprite) const
{
    sprite.animadores.clear();
    for (auto it = sprite.frames.constBegin(); it != sprite.frames.constEnd(); ++it) {
        SpriteAnimator animator;
        const bool ataque = it.key() == AnimacionSprite::LateralGolpe
            || it.key() == AnimacionSprite::LateralGancho
            || it.key() == AnimacionSprite::CenitalGolpe;
        const bool movimiento = it.key() == AnimacionSprite::LateralMover
            || it.key() == AnimacionSprite::LateralSaltar
            || it.key() == AnimacionSprite::CenitalMover
            || it.key() == AnimacionSprite::CenitalMoverDerecha
            || it.key() == AnimacionSprite::CenitalMoverIzquierda
            || it.key() == AnimacionSprite::CenitalMoverAbajo
            || it.key() == AnimacionSprite::CenitalMoverArriba;
        const int ticks = ataque ? 3 : movimiento ? 4 : 8;
        animator.configurar(sprite.hoja, it.value(), ticks);
        sprite.animadores.insert(it.key(), animator);
    }
}

QPixmap GameWidget::cargarSpriteSheet(const QString& ruta) const
{
    QPixmap sheet(ruta);
    if (sheet.isNull()) {
        return {};
    }

    if (!sheet.hasAlphaChannel()) {
        aplicarMascaraColor(sheet, QColor(255, 0, 255));
        aplicarMascaraColor(sheet, QColor(0, 255, 0));
        aplicarMascaraColor(sheet, QColor(0, 0, 0));
    }
    return sheet;
}

void GameWidget::tickAnimadores()
{
    auto tickSprite = [](SpriteAnimado& sprite) {
        for (auto it = sprite.animadores.begin(); it != sprite.animadores.end(); ++it) {
            it.value().tick();
        }
    };
    tickSprite(m_ironMan);
    tickSprite(m_spiderman);
    tickSprite(m_spidermanNivel2);
    tickSprite(m_thor);
    tickSprite(m_snoopy);
    tickSprite(m_mau);
    tickSprite(m_yeng);
}

void GameWidget::reiniciarAnimadores()
{
    auto resetSprite = [this](SpriteAnimado& sprite) {
        prepararAnimadores(sprite);
    };
    resetSprite(m_ironMan);
    resetSprite(m_spiderman);
    resetSprite(m_spidermanNivel2);
    resetSprite(m_thor);
    resetSprite(m_snoopy);
    resetSprite(m_mau);
    resetSprite(m_yeng);
}

void GameWidget::reiniciarAnimacionActual(const Personaje& personaje)
{
    SpriteAnimado& sprite = const_cast<SpriteAnimado&>(spritesPara(personaje));
    const AnimacionSprite animacion = animacionPara(personaje);
    auto it = sprite.animadores.find(animacion);
    if (it != sprite.animadores.end()) {
        it.value().reset();
    }
}

void GameWidget::actualizarAnimacionesActuales()
{
    const AnimacionSprite jugador = animacionPara(m_nivel.jugador());
    const AnimacionSprite enemigo = animacionPara(m_nivel.enemigo());
    if (jugador != m_animacionJugadorAnterior) {
        m_animacionJugadorAnterior = jugador;
        reiniciarAnimacionActual(m_nivel.jugador());
    }
    if (enemigo != m_animacionEnemigoAnterior) {
        m_animacionEnemigoAnterior = enemigo;
        reiniciarAnimacionActual(m_nivel.enemigo());
    }
}

bool GameWidget::nombreTiene(const Personaje& personaje, const QString& token) const
{
    return personaje.nombre().contains(token, Qt::CaseInsensitive);
}

void GameWidget::activarSuperPersonaje(bool delJugador)
{
    Personaje& personaje = delJugador ? static_cast<Personaje&>(m_nivel.jugador()) : static_cast<Personaje&>(m_nivel.enemigo());
    const bool activado = delJugador ? m_nivel.jugador().activarSuper() : m_nivel.enemigo().activarSuper();
    if (!activado) {
        return;
    }
    reproducirSuper();
    crearSuperEventos(delJugador);
    if (nombreTiene(personaje, "Snoopy")) {
        personaje.aplicarModificadores(3.2f, 1.25f, 3.0f);
    }
}

void GameWidget::crearSuperEventos(bool delJugador)
{
    const Personaje& usuario = delJugador ? static_cast<const Personaje&>(m_nivel.jugador()) : static_cast<const Personaje&>(m_nivel.enemigo());
    const Personaje& objetivo = delJugador ? static_cast<const Personaje&>(m_nivel.enemigo()) : static_cast<const Personaje&>(m_nivel.jugador());
    const Vector2D direccion = (objetivo.posicion() - usuario.posicion()).normalizar();
    const Vector2D baseDir = direccion.magnitud() > 0.001f ? direccion : Vector2D{usuario.mirandoDerecha() ? 1.0f : -1.0f, 0.0f};

    if (nombreTiene(usuario, "Mau")) {
        for (int i = -1; i <= 1; ++i) {
            SuperEvento evento;
            evento.tipo = TipoSuperEvento::Guante;
            evento.posicion = usuario.posicion() + Vector2D{0.0f, -38.0f + i * 18.0f};
            evento.velocidad = Vector2D{baseDir.x * 620.0f, baseDir.y * 220.0f + i * 75.0f};
            evento.delJugador = delJugador;
            evento.ticksVida = 70;
            evento.ticksTotales = evento.ticksVida;
            m_superEventos.push_back(evento);
        }
    } else if (nombreTiene(usuario, "Thor")) {
        for (int i = -2; i <= 2; ++i) {
            SuperEvento evento;
            evento.tipo = TipoSuperEvento::Rayo;
            evento.posicion = objetivo.posicion() + Vector2D{i * 70.0f, 0.0f};
            evento.delJugador = delJugador;
            evento.ticksVida = 48 + (i + 2) * 4;
            evento.ticksTotales = evento.ticksVida;
            m_superEventos.push_back(evento);
        }
    } else if (nombreTiene(usuario, "Spider")) {
        for (int i = -1; i <= 1; ++i) {
            SuperEvento evento;
            evento.tipo = TipoSuperEvento::Telarana;
            evento.posicion = usuario.posicion() + Vector2D{0.0f, -32.0f + i * 14.0f};
            evento.velocidad = Vector2D{baseDir.x * 520.0f, baseDir.y * 160.0f + i * 45.0f};
            evento.delJugador = delJugador;
            evento.ticksVida = 80;
            evento.ticksTotales = evento.ticksVida;
            m_superEventos.push_back(evento);
        }
    } else if (!nombreTiene(usuario, "Snoopy")) {
        SuperEvento evento;
        evento.tipo = TipoSuperEvento::Rayo;
        evento.posicion = objetivo.posicion();
        evento.delJugador = delJugador;
        evento.ticksVida = 42;
        evento.ticksTotales = evento.ticksVida;
        m_superEventos.push_back(evento);
    }
}

void GameWidget::actualizarSuperEventos(float dt)
{
    for (SuperEvento& evento : m_superEventos) {
        evento.posicion += evento.velocidad * dt;
        --evento.ticksVida;
        Personaje& objetivo = evento.delJugador ? static_cast<Personaje&>(m_nivel.enemigo()) : static_cast<Personaje&>(m_nivel.jugador());
        const float distancia = (objetivo.posicion() - evento.posicion).magnitud();
        if (!evento.impactoAplicado && evento.tipo == TipoSuperEvento::Guante && distancia < 72.0f) {
            if (objetivo.estado() != EstadoPersonaje::Bloqueando) {
                objetivo.recibirDanio(25);
                objetivo.aplicarRetrocesoDesde(evento.posicion, 12.0f);
            }
            evento.impactoAplicado = true;
            evento.ticksVida = std::min(evento.ticksVida, 8);
        } else if (!evento.impactoAplicado && evento.tipo == TipoSuperEvento::Telarana && distancia < 78.0f) {
            if (objetivo.estado() != EstadoPersonaje::Bloqueando) {
                objetivo.recibirDanio(25);
                objetivo.aplicarModificadores(0.42f, 0.8f, 2.5f);
            }
            evento.impactoAplicado = true;
            evento.ticksVida = std::min(evento.ticksVida, 12);
        } else if (!evento.impactoAplicado && evento.tipo == TipoSuperEvento::Rayo && evento.ticksVida < evento.ticksTotales - 18 && distancia < 92.0f) {
            if (objetivo.estado() != EstadoPersonaje::Bloqueando) {
                objetivo.recibirDanio(25);
                objetivo.aplicarRetrocesoDesde(evento.posicion + Vector2D{0.0f, -160.0f}, 14.0f);
            }
            evento.impactoAplicado = true;
        }
    }

    auto it = std::remove_if(m_superEventos.begin(), m_superEventos.end(), [](const SuperEvento& evento) {
        return evento.ticksVida <= 0;
    });
    m_superEventos.erase(it, m_superEventos.end());
}

GameWidget::SpriteAnimado GameWidget::crearSpritesIronMan() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/iron_man.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(29, 134, 120, 134)};
    s.frames[AnimacionSprite::LateralMover] = {
        QRect(29, 134, 120, 134),
        QRect(150, 134, 120, 134),
        QRect(271, 134, 120, 134),
        QRect(392, 134, 120, 134)
    };
    s.frames[AnimacionSprite::LateralSaltar] = {
        QRect(1055, 134, 120, 156),
        QRect(1176, 134, 120, 156),
        QRect(1297, 134, 120, 156),
        QRect(1418, 134, 120, 156),
        QRect(1539, 134, 120, 156),
        QRect(1660, 134, 120, 156),
        QRect(1781, 134, 120, 156),
        QRect(1902, 134, 120, 156)
    };
    s.frames[AnimacionSprite::LateralBloquear] = {
        QRect(29, 309, 120, 150),
        QRect(150, 309, 120, 150),
        QRect(271, 309, 120, 150)
    };
    s.frames[AnimacionSprite::LateralDanio] = {
        QRect(392, 309, 120, 150),
        QRect(513, 309, 120, 150)
    };
    s.frames[AnimacionSprite::LateralGolpe] = {
        QRect(30, 507, 120, 159),
        QRect(150, 507, 120, 159),
        QRect(271, 507, 120, 159),
        QRect(392, 507, 120, 159),
        QRect(513, 507, 120, 159)
    };
    s.frames[AnimacionSprite::LateralGancho] = {
        QRect(1056, 507, 120, 159),
        QRect(1177, 507, 120, 159),
        QRect(1298, 507, 120, 159),
        QRect(1419, 507, 120, 159),
        QRect(1540, 507, 120, 159),
        QRect(1661, 507, 120, 159),
        QRect(1782, 507, 120, 159)
    };
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(29, 761, 150, 139)};
    s.frames[AnimacionSprite::CenitalMover] = {
        QRect(29, 761, 150, 139),
        QRect(179, 761, 150, 139),
        QRect(329, 761, 150, 139),
        QRect(479, 761, 150, 139)
    };
    s.frames[AnimacionSprite::CenitalGolpe] = {
        QRect(1058, 762, 150, 139),
        QRect(1208, 762, 150, 139),
        QRect(1358, 762, 150, 139),
        QRect(1508, 762, 150, 139),
        QRect(1658, 762, 150, 139)
    };
    s.frames[AnimacionSprite::CenitalDanio] = {
        QRect(323, 948, 140, 151),
        QRect(463, 948, 140, 151),
        QRect(603, 948, 140, 151)
    };
    s.frames[AnimacionSprite::ItemTomate] = {QRect(29, 947, 168, 153), QRect(197, 947, 126, 153)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1032, 947, 164, 153)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesSpiderman() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/spiderman.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(1066, 302, 184, 284)};
    s.frames[AnimacionSprite::LateralMover] = {
        QRect(1066, 302, 184, 284),
        QRect(1248, 302, 176, 284),
        QRect(1448, 302, 170, 284),
        QRect(1640, 302, 180, 284),
        QRect(1822, 302, 188, 284)
    };
    s.frames[AnimacionSprite::LateralSaltar] = {
        QRect(134, 760, 146, 252),
        QRect(370, 736, 148, 262),
        QRect(604, 700, 142, 248),
        QRect(858, 692, 138, 240),
        QRect(1088, 692, 150, 238),
        QRect(1320, 700, 136, 228),
        QRect(1496, 720, 258, 258),
        QRect(1774, 760, 222, 224)
    };
    s.frames[AnimacionSprite::LateralBloquear] = {
        QRect(158, 1450, 156, 232),
        QRect(398, 1414, 178, 268)
    };
    s.frames[AnimacionSprite::LateralGolpe] = {
        QRect(708, 1418, 230, 262),
        QRect(964, 1410, 230, 270),
        QRect(1280, 1410, 232, 270)
    };
    s.frames[AnimacionSprite::LateralGancho] = {
        QRect(128, 1940, 154, 236),
        QRect(406, 1900, 148, 276),
        QRect(706, 1770, 132, 406),
        QRect(1010, 1756, 192, 264),
        QRect(1320, 1630, 164, 388),
        QRect(1610, 1560, 172, 450)
    };
    s.frames[AnimacionSprite::LateralDanio] = {
        QRect(600, 1200, 150, 252),
        QRect(850, 1190, 132, 252)
    };
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(1082, 240, 142, 244)};
    s.frames[AnimacionSprite::CenitalMover] = {
        QRect(1082, 240, 142, 244),
        QRect(1242, 240, 142, 244),
        QRect(1402, 240, 142, 244),
        QRect(1562, 240, 142, 244),
        QRect(1722, 240, 142, 244),
        QRect(1882, 240, 142, 244)
    };
    s.frames[AnimacionSprite::CenitalGolpe] = {
        QRect(1082, 606, 142, 112),
        QRect(1242, 606, 142, 112),
        QRect(1402, 606, 142, 112),
        QRect(1562, 606, 142, 112),
        QRect(1722, 606, 142, 112),
        QRect(1882, 606, 142, 112)
    };
    s.frames[AnimacionSprite::CenitalDanio] = {
        QRect(1082, 606, 142, 112),
        QRect(1242, 606, 142, 112)
    };
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1082, 834, 140, 120)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1242, 834, 140, 120)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesSpidermanNivel2() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/Lv2_spiderman_DASW.png");
    s.frames[AnimacionSprite::CenitalMoverDerecha] = {
        QRect(0, 0, 400, 500),
        QRect(400, 0, 400, 500),
        QRect(800, 0, 400, 500),
        QRect(1200, 0, 400, 500)
    };
    s.frames[AnimacionSprite::CenitalMoverIzquierda] = {
        QRect(1600, 0, 400, 500),
        QRect(2000, 0, 400, 500),
        QRect(2400, 0, 400, 500),
        QRect(2800, 0, 400, 500)
    };
    s.frames[AnimacionSprite::CenitalMoverAbajo] = {
        QRect(3200, 0, 400, 500),
        QRect(3600, 0, 400, 500),
        QRect(4000, 0, 400, 500)
    };
    s.frames[AnimacionSprite::CenitalMoverArriba] = {
        QRect(4400, 0, 400, 500),
        QRect(4800, 0, 400, 500),
        QRect(5200, 0, 400, 500)
    };
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(0, 0, 400, 500)};
    s.frames[AnimacionSprite::CenitalMover] = s.frames[AnimacionSprite::CenitalMoverDerecha];
    s.frames[AnimacionSprite::CenitalGolpe] = s.frames[AnimacionSprite::CenitalMoverDerecha];
    s.frames[AnimacionSprite::CenitalDanio] = {QRect(3200, 0, 400, 500)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesThor() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/thor.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(22, 122, 82, 95)};
    s.frames[AnimacionSprite::LateralMover] = {QRect(22, 122, 82, 95), QRect(104, 122, 82, 95), QRect(186, 122, 82, 95), QRect(268, 122, 82, 95)};
    s.frames[AnimacionSprite::LateralSaltar] = {QRect(44, 316, 58, 82), QRect(132, 316, 58, 82), QRect(226, 302, 58, 96), QRect(328, 280, 58, 118), QRect(432, 258, 58, 140), QRect(534, 258, 58, 140), QRect(636, 316, 58, 82), QRect(736, 316, 58, 82)};
    s.frames[AnimacionSprite::LateralBloquear] = {QRect(22, 437, 90, 90), QRect(114, 437, 90, 90), QRect(206, 437, 90, 90), QRect(298, 437, 90, 90)};
    s.frames[AnimacionSprite::LateralGolpe] = {QRect(22, 566, 110, 92), QRect(134, 566, 110, 92), QRect(246, 566, 110, 92), QRect(358, 566, 110, 92), QRect(470, 566, 110, 92)};
    s.frames[AnimacionSprite::LateralGancho] = {QRect(22, 697, 92, 105), QRect(116, 697, 92, 105), QRect(230, 665, 106, 137), QRect(346, 697, 106, 105), QRect(480, 665, 106, 137), QRect(595, 665, 106, 137)};
    s.frames[AnimacionSprite::LateralDanio] = {QRect(614, 306, 90, 100), QRect(714, 306, 90, 100)};
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(862, 184, 70, 92)};
    s.frames[AnimacionSprite::CenitalMover] = {QRect(862, 184, 70, 92), QRect(990, 184, 70, 92), QRect(1118, 184, 70, 92), QRect(1240, 184, 70, 92), QRect(1370, 184, 70, 92), QRect(1488, 184, 70, 92)};
    s.frames[AnimacionSprite::CenitalGolpe] = {QRect(858, 674, 76, 80), QRect(986, 674, 76, 80), QRect(1114, 674, 76, 80), QRect(1242, 674, 76, 80)};
    s.frames[AnimacionSprite::CenitalDanio] = {QRect(1114, 454, 76, 92), QRect(1366, 454, 76, 92)};
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1366, 674, 60, 58)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1494, 668, 46, 70)};
    escalarFrames(s, QSizeF(1600, 900));
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesSnoopy() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/snoopy.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(28, 160, 94, 108)};
    s.frames[AnimacionSprite::LateralMover] = {
        QRect(28, 160, 94, 108),
        QRect(128, 160, 84, 108),
        QRect(216, 160, 78, 108),
        QRect(300, 160, 88, 108),
        QRect(389, 160, 86, 108)
    };
    s.frames[AnimacionSprite::LateralSaltar] = {
        QRect(29, 383, 96, 122),
        QRect(146, 360, 108, 145),
        QRect(263, 359, 111, 145),
        QRect(389, 323, 100, 141),
        QRect(505, 323, 88, 126),
        QRect(619, 324, 91, 142),
        QRect(737, 384, 103, 121)
    };
    s.frames[AnimacionSprite::LateralBloquear] = {
        QRect(29, 553, 104, 117),
        QRect(149, 553, 106, 117),
        QRect(270, 553, 106, 117),
        QRect(391, 553, 106, 117),
        QRect(514, 553, 105, 117),
        QRect(635, 553, 106, 117)
    };
    s.frames[AnimacionSprite::LateralGolpe] = {
        QRect(30, 716, 124, 118),
        QRect(162, 716, 127, 118),
        QRect(300, 716, 129, 118),
        QRect(438, 716, 145, 118),
        QRect(594, 716, 130, 118)
    };
    s.frames[AnimacionSprite::LateralGancho] = {
        QRect(29, 918, 117, 127),
        QRect(160, 918, 112, 127),
        QRect(288, 887, 116, 157),
        QRect(425, 888, 151, 156)
    };
    s.frames[AnimacionSprite::LateralDanio] = {
        QRect(29, 553, 104, 117),
        QRect(149, 553, 106, 117)
    };
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(1063, 230, 138, 242)};
    s.frames[AnimacionSprite::CenitalMover] = {
        QRect(1063, 230, 138, 242),
        QRect(1222, 230, 138, 242),
        QRect(1380, 230, 138, 242),
        QRect(1540, 230, 140, 242),
        QRect(1705, 230, 142, 242),
        QRect(1870, 230, 138, 242)
    };
    s.frames[AnimacionSprite::CenitalGolpe] = {
        QRect(1062, 830, 138, 122),
        QRect(1220, 830, 140, 122),
        QRect(1382, 830, 138, 122),
        QRect(1540, 830, 140, 122)
    };
    s.frames[AnimacionSprite::CenitalDanio] = {
        QRect(1220, 598, 140, 114),
        QRect(1540, 598, 140, 114)
    };
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1712, 795, 140, 140)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1882, 795, 126, 140)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesMau() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/mau.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(28, 141, 84, 107)};
    s.frames[AnimacionSprite::LateralMover] = {QRect(28, 141, 84, 107), QRect(123, 141, 87, 107), QRect(219, 141, 92, 107), QRect(323, 141, 94, 107), QRect(428, 141, 92, 107)};
    s.frames[AnimacionSprite::LateralSaltar] = {QRect(29, 343, 89, 103), QRect(132, 343, 94, 103), QRect(244, 321, 94, 125), QRect(355, 300, 93, 146), QRect(459, 300, 91, 146), QRect(563, 300, 93, 146), QRect(674, 343, 96, 103), QRect(787, 343, 91, 103)};
    s.frames[AnimacionSprite::LateralBloquear] = {QRect(29, 486, 91, 101), QRect(134, 486, 89, 101), QRect(234, 486, 89, 101), QRect(354, 486, 89, 101), QRect(456, 486, 89, 101), QRect(558, 486, 99, 101)};
    s.frames[AnimacionSprite::LateralGolpe] = {QRect(29, 625, 100, 96), QRect(137, 625, 111, 96), QRect(268, 625, 119, 96), QRect(407, 625, 124, 96), QRect(550, 625, 107, 96)};
    s.frames[AnimacionSprite::LateralGancho] = {QRect(29, 756, 101, 101), QRect(148, 756, 96, 101), QRect(263, 756, 96, 101), QRect(407, 756, 105, 101), QRect(550, 728, 105, 129), QRect(697, 728, 116, 129)};
    s.frames[AnimacionSprite::LateralDanio] = {QRect(672, 486, 96, 101), QRect(781, 486, 96, 101)};
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(934, 190, 120, 203)};
    s.frames[AnimacionSprite::CenitalMover] = {QRect(934, 190, 120, 203), QRect(1075, 190, 122, 203), QRect(1220, 190, 124, 203), QRect(1366, 190, 118, 203), QRect(1508, 190, 118, 203), QRect(1642, 190, 118, 203)};
    s.frames[AnimacionSprite::CenitalGolpe] = {QRect(934, 700, 132, 116), QRect(1082, 700, 132, 116), QRect(1222, 700, 132, 116), QRect(1364, 700, 132, 116)};
    s.frames[AnimacionSprite::CenitalDanio] = {QRect(1366, 504, 118, 100), QRect(1508, 504, 118, 100)};
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1484, 670, 118, 128)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1620, 670, 112, 128)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesYeng() const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(":/sprites/yeng.png");
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(27, 141, 78, 100)};
    s.frames[AnimacionSprite::LateralMover] = {QRect(27, 141, 78, 100), QRect(113, 141, 84, 100), QRect(207, 141, 86, 100), QRect(307, 141, 88, 100), QRect(406, 141, 86, 100)};
    s.frames[AnimacionSprite::LateralSaltar] = {QRect(27, 338, 88, 101), QRect(127, 338, 96, 101), QRect(236, 319, 96, 120), QRect(344, 296, 91, 143), QRect(445, 296, 90, 143), QRect(544, 296, 91, 143), QRect(649, 338, 92, 101), QRect(755, 338, 90, 101)};
    s.frames[AnimacionSprite::LateralBloquear] = {QRect(27, 483, 89, 102), QRect(130, 483, 88, 102), QRect(234, 483, 88, 102), QRect(336, 483, 88, 102), QRect(438, 483, 88, 102), QRect(542, 483, 88, 102)};
    s.frames[AnimacionSprite::LateralGolpe] = {QRect(27, 625, 101, 100), QRect(142, 625, 105, 100), QRect(262, 625, 118, 100), QRect(392, 625, 118, 100), QRect(525, 625, 102, 100)};
    s.frames[AnimacionSprite::LateralGancho] = {QRect(27, 758, 100, 106), QRect(142, 758, 96, 106), QRect(262, 758, 96, 106), QRect(380, 758, 115, 106), QRect(525, 728, 100, 136), QRect(650, 728, 106, 136)};
    s.frames[AnimacionSprite::LateralDanio] = {QRect(646, 483, 90, 102), QRect(753, 483, 88, 102)};
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(868, 189, 122, 222)};
    s.frames[AnimacionSprite::CenitalMover] = {QRect(868, 189, 122, 222), QRect(1008, 189, 122, 222), QRect(1150, 189, 122, 222), QRect(1288, 189, 118, 222), QRect(1428, 189, 118, 222), QRect(1562, 189, 118, 222)};
    s.frames[AnimacionSprite::CenitalGolpe] = {QRect(868, 728, 130, 112), QRect(1010, 728, 130, 112), QRect(1150, 728, 130, 112), QRect(1288, 728, 130, 112)};
    s.frames[AnimacionSprite::CenitalDanio] = {QRect(1288, 526, 118, 100), QRect(1428, 526, 118, 100)};
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1416, 672, 116, 126)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1550, 672, 110, 126)};
    prepararAnimadores(s);
    return s;
}

GameWidget::SpriteAnimado GameWidget::crearSpritesBoxerGenerico(const QString& ruta, QSizeF tamanoBase) const
{
    SpriteAnimado s;
    s.hoja = cargarSpriteSheet(ruta);
    s.frames[AnimacionSprite::LateralQuieto] = {QRect(22, 130, 76, 82)};
    s.frames[AnimacionSprite::LateralMover] = {QRect(22, 130, 76, 82), QRect(98, 130, 76, 82), QRect(174, 130, 76, 82), QRect(250, 130, 76, 82)};
    s.frames[AnimacionSprite::LateralSaltar] = {QRect(42, 312, 58, 72), QRect(132, 306, 58, 78), QRect(224, 278, 58, 106), QRect(326, 258, 58, 126), QRect(428, 258, 58, 126), QRect(530, 258, 58, 126), QRect(632, 312, 58, 72), QRect(734, 312, 58, 72)};
    s.frames[AnimacionSprite::LateralBloquear] = {QRect(22, 442, 84, 82), QRect(108, 442, 84, 82), QRect(194, 442, 84, 82), QRect(280, 442, 84, 82)};
    s.frames[AnimacionSprite::LateralGolpe] = {QRect(22, 563, 110, 90), QRect(136, 563, 110, 90), QRect(250, 563, 110, 90), QRect(364, 563, 110, 90), QRect(478, 563, 110, 90)};
    s.frames[AnimacionSprite::LateralGancho] = {QRect(22, 720, 92, 100), QRect(116, 720, 92, 100), QRect(230, 690, 98, 130), QRect(346, 720, 98, 100), QRect(470, 690, 98, 130), QRect(590, 690, 98, 130)};
    s.frames[AnimacionSprite::LateralDanio] = {QRect(612, 300, 86, 92), QRect(714, 300, 86, 92)};
    s.frames[AnimacionSprite::CenitalQuieto] = {QRect(860, 184, 68, 88)};
    s.frames[AnimacionSprite::CenitalMover] = {QRect(860, 184, 68, 88), QRect(986, 184, 68, 88), QRect(1112, 184, 68, 88), QRect(1238, 184, 68, 88), QRect(1364, 184, 68, 88), QRect(1490, 184, 68, 88)};
    s.frames[AnimacionSprite::CenitalGolpe] = {QRect(858, 650, 76, 74), QRect(986, 650, 76, 74), QRect(1114, 650, 76, 74), QRect(1242, 650, 76, 74)};
    s.frames[AnimacionSprite::CenitalDanio] = {QRect(1114, 478, 76, 74), QRect(1366, 478, 76, 74)};
    s.frames[AnimacionSprite::ItemTomate] = {QRect(1364, 650, 58, 54)};
    s.frames[AnimacionSprite::ItemBebida] = {QRect(1490, 646, 46, 68)};
    escalarFrames(s, tamanoBase);
    prepararAnimadores(s);
    return s;
}

void GameWidget::setDificultad(Dificultad dificultad)
{
    m_nivel.setDificultad(dificultad);
    emit estadoCambiado("Dificultad actualizada");
}

void GameWidget::setSpriteJugador(const QString& sprite)
{
    m_spriteJugador = sprite;
    update();
}

void GameWidget::setSpriteEnemigo(const QString& sprite)
{
    m_spriteEnemigo = sprite;
    update();
}

void GameWidget::iniciarPartida(int nivel, const QString& spriteJugador, const QString& spriteEnemigo)
{
    m_spriteJugador = spriteJugador;
    m_spriteEnemigo = spriteEnemigo;
    m_nivel.configurarAreaJuego(static_cast<float>(width()), static_cast<float>(height()));
    m_nivel.cargarNivel(nivel);
    m_nivel.jugador().setNombre(spriteJugador);
    m_nivel.enemigo().setNombre(spriteEnemigo + " IA");
    reiniciarAnimadores();
    m_animacionJugadorAnterior = animacionPara(m_nivel.jugador());
    m_animacionEnemigoAnterior = animacionPara(m_nivel.enemigo());
    m_superEventos.clear();
    m_resultadoActivo = false;
    m_tickResultado = 0;
    m_vidaJugadorAnterior = m_nivel.jugador().vida();
    m_vidaEnemigoAnterior = m_nivel.enemigo().vida();
    m_ultimoMensaje = m_nivel.mensajeEstado();
    setFocus();
    emit estadoCambiado(m_nivel.mensajeEstado());
    update();
}

void GameWidget::reiniciar()
{
    m_nivel.configurarAreaJuego(static_cast<float>(width()), static_cast<float>(height()));
    m_nivel.reiniciar();
    m_nivel.jugador().setNombre(m_spriteJugador);
    m_nivel.enemigo().setNombre(m_spriteEnemigo + " IA");
    reiniciarAnimadores();
    m_animacionJugadorAnterior = animacionPara(m_nivel.jugador());
    m_animacionEnemigoAnterior = animacionPara(m_nivel.enemigo());
    m_superEventos.clear();
    m_resultadoActivo = false;
    m_tickResultado = 0;
    m_vidaJugadorAnterior = m_nivel.jugador().vida();
    m_vidaEnemigoAnterior = m_nivel.enemigo().vida();
    m_ultimoMensaje = m_nivel.mensajeEstado();
    emit estadoCambiado(m_nivel.mensajeEstado());
    update();
}

void GameWidget::cambiarNivel()
{
    m_nivel.cambiarNivel();
    m_nivel.jugador().setNombre(m_spriteJugador);
    m_nivel.enemigo().setNombre(m_spriteEnemigo + " IA");
    reiniciarAnimadores();
    m_animacionJugadorAnterior = animacionPara(m_nivel.jugador());
    m_animacionEnemigoAnterior = animacionPara(m_nivel.enemigo());
    m_superEventos.clear();
    m_resultadoActivo = false;
    m_tickResultado = 0;
    m_vidaJugadorAnterior = m_nivel.jugador().vida();
    m_vidaEnemigoAnterior = m_nivel.enemigo().vida();
    m_ultimoMensaje = m_nivel.mensajeEstado();
    emit estadoCambiado(m_nivel.mensajeEstado());
    update();
}

void GameWidget::tick()
{
    const float dt = qMin(0.033f, static_cast<float>(m_reloj.restart()) / 1000.0f);
    ++m_tickLogica;
    try {
        if (!m_nivel.finalizarNivel()) {
            m_nivel.configurarAreaJuego(static_cast<float>(width()), static_cast<float>(height()));
            m_nivel.actualizar(dt, m_control);
            actualizarAnimacionesActuales();
            tickAnimadores();
            if (m_nivel.enemigo().vida() < m_vidaEnemigoAnterior) {
                m_nivel.jugador().agregarCarga(static_cast<float>(m_vidaEnemigoAnterior - m_nivel.enemigo().vida()) * 0.35f);
            }
            if (m_nivel.jugador().vida() < m_vidaJugadorAnterior) {
                m_nivel.enemigo().agregarCarga(static_cast<float>(m_vidaJugadorAnterior - m_nivel.jugador().vida()) * 0.45f);
            }
            if (m_nivel.enemigo().superCarga() >= 100.0f && !m_nivel.enemigo().superActivo()) {
                activarSuperPersonaje(false);
            }
            actualizarSuperEventos(dt);
        }
    } catch (const std::exception& ex) {
        emit estadoCambiado(QString("Error logico: %1").arg(ex.what()));
        m_timer.stop();
    }
    m_vidaJugadorAnterior = m_nivel.jugador().vida();
    m_vidaEnemigoAnterior = m_nivel.enemigo().vida();

    if (m_nivel.finalizarNivel()) {
        if (!m_resultadoActivo) {
            m_resultadoActivo = true;
            m_tickResultado = 0;
        } else {
            ++m_tickResultado;
        }
        emit estadoCambiado("Partida terminada: presiona R para volver a jugar");
    } else if (m_nivel.mensajeEstado() != m_ultimoMensaje) {
        m_ultimoMensaje = m_nivel.mensajeEstado();
        emit estadoCambiado(m_ultimoMensaje);
    }
    update();
}

const GameWidget::SpriteAnimado& GameWidget::spritesPara(const Personaje& personaje) const
{
    if (&personaje == &m_nivel.jugador()) {
        if (m_nivel.nivelActual() == 2 && m_spriteJugador.contains("Spider", Qt::CaseInsensitive)) {
            return m_spidermanNivel2;
        }
        return spritesPorNombre(m_spriteJugador);
    }
    if (&personaje == &m_nivel.enemigo()) {
        if (m_nivel.nivelActual() == 2 && m_spriteEnemigo.contains("Spider", Qt::CaseInsensitive)) {
            return m_spidermanNivel2;
        }
        return spritesPorNombre(m_spriteEnemigo);
    }
    if (m_nivel.nivelActual() == 2 && personaje.nombre().contains("Spider", Qt::CaseInsensitive)) {
        return m_spidermanNivel2;
    }
    return spritesPorNombre(personaje.nombre());
}

const GameWidget::SpriteAnimado& GameWidget::spritesPorNombre(const QString& nombre) const
{
    if (nombre.contains("Iron", Qt::CaseInsensitive)) {
        return m_ironMan;
    }
    if (nombre.contains("Spider", Qt::CaseInsensitive)) {
        return m_spiderman;
    }
    if (nombre.contains("Thor", Qt::CaseInsensitive)) {
        return m_thor;
    }
    if (nombre.contains("Mau", Qt::CaseInsensitive)) {
        return m_mau;
    }
    if (nombre.contains("Yeng", Qt::CaseInsensitive)) {
        return m_yeng;
    }
    return m_snoopy;
}

GameWidget::AnimacionSprite GameWidget::animacionPara(const Personaje& personaje) const
{
    const bool cenital = m_nivel.nivelActual() == 2;
    if (&personaje == &m_nivel.jugador() && m_nivel.jugador().superActivo()) {
        if (m_nivel.jugador().superEsVeloz()) {
            if (cenital) {
                const Vector2D direccion = personaje.direccion();
                if (std::abs(direccion.x) >= std::abs(direccion.y)) {
                    return direccion.x < 0.0f ? AnimacionSprite::CenitalMoverIzquierda : AnimacionSprite::CenitalMoverDerecha;
                }
                return direccion.y < 0.0f ? AnimacionSprite::CenitalMoverArriba : AnimacionSprite::CenitalMoverAbajo;
            }
            return AnimacionSprite::LateralMover;
        }
        if (m_nivel.jugador().superEsAgil()) {
            return cenital ? AnimacionSprite::CenitalGolpe : AnimacionSprite::LateralSaltar;
        }
        return cenital ? AnimacionSprite::CenitalGolpe : AnimacionSprite::LateralGancho;
    }
    if (personaje.estado() == EstadoPersonaje::Daniado || personaje.estado() == EstadoPersonaje::Derrotado) {
        return cenital ? AnimacionSprite::CenitalDanio : AnimacionSprite::LateralDanio;
    }
    if (personaje.estado() == EstadoPersonaje::Saltando) {
        return AnimacionSprite::LateralSaltar;
    }
    if (personaje.estado() == EstadoPersonaje::Bloqueando) {
        return cenital ? AnimacionSprite::CenitalQuieto : AnimacionSprite::LateralBloquear;
    }
    if (personaje.estado() == EstadoPersonaje::Atacando) {
        if (personaje.ultimaAccion() == TipoAccion::Gancho) {
            return cenital ? AnimacionSprite::CenitalGolpe : AnimacionSprite::LateralGancho;
        }
        return cenital ? AnimacionSprite::CenitalGolpe : AnimacionSprite::LateralGolpe;
    }
    if (personaje.estado() == EstadoPersonaje::Caminando || personaje.estado() == EstadoPersonaje::Esquivando) {
        if (cenital) {
            const Vector2D direccion = personaje.direccion();
            if (std::abs(direccion.x) >= std::abs(direccion.y)) {
                return direccion.x < 0.0f ? AnimacionSprite::CenitalMoverIzquierda : AnimacionSprite::CenitalMoverDerecha;
            }
            return direccion.y < 0.0f ? AnimacionSprite::CenitalMoverArriba : AnimacionSprite::CenitalMoverAbajo;
        }
        return AnimacionSprite::LateralMover;
    }
    return cenital ? AnimacionSprite::CenitalQuieto : AnimacionSprite::LateralQuieto;
}

QRect GameWidget::frameActual(const SpriteAnimado& sprite, AnimacionSprite animacion) const
{
    const auto animator = sprite.animadores.constFind(animacion);
    if (animator != sprite.animadores.constEnd()) {
        return animator.value().sourceActual();
    }
    auto frames = sprite.frames.value(animacion);
    if (frames.empty()
        && (animacion == AnimacionSprite::CenitalMoverDerecha
            || animacion == AnimacionSprite::CenitalMoverIzquierda
            || animacion == AnimacionSprite::CenitalMoverAbajo
            || animacion == AnimacionSprite::CenitalMoverArriba)) {
        frames = sprite.frames.value(AnimacionSprite::CenitalMover);
    }
    if (frames.empty()) {
        return QRect(0, 0, sprite.hoja.width(), sprite.hoja.height());
    }
    return frames.front();
}

QPixmap GameWidget::frameAnimadoLimpio(const SpriteAnimado& sprite, AnimacionSprite animacion) const
{
    const auto animator = sprite.animadores.constFind(animacion);
    if (animator == sprite.animadores.constEnd()) {
        const QRect source = frameActual(sprite, animacion);
        if (sprite.hoja.isNull() || !source.isValid()) {
            return {};
        }
        return sprite.hoja.copy(source.x(), source.y(), source.width(), source.height());
    }
    return animator.value().getFrame();
}

QRect GameWidget::sourceParaPersonaje(const Personaje& personaje, const SpriteAnimado& sprite, AnimacionSprite animacion) const
{
    if (animacion == AnimacionSprite::LateralSaltar) {
        const auto frames = sprite.frames.value(animacion);
        if (!frames.empty()) {
            const int index = personaje.velocidadVertical() < -20.0f
                ? std::min(2, static_cast<int>(frames.size()) - 1)
                : personaje.velocidadVertical() > 35.0f
                    ? static_cast<int>(frames.size()) - 1
                    : std::min(3, static_cast<int>(frames.size()) - 1);
            return frames.at(index);
        }
    }
    return frameActual(sprite, animacion);
}

QPixmap GameWidget::frameParaPersonaje(const Personaje& personaje, const SpriteAnimado& sprite, AnimacionSprite animacion) const
{
    if (animacion == AnimacionSprite::LateralSaltar) {
        const QRect source = sourceParaPersonaje(personaje, sprite, animacion);
        if (sprite.hoja.isNull() || !source.isValid()) {
            return {};
        }
        return sprite.hoja.copy(source.x(), source.y(), source.width(), source.height());
    }
    return frameAnimadoLimpio(sprite, animacion);
}

QRect GameWidget::boundsContenido(const SpriteAnimado& sprite, const QRect& source) const
{
    if (sprite.hoja.isNull() || !source.isValid()) {
        return {};
    }
    const QString key = QString("%1:%2:%3:%4:%5")
        .arg(sprite.hoja.cacheKey())
        .arg(source.x())
        .arg(source.y())
        .arg(source.width())
        .arg(source.height());
    auto cached = m_cacheBounds.constFind(key);
    if (cached != m_cacheBounds.constEnd()) {
        return cached.value();
    }

    const QImage image = sprite.hoja.copy(source).toImage().convertToFormat(QImage::Format_ARGB32);
    int minX = image.width();
    int minY = image.height();
    int maxX = -1;
    int maxY = -1;
    for (int y = 0; y < image.height(); ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 24) {
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }
    QRect bounds = maxX >= minX && maxY >= minY
        ? QRect(QPoint(minX, minY), QPoint(maxX, maxY)).adjusted(-1, -1, 1, 1).intersected(QRect(QPoint(0, 0), image.size()))
        : QRect(QPoint(0, 0), image.size());
    m_cacheBounds.insert(key, bounds);
    return bounds;
}

QSizeF GameWidget::tamanoDibujo(const Personaje& personaje, AnimacionSprite animacion, const QSize& frameSize) const
{
    const bool cenital = m_nivel.nivelActual() == 2;
    if (cenital) {
        qreal base = 56.0;
        if (personaje.nombre().contains("Spider", Qt::CaseInsensitive)) {
            base = 98.0;
        } else if (personaje.nombre().contains("Iron", Qt::CaseInsensitive)) {
            base = 58.0;
        }
        const qreal ratio = frameSize.height() > 0 ? static_cast<qreal>(frameSize.width()) / frameSize.height() : 1.0;
        return QSizeF(base * std::clamp(ratio, 0.75, 1.18), base);
    }

    qreal alto = 96.0;
    if (animacion == AnimacionSprite::LateralSaltar) {
        alto = 108.0;
    } else if (animacion == AnimacionSprite::LateralGancho || animacion == AnimacionSprite::LateralGolpe) {
        alto = 100.0;
    } else if (animacion == AnimacionSprite::LateralBloquear) {
        alto = 98.0;
    }
    qreal ratio = frameSize.height() > 0 ? static_cast<qreal>(frameSize.width()) / frameSize.height() : 0.75;
    if (animacion == AnimacionSprite::LateralGancho || animacion == AnimacionSprite::LateralGolpe) {
        ratio = std::clamp(ratio, 0.48, 1.05);
    } else {
        ratio = std::clamp(ratio, 0.42, 0.88);
    }
    return QSizeF(alto * ratio, alto);
}

void GameWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.setRenderHint(QPainter::TextAntialiasing);
    if (m_nivel.nivelActual() == 1) {
        dibujarNivelLunar(painter);
    } else {
        dibujarNivelCenital(painter);
    }
    dibujarSuperEventos(painter);
    dibujarHud(painter);
    dibujarEfectoSuper(painter);
    dibujarResultado(painter);
}

void GameWidget::dibujarNivelLunar(QPainter& painter)
{
    dibujarFondo(painter, m_fondoNivel1);

    for (const Plataforma* plataforma : m_nivel.plataformas()) {
        const Vector2D pos = plataforma->obtenerPosicion();
        painter.setPen(QPen(QColor(230, 235, 244), 2));
        painter.setBrush(QColor(102, 156, 220, 210));
        painter.drawRoundedRect(QRectF(pos.x - 55, pos.y - 8, 110, 16), 5, 5);
    }
    dibujarPersonaje(painter, m_nivel.jugador(), spritesPara(m_nivel.jugador()), QColor(220, 58, 55));
    dibujarPersonaje(painter, m_nivel.enemigo(), spritesPara(m_nivel.enemigo()), QColor(52, 106, 220));
}

void GameWidget::dibujarNivelCenital(QPainter& painter)
{
    dibujarFondo(painter, m_fondoNivel2);

    for (const Objeto* objeto : m_nivel.objetos()) {
        dibujarObjeto(painter, *objeto);
    }
    dibujarPersonaje(painter, m_nivel.jugador(), spritesPara(m_nivel.jugador()), QColor(220, 58, 55));
    dibujarPersonaje(painter, m_nivel.enemigo(), spritesPara(m_nivel.enemigo()), QColor(52, 106, 220));
}

void GameWidget::dibujarFondo(QPainter& painter, const QPixmap& fondo)
{
    if (fondo.isNull()) {
        painter.fillRect(rect(), QColor(14, 17, 23));
        return;
    }
    const bool nivelUno = fondo.cacheKey() == m_fondoNivel1.cacheKey();
    QPixmap& cache = nivelUno ? m_fondoNivel1Cache : m_fondoNivel2Cache;
    QSize& tamanoCache = nivelUno ? m_tamanoFondoNivel1Cache : m_tamanoFondoNivel2Cache;

    if (cache.isNull() || tamanoCache != size()) {
        cache = QPixmap(size());
        cache.fill(Qt::transparent);
        QPainter cachePainter(&cache);
        cachePainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        cachePainter.setRenderHint(QPainter::Antialiasing, false);
        cachePainter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        const qreal escala = std::max(static_cast<qreal>(width()) / fondo.width(),
                                      static_cast<qreal>(height()) / fondo.height());
        const qreal sourceW = width() / escala;
        const qreal sourceH = height() / escala;
        const QRectF source((fondo.width() - sourceW) * 0.5,
                            (fondo.height() - sourceH) * 0.5,
                            sourceW,
                            sourceH);
        cachePainter.drawPixmap(rect(), fondo, source);
        tamanoCache = size();
    }
    painter.drawPixmap(0, 0, cache);
}

void GameWidget::dibujarPersonaje(QPainter& painter, const Personaje& personaje, const SpriteAnimado& sprite, QColor color)
{
    const Vector2D pos = personaje.posicion();
    const AnimacionSprite animacion = animacionPara(personaje);
    const QRect source = sourceParaPersonaje(personaje, sprite, animacion);
    const QPixmap frame = frameParaPersonaje(personaje, sprite, animacion);
    const bool cenital = m_nivel.nivelActual() == 2;
    const bool anclarSilueta = !cenital;
    const QRect contenido = anclarSilueta ? boundsContenido(sprite, source) : QRect(QPoint(0, 0), frame.size());
    const QSize frameSize = anclarSilueta && contenido.isValid()
        ? contenido.size()
        : (frame.isNull() ? source.size() : frame.size());
    const QSizeF visual = tamanoDibujo(personaje, animacion, frameSize);
    const float ancho = visual.width();
    const float alto = visual.height();
    const QRectF caja = cenital
        ? QRectF(pos.x - ancho * 0.5f, pos.y - alto * 0.54f, ancho, alto)
        : QRectF(pos.x - ancho * 0.5f, pos.y - alto + 10.0f, ancho, alto);
    const bool recibiendoGolpe = personaje.tiempoDanio() > 0.0f;
    const qreal retrocesoCabeza = recibiendoGolpe ? (personaje.mirandoDerecha() ? -3.0 : 3.0) : 0.0;
    const qreal giroGolpe = recibiendoGolpe ? (personaje.mirandoDerecha() ? -2.8 : 2.8) : 0.0;
    const QRectF cajaDibujo = caja;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 80));
    painter.drawEllipse(QPointF(pos.x, cenital ? pos.y + alto * 0.24f : pos.y + 4), ancho * 0.38f, cenital ? 8 : 10);
    if (cenital && personaje.velocidadActual() != personaje.velocidadBase()) {
        const bool boost = personaje.velocidadActual() > personaje.velocidadBase();
        painter.setBrush(boost ? QColor(72, 255, 120, 90) : QColor(255, 80, 80, 90));
        painter.setPen(QPen(boost ? QColor(134, 255, 170, 150) : QColor(255, 130, 130, 150), 2));
        painter.drawEllipse(QPointF(pos.x, pos.y - alto * 0.42f), ancho * 0.68f, alto * 0.62f);
    }
    if (cenital && personaje.estado() == EstadoPersonaje::Atacando) {
        const bool gancho = personaje.ultimaAccion() == TipoAccion::Gancho;
        const float radio = gancho ? 58.0f : 45.0f;
        QPointF centroAtaque(pos.x, pos.y);
        const Personaje& objetivo = (&personaje == &m_nivel.jugador()) ? static_cast<const Personaje&>(m_nivel.enemigo()) : static_cast<const Personaje&>(m_nivel.jugador());
        const Vector2D delta = objetivo.posicion() - personaje.posicion();
        const float distancia = delta.magnitud();
        if (distancia > 0.001f) {
            const float ayuda = (&personaje == &m_nivel.jugador()) ? (gancho ? 38.0f : 30.0f) : 18.0f;
            centroAtaque += QPointF(delta.x / distancia * ayuda, delta.y / distancia * ayuda);
        }
        painter.setPen(QPen(gancho ? QColor(255, 232, 120, 140) : QColor(255, 255, 255, 115), gancho ? 3 : 2));
        painter.setBrush(gancho ? QColor(255, 210, 70, 32) : QColor(255, 255, 255, 24));
        painter.drawEllipse(centroAtaque, radio, radio * 0.62f);
    }
    if (!frame.isNull()) {
        if (&personaje == &m_nivel.jugador() && m_nivel.jugador().superActivo() && m_nivel.jugador().superEsVeloz()) {
            int alpha = 42;
            for (auto it = m_nivel.jugador().estelaSuper().rbegin(); it != m_nivel.jugador().estelaSuper().rend(); ++it) {
                const QRectF ghost = cenital
                    ? QRectF(it->x - ancho * 0.5f, it->y - alto * 0.54f, ancho, alto)
                    : QRectF(it->x - ancho * 0.5f, it->y - alto + 10.0f, ancho, alto);
                painter.save();
                painter.setOpacity(alpha / 255.0);
                painter.drawPixmap(ghost.toRect(), frame);
                painter.restore();
                alpha = std::min(150, alpha + 18);
            }
        }
        const QPixmap frameDibujo = anclarSilueta && contenido.isValid() && contenido != QRect(QPoint(0, 0), frame.size())
            ? frame.copy(contenido)
            : frame;
        const qreal escalaY = anclarSilueta
            ? (contenido.height() > 0 ? cajaDibujo.height() / contenido.height() : 1.0)
            : (source.height() > 0 ? cajaDibujo.height() / source.height() : 1.0);
        QRectF destinoFrame = cajaDibujo;
        if (anclarSilueta && contenido.isValid()) {
            const qreal escalaUniforme = escalaY;
            destinoFrame = QRectF(cajaDibujo.center().x() - contenido.width() * escalaUniforme * 0.5,
                                  cajaDibujo.bottom() - contenido.height() * escalaUniforme,
                                  contenido.width() * escalaUniforme,
                                  contenido.height() * escalaUniforme);
        }
        painter.save();
        if (recibiendoGolpe) {
            const QPointF pivote(cajaDibujo.center().x(), cajaDibujo.top() + cajaDibujo.height() * 0.22);
            painter.translate(pivote);
            painter.rotate(giroGolpe);
            painter.translate(-pivote.x() + retrocesoCabeza, -pivote.y());
        }
        const bool direccionCenital = animacion == AnimacionSprite::CenitalMoverDerecha
            || animacion == AnimacionSprite::CenitalMoverIzquierda
            || animacion == AnimacionSprite::CenitalMoverAbajo
            || animacion == AnimacionSprite::CenitalMoverArriba;
        if (!personaje.mirandoDerecha() && !direccionCenital) {
            painter.translate(cajaDibujo.center().x(), 0);
            painter.scale(-1.0, 1.0);
            QRectF flipped(anclarSilueta ? -destinoFrame.width() * 0.5 : -cajaDibujo.width() * 0.5f,
                           destinoFrame.y(),
                           destinoFrame.width(),
                           destinoFrame.height());
            painter.drawPixmap(flipped.toRect(), frameDibujo);
        } else {
            painter.drawPixmap(destinoFrame.toRect(), frameDibujo);
        }
        painter.restore();
    } else {
        painter.setBrush(color);
        painter.drawEllipse(cajaDibujo);
    }
    if (&personaje == &m_nivel.jugador() && m_nivel.jugador().superActivo() && m_nivel.jugador().superEsFuerte()) {
        const int radio = 130 + (180 - m_nivel.jugador().timerSuper()) % 35;
        painter.setPen(QPen(QColor(255, 216, 82, 150), 4));
        painter.setBrush(QColor(255, 190, 60, 34));
        painter.drawEllipse(QPointF(pos.x, cenital ? pos.y : pos.y - alto * 0.45f), radio, radio * 0.45f);
    }
    if (cenital && personaje.velocidadActual() != personaje.velocidadBase()) {
        const bool boost = personaje.velocidadActual() > personaje.velocidadBase();
        painter.setFont(QFont("Segoe UI", 8, QFont::Black));
        painter.setPen(boost ? QColor(170, 255, 190) : QColor(255, 170, 150));
        painter.drawText(QRectF(pos.x - 38, cajaDibujo.top() - 16, 76, 14),
                         Qt::AlignCenter,
                         boost ? "BOOST" : "SLOW");
    }
    if (personaje.estado() == EstadoPersonaje::Bloqueando) {
        const float lado = personaje.mirandoDerecha() ? 1.0f : -1.0f;
        const QPointF centroEscudo(cajaDibujo.center().x() + lado * cajaDibujo.width() * 0.30f, cajaDibujo.center().y() + cajaDibujo.height() * 0.05f);
        QRadialGradient escudo(centroEscudo, cajaDibujo.height() * 0.55f);
        escudo.setColorAt(0.0, QColor(142, 226, 255, 120));
        escudo.setColorAt(0.55, QColor(76, 174, 255, 80));
        escudo.setColorAt(1.0, QColor(120, 210, 255, 0));
        painter.setPen(QPen(QColor(185, 240, 255), 3));
        painter.setBrush(escudo);
        painter.drawEllipse(centroEscudo, cajaDibujo.width() * 0.35f, cajaDibujo.height() * 0.46f);
        painter.setPen(QPen(QColor(255, 255, 255, 160), 2));
        painter.drawArc(QRectF(centroEscudo.x() - cajaDibujo.width() * 0.28f,
                               centroEscudo.y() - cajaDibujo.height() * 0.36f,
                               cajaDibujo.width() * 0.56f,
                               cajaDibujo.height() * 0.72f),
                        personaje.mirandoDerecha() ? 300 * 16 : 60 * 16,
                        120 * 16);
    }
}

void GameWidget::dibujarObjeto(QPainter& painter, const Objeto& objeto)
{
    const Vector2D pos = objeto.posicion();
    const AnimacionSprite animacion = objeto.tipo() == TipoObjeto::Tomate ? AnimacionSprite::ItemTomate : AnimacionSprite::ItemBebida;
    const SpriteAnimado& sprite = m_thor;
    const QRect source = frameActual(sprite, animacion);
    const QPixmap frame = frameAnimadoLimpio(sprite, animacion);
    const QSize frameSize = frame.isNull() ? source.size() : frame.size();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 75));
    painter.drawEllipse(QPointF(pos.x, pos.y), 24, 10);
    if (objeto.altura() <= 22.0f) {
        painter.setBrush(objeto.tipo() == TipoObjeto::Tomate ? QColor(255, 76, 76, 55) : QColor(92, 255, 132, 55));
        painter.drawEllipse(QPointF(pos.x, pos.y), 34, 18);
    }

    const float escala = objeto.tipo() == TipoObjeto::Tomate ? 0.62f : 0.56f;
    const float ancho = frameSize.width() * escala;
    const float alto = frameSize.height() * escala;
    const QRectF target(pos.x - ancho * 0.5f, pos.y - objeto.altura() * 0.45f - alto * 0.5f, ancho, alto);
    if (!frame.isNull()) {
        painter.drawPixmap(target.toRect(), frame);
    } else {
        painter.setBrush(objeto.tipo() == TipoObjeto::Tomate ? QColor(221, 43, 40) : QColor(54, 207, 114));
        painter.drawEllipse(target);
    }
}

void GameWidget::dibujarSuperEventos(QPainter& painter)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (const SuperEvento& evento : m_superEventos) {
        const float vida = evento.ticksTotales > 0 ? static_cast<float>(evento.ticksVida) / evento.ticksTotales : 0.0f;
        const QPointF p(evento.posicion.x, evento.posicion.y);
        if (evento.tipo == TipoSuperEvento::Guante) {
            painter.setPen(QPen(QColor(255, 240, 190, 220), 3));
            painter.setBrush(evento.delJugador ? QColor(255, 80, 70, 210) : QColor(70, 145, 255, 210));
            painter.drawEllipse(p, 22, 15);
            painter.setPen(QPen(QColor(60, 35, 25, 190), 3));
            painter.drawLine(p + QPointF(-10, -5), p + QPointF(12, 5));
        } else if (evento.tipo == TipoSuperEvento::Telarana) {
            painter.setPen(QPen(QColor(230, 245, 255, 225), 2));
            painter.setBrush(QColor(210, 235, 255, 80));
            painter.drawEllipse(p, 26, 18);
            painter.drawLine(p + QPointF(-24, 0), p + QPointF(24, 0));
            painter.drawLine(p + QPointF(0, -18), p + QPointF(0, 18));
            painter.drawLine(p + QPointF(-18, -12), p + QPointF(18, 12));
            painter.drawLine(p + QPointF(-18, 12), p + QPointF(18, -12));
        } else {
            const float alto = height() * 0.72f;
            const QColor color = evento.delJugador ? QColor(255, 226, 80, 220) : QColor(118, 196, 255, 220);
            painter.setPen(QPen(color, 5));
            painter.drawLine(QPointF(p.x() - 8, p.y() - alto), QPointF(p.x() + 4, p.y() + 18));
            painter.drawLine(QPointF(p.x() + 16, p.y() - alto * 0.72f), QPointF(p.x() - 4, p.y() + 20));
            painter.setBrush(QColor(color.red(), color.green(), color.blue(), static_cast<int>(70 * vida)));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(p, 58, 20);
        }
    }
    painter.restore();
}

void GameWidget::dibujarHud(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 132));
    painter.drawRect(0, 0, width(), 98);
    dibujarBarraVida(painter, QRect(24, 20, 300, 22), m_nivel.jugador(), m_spriteJugador, QColor(220, 58, 55));
    dibujarBarraCarga(painter);
    dibujarBarraVida(painter, QRect(width() - 324, 20, 300, 22), m_nivel.enemigo(), m_spriteEnemigo + " IA", QColor(52, 106, 220));

    const int tiempoRestante = std::max(0, DuracionCombateSegundos - static_cast<int>(m_nivel.tiempoNivel()));
    const QRect timerRect(width() / 2 - 74, 10, 148, 58);
    QLinearGradient timerGrad(timerRect.topLeft(), timerRect.bottomRight());
    timerGrad.setColorAt(0.0, QColor(44, 50, 66, 235));
    timerGrad.setColorAt(0.55, QColor(16, 19, 28, 240));
    timerGrad.setColorAt(1.0, QColor(64, 53, 29, 235));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(tiempoRestante <= 10 ? QColor(255, 92, 82) : QColor(255, 218, 91), 2));
    painter.setBrush(timerGrad);
    painter.drawRoundedRect(timerRect, 7, 7);
    painter.setFont(QFont("Segoe UI", 28, QFont::Black));
    painter.setPen(tiempoRestante <= 10 ? QColor(255, 112, 102) : QColor(255, 241, 170));
    painter.drawText(timerRect.adjusted(0, -4, 0, 0), Qt::AlignCenter, QString::number(tiempoRestante));
    painter.setFont(QFont("Segoe UI", 8, QFont::Black));
    painter.setPen(QColor(210, 220, 235));
    painter.drawText(timerRect.adjusted(0, 35, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "TIEMPO");
    painter.setRenderHint(QPainter::Antialiasing, false);

    painter.setFont(QFont("Segoe UI", 8, QFont::Black));
    painter.setPen(m_nivel.enemigo().superCarga() >= 100.0f ? QColor(255, 224, 90) : QColor(210, 220, 235));
    painter.drawText(QRect(width() - 324, 48, 300, 18),
                     Qt::AlignCenter,
                     m_nivel.enemigo().superCarga() >= 100.0f ? "IA SUPER MAX" : QString("IA SUPER %1%").arg(static_cast<int>(m_nivel.enemigo().superCarga())));
    painter.setPen(QColor(245, 245, 245));
    painter.setFont(QFont("Segoe UI", 10, QFont::DemiBold));
    painter.drawText(QRect(0, 72, width(), 22), Qt::AlignCenter,
                     QString("Nivel %1  |  A/D mover, W saltar/subir, S bajar en nivel 2, J golpe, K gancho, L cubrirse, F super")
                         .arg(m_nivel.nivelActual()));
    if (m_nivel.nivelActual() == 2) {
        painter.setFont(QFont("Segoe UI", 9, QFont::DemiBold));
        painter.setPen(QColor(220, 242, 230));
        painter.drawText(QRect(0, 92, width(), 18), Qt::AlignCenter, m_nivel.mensajeEstado());
    }
}

void GameWidget::dibujarBarraCarga(QPainter& painter)
{
    m_nivel.jugador().dibujarBarraCarga(painter, 24, 48, 300, 18);
}

void GameWidget::dibujarEfectoSuper(QPainter& painter)
{
    m_nivel.jugador().dibujarEfectoSuper(painter, rect());
}

void GameWidget::dibujarResultado(QPainter& painter)
{
    const QString resultado = m_nivel.resultadoFinal();
    if (resultado.isEmpty()) {
        return;
    }
    const bool empate = resultado == "EMPATE";
    const bool jugadorGana = resultado == "GANASTE";
    const Personaje& ganador = jugadorGana || empate
        ? static_cast<const Personaje&>(m_nivel.jugador())
        : static_cast<const Personaje&>(m_nivel.enemigo());
    const SpriteAnimado& spriteGanador = spritesPara(ganador);
    const AnimacionSprite poseResultado = m_nivel.nivelActual() == 2
        ? AnimacionSprite::CenitalQuieto
        : AnimacionSprite::LateralQuieto;
    std::vector<QRect> framesResultado = spriteGanador.frames.value(poseResultado);
    if (framesResultado.empty()) {
        framesResultado = spriteGanador.frames.value(AnimacionSprite::LateralQuieto);
    }
    const QRect sourceGanador = framesResultado.empty()
        ? QRect(0, 0, spriteGanador.hoja.width(), spriteGanador.hoja.height())
        : framesResultado.front();
    const QPixmap frameGanador = spriteGanador.hoja.copy(sourceGanador);
    const qreal apertura = std::clamp(m_tickResultado / 54.0, 0.0, 1.0);
    const qreal pulso = std::sin(m_tickResultado * 0.08) * 0.5 + 0.5;
    const QString nombreGanador = QString(ganador.nombre()).remove(" IA");

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 230));
    painter.drawRect(rect());

    QLinearGradient brillo(0, 0, width(), height());
    brillo.setColorAt(0.0, QColor(255, 59, 78, 58));
    brillo.setColorAt(0.48, QColor(255, 224, 92, 92));
    brillo.setColorAt(1.0, QColor(58, 152, 255, 58));
    painter.setBrush(brillo);
    painter.drawRect(rect());

    painter.setRenderHint(QPainter::Antialiasing, true);
    const int cortina = static_cast<int>((width() * 0.5) * (1.0 - apertura));
    QLinearGradient izquierda(0, 0, cortina, 0);
    izquierda.setColorAt(0.0, QColor(90, 8, 18, 245));
    izquierda.setColorAt(1.0, QColor(28, 8, 18, 245));
    QLinearGradient derecha(width() - cortina, 0, width(), 0);
    derecha.setColorAt(0.0, QColor(28, 8, 18, 245));
    derecha.setColorAt(1.0, QColor(90, 8, 18, 245));
    painter.setBrush(izquierda);
    painter.drawRect(0, 0, cortina, height());
    painter.setBrush(derecha);
    painter.drawRect(width() - cortina, 0, cortina, height());

    const QPointF centro(width() * 0.5, height() * 0.56);
    QRadialGradient halo(centro, std::max(width(), height()) * 0.42);
    halo.setColorAt(0.0, QColor(255, 230, 122, 150));
    halo.setColorAt(0.42, QColor(255, 120, 70, 72));
    halo.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.setBrush(halo);
    painter.drawEllipse(centro, width() * 0.34, height() * 0.42);

    if (!frameGanador.isNull()) {
        const qreal altoSprite = std::min<qreal>(height() * 0.46, 330.0) * (0.92 + pulso * 0.04);
        const qreal ratio = frameGanador.height() > 0
            ? static_cast<qreal>(frameGanador.width()) / frameGanador.height()
            : 0.75;
        const qreal anchoSprite = altoSprite * std::clamp(ratio, 0.45, 1.15);
        const QRectF destino(width() * 0.5 - anchoSprite * 0.5,
                             height() * 0.61 - altoSprite * 0.5,
                             anchoSprite,
                             altoSprite);
        painter.save();
        painter.setOpacity(0.25);
        painter.setBrush(QColor(0, 0, 0, 210));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(width() * 0.5, destino.bottom() - 4), anchoSprite * 0.34, 18);
        painter.setOpacity(1.0);
        if (!ganador.mirandoDerecha()) {
            painter.translate(destino.center().x(), 0);
            painter.scale(-1.0, 1.0);
            painter.drawPixmap(QRectF(-destino.width() * 0.5, destino.y(), destino.width(), destino.height()).toRect(), frameGanador);
        } else {
            painter.drawPixmap(destino.toRect(), frameGanador);
        }
        painter.restore();
    }

    painter.setFont(QFont("Segoe UI", 22, QFont::Black));
    painter.setPen(QColor(255, 239, 160));
    painter.drawText(QRect(0, 36, width(), 34), Qt::AlignCenter, empate ? "EMPATE" : jugadorGana ? "VICTORIA" : "GANADOR IA");
    painter.setFont(QFont("Segoe UI", 42, QFont::Black));
    painter.setPen(QColor(18, 18, 22, 190));
    painter.drawText(QRect(0, 78, width(), 64).translated(4, 4), Qt::AlignCenter, nombreGanador);
    painter.setPen(jugadorGana ? QColor(102, 255, 162) : QColor(128, 196, 255));
    painter.drawText(QRect(0, 78, width(), 64), Qt::AlignCenter, nombreGanador);
    painter.setFont(QFont("Segoe UI", 13, QFont::DemiBold));
    painter.setPen(QColor(245, 245, 245));
    painter.drawText(rect().adjusted(0, height() - 92, 0, 0), Qt::AlignHCenter | Qt::AlignTop, "R reiniciar nivel\nL volver al menu");
    painter.setRenderHint(QPainter::Antialiasing, false);
}

void GameWidget::dibujarBarraVida(QPainter& painter, QRect rect, const Personaje& personaje, const QString& nombreMostrado, QColor color)
{
    painter.setPen(QPen(QColor(245, 245, 245), 1));
    painter.setBrush(QColor(35, 35, 35));
    painter.drawRoundedRect(rect, 4, 4);
    const float fraccion = static_cast<float>(personaje.vida()) / static_cast<float>(personaje.vidaMaxima());
    QRect relleno = rect.adjusted(2, 2, -2, -2);
    relleno.setWidth(static_cast<int>(relleno.width() * fraccion));
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(relleno, 3, 3);
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(rect, Qt::AlignCenter, QString("%1  %2/%3").arg(nombreMostrado).arg(personaje.vida()).arg(personaje.vidaMaxima()));
}

void GameWidget::configurarSonidos()
{
    m_sonidoGolpe = new QMediaPlayer(this);
    m_sonidoSuper = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    m_audioGolpe = new QAudioOutput(this);
    m_audioSuper = new QAudioOutput(this);
    m_audioGolpe->setVolume(0.72);
    m_audioSuper->setVolume(0.86);
    m_sonidoGolpe->setAudioOutput(m_audioGolpe);
    m_sonidoSuper->setAudioOutput(m_audioSuper);
    m_sonidoGolpe->setSource(QUrl("qrc:/sonidos/golpe.mp3"));
    m_sonidoSuper->setSource(QUrl("qrc:/sonidos/super.mp3"));
#else
    m_sonidoGolpe->setMedia(QMediaContent(QUrl("qrc:/sonidos/golpe.mp3")));
    m_sonidoSuper->setMedia(QMediaContent(QUrl("qrc:/sonidos/super.mp3")));
    m_sonidoGolpe->setVolume(72);
    m_sonidoSuper->setVolume(86);
#endif
}

void GameWidget::reproducirGolpe()
{
    if (!m_sonidoGolpe) {
        return;
    }
    m_sonidoGolpe->stop();
    m_sonidoGolpe->setPosition(0);
    m_sonidoGolpe->play();
}

void GameWidget::reproducirSuper()
{
    if (!m_sonidoSuper) {
        return;
    }
    m_sonidoSuper->stop();
    m_sonidoSuper->setPosition(0);
    m_sonidoSuper->play();
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) {
        m_control.presionar(event->key());
    }
    if (!event->isAutoRepeat() && (event->key() == Qt::Key_J || event->key() == Qt::Key_K)) {
        reproducirGolpe();
    }
    if (!event->isAutoRepeat() && (event->key() == Qt::Key_F || event->key() == Qt::Key_Space)) {
        activarSuperPersonaje(true);
        return;
    }
    if (event->key() == Qt::Key_R) {
        reiniciar();
    }
    if (event->key() == Qt::Key_L && !m_nivel.resultadoFinal().isEmpty()) {
        emit volverMenuSolicitado();
    }
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (!event->isAutoRepeat()) {
        m_control.soltar(event->key());
    }
}
