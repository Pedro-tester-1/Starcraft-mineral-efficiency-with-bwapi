# Historial de versiones

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
