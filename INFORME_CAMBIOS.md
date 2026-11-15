# Informe de cambios: medición de minerales y predictor de victoria

Este documento describe el comportamiento comprobado en `src/starterbot/`. La medición de minerales está implementada en `MineralTracker`; `StarterBot` la actualiza y dibuja cada frame, además de guardar datos al terminar.

## Medición de minerales

- `MineralTracker::onFrame` recorre `getMinerals()` en cada frame y conserva un registro por ID de unidad.
- Se registra una extracción cuando los recursos bajan **al menos 8** (`>= 8`) entre dos observaciones. La primera bajada detectada de cada campo solo se descarta como conteo inicial; las siguientes se añaden al historial.
- El historial conserva los últimos 20 conteos. El promedio CSV usa esos conteos y se escribe como `NA` si hay menos de 3. `n_conteos` informa cuántos conteos hay disponibles.
- Las etiquetas muestran el conteo anterior, el promedio disponible y el contador actual. Se dibujan en el mapa junto al mineral.
- `minerales.csv` se actualiza cada 1000 frames y otra vez en `StarterBot::onEnd`.
- En el frame 5000, una sola vez por partida, se escribe `minerales_f5000.csv`. Sus columnas, en orden, son `id,x,y,formacion,raza,promedio_frames,n_conteos,es_propio,lado_inicio,trabajadores_propios,raza_rival,frame`.
- `es_propio` vale 1 si el campo está a 320 píxeles o menos de la posición inicial del bot; en otro caso vale 0. La posición inicial proviene de `getStartLocation()`.
- La formación se clasifica con `mapWidth() * 16`, la mitad horizontal del mapa. En el mapa usado, ese punto medio equivale a 2048 píxeles.

Los CSV usan rutas relativas al directorio de trabajo del proceso. El lanzador Linux cambia primero a `bin_linux/`, por lo que allí quedan las salidas al ejecutarlo mediante `bin_linux/RunStarterBotAndStarcraft.sh`.

## Predictor de victoria

`WinPredictor::calculate` calcula una probabilidad relativa para cada jugador no neutral cada 24 frames. Para cada jugador, suma seis variables normalizadas respecto de los totales de todos los jugadores:

| Variable | Cálculo | Peso |
|---|---|---:|
| Ejército | Costo mineral + 1.5 veces el costo de gas de unidades que no son edificios ni trabajadores | 0.35 |
| Recursos | Minerales + gas disponibles | 0.20 |
| Edificios | Cantidad de edificios | 0.15 |
| Tecnología | Niveles de mejoras + tecnologías investigadas | 0.10 |
| Control del mapa | Centros de recursos (`isResourceDepot`) | 0.10 |
| Bajas | Costo de unidades enemigas destruidas, acreditado al bot propio | 0.10 |

Los puntajes ponderados se vuelven a normalizar para que las probabilidades sumen 1; si el total es cero, se asigna la misma probabilidad a cada jugador. El panel dibuja una barra y porcentaje por jugador. `predicciones.csv` guarda las muestras y `ganador_id`. BWAPI solo indica al callback de fin si el bot ganó: en una partida de dos jugadores, al perder el código registra al único rival como ganador; con varios rivales deja `ganador_id` en `-1`. La variable de bajas es una aproximación: el callback identifica la unidad destruida, no quién la destruyó, y el código acredita las unidades enemigas al bot.

## Análisis de las partidas

`analyse_v2/analisis_final.py` combina los datos del frame 5000, minerales y predicciones. Con los archivos presentes analiza 18 partidas (tres por raza y lado de inicio) y genera tablas top 4 para las seis combinaciones de raza y lado, junto con resúmenes, alertas y gráficos en `analyse_v2/resultados/`. La interpretación y el detalle de los archivos están en `analyse_v2/LEEME.md`.

## Requisitos del proyecto

La implementación del contador y la medición adicional se cotejan con el enunciado en `CUMPLIMIENTO_ENUNCIADO.md`. El bot también conserva las acciones de recolección, producción de trabajadores y suministro del STARTcraft base; no implementa lógica de combate.
