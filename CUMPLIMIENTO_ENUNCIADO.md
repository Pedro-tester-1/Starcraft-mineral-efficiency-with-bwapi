# Estado de cumplimiento del enunciado

Revisión del código de `src/starterbot/`, de `analyse_v2/` y de los datos y resultados presentes en el repositorio. Esto verifica la implementación y los artefactos disponibles; no acredita una exposición ni un informe académico entregado.

## Pregunta 1: eficiencia de campos minerales

| Requisito | Estado comprobado |
|---|---|
| Monitorear cada campo en cada frame | Cumplido en `MineralTracker::onFrame`, llamado desde `StarterBot::onFrame`; recorre el conjunto completo `getMinerals()` y registra por ID. |
| Detectar una extracción comparando recursos | Cumplido: compara con la observación anterior y detecta una bajada de al menos 8 (`>= 8`). |
| Contador por campo y reinicio al detectar extracción | Cumplido: `currentFrames` se reinicia ante la bajada. El primer conteo detectado de cada campo se descarta; los posteriores se registran. |
| Mostrar último conteo, promedio y contador actual | Cumplido: `draw()` usa `drawTextMap` y presenta último conteo, promedio y contador actual junto a cada campo activo. El promedio dibujado usa los datos disponibles. |
| Promedio de hasta los últimos 20 conteos | Cumplido: el historial elimina el dato más antiguo al superar 20. El CSV muestra `NA` cuando hay menos de 3 conteos. |
| Medir minerales más allá de los nueve iniciales | Cumplido: el registro recorre todos los campos que expone `getMinerals()`, no fija el muestreo a los campos propios. |
| Comparar top 4 por raza y lado | Hay resultados presentes en `analyse_v2/resultados/tablas_top4.md` para las seis combinaciones (tres razas por dos lados), calculados con `analyse_v2/analisis_final.py` a partir de archivos de 18 partidas. |
| Explicar razas y recolección en el informe académico | No verificable en el código ni en los artefactos de análisis de este repositorio. |

## Pregunta 2: medir otra variable del juego

| Requisito | Estado comprobado |
|---|---|
| Medir una variable adicional y mostrarla | Cumplido como extensión: `WinPredictor` estima y dibuja probabilidades relativas de victoria por jugador, basadas en ejército, recursos, edificios, tecnología, centros de recursos y bajas. |
| Guardar muestras y resultado | Cumplido: se guardan muestras cada 24 frames y `predicciones.csv` se escribe al finalizar. Cuando el bot pierde, el rival se registra como ganador solo si hay exactamente un rival; de otro modo el ID queda en `-1`. |

La variable de bajas tiene una limitación comprobada en `WinPredictor::onUnitDestroyed`: BWAPI informa qué unidad murió, pero no quién la destruyó; el código acredita al propio bot el valor de unidades enemigas destruidas. El bot tampoco tiene lógica de combate, así que esta variable puede permanecer en cero durante una partida.

## Límites de esta revisión

- La evaluación del proyecto también considera claridad, eficiencia y profundidad del análisis; este documento no sustituye una evaluación académica.
- No se afirma que el enunciado escrito del informe (antecedentes, fundamento teórico, métodos, resultados y conclusiones) esté completo: esos contenidos no son parte de las comprobaciones de código aquí descritas.
- La clasificación de campos propios usa una distancia de 320 píxeles a la posición inicial. La clasificación izquierda/derecha usa `mapWidth() * 16` (2048 píxeles en el mapa de 4096 píxeles de ancho usado por el análisis).
