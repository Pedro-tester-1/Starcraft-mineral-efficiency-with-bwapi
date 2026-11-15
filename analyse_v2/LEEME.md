# Guía de `analyse_v2`

El análisis vigente es `analyse_v2/analisis_final.py`. Lee las tres carpetas de datos, empareja partidas por raza, número y lado de inicio, y escribe los resúmenes y gráficos en `resultados/`. Se ejecuta desde la raíz del repositorio con:

```bash
python3 analyse_v2/analisis_final.py
```

## Datos de entrada

- `Frame_5000/`: 18 CSV, uno por partida, con todos los campos y metadatos guardados al frame 5000. El análisis usa `es_propio`, `promedio_frames`, `n_conteos`, raza, lado de inicio y trabajadores para validar y comparar.
- `minerals/`: 18 CSV con las posiciones y mediciones de los campos minerales de cada partida. El script usa estos datos para generar el mapa general de grupos de minerales.
- `prediccions/`: 18 CSV con la serie temporal de predicción por jugador y el ganador registrado. El script resume el ganador, la probabilidad asociada y si el jugador con mayor probabilidad al final coincide con él.

Los nombres `prediccions/` y algunos nombres de archivo con `Iquierda` tienen esa ortografía porque así se guardaron los datos. El script reconoce esas variantes al emparejar las partidas.

## Archivos de `resultados/`

- `alertas.txt`: verificaciones de cantidad de partidas, campos propios, frames y metadatos; indica si se encontraron problemas.
- `campos_excluidos_top4.csv`: campos descartados del ranking por tener mediciones en menos de dos partidas, con el motivo.
- `control_f5000.csv`: control por archivo del frame, raza, lado, campos propios, valores NA, conteos y trabajadores.
- `mapa_minerales_final.png`: mapa general de los campos y sus grupos espaciales.
- `mapa_top4.png`: figura comparativa de las seis combinaciones de raza y lado; muestra promedios y posiciones del top 4.
- `mapa_top4_300dpi.png`: la misma figura top 4 exportada a 300 dpi.
- `resumen_combinaciones.csv`: promedio de los campos propios por partida, resumido por raza y lado, con dispersión y trabajadores.
- `resumen_predicciones.csv`: resultado final de cada predicción por partida, probabilidad del ganador y acierto del jugador con máxima probabilidad.
- `tablas_top4.md`: seis tablas legibles, una por combinación de raza y lado, además de los campos excluidos.
- `top4_por_raza_lado.csv`: filas del top 4 con promedio, desviación, rango, cantidad de partidas y frecuencia en el top 4 individual.

## Diseño anterior

`bin_linux/analisis_top4.py` pertenece al diseño anterior: mezcla datos minerales con la base del rival, por lo que no es el flujo de análisis vigente. Usa `analyse_v2/analisis_final.py` para reproducir los resultados actuales. Se conserva el archivo anterior como referencia; no forma parte de este análisis.
