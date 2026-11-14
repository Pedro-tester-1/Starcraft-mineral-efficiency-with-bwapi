#include "MineralTracker.h"

#include <fstream>
#include <iomanip>
#include <cstdlib>

/** Inicializa el historial que se irá completando durante la partida. */
MineralTracker::MineralTracker()
    : m_lastCsvFrame(0)
{
}

/** Encuentra el registro de un mineral sin crear copias del historial. */
MineralTracker::MineralData* MineralTracker::findMineral(int id)
{
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        if (m_minerals[i].id == id) { return &m_minerals[i]; }
    }
    return nullptr;
}

/** Asocia cada mineral actual con su registro y detecta cuando se extrae. */
void MineralTracker::onFrame()
{
    const BWAPI::Unitset& fields = BWAPI::Broodwar->getMinerals();
    for (auto& unit : fields)
    {
        if (!unit || !unit->exists()) { continue; }

        MineralData* data = findMineral(unit->getID());
        if (!data)
        {
            MineralData fresh;
            fresh.id = unit->getID();
            fresh.unit = unit;
            fresh.previousResources = unit->getResources();
            fresh.currentFrames = 0;
            fresh.lastFrames = 0;
            fresh.position = unit->getPosition();
            fresh.finalAverage = 0.0;
            fresh.hasAverage = false;
            fresh.active = true;
            m_minerals.push_back(fresh);
            data = &m_minerals.back();
        }

        data->unit = unit;
        data->position = unit->getPosition();
        data->active = true;
        const int resources = unit->getResources();
        if (resources < data->previousResources)
        {
            data->lastFrames = data->currentFrames;
            data->history.push_back(data->currentFrames);
            if (data->history.size() > 20) { data->history.pop_front(); }
            data->currentFrames = 0;
        }
        else
        {
            data->currentFrames++;
        }
        data->previousResources = resources;
    }

    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        bool found = false;
        for (auto& unit : fields)
        {
            if (unit && unit->getID() == m_minerals[i].id) { found = true; break; }
        }
        m_minerals[i].active = found;
        if (!m_minerals[i].history.empty())
        {
            double sum = 0.0;
            for (size_t j = 0; j < m_minerals[i].history.size(); ++j) { sum += m_minerals[i].history[j]; }
            m_minerals[i].finalAverage = sum / m_minerals[i].history.size();
            m_minerals[i].hasAverage = true;
        }
    }

    const int frame = BWAPI::Broodwar->getFrameCount();
    if (frame - m_lastCsvFrame >= 1000)
    {
        saveCsv("minerales.csv");
        m_lastCsvFrame = frame;
    }
}

/** Muestra tres datos del mineral con pequeñas variaciones verticales. */
void MineralTracker::draw() const
{
    std::vector<BWAPI::Position> labelPositions;
    const int yOffsets[] = { 0, -40, 40, -80, 80, -120, 120, -160, 160 };
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        const MineralData& data = m_minerals[i];
        if (!data.active) { continue; }
        double sum = 0.0;
        for (size_t j = 0; j < data.history.size(); ++j) { sum += data.history[j]; }
        const double average = data.history.empty() ? 0.0 : sum / data.history.size();
        int textY = data.position.y - 28;
        for (size_t offset = 0; offset < sizeof(yOffsets) / sizeof(yOffsets[0]); ++offset)
        {
            const int candidateY = data.position.y - 28 + yOffsets[offset];
            bool overlaps = false;
            for (size_t j = 0; j < labelPositions.size(); ++j)
            {
                if (std::abs(labelPositions[j].x - (data.position.x - 48)) < 72
                    && std::abs(labelPositions[j].y - candidateY) < 40)
                {
                    overlaps = true;
                    break;
                }
            }
            textY = candidateY;
            if (!overlaps) { break; }
        }
        labelPositions.push_back(BWAPI::Position(data.position.x - 48, textY));
        BWAPI::Broodwar->drawTextMap(data.position.x - 48, textY,
            "%d\n%.2f\n%d", data.lastFrames, average, data.currentFrames);
    }
}

/** Escribe un CSV con posición, formación, raza propia y promedio observado. */
void MineralTracker::saveCsv(const std::string& path) const
{
    std::ofstream out(path.c_str());
    if (!out) { return; }
    out << "id,x,y,formacion,raza,promedio_frames\n";
    out << std::fixed << std::setprecision(2);
    std::string ownRace = "Desconocida";
    if (BWAPI::Broodwar->self()) { ownRace = BWAPI::Broodwar->self()->getRace().getName(); }
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        const MineralData& data = m_minerals[i];
        const std::string formation = data.position.x < BWAPI::Broodwar->mapWidth() * 16 ? "izquierda" : "derecha";
        out << data.id << ',' << data.position.x << ',' << data.position.y << ',' << formation << ',' << ownRace << ',';
        if (data.hasAverage) { out << data.finalAverage; }
        else { out << "NA"; }
        out << '\n';
    }
}
