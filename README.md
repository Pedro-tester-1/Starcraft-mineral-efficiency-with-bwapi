# StarCraft Mineral Efficiency (BWAPI)

Proyecto final de Programación II (UTEC, 2026-2). Bot en C++ sobre
[STARTcraft](https://github.com/davechurchill/STARTcraft) de Dave Churchill que:

1. **Mide la eficiencia de cada campo de minerales:** frames entre extracciones,
   último conteo y promedio de los últimos 20, dibujados sobre cada mineral.
2. **Predice la probabilidad de victoria** de cada jugador en tiempo real a partir de
   ejército, recursos, expansiones, tecnología, control del mapa y bajas.

## Requisitos
- StarCraft: Brood War 1.16 + BWAPI 4.4.0 (zip `scbw_bwapi440`, no incluido en este repo)
- Windows: Visual Studio 2026 (toolsets v143/v142/v141, SDK Windows 11), compilar en Release/x86
- Linux: Wine + mingw-w64 (`make`)

## Cómo ejecutarlo
1. Extrae el zip de StarCraft dentro de `starcraft/`.
2. Compila el proyecto y ejecuta el bot.
3. Lanza `RunStarcraftWithBWAPI.bat` (o `RunStarterBotAndStarcraft.sh` en Linux).
4. Single Player > Expansion > Play Custom > elige el mapa en modo Melee.

## Archivos principales
- `src/MineralTracker.*`: contador de frames por mineral
- `src/WinPredictor.*`: predictor de victoria
- `INFORME_CAMBIOS.md`: detalle de los cambios

## Créditos
Basado en STARTcraft de Dave Churchill y en BWAPI.
