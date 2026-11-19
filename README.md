# StarCraft Mineral Efficiency (BWAPI)

Proyecto final de Programación II (UTEC, 2026-2). Bot en C++ sobre
[STARTcraft](https://github.com/davechurchill/STARTcraft) de Dave Churchill que:

Versión actual: 2.0 (ver `RELEASE_NOTES_v2.0.md`)

1. **Mide la eficiencia de cada campo de minerales:** frames entre extracciones,
   último conteo y promedio de los últimos 20, dibujados sobre cada mineral.
2. **Predice la probabilidad de victoria** de cada jugador en tiempo real a partir de
   ejército, recursos, expansiones, tecnología, control del mapa y bajas.

## Requisitos
- StarCraft: Brood War 1.16 + BWAPI 4.4.0 (zip `scbw_bwapi440`, no incluido en este repo)
- Windows: Visual Studio 2026 (toolsets v143/v142/v141, SDK Windows 11), compilar en Release/x86
- Linux: Wine + mingw-w64 (`make`)

## Medición de minerales

En Linux, compila desde la raíz con `make` y ejecuta `bin_linux/RunStarterBotAndStarcraft.sh`. El lanzador trabaja desde `bin_linux/`; allí se generan `minerales.csv` (cada 1000 frames y al finalizar), `minerales_f5000.csv` (una vez en el frame 5000) y `predicciones.csv` (al finalizar). Los datos y resultados de las partidas analizadas están en `analyse/`.

Los comandos de chat no distinguen mayúsculas ni espacios al inicio o al final. `hitbox` alterna las cajas de unidades, `etiquetas` las etiquetas de minerales, `ordenes` las líneas de órdenes, `/map` el dibujo de información del mapa, `frame` o `/frame` los FPS, y `prediccion` el panel del predictor. `velocidad` cambia la velocidad local a 42; `velocidad_`, `/velocidad_`, `velocidad-` o `velocidad 0` la devuelven a 0.

Para regenerar las tablas y gráficos del análisis actual, ejecuta desde la raíz `python3 analyse/analisis_final.py`.

## Cómo ejecutarlo

1. Extrae el zip de StarCraft dentro de `starcraft/`.
2. Compila con `make` y lanza `bin_linux/RunStarterBotAndStarcraft.sh` (o el lanzador de Windows).
3. En el juego, abre Single Player > Expansion > Play Custom y selecciona el mapa en modo Melee.

## Comandos en el juego
- Escribe `hitbox` en el chat para alternar los recuadros de las unidades. El estado inicial es encendido y el bot muestra `Hitbox: ON` o `Hitbox: OFF`.
- Escribe `/map` para alternar el dibujo de información del mapa.
- Escribe `etiquetas`, `ordenes`, `prediccion` o `frame` para alternar la visualización correspondiente; `frame` también acepta `/frame`.
- Escribe `velocidad` para usar velocidad local 42. Para volver a velocidad 0, escribe `velocidad_`, `/velocidad_`, `velocidad-` o `velocidad 0`.

## Archivos principales
- `src/starterbot/MineralTracker.*`: contador de frames por mineral
- `src/starterbot/WinPredictor.*`: predictor de victoria
- `INFORME_CAMBIOS.md`: detalle del comportamiento actual
- `CHANGELOG.md`: historial de cambios
- `RELEASE_NOTES_v2.0.md`: resumen de esta versión

## Créditos
Basado en STARTcraft de Dave Churchill y en BWAPI.
