#include "MineralTracker.h"

#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cmath>

namespace
{
    // Estos umbrales clasifican promedios más bajos como una extracción más eficiente.
    const double PROMEDIO_EFICIENTE_MAX = 160.0;
    const double PROMEDIO_MEDIO_MAX = 200.0;
    const double RADIO_BASE_PROPIA_PIXELES = 320.0;
    const int CAMPOS_ESPERADOS_BASE = 9;
}

/** Inicializa el historial que se irá completando durante la partida. */
MineralTracker::MineralTracker()
    : m_lastCsvFrame(0), m_f5000CsvWritten(false)
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
            fresh.discardedFirstCount = false;
            m_minerals.push_back(fresh);
            data = &m_minerals.back();
        }

        data->unit = unit;
        data->position = unit->getPosition();
        data->active = true;
        const int resources = unit->getResources();
        if (data->previousResources - resources >= 8)
        {
            if (data->discardedFirstCount)
            {
                data->lastFrames = data->currentFrames;
                data->history.push_back(data->currentFrames);
                if (data->history.size() > 20) { data->history.pop_front(); }
            }
            else { data->discardedFirstCount = true; }
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
    if (frame == 5000 && !m_f5000CsvWritten)
    {
        saveFrame5000Csv("minerales_f5000.csv");
        m_f5000CsvWritten = true;
    }
    if (frame - m_lastCsvFrame >= 1000)
    {
        saveCsv("minerales.csv");
        m_lastCsvFrame = frame;
    }
}

/** Dibuja etiquetas compactas, coloreadas y separadas junto a los minerales. */
void MineralTracker::draw() const
{
    BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Small);
    const int halfMapX = BWAPI::Broodwar->mapWidth() * 16;
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        const MineralData& data = m_minerals[i];
        if (!data.active) { continue; }
        double sum = 0.0;
        for (size_t j = 0; j < data.history.size(); ++j) { sum += data.history[j]; }
        const double average = data.history.empty() ? 0.0 : sum / data.history.size();
        int color = BWAPI::Text::White;
        if (!data.history.empty() && average <= PROMEDIO_EFICIENTE_MAX) { color = BWAPI::Text::Green; }
        else if (!data.history.empty() && average <= PROMEDIO_MEDIO_MAX) { color = BWAPI::Text::Yellow; }
        else if (!data.history.empty()) { color = BWAPI::Text::Red; }

        const int textX = data.position.x + (data.position.x >= halfMapX ? -140 : 40);
        const int textY = data.position.y;
        BWAPI::Broodwar->drawBoxMap(textX - 3, textY - 3, textX + 103, textY + 13,
                                    BWAPI::Colors::Black, true);
        BWAPI::Broodwar->drawTextMap(textX, textY,
            "%cU%d P%.1f A%d", color, data.lastFrames, average, data.currentFrames);
    }
    BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Default);
}

/** Escribe un CSV con posición, formación, raza propia y promedio observado. */
void MineralTracker::saveCsv(const std::string& path) const
{
    std::ofstream out(path.c_str());
    if (!out) { return; }
    out << "id,x,y,formacion,raza,promedio_frames,n_conteos\n";
    out << std::fixed << std::setprecision(2);
    std::string ownRace = "Desconocida";
    if (BWAPI::Broodwar->self()) { ownRace = BWAPI::Broodwar->self()->getRace().getName(); }
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        const MineralData& data = m_minerals[i];
        const std::string formation = data.position.x < BWAPI::Broodwar->mapWidth() * 16 ? "izquierda" : "derecha";
        out << data.id << ',' << data.position.x << ',' << data.position.y << ',' << formation << ',' << ownRace << ',';
        if (data.history.size() >= 3 && data.hasAverage) { out << data.finalAverage; }
        else { out << "NA"; }
        out << ',' << data.history.size() << '\n';
    }
}

/** Escribe posición, mediciones y contexto de partida en el frame 5000. */
void MineralTracker::saveFrame5000Csv(const std::string& path) const
{
    std::ofstream out(path.c_str());
    if (!out) { return; }

    BWAPI::Player self = BWAPI::Broodwar->self();
    out << "id,x,y,formacion,raza,promedio_frames,n_conteos,es_propio,lado_inicio,trabajadores_propios,raza_rival,frame\n";
    out << std::fixed << std::setprecision(2);

    std::string ownRace = "Desconocida";
    std::string rivalRace = "Unknown";
    std::string startSide = "desconocido";
    int workers = 0;
    BWAPI::Position startPosition = BWAPI::Positions::Unknown;
    bool hasStartPosition = false;

    if (self)
    {
        ownRace = self->getRace().getName();
        const BWAPI::TilePosition startTile = self->getStartLocation();
        if (startTile.isValid())
        {
            startPosition = BWAPI::Position(startTile);
            hasStartPosition = true;
            startSide = startPosition.x < BWAPI::Broodwar->mapWidth() * 16 ? "izquierda" : "derecha";
        }
        for (auto& unit : self->getUnits())
        {
            if (unit && unit->exists() && unit->getType().isWorker()) { ++workers; }
        }
    }

    BWAPI::Player enemy = BWAPI::Broodwar->enemy();
    if (enemy && enemy->getRace() != BWAPI::Races::Unknown)
    {
        rivalRace = enemy->getRace().getName();
    }

    int ownMinerals = 0;
    for (size_t i = 0; i < m_minerals.size(); ++i)
    {
        const MineralData& data = m_minerals[i];
        const std::string formation = data.position.x < BWAPI::Broodwar->mapWidth() * 16 ? "izquierda" : "derecha";
        const bool isOwn = hasStartPosition
            && data.position.getDistance(startPosition) <= RADIO_BASE_PROPIA_PIXELES;
        if (isOwn) { ++ownMinerals; }

        out << data.id << ',' << data.position.x << ',' << data.position.y << ',' << formation << ',' << ownRace << ',';
        if (data.history.size() >= 3 && data.hasAverage) { out << data.finalAverage; }
        else { out << "NA"; }
        out << ',' << data.history.size() << ',' << (isOwn ? 1 : 0) << ',' << startSide << ','
            << workers << ',' << rivalRace << ',' << BWAPI::Broodwar->getFrameCount() << '\n';
    }

    if (ownMinerals != CAMPOS_ESPERADOS_BASE)
    {
        BWAPI::Broodwar->printf("MineralTracker: umbral de %.0f px clasifico %d campos propios; se esperaban %d.",
                                RADIO_BASE_PROPIA_PIXELES, ownMinerals, CAMPOS_ESPERADOS_BASE);
    }
}
