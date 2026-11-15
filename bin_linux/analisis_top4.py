#!/usr/bin/env python3
"""Analiza minerales y predicciones de las partidas StarterBot."""
import csv
import math
import statistics
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "analyse" / "resultados"
RAZAS = ("Terran", "Protoss", "Zerg")
FORMAS = ("izquierda", "derecha")


def read_csv(path):
    with path.open(newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def minerals():
    files = sorted((ROOT / "analyse" / "minerals").rglob("*.csv"))
    partida_rows, per_game = [], defaultdict(dict)
    for path in files:
        rows = read_csv(path)
        filename = path.stem.lower()
        if "izquerda" in filename or "izquierda" in filename:
            start_side = "izquierda"
        elif "derecha" in filename:
            start_side = "derecha"
        else:
            start_side = "desconocido"
        kept = []
        discarded_other = 0
        for row in rows:
            form = row.get("formacion", "").strip().lower()
            if form != start_side:
                if form in FORMAS:
                    discarded_other += 1
                continue
            kept.append(row)
        races = {r.get("raza", "").strip() for r in kept if r.get("raza", "").strip()}
        race = next(iter(races)) if len(races) == 1 else ";".join(sorted(races)) if races else ""
        by_position = {}
        valid_races = defaultdict(dict)
        for row in kept:
            try:
                value = float(row.get("promedio_frames", ""))
                x, y = float(row["x"]), float(row["y"])
            except (ValueError, KeyError, TypeError):
                continue
            pos = (x, y)
            by_position[pos] = value
            # Mantener los datos por raza si las filas de la base propia discrepan.
            row_race = row.get("raza", "").strip()
            if row_race:
                valid_races[row_race][pos] = value
        relpath = str(path.relative_to(ROOT))
        for row_race, minerals_at_positions in valid_races.items():
            for pos, value in minerals_at_positions.items():
                per_game[(row_race, start_side, pos)][relpath] = value
        partida_rows.append({"archivo": relpath, "raza_base_inicio": race,
                             "lado_inicio": start_side,
                             "minerales_con_datos_lado_inicio": len(by_position),
                             "filas_descartadas_lado_opuesto": discarded_other})

    with (OUT / "control_partidas.csv").open("w", newline="", encoding="utf-8") as f:
        fields = list(partida_rows[0]) if partida_rows else ["archivo"]
        w = csv.DictWriter(f, fieldnames=fields); w.writeheader(); w.writerows(partida_rows)

    agg = {}
    for key, games in per_game.items():
        vals = list(games.values())
        agg[key] = {"raza": key[0], "formacion": key[1], "x": key[2][0], "y": key[2][1],
                    "promedio_frames": statistics.mean(vals),
                    "desviacion_estandar": statistics.stdev(vals) if len(vals) > 1 else 0.0,
                    "n_partidas": len(vals)}
    tops = {}
    rows_out = []
    for race in RAZAS:
        for form in FORMAS:
            all_group = sorted((v for k, v in agg.items() if k[0] == race and k[1] == form),
                               key=lambda r: (r["promedio_frames"], r["x"], r["y"]))
            eligible = [r for r in all_group if r["n_partidas"] >= 3]
            sparse = [r for r in all_group if r["n_partidas"] < 3]
            tops[(race, form)] = eligible[:4]
            if len(eligible) < 4:
                print(f"Aviso: {race}/{form} solo tiene {len(eligible)} minerales con n_partidas >= 3.")
            if sparse:
                print(f"\nMinerales con menos de 3 partidas — {race}/{form}:")
                print("| x | y | Promedio | Desv. estándar | n_partidas |\n|---:|---:|---:|---:|---:|")
                for row in sparse:
                    print(f"| {row['x']:g} | {row['y']:g} | {row['promedio_frames']:.2f} | {row['desviacion_estandar']:.2f} | {row['n_partidas']} |")
            for rank, row in enumerate(eligible[:4], 1):
                row = dict(row); row["puesto"] = rank
                rows_out.append(row)
            print(f"\n### {race} — {form}\n")
            print("| Puesto | x | y | Promedio | Desv. estándar | n_partidas |\n|---:|---:|---:|---:|---:|---:|")
            for rank, row in enumerate(eligible[:4], 1):
                print(f"| {rank} | {row['x']:g} | {row['y']:g} | {row['promedio_frames']:.2f} | {row['desviacion_estandar']:.2f} | {row['n_partidas']} |")
    fields = ["raza", "formacion", "puesto", "x", "y", "promedio_frames", "desviacion_estandar", "n_partidas"]
    with (OUT / "top4_por_raza_formacion.csv").open("w", newline="", encoding="utf-8") as f:
        w=csv.DictWriter(f, fieldnames=fields); w.writeheader(); w.writerows(rows_out)
    make_map(agg, tops)
    return []


def make_map(agg, tops):
    fig, axes = plt.subplots(1, 2, figsize=(15, 7), constrained_layout=True)
    cmap = plt.get_cmap("viridis_r")
    for ax, form in zip(axes, FORMAS):
        data = [v for v in agg.values() if v["formacion"] == form]
        if not data: continue
        sc = ax.scatter([v["x"] for v in data], [v["y"] for v in data],
                         c=[v["promedio_frames"] for v in data], cmap=cmap,
                         s=50, alpha=.78, edgecolors="#333333", linewidths=.4)
        for race, marker in zip(RAZAS, ("*", "D", "P")):
            winners = tops[(race, form)]
            if winners:
                ax.scatter([v["x"] for v in winners], [v["y"] for v in winners],
                           marker=marker, s=190, facecolors="none", edgecolors="red",
                           linewidths=1.5, label=f"Top 4 {race}")
        ax.set(title=form.capitalize(), xlabel="X", ylabel="Y")
        ax.invert_yaxis(); ax.grid(alpha=.18); ax.legend(fontsize=8)
    fig.colorbar(sc, ax=axes, label="Promedio de promedio_frames")
    fig.suptitle("Minerales por posición; símbolos rojos = top 4 por raza y formación")
    fig.savefig(OUT / "mapa_top4.png", dpi=180); plt.close(fig)


def predictions():
    files = sorted((ROOT / "analyse" / "prediccions").rglob("*.csv"))
    results=[]; correct=total=0
    for path in files:
        rows=read_csv(path)
        if not rows: continue
        frames=defaultdict(dict); winners=set()
        for r in rows:
            try:
                frame=int(r["frame"]); pid=int(r["player_id"]); prob=float(r["probabilidad"])
                wid=int(r["ganador_id"])
            except (ValueError, KeyError): continue
            frames[frame][pid]=prob; winners.add(wid)
        if not frames: continue
        final_frame=max(frames); final=frames[final_frame]
        winner=next(iter(winners)) if len(winners)==1 else -1
        # El CSV no identifica cuál player_id es self; se informa explícitamente player 0.
        pbot=final.get(0)
        mean0=statistics.mean(vals[0] for vals in frames.values() if 0 in vals)
        predicted=max(final, key=final.get) if final else None
        hit="sin dato" if winner == -1 else ("sí" if predicted == winner else "no")
        if winner != -1:
            total+=1; correct += predicted == winner
        results.append({"archivo":str(path.relative_to(ROOT)), "ultimo_frame":final_frame,
                        "probabilidad_player_0_ultimo_frame":pbot,
                        "probabilidad_player_0_promedio":mean0,
                        "jugador_mayor_probabilidad_final":predicted,
                        "ganador_id":winner,"acierto":hit})
    fields=list(results[0]) if results else ["archivo"]
    with (OUT / "resumen_predicciones.csv").open("w",newline="",encoding="utf-8") as f:
        w=csv.DictWriter(f,fieldnames=fields);w.writeheader();w.writerows(results)
    pct=100*correct/total if total else math.nan
    print(f"\nPredicciones: aciertos {correct}/{total} = {pct:.1f}% (excluye ganador_id=-1).")
    print("La columna de probabilidad usa player_id=0; los CSV no identifican explícitamente el ID del bot.")
    return pct


def main():
    OUT.mkdir(parents=True,exist_ok=True)
    warnings=minerals(); predictions()
    print(f"\nResultados guardados en {OUT}")


if __name__ == "__main__": main()
