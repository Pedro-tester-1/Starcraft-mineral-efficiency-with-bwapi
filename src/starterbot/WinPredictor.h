#pragma once

#include <BWAPI.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

/** Calcula y registra una estimación sencilla de la probabilidad de victoria. */
class WinPredictor
{
    /** Guarda la probabilidad de todos los jugadores en un instante de la partida. */
    struct Snapshot
    {
        int frame;
        std::map<int, double> probabilities;
    };

    std::map<int, double> m_killedValue;
    std::vector<Snapshot> m_history;
    std::map<int, double> m_currentProbabilities;
    int m_lastUpdateFrame;

    /** Calcula todas las probabilidades normalizadas usando el estado actual. */
    std::map<int, double> calculate() const;

public:
    /** Inicializa los datos del predictor para una partida nueva. */
    WinPredictor();

    /** Actualiza el predictor y conserva una muestra cada 24 frames. */
    void onFrame();

    /** Acredita al jugador propio el valor de una unidad enemiga destruida. */
    void onUnitDestroyed(BWAPI::Unit unit);

    /** Dibuja barras de probabilidad en pantalla para los jugadores activos. */
    void draw() const;

    /** Guarda cada muestra histórica y el ganador real indicado por BWAPI. */
    void saveCsv(const std::string& path, int winnerId) const;
};
