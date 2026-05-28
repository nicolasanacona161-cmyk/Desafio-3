# Marvel Boxing - Momento III

Implementacion en C++ y Qt del videojuego propuesto en el Momento II: boxeo en el universo Marvel con dos niveles, fisicas parametrizadas, POO, memoria dinamica, GUI y agente inteligente.

## Requisitos

- Qt 5 o Qt 6 con los modulos Widgets y Multimedia.
- CMake 3.16 o superior.
- Compilador C++17.

## Compilacion

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\MarvelBoxing.exe
```

En generadores de un solo perfil, el ejecutable queda directamente en `build`.

Tambien se incluye `MarvelBoxing.pro` para abrir o compilar el proyecto con Qt Creator/qmake:

```powershell
qmake MarvelBoxing.pro
nmake
# o mingw32-make, segun el kit de Qt instalado
```

En este equipo ya se detecto Qt 6.10.2 en `C:\Qt`. Para recompilar con las mismas rutas usadas durante la verificacion:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_qt.ps1
```

## Controles

- `A` / `D`: mover izquierda y derecha.
- `W`: saltar en el nivel lunar; subir en la arena cenital.
- `S`: bloquear en el nivel lunar; bajar en la arena cenital.
- `J`: golpe frontal.
- `K`: gancho.
- `L`: cubrirse. Reduce mucho el daño recibido.
- `R`: reiniciar combate.
- `N`: cambiar de nivel.

La barra superior permite escoger la hoja de sprites del jugador y del enemigo entre Iron Man, Spider-Man, Thor y Snoopy.

## Elementos implementados del Momento III

- Capa logica basada en el diagrama de clases: `Personaje`, `Jugador`, `Enemigo`, `GestorNivel`, `Fisica`, `SistemaCombate`, `Objeto`, `Efecto`, `Plataforma`, `AgenteInteligente`, `Percepcion`, `Razonador`, `MemoriaIA` y `DecisionIA`.
- Herencia propia: `Jugador` y `Enemigo` heredan de `Personaje`.
- Memoria dinamica: `GestorNivel` administra `vector<Personaje*>`, `vector<Objeto*>` y `vector<Plataforma*>`; `Enemigo` compone dinamicamente su `AgenteInteligente`.
- Fisicas: gravedad lunar con flotacion, oscilacion senoidal de plataformas y caida vertical de objetos con sombra previa.
- Dificultad fija balanceada: la IA es activa y dificil de vencer, pero mantiene margen de reaccion para el jugador.
- Agente inteligente PREA: percibe jugador/objetos/vida/distancia, razona reglas, actua y aprende patrones con `deque` y `map`.
- Excepciones: validacion de parametros fisicos, daño, efectos y carga de niveles.
- Sonido: queda desactivado por ahora para integrar despues los audios finales del equipo.
- Sprites: cada hoja se corta por frames y se anima segun movimiento, salto, defensa, golpe, gancho, daño y vista cenital.
- Separacion por niveles: la logica especifica del combate lunar esta en `src/model/niveles/NivelUnoLunar.*` y la arena cenital en `src/model/niveles/NivelDosCenital.*`; `GestorNivel` solo coordina recursos comunes.

## Entregables pendientes externos

La guia tambien exige trailer, video de sustentacion, informe final en formato del curso y ejecutable versionado. Este repositorio deja lista la base compilable para generar el ejecutable y grabar el material.
