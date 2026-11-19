#!/usr/bin/env python3
"""Análisis reproducible de las mediciones minerales de analyse."""
import csv
import math
import re
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parent
OUT = ROOT / "resultados"
RACES = ("Terran", "Protoss", "Zerg")
SIDES = ("izquierda", "derecha")
MISS = {"", "na", "nan", "null", "none"}


def read_csv(path):
    with path.open(newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def val(row, key):
    s = str(row.get(key, "")).strip()
    return None if s.lower() in MISS else s


def num(row, key):
    v = val(row, key)
    return None if v is None else float(v)


def canon_side(s):
    t = re.sub(r"[^a-z]", "", s.lower())
    if t.startswith(("izq", "iqq", "izq", "iqu", "izqu")) or t in ("left",): return "izquierda"
    if t.startswith(("der",)) or t == "right": return "derecha"
    return None


def parse_name(path):
    m = re.search(r"(Terran|Protoss|Zerg)[_-](\d+).*?(Izquierda|Iquierda|Izquerda|Izq[u]?ierda|Derecha)", path.stem, re.I)
    if not m:
        # La escritura del lado puede variar; extraer raza y número y reconocer prefijo del lado.
        m2 = re.search(r"(Terran|Protoss|Zerg)[_-](\d+)", path.stem, re.I)
        side = canon_side(path.stem.split("Minerales_")[-1].split("_")[0]) if "Minerales_" in path.stem else None
        if not m2: return None
        return m2.group(1).title(), int(m2.group(2)), side
    return m.group(1).title(), int(m.group(2)), canon_side(m.group(3))


def mean(xs): return sum(xs) / len(xs) if xs else None
def sd(xs):
    if len(xs) < 2: return None
    m = mean(xs)
    return math.sqrt(sum((x-m)**2 for x in xs)/(len(xs)-1))
def fmt(x, digits=2): return "" if x is None else f"{x:.{digits}f}"


def write_rows(path, rows, fields):
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        w.writeheader(); w.writerows(rows)


def main():
    OUT.mkdir(exist_ok=True)
    alerts = []
    matches = {}
    f5000_files = sorted((ROOT/"Frame_5000").glob("*.csv"))
    control = []
    for path in f5000_files:
        rows = read_csv(path)
        own = [r for r in rows if val(r, "es_propio") == "1"]
        race_vals = {val(r,"raza") for r in rows if val(r,"raza")}
        side_vals = {canon_side(val(r,"lado_inicio") or "") for r in rows if val(r,"lado_inicio")}
        race = next(iter(race_vals), None); side = next(iter(side_vals), None)
        meta = parse_name(path)
        missing = sum(val(r,"promedio_frames") is None for r in own)
        counts = [num(r,"n_conteos") for r in own if num(r,"n_conteos") is not None]
        frames = {val(r,"frame") for r in rows if val(r,"frame")}
        workers = next((num(r,"trabajadores_propios") for r in rows if num(r,"trabajadores_propios") is not None), None)
        control.append(dict(archivo=path.name, raza=race or "", lado_inicio=side or "", propios=len(own), propios_NA=missing,
                            n_conteos_min=min(counts) if counts else "", n_conteos_max=max(counts) if counts else "",
                            trabajadores_propios=workers if workers is not None else "", frame=";".join(sorted(frames))))
        if len(own) != 9: alerts.append(f"{path.name}: {len(own)} campos propios; se esperaban 9")
        if frames != {"5000"}: alerts.append(f"{path.name}: frame(s) {sorted(frames)}, se esperaba solo 5000")
        if meta and meta[0].lower() != (race or "").lower(): alerts.append(f"{path.name}: raza del nombre ({meta[0]}) no coincide con contenido ({race})")
        if meta and meta[2] and meta[2] != side: alerts.append(f"{path.name}: lado del nombre ({meta[2]}) no coincide con contenido ({side})")
        if not meta: alerts.append(f"{path.name}: no se pudo identificar raza/lado/partida en el nombre")
        if meta:
            key=(meta[0],meta[1],meta[2])
            matches[key] = {"rows": own, "all": rows, "file": path.name, "race": race or meta[0], "side": side or meta[2], "workers": workers,
                            "atypical": workers is not None and workers < 10 and meta[0] != "Zerg"}
            if matches[key]["atypical"]: alerts.append(f"{path.name}: atípica, trabajadores_propios={workers:g} (<10; {meta[0]})")
    write_rows(OUT/"control_f5000.csv", control, ["archivo","raza","lado_inicio","propios","propios_NA","n_conteos_min","n_conteos_max","trabajadores_propios","frame"])

    for race in RACES:
        for side in SIDES:
            n=sum(1 for (r,_,s) in matches if r==race and s==side)
            if n != 3: alerts.append(f"{race} {side}: {n} partidas; se esperaban 3")

    # Agrega medias de cada partida por coordenada y registra top individual.
    per_combo = defaultdict(list)
    own_top = {}
    for key, m in matches.items():
        coords=defaultdict(list)
        for r in m["rows"]:
            x=num(r,"x"); y=num(r,"y"); avg=num(r,"promedio_frames"); nc=num(r,"n_conteos")
            if x is not None and y is not None:
                coords[(int(x),int(y))].append((avg,nc))
        val_by_coord={c:mean([a for a,nc in vals if a is not None]) for c,vals in coords.items()}
        own_top[key]=set(c for c,a in sorted(val_by_coord.items(), key=lambda z:(z[1] is None,z[1] or 0))[:4] if a is not None)
        for c, vals in coords.items():
            avg=mean([a for a,nc in vals if a is not None])
            per_combo[(key[0],key[2],c)].append((key, avg, vals))

    top_rows=[]; excluded=[]; top_by_combo={}; top_without={}
    for race in RACES:
      for side in SIDES:
        group=[(k,v) for (r,s,k),v in per_combo.items() if r==race and s==side]
        stats=[]
        for coord, observations in group:
            valid=[(k,a,vals) for k,a,vals in observations if a is not None]
            n=len(valid)
            avgs=[a for k,a,vals in valid]
            counts=[nc for k,a,vals in valid for aa,nc in vals if nc is not None]
            times=sum(coord in own_top.get(k,set()) for k,a,vals in observations)
            low=any(nc is not None and nc<10 for k,a,vals in observations for aa,nc in vals)
            stats.append(dict(x=coord[0],y=coord[1],promedio=mean(avgs),desviacion=sd(avgs),minimo=min(avgs) if avgs else None,
                              maximo=max(avgs) if avgs else None,n_partidas=n,promedio_n_conteos=mean(counts),veces_top4=times,
                              conteos_bajos=low, obs=observations))
            if n<2: excluded.append(dict(raza=race,lado_inicio=side,x=coord[0],y=coord[1],n_partidas=n,motivo="n_partidas menor que 2"))
        eligible=sorted((d for d in stats if d["n_partidas"]>=2),key=lambda d:d["promedio"])
        top_by_combo[(race,side)]=eligible[:4]
        keys=[k for k in matches if k[0]==race and k[2]==side]
        if any(matches[k]["atypical"] for k in keys):
            normals=set(k for k in keys if not matches[k]["atypical"])
            vals=[]
            for d in stats:
                obs=[(k,a,vs) for k,a,vs in d["obs"] if k in normals and a is not None]
                if len(obs)>=2:
                    aa=[a for k,a,vs in obs]; vals.append((mean(aa),d["x"],d["y"],len(obs)))
            top_without[(race,side)]=sorted(vals)[:4]
        for pos,d in enumerate(top_by_combo[(race,side)],1):
            top_rows.append(dict(raza=race,lado_inicio=side,posicion=pos,x=d["x"],y=d["y"],promedio_frames=fmt(d["promedio"]),
              desviacion_muestral=fmt(d["desviacion"]),minimo=fmt(d["minimo"]),maximo=fmt(d["maximo"]),n_partidas=d["n_partidas"],
              promedio_n_conteos=fmt(d["promedio_n_conteos"]),veces_top4_partida=f"{d['veces_top4']}/3",conteos_menor_10="sí" if d["conteos_bajos"] else "no"))
    write_rows(OUT/"top4_por_raza_lado.csv",top_rows,["raza","lado_inicio","posicion","x","y","promedio_frames","desviacion_muestral","minimo","maximo","n_partidas","promedio_n_conteos","veces_top4_partida","conteos_menor_10"])
    write_rows(OUT/"campos_excluidos_top4.csv",excluded,["raza","lado_inicio","x","y","n_partidas","motivo"])

    # Resumen por partida: media de los nueve campos propios con dato, y resumen entre tres partidas.
    summary=[]
    for race in RACES:
      for side in SIDES:
        ms=[(k,m) for k,m in matches.items() if k[0]==race and k[2]==side]
        game_means=[]; workers=[]
        for k,m in ms:
            vv=[num(r,"promedio_frames") for r in m["rows"]]; vv=[x for x in vv if x is not None]
            if vv: game_means.append(mean(vv))
            if m["workers"] is not None: workers.append(m["workers"])
        summary.append(dict(raza=race,lado_inicio=side,n_partidas=len(ms),promedio_9_campos=fmt(mean(game_means)),
                            desviacion_entre_partidas=fmt(sd(game_means)),promedio_trabajadores_propios=fmt(mean(workers)),
                            minimo_trabajadores=fmt(min(workers) if workers else None),maximo_trabajadores=fmt(max(workers) if workers else None)))
    write_rows(OUT/"resumen_combinaciones.csv",summary,["raza","lado_inicio","n_partidas","promedio_9_campos","desviacion_entre_partidas","promedio_trabajadores_propios","minimo_trabajadores","maximo_trabajadores"])

    # Predicciones: join por raza, número y lado; jugador ganador y máxima probabilidad al final.
    pred_rows=[]
    for path in sorted((ROOT/"prediccions").glob("*.csv")):
        meta=parse_name(path)
        if not meta:
            alerts.append(f"Predicción {path.name}: no se pudo emparejar con partida")
            continue
        rows=read_csv(path)
        if not rows: alerts.append(f"Predicción {path.name}: archivo vacío"); continue
        frames=sorted({num(r,"frame") for r in rows if num(r,"frame") is not None})
        last=frames[-1] if frames else None
        last_rows=[r for r in rows if num(r,"frame")==last]
        winners={val(r,"ganador_id") for r in last_rows if val(r,"ganador_id") is not None}
        winner=next(iter(winners),None)
        closest=min(frames,key=lambda x:abs(x-5000)) if frames else None
        close_rows=[r for r in rows if num(r,"frame")==closest]
        pwin_last=next((num(r,"probabilidad") for r in last_rows if val(r,"player_id")==winner),None)
        pwin_near=next((num(r,"probabilidad") for r in close_rows if val(r,"player_id")==winner),None)
        byplayer={}
        for r in last_rows:
            p=num(r,"probabilidad"); pid=val(r,"player_id")
            if p is not None and pid is not None: byplayer[pid]=p
        maxplayer=max(byplayer,key=byplayer.get) if byplayer else None
        correct=maxplayer==winner if maxplayer is not None and winner is not None else False
        pred_rows.append(dict(archivo=path.name,raza=meta[0],partida=meta[1],lado_inicio=meta[2] or "",ultimo_frame=fmt(last,0),ganador_id=winner or "",
             prob_ganador_ultimo=fmt(pwin_last,6),frame_mas_cercano_5000=fmt(closest,0),prob_ganador_cercano_5000=fmt(pwin_near,6),
             max_probabilidad_player_id=maxplayer or "",acierto="sí" if correct else "no"))
    write_rows(OUT/"resumen_predicciones.csv",pred_rows,["archivo","raza","partida","lado_inicio","ultimo_frame","ganador_id","prob_ganador_ultimo","frame_mas_cercano_5000","prob_ganador_cercano_5000","max_probabilidad_player_id","acierto"])

    # Figura de 9 campos propios por combinación, con escala común en frames.
    fig,axes=plt.subplots(2,3,figsize=(15,12),sharey="row",constrained_layout=True)
    cmap=plt.get_cmap("viridis"); norm=plt.Normalize(vmin=100,vmax=180)
    row_coords={side:[coord for (r,s,coord) in per_combo if s==side] for side in SIDES}
    for row,side in enumerate(SIDES):
        for col,race in enumerate(RACES):
            ax=axes[row,col]
            coord_stats={(d["x"],d["y"]):d for d in [
                {"x":coord[0],"y":coord[1],"promedio":mean([a for k,a,vs in obs if a is not None])}
                for (r,s,coord),obs in per_combo.items() if r==race and s==side
            ]}
            top_rank={(d["x"],d["y"]):rank for rank,d in enumerate(top_by_combo[(race,side)],1)}
            xs=[c[0] for c in coord_stats]; ys=[c[1] for c in coord_stats]
            for (x,y),d in coord_stats.items():
                avg=d["promedio"]
                if avg is None: continue
                # s=260 pt² equivale a un diámetro visual menor que 32 px a 180 dpi.
                ax.scatter(x,y,s=260,c=[cmap(norm(avg))],edgecolors="black",linewidths=.7,zorder=2)
                if (x,y) in top_rank:
                    ax.text(x,y,str(top_rank[(x,y)]),ha="center",va="center",color="white",fontweight="bold",fontsize=9,zorder=3)
                # Prefiere fuera del grupo (a la derecha en base izquierda y viceversa).
                # Si hay otro círculo cerca de ese rótulo, desplaza al lado contrario.
                preferred=1 if side=="izquierda" else -1
                def blocked(direction):
                    return any(0 < direction*(xx-x) < 60 and abs(yy-y) < 18
                               for xx,yy in coord_stats if (xx,yy)!=(x,y))
                direction=-preferred if blocked(preferred) and not blocked(-preferred) else preferred
                if blocked(direction): direction=-direction
                offset=20*direction
                ax.annotate(f"{avg:.0f}",(x,y),xytext=(offset,0),textcoords="offset points",
                            ha="left" if direction>0 else "right",va="center",fontsize=7,color="#222222",
                            bbox=dict(boxstyle="round,pad=0.16",facecolor="white",edgecolor="none",alpha=.78),zorder=4)
            ax.set_title(f"{race} · {side}")
            ax.set_xlabel("Posición X (píxeles)")
            if col==0: ax.set_ylabel("Posición Y (píxeles)")
            ax.set_aspect("equal",adjustable="box"); ax.grid(alpha=.2)
        # Límites idénticos en los tres paneles de la fila, con escala espacial 1:1.
        coords=row_coords[side]
        if coords:
            xmin=min(x for x,y in coords)-100; xmax=max(x for x,y in coords)+100
            ymin=min(y for x,y in coords)-100; ymax=max(y for x,y in coords)+115
            for ax in axes[row,:]:
                ax.set_xlim(xmin,xmax); ax.set_ylim(ymax,ymin)
    sm=plt.cm.ScalarMappable(norm=norm,cmap=cmap); sm.set_array([])
    fig.colorbar(sm,ax=axes.ravel().tolist(),label="Promedio de frames (menor = más rápido)",shrink=.88)
    fig.suptitle("Extracción de minerales por raza y lado de inicio")
    fig.savefig(OUT/"mapa_top4.png",dpi=180); fig.savefig(OUT/"mapa_top4_300dpi.png",dpi=300); plt.close(fig)

    # Mapa de minerales: agrupa por componentes conexos con distancia <=350 px.
    mineral_files=sorted((ROOT/"minerals").glob("*.csv"))
    maprows=read_csv(mineral_files[0]) if mineral_files else []
    pts=[(int(float(r["x"])),int(float(r["y"])),r) for r in maprows]
    parent=list(range(len(pts)))
    def find(i):
        while parent[i]!=i: parent[i]=parent[parent[i]]; i=parent[i]
        return i
    for i in range(len(pts)):
        for j in range(i):
            if math.hypot(pts[i][0]-pts[j][0],pts[i][1]-pts[j][1])<=350:
                a,b=find(i),find(j)
                if a!=b: parent[a]=b
    groups=defaultdict(list)
    for i,p in enumerate(pts): groups[find(i)].append(i)
    group_list=sorted(groups.values(),key=lambda ids:(mean([pts[i][0] for i in ids]),mean([pts[i][1] for i in ids])))
    if len(pts)!=98: alerts.append(f"CSV minerales usado para mapa tiene {len(pts)} campos, se esperaban 98")
    if len(group_list)!=13: alerts.append(f"Agrupación por distancia <=350 px produjo {len(group_list)} grupos, se esperaban 13")
    fig,ax=plt.subplots(figsize=(10,10),constrained_layout=True)
    cmap=plt.get_cmap("tab20",max(1,len(group_list)))
    bases=[]
    for gi,ids in enumerate(group_list,1):
        xs=[pts[i][0] for i in ids]; ys=[pts[i][1] for i in ids]
        ax.scatter(xs,ys,color=[cmap(gi-1)],s=35,edgecolors="black",linewidths=.3,label=f"Grupo {gi}")
        cx,cy=mean(xs),mean(ys); ax.text(cx,cy,str(gi),ha="center",va="center",fontsize=9,fontweight="bold",bbox=dict(facecolor="white",alpha=.75,edgecolor="none",pad=1))
        if len(ids)==9: bases.append((gi,cx,cy))
    ax.axvline(2048,color="black",linestyle=":",linewidth=1.5,label="Mitad (2048 px)")
    for gi,cx,cy in bases: ax.scatter(cx,cy,s=250,facecolors="none",edgecolors="red",linewidths=2)
    ax.set_title("Campos minerales del mapa (98 campos, 13 grupos)"); ax.set_xlabel("X (píxeles)"); ax.set_ylabel("Y (píxeles)"); ax.invert_yaxis(); ax.set_aspect("equal",adjustable="box"); ax.legend(title="Formación / grupos",bbox_to_anchor=(1.02,1),loc="upper left",fontsize=8)
    fig.savefig(OUT/"mapa_minerales_final.png",dpi=180,bbox_inches="tight"); plt.close(fig)

    # Reporte reproducible de alertas, top y resultados de predicción.
    md=["# Tablas top 4", "", "Posición por promedio ascendente. Desviación muestral calculada entre partidas con dato.", ""]
    for race in RACES:
      for side in SIDES:
        md += [f"## {race} — base {side}", "", "| Posición | x | y | Promedio | Desviación | n_partidas | Veces en top 4 por partida |", "|---:|---:|---:|---:|---:|---:|---:|"]
        for pos,d in enumerate(top_by_combo[(race,side)],1): md.append(f"| {pos} | {d['x']} | {d['y']} | {fmt(d['promedio'])} | {fmt(d['desviacion'])} | {d['n_partidas']} | {d['veces_top4']}/3 |")
        md.append("")
        if top_without.get((race,side)):
            md += [f"Partidas atípicas excluidas: top recalculado: {', '.join(f'({x},{y}) {a:.2f}' for a,x,y,n in top_without[(race,side)])}.", ""]
    md += ["## Campos excluidos", "", "| Raza | Lado | x | y | n_partidas | Motivo |", "|---|---|---:|---:|---:|---|"]
    for d in excluded: md.append(f"| {d['raza']} | {d['lado_inicio']} | {d['x']} | {d['y']} | {d['n_partidas']} | {d['motivo']} |")
    (OUT/"tablas_top4.md").write_text("\n".join(md)+"\n",encoding="utf-8")
    (OUT/"alertas.txt").write_text("\n".join(alerts) + ("\n" if alerts else "Sin problemas detectados.\n"),encoding="utf-8")
    print("Aciertos del predictor: %d de %d" % (sum(r["acierto"]=="sí" for r in pred_rows),len(pred_rows)))
    print("Alertas:")
    print("\n".join(alerts) if alerts else "Sin problemas detectados.")
    print("Top 4 por combinación:")
    print("\n".join(md))
    print("Resumen de combinaciones:")
    for r in summary: print(r)
    print("Cambios top al excluir atípicas:")
    for combo, without in top_without.items():
        print(combo, "con:", [(d["x"],d["y"],fmt(d["promedio"])) for d in top_by_combo[combo]], "sin:", [(x,y,fmt(a)) for a,x,y,n in without])


if __name__ == "__main__": main()
