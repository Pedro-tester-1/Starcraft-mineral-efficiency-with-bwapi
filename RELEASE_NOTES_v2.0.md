# StarCraft Mineral Efficiency v2.0

## Novedades

- Nuevos comandos `frame`/`/frame` para alternar el contador FPS y `prediccion` para mostrar u ocultar el panel del predictor.
- `velocidad` establece la velocidad local de BWAPI en 42; `velocidad_`, `/velocidad_`, `velocidad-` o `velocidad 0` la restablecen a 0.
- Los comandos ignoran mayúsculas y espacios al principio o al final del mensaje.
- El análisis y sus datos pasan a `analyse/`; se conservan las 18 partidas y los resultados top 4, resúmenes y gráficos.

## Reproducir

Desde la raíz del repositorio, ejecuta `make`, luego `bin_linux/RunStarterBotAndStarcraft.sh`. En el chat del juego puedes usar `hitbox`, `etiquetas`, `ordenes`, `/map`, `frame`, `/frame`, `prediccion` y los comandos de velocidad descritos arriba. Para regenerar el análisis, ejecuta `python3 analyse/analisis_final.py`.

## Limitaciones conocidas

El conjunto analizado incluye tres partidas por raza y lado de inicio. La estimación de bajas del predictor acredita al bot las unidades enemigas destruidas porque el callback no identifica quién las destruyó. Si el bot pierde, el ganador se identifica como el único rival solo en partidas de dos jugadores; con más rivales queda como `-1`.
