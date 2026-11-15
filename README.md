# StarCraft Mineral Efficiency (BWAPI)

Proyecto final de Programación II (UTEC, 2026-2). Bot en C++ sobre
[STARTcraft](https://github.com/davechurchill/STARTcraft) de Dave Churchill que:

Versión actual: 1.2 (ver `RELEASE_NOTES_v1.2.md`)

1. **Mide la eficiencia de cada campo de minerales:** frames entre extracciones,
   último conteo y promedio de los últimos 20, dibujados sobre cada mineral.
2. **Predice la probabilidad de victoria** de cada jugador en tiempo real a partir de
   ejército, recursos, expansiones, tecnología, control del mapa y bajas.

## Requisitos
- StarCraft: Brood War 1.16 + BWAPI 4.4.0 (zip `scbw_bwapi440`, no incluido en este repo)
- Windows: Visual Studio 2026 (toolsets v143/v142/v141, SDK Windows 11), compilar en Release/x86
- Linux: Wine + mingw-w64 (`make`)

## Medición de minerales

En Linux, compila desde la raíz con `make` y ejecuta `bin_linux/RunStarterBotAndStarcraft.sh`. El lanzador trabaja desde `bin_linux/`; allí se generan `minerales.csv` (cada 1000 frames y al finalizar), `minerales_f5000.csv` (una vez en el frame 5000) y `predicciones.csv` (al finalizar). Los archivos de las partidas analizadas y sus resultados están en `analyse_v2/`.

En el chat del juego, `hitbox` alterna las cajas de unidades, `etiquetas` alterna las etiquetas de minerales, `ordenes` alterna las líneas de órdenes y `/map` alterna el dibujo de información del mapa. Estos comandos están implementados en `StarterBot::onSendText`.

Para regenerar las tablas y gráficos del análisis actual, ejecuta desde la raíz `python3 analyse_v2/analisis_final.py`.

## Cómo ejecutarlo

1. Extrae el zip de StarCraft dentro de `starcraft/`.
2. Compila con `make` y lanza `bin_linux/RunStarterBotAndStarcraft.sh` (o el lanzador de Windows).
3. En el juego, abre Single Player > Expansion > Play Custom y selecciona el mapa en modo Melee.

## Comandos en el juego
- Escribe `hitbox` en el chat para alternar los recuadros de las unidades. El estado inicial es encendido y el bot muestra `Hitbox: ON` o `Hitbox: OFF`.
- Escribe `/map` para alternar el dibujo de información del mapa.

## Archivos principales
- `src/MineralTracker.*`: contador de frames por mineral
- `src/WinPredictor.*`: predictor de victoria
- `INFORME_CAMBIOS.md`: detalle del comportamiento actual
- `CUMPLIMIENTO_ENUNCIADO.md`: revisión de las Preguntas 1 y 2

## Créditos
Basado en STARTcraft de Dave Churchill y en BWAPI.
