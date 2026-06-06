#pragma once

#include "Control.h"
#include "GestorNivel.h"

#include <QElapsedTimer>
#include <QHash>
#include <QImage>
#include <QMap>
#include <QPixmap>
#include <QRect>
#include <QTimer>
#include <QWidget>
#include <vector>

class QAudioOutput;
class QMediaPlayer;

class GameWidget : public QWidget {
    Q_OBJECT

public:
    explicit GameWidget(QWidget* parent = nullptr);
    void setDificultad(Dificultad dificultad);
    void setSpriteJugador(const QString& sprite);
    void setSpriteEnemigo(const QString& sprite);
    void iniciarPartida(int nivel, const QString& spriteJugador, const QString& spriteEnemigo);
    void reiniciar();
    void cambiarNivel();

signals:
    void estadoCambiado(const QString& mensaje);
    void volverMenuSolicitado();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void tick();

private:
    enum class AnimacionSprite {
        LateralQuieto,
        LateralMover,
        LateralSaltar,
        LateralBloquear,
        LateralGolpe,
        LateralGancho,
        LateralDanio,
        CenitalQuieto,
        CenitalMover,
        CenitalMoverDerecha,
        CenitalMoverIzquierda,
        CenitalMoverAbajo,
        CenitalMoverArriba,
        CenitalGolpe,
        CenitalDanio,
        ItemTomate,
        ItemBebida
    };

    class SpriteAnimator {
    public:
        QPixmap spriteSheet;
        int anchoFrame = 0;
        int altoFrame = 0;
        int cantidadFrames = 0;
        int frameActual = 0;
        int ticksPorFrame = 6;

        void configurar(const QPixmap& hoja, const std::vector<QRect>& frames, int ticks);
        void tick();
        void reset();
        QPixmap getFrame() const;
        QRect sourceActual() const;

    private:
        std::vector<QRect> m_frames;
        int m_tickActual = 0;
    };

    struct SpriteAnimado {
        QPixmap hoja;
        QMap<AnimacionSprite, std::vector<QRect>> frames;
        QMap<AnimacionSprite, SpriteAnimator> animadores;
    };

    enum class TipoSuperEvento {
        Guante,
        Rayo,
        Telarana
    };

    struct SuperEvento {
        TipoSuperEvento tipo = TipoSuperEvento::Guante;
        Vector2D posicion;
        Vector2D velocidad;
        bool delJugador = true;
        int ticksVida = 0;
        int ticksTotales = 0;
        bool impactoAplicado = false;
    };

    SpriteAnimado crearSpritesIronMan() const;
    SpriteAnimado crearSpritesSpiderman() const;
    SpriteAnimado crearSpritesSpidermanNivel2() const;
    SpriteAnimado crearSpritesThor() const;
    SpriteAnimado crearSpritesSnoopy() const;
    SpriteAnimado crearSpritesMau() const;
    SpriteAnimado crearSpritesYeng() const;
    SpriteAnimado crearSpritesBoxerGenerico(const QString& ruta, QSizeF tamanoBase) const;
    QPixmap cargarSpriteSheet(const QString& ruta) const;
    void escalarFrames(SpriteAnimado& sprite, QSizeF tamanoBase) const;
    void prepararAnimadores(SpriteAnimado& sprite) const;
    void tickAnimadores();
    void reiniciarAnimadores();
    void reiniciarAnimacionActual(const Personaje& personaje);
    void actualizarAnimacionesActuales();
    void actualizarSuperEventos(float dt);
    void activarSuperPersonaje(bool delJugador);
    void crearSuperEventos(bool delJugador);
    void dibujarSuperEventos(QPainter& painter);
    bool nombreTiene(const Personaje& personaje, const QString& token) const;
    const SpriteAnimado& spritesPara(const Personaje& personaje) const;
    const SpriteAnimado& spritesPorNombre(const QString& nombre) const;
    AnimacionSprite animacionPara(const Personaje& personaje) const;
    QRect frameActual(const SpriteAnimado& sprite, AnimacionSprite animacion) const;
    QPixmap frameAnimadoLimpio(const SpriteAnimado& sprite, AnimacionSprite animacion) const;
    QPixmap frameParaPersonaje(const Personaje& personaje, const SpriteAnimado& sprite, AnimacionSprite animacion) const;
    QRect sourceParaPersonaje(const Personaje& personaje, const SpriteAnimado& sprite, AnimacionSprite animacion) const;
    QRect boundsContenido(const SpriteAnimado& sprite, const QRect& source) const;
    QSizeF tamanoDibujo(const Personaje& personaje, AnimacionSprite animacion, const QSize& frameSize) const;
    void dibujarNivelLunar(QPainter& painter);
    void dibujarNivelCenital(QPainter& painter);
    void dibujarFondo(QPainter& painter, const QPixmap& fondo);
    void dibujarPersonaje(QPainter& painter, const Personaje& personaje, const SpriteAnimado& sprite, QColor color);
    void dibujarObjeto(QPainter& painter, const Objeto& objeto);
    void dibujarHud(QPainter& painter);
    void dibujarBarraCarga(QPainter& painter);
    void dibujarEfectoSuper(QPainter& painter);
    void dibujarResultado(QPainter& painter);
    void dibujarBarraVida(QPainter& painter, QRect rect, const Personaje& personaje, const QString& nombreMostrado, QColor color);
    void configurarSonidos();
    void reproducirGolpe();
    void reproducirSuper();

    GestorNivel m_nivel;
    Control m_control;
    QTimer m_timer;
    QElapsedTimer m_reloj;
    SpriteAnimado m_ironMan;
    SpriteAnimado m_spiderman;
    SpriteAnimado m_spidermanNivel2;
    SpriteAnimado m_thor;
    SpriteAnimado m_snoopy;
    SpriteAnimado m_mau;
    SpriteAnimado m_yeng;
    QPixmap m_fondoNivel1;
    QPixmap m_fondoNivel2;
    QPixmap m_fondoNivel1Cache;
    QPixmap m_fondoNivel2Cache;
    QSize m_tamanoFondoNivel1Cache;
    QSize m_tamanoFondoNivel2Cache;
    QString m_spriteJugador = "Iron Man";
    QString m_spriteEnemigo = "Spider-Man";
    QString m_ultimoMensaje = "Listo";
    mutable QHash<QString, QRect> m_cacheBounds;
    int m_vidaJugadorAnterior = 300;
    int m_vidaEnemigoAnterior = 300;
    int m_tickLogica = 0;
    int m_tickResultado = 0;
    bool m_resultadoActivo = false;
    AnimacionSprite m_animacionJugadorAnterior = AnimacionSprite::LateralQuieto;
    AnimacionSprite m_animacionEnemigoAnterior = AnimacionSprite::LateralQuieto;
    std::vector<SuperEvento> m_superEventos;
    QMediaPlayer* m_sonidoGolpe = nullptr;
    QMediaPlayer* m_sonidoSuper = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QAudioOutput* m_audioGolpe = nullptr;
    QAudioOutput* m_audioSuper = nullptr;
#endif
};
