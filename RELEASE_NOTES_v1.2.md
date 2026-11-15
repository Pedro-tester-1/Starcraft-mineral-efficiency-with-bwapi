# Notas de versión v1.2

Esta versión añade medición de minerales en el frame 5000 y prepara el análisis de 18 partidas.

## Novedades

- Exportación única de `minerales_f5000.csv` con contexto de partida y las columnas `es_propio`, `lado_inicio`, `trabajadores_propios`, `raza_rival` y `frame`.
- Clasificación de campos propios por distancia de hasta 320 píxeles a la posición inicial del bot.
- Descarte del primer conteo de cada campo; el promedio se calcula con hasta los últimos 20 conteos y queda como `NA` con menos de tres.
- Datos de 18 partidas (tres por cada combinación de raza y lado) y análisis final reproducible con seis tablas top 4, resúmenes y mapas en `analyse_v2/resultados/`.

## Limitaciones conocidas

- Los resultados contienen tres partidas por combinación; no describen todos los mapas ni todas las configuraciones de juego.
- `es_propio` depende del radio de 320 píxeles y de la posición inicial detectada. La formación izquierda/derecha usa la mitad horizontal del mapa.
- Si el bot pierde, `ganador_id` identifica al rival solo cuando hay exactamente un rival. Con varios rivales queda en `-1`.
- El valor de bajas se acredita al bot cuando muere una unidad enemiga, porque el callback usado no identifica al responsable. El bot no implementa combate.

## Cómo reproducir

Desde la raíz, compila con `make` y ejecuta `bin_linux/RunStarterBotAndStarcraft.sh`. Para generar los resultados del conjunto de datos incluido, ejecuta `python3 analyse_v2/analisis_final.py`. La guía de carpetas y resultados está en `analyse_v2/LEEME.md`.
