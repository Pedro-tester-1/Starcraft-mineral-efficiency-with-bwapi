#pragma once

#include <BWAPI.h>
#include <deque>
#include <string>
#include <vector>

/** Guarda las mediciones de extracción de cada campo de minerales de la partida. */
class MineralTracker
{
    /** Representa el estado y el historial de un único campo de minerales. */
    struct MineralData
    {
        int id;
        BWAPI::Unit unit;
        int previousResources;
        int currentFrames;
        int lastFrames;
        std::deque<int> history;
        BWAPI::Position position;
        double finalAverage;
        bool hasAverage;
        bool active;
    };

    std::vector<MineralData> m_minerals;
    int m_lastCsvFrame;

    /** Busca una medición por el identificador estable que entrega BWAPI. */
    MineralData* findMineral(int id);

public:
    /** Crea un registro vacío antes de que empiece la partida. */
    MineralTracker();

    /** Actualiza una vez por frame todos los minerales visibles para BWAPI. */
    void onFrame();

    /** Dibuja conteo reciente, promedio y tiempo actual junto a cada mineral. */
    void draw() const;

    /** Escribe el promedio final de cada mineral para el análisis del informe. */
    void saveCsv(const std::string& path) const;
};
