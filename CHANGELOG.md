# Historial de versiones

## v2.0

### Novedades

- Comandos de chat normalizados: aceptan mayúsculas y espacios al inicio/final.
- Se puede alternar la visualización de FPS (`frame` o `/frame`) y el panel de predicción (`prediccion`), además de hitboxes, etiquetas, órdenes y mapa.
- Se puede cambiar la velocidad local a 42 con `velocidad` y restablecerla a 0 con `velocidad_`, `/velocidad_`, `velocidad-` o `velocidad 0`.
- El directorio de análisis pasa de `analyse_v2/` a `analyse/`; se conservan los datos y resultados del análisis.

### Reproducción

Compila con `make`, ejecuta `bin_linux/RunStarterBotAndStarcraft.sh` y regenera el análisis desde la raíz con `python3 analyse/analisis_final.py`.

## v1.2

Esta versión añade medición de minerales en el frame 5000 y prepara el análisis de 18 partidas.

### Novedades

- Exportación única de `minerales_f5000.csv` con contexto de partida y las columnas `es_propio`, `lado_inicio`, `trabajadores_propios`, `raza_rival` y `frame`.
- Clasificación de campos propios por distancia de hasta 320 píxeles a la posición inicial del bot.
- Descarte del primer conteo de cada campo; el promedio se calcula con hasta los últimos 20 conteos y queda como `NA` con menos de tres.
- Datos de 18 partidas (tres por cada combinación de raza y lado) y análisis final reproducible con seis tablas top 4, resúmenes y mapas en `analyse_v2/resultados/`.

### Limitaciones conocidas

- Los resultados contienen tres partidas por combinación; no describen todos los mapas ni todas las configuraciones de juego.
- `es_propio` depende del radio de 320 píxeles y de la posición inicial detectada. La formación izquierda/derecha usa la mitad horizontal del mapa.
- Si el bot pierde, `ganador_id` identifica al rival solo cuando hay exactamente un rival. Con varios rivales queda en `-1`.
- El valor de bajas se acredita al bot cuando muere una unidad enemiga, porque el callback usado no identifica al responsable. El bot no implementa combate.

### Cómo reproducir

Desde la raíz, compila con `make` y ejecuta `bin_linux/RunStarterBotAndStarcraft.sh`. Para generar los resultados del conjunto de datos incluido, ejecuta `python3 analyse_v2/analisis_final.py`. La guía de carpetas y resultados está en `analyse_v2/LEEME.md`.

## v1.1

- Se configuró la partida para ejecutarse a velocidad máxima de BWAPI (`setLocalSpeed(0)`), imitando el ajuste de velocidad usado por AIIDE. Esto no implica cumplir todas las reglas del torneo: `CompleteMapInformation` sigue activado.
- Se documentó y añadió el comando de chat `hitbox`, que alterna los recuadros de unidades y confirma si están encendidos o apagados.
- Se mejoró la presentación de los datos de minerales con etiquetas compactas, búsqueda de posiciones para reducir solapamientos y colores por eficiencia (verde hasta 160 frames, amarillo hasta 200, rojo por encima; blanco si aún no hay promedio). Una extracción se registra al detectar una bajada de al menos 8 recursos.
- Se reorganizó el panel del predictor con fondo, título y barras del color de cada jugador. El color identifica al jugador; la longitud de la barra y el porcentaje representan la probabilidad calculada, no la calidad de la predicción.
- Se añadió `CUMPLIMIENTO_ENUNCIADO.md`, una tabla que contrasta los requisitos del proyecto con su evidencia y pendientes.
- Se actualizó `README.md` con la versión y el comando `hitbox`; se amplió `INFORME_CAMBIOS.md` con la velocidad, la interpretación de los colores y los cambios visuales.
- Para la instalación local, se dejó el mapa del profesor como único mapa dentro de `starcraft/maps/BroodWar/` y se trasladaron los otros mapas a `../mapas_respaldo/`. La carpeta `starcraft/` está ignorada por Git, por lo que los mapas y archivos del juego no forman parte de esta versión publicada.

## v1.0

- Versión inicial del proyecto: clase `MineralTracker` que mide por campo los frames entre extracciones, mantiene un historial de hasta 20 observaciones y exporta `minerales.csv` con ID, posición, formación, raza y promedio.
- Clase `WinPredictor` que estima y dibuja probabilidades de victoria a partir de ejército, recursos, edificios/expansiones, tecnología, control del mapa y bajas; exporta las muestras y el resultado en `predicciones.csv`.
- Se añadió `INFORME_CAMBIOS.md` con la explicación de la implementación, decisiones, CSV, compilación y limitaciones conocidas.
