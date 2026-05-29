#pragma once

enum class EstadoPersonaje {
    Quieto,
    Caminando,
    Saltando,
    Atacando,
    Bloqueando,
    Esquivando,
    Daniado,
    Derrotado
};

enum class TipoAccion {
    Ninguna,
    Mover,
    Saltar,
    GolpeFrontal,
    Gancho,
    Bloquear,
    Esquivar,
    RecogerObjeto
};

enum class TipoObjeto {
    Tomate,
    BebidaEnergetica
};

enum class Dificultad {
    Entrenamiento,
    Normal,
    Vibranium
};
