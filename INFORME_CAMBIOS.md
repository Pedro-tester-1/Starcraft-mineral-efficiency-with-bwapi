# Informe de cambios: medición de minerales y predictor de victoria

## Archivos creados y modificados

- `src/starterbot/MineralTracker.h` (nuevo): declara la clase que mantiene el estado de cada campo mineral.
- `src/starterbot/MineralTracker.cpp` (nuevo): actualiza los contadores, dibuja sus valores y exporta `minerales.csv`.
- `src/starterbot/WinPredictor.h` (nuevo): declara el predictor, sus muestras históricas y sus métodos de actualización.
- `src/starterbot/WinPredictor.cpp` (nuevo): calcula los puntajes, normaliza las probabilidades, dibuja las barras y exporta `predicciones.csv`.
- `src/starterbot/StarterBot.h` (modificado): agrega los dos objetos de medición y documenta en español la clase y sus eventos.
- `src/starterbot/StarterBot.cpp` (modificado): activa la información completa del mapa y conecta las mediciones con los eventos de inicio, frame, destrucción y fin.
- `visualstudio/StarterBot.vcxproj` (modificado): incluye los nuevos encabezados y archivos `.cpp` del predictor y del contador de minerales en el proyecto de Visual Studio.
- `visualstudio/StarterBot.vcxproj.filters` (modificado): incluye los mismos archivos para que aparezcan en el explorador del proyecto de Visual Studio.
- `INFORME_CAMBIOS.md` (nuevo): contiene esta explicación para usar como base del informe del proyecto.
- `bin_linux/StarterBot.exe` (generado por `make`): ejecutable recompilado con los cambios.

No se modificó `Makefile`: este ya busca automáticamente los archivos `.cpp` dentro de `src/`.
El estado de Git también muestra cambios en `bin_linux/libgcc_s_dw2-1.dll` y `bin_linux/libstdc++-6.dll`, además de archivos sin seguimiento bajo `starcraft/`. No son cambios de la lógica fuente de esta tarea; se dejaron intactos.

## Organización existente de `src/`

- `main.cpp` se conecta al cliente BWAPI y entrega los eventos del juego a `StarterBot` o `ReplayParser`.
- `StarterBot.h/.cpp` implementan el bot de partida, sus callbacks y acciones actuales (trabajadores, suministro y dibujo).
- `Tools.h/.cpp` agrupan funciones reutilizables, como encontrar unidades, construir y dibujar información.
- `MapTools.h/.cpp` preparan y consultan datos del mapa, por ejemplo las celdas transitables y visibles.
- `ReplayParser.h/.cpp` procesa partidas grabadas.
- `bwapi/` contiene la interfaz y la implementación local de BWAPI.

## Tarea 1: frames por campo de minerales

1. En cada `onFrame`, `MineralTracker::onFrame` recorre el conjunto actual de `getMinerals()`.
2. Para cada mineral, busca su registro por ID. Si es nuevo, crea uno con el valor de recursos actual y contador cero.
3. Si los recursos bajaron respecto al frame anterior, copia el contador al último conteo, lo agrega al historial y lo reinicia. El historial elimina el elemento más antiguo cuando supera 20 datos.
4. Si los recursos no bajaron, aumenta el contador actual en uno.
5. `draw()` presenta último conteo, promedio del historial (con dos decimales) y contador actual. Prueba varias posiciones verticales junto al mineral para reducir el solapamiento entre etiquetas cercanas.
6. Los minerales que ya no aparecen en el conjunto se marcan inactivos: no se vuelve a consultar su unidad ni se dibuja, pero su medición queda disponible para el CSV.
7. `MineralTracker::onFrame` guarda `minerales.csv` al menos cada 1000 frames, además del guardado final de `onEnd`. El archivo se sobrescribe con el estado acumulado actual, para que haya una copia reciente durante una partida larga.

El archivo nuevo `MineralTracker.h` contiene `MineralData`: ID, unidad asociada, recursos previos, contador actual, último conteo, historial, posición y valores de exportación. Aunque se guarda la referencia de unidad, no se desreferencia tras desaparecer el mineral.

## Tarea 2: predictor de probabilidad

1. Cada 24 frames, `WinPredictor::calculate` recorre jugadores no neutrales y calcula sus variables.
2. Ejército: suma el costo mineral más 1.5 veces el costo de gas de unidades que no son edificios ni trabajadores.
3. Recursos: suma minerales y gas disponibles.
4. Expansiones/edificios: cuenta edificios. Control del mapa: cuenta edificios de tipo centro de recursos (`isResourceDepot`).
5. Tecnología: suma niveles de mejoras y tecnologías investigadas.
6. Bajas: agrega el costo mineral más 1.5 veces el costo de gas de unidades enemigas destruidas, registrándolo para el jugador propio.
7. Cada variable se divide entre el total correspondiente de todos los jugadores. Luego se aplican los pesos 0.35, 0.20, 0.15, 0.10, 0.10 y 0.10. Finalmente, los puntajes se normalizan para que las probabilidades sumen 1. Si todos los puntajes son cero, se reparte la probabilidad por igual.
8. Se dibujan barras y porcentajes en pantalla y se guarda una muestra cada 24 frames.
9. En `onEnd`, se escriben las muestras y el resultado conocido a `predicciones.csv`.

`CompleteMapInformation` se activa en `onStart` porque la recopilación de unidades de otros jugadores necesita información de todo el mapa, incluso bajo la niebla.

## CSV generados e interpretación

Los dos archivos se crean en el directorio de trabajo desde el que se ejecuta el bot. El script `bin_linux/RunStarterBotAndStarcraft.sh` hace `cd bin_linux`, por lo que al usar ese script los CSV quedan en `bin_linux/`:

- `minerales.csv`: columnas `id,x,y,formacion,raza,promedio_frames`. La formación es `izquierda` o `derecha` según si la posición X del mineral está antes o después de la mitad del mapa. `raza` es la raza del jugador que controla el bot, igual en todas las filas. Menor promedio significa una extracción observada más rápida. Si nunca se observó una extracción, el promedio figura como `NA`. Se guarda cada 1000 frames y de nuevo al terminar la partida.
- `predicciones.csv`: columnas `frame,player_id,probabilidad,ganador_id`. Cada muestra genera una fila por jugador; `probabilidad` está entre 0 y 1 y el ganador se repite en las filas para comparar predicción y resultado real.

Para encontrar los cuatro minerales más eficientes por raza y formación, se puede filtrar `minerales.csv` por esas dos columnas, excluir `NA` y ordenar `promedio_frames` de menor a mayor.

## Decisiones y supuestos

- Se identifica cada campo por el ID de unidad que provee BWAPI, lo que permite incorporar campos que aparezcan durante la partida.
- Cualquier disminución de recursos cuenta como una extracción. No se exige comprobar que la diferencia sea exactamente ocho, por si una actualización agrupa varias extracciones.
- “Control del mapa” se definió como cantidad de centros de recursos. Es una aproximación sencilla solicitada; no mide visión, terreno o distancia de tropas.
- `onUnitDestroyed` informa qué unidad murió, pero no quién la destruyó. Por eso el valor de unidades enemigas destruidas se acredita al bot propio como aproximación. El bot actual no combate, así que esta dimensión puede quedarse en cero en una partida normal.
- BWAPI entrega al callback de fin únicamente si el bot ganó. En partidas de dos jugadores, cuando el bot pierde se toma al único rival como ganador. Con varios rivales no se puede identificar al ganador con ese evento y `ganador_id` queda como `-1`.
- La posición izquierda/derecha se decide con el punto medio horizontal del mapa; no representa necesariamente una formación estratégica de bases.

## Limitaciones y pruebas

- `make` completó correctamente y generó `bin_linux/StarterBot.exe`.
- La compilación muestra advertencias deprecadas de captura implícita de `this` dentro de headers existentes de BWAPI (`UnaryFilter.h`, `ComparisonFilter.h` y `BestFilter.h`). No son errores ni se modificaron esos headers.
- No se ejecutó una partida con StarCraft/Wine en esta verificación; por ello falta comprobar visualmente la ubicación de etiquetas y barras, y confirmar la creación de los CSV al terminar una partida real.
- El guardado periódico reescribe una instantánea del estado acumulado; no conserva una fila histórica por cada intervalo.
- El dibujo intenta evitar etiquetas próximas usando posiciones alternativas, pero una gran cantidad de minerales casi en el mismo punto puede agotar esas posiciones.

## Compilar y ejecutar

Desde la raíz del repositorio:

```bash
make
bin_linux/RunStarterBotAndStarcraft.sh
```

Al terminar la partida, revisar `bin_linux/minerales.csv` y `bin_linux/predicciones.csv`.
