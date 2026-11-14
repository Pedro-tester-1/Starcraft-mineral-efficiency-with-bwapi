#include "WinPredictor.h"

namespace
{
    const double ARMY_WEIGHT = 0.35;
    const double RESOURCE_WEIGHT = 0.20;
    const double BUILDING_WEIGHT = 0.15;
    const double TECH_WEIGHT = 0.10;
    const double MAP_WEIGHT = 0.10;
    const double KILL_WEIGHT = 0.10;
}

/** Prepara el predictor sin asignar ventaja inicial a ningún jugador. */
WinPredictor::WinPredictor()
    : m_lastUpdateFrame(-24)
{
}

/** Calcula seis variables, las normaliza entre jugadores y suma sus pesos. */
std::map<int, double> WinPredictor::calculate() const
{
    std::map<int, double> armies, resources, buildings, technologies, mapControl, kills;
    std::vector<BWAPI::Player> players;
    for (auto& player : BWAPI::Broodwar->getPlayers())
    {
        if (!player || player->isNeutral()) { continue; }
        players.push_back(player);
        armies[player->getID()] = 0.0;
        resources[player->getID()] = player->minerals() + player->gas();
        buildings[player->getID()] = 0.0;
        technologies[player->getID()] = 0.0;
        mapControl[player->getID()] = 0.0;
        std::map<int, double>::const_iterator killed = m_killedValue.find(player->getID());
        kills[player->getID()] = killed == m_killedValue.end() ? 0.0 : killed->second;

        for (auto& unit : player->getUnits())
        {
            if (!unit || !unit->exists()) { continue; }
            const BWAPI::UnitType type = unit->getType();
            if (!type.isBuilding() && !type.isWorker())
            {
                armies[player->getID()] += type.mineralPrice() + 1.5 * type.gasPrice();
            }
            if (type.isBuilding()) { buildings[player->getID()] += 1.0; }
            if (type.isResourceDepot()) { mapControl[player->getID()] += 1.0; }
        }
        for (auto& upgrade : BWAPI::UpgradeTypes::allUpgradeTypes())
        {
            technologies[player->getID()] += player->getUpgradeLevel(upgrade);
        }
        for (auto& tech : BWAPI::TechTypes::allTechTypes())
        {
            if (player->hasResearched(tech)) { technologies[player->getID()] += 1.0; }
        }
    }

    std::map<int, double> totals;
    double armyTotal = 0.0, resourceTotal = 0.0, buildingTotal = 0.0;
    double techTotal = 0.0, mapTotal = 0.0, killTotal = 0.0;
    for (size_t i = 0; i < players.size(); ++i)
    {
        const int id = players[i]->getID();
        armyTotal += armies[id]; resourceTotal += resources[id]; buildingTotal += buildings[id];
        techTotal += technologies[id]; mapTotal += mapControl[id]; killTotal += kills[id];
    }
    for (size_t i = 0; i < players.size(); ++i)
    {
        const int id = players[i]->getID();
        totals[id] = ARMY_WEIGHT * (armyTotal > 0 ? armies[id] / armyTotal : 0.0)
            + RESOURCE_WEIGHT * (resourceTotal > 0 ? resources[id] / resourceTotal : 0.0)
            + BUILDING_WEIGHT * (buildingTotal > 0 ? buildings[id] / buildingTotal : 0.0)
            + TECH_WEIGHT * (techTotal > 0 ? technologies[id] / techTotal : 0.0)
            + MAP_WEIGHT * (mapTotal > 0 ? mapControl[id] / mapTotal : 0.0)
            + KILL_WEIGHT * (killTotal > 0 ? kills[id] / killTotal : 0.0);
    }
    double scoreTotal = 0.0;
    for (std::map<int, double>::const_iterator it = totals.begin(); it != totals.end(); ++it) { scoreTotal += it->second; }
    if (scoreTotal > 0.0)
    {
        for (std::map<int, double>::iterator it = totals.begin(); it != totals.end(); ++it) { it->second /= scoreTotal; }
    }
    else if (!players.empty())
    {
        for (size_t i = 0; i < players.size(); ++i) { totals[players[i]->getID()] = 1.0 / players.size(); }
    }
    return totals;
}

/** Guarda un punto de predicción cada 24 frames y actualiza el dibujo. */
void WinPredictor::onFrame()
{
    const int frame = BWAPI::Broodwar->getFrameCount();
    if (frame - m_lastUpdateFrame < 24) { return; }
    m_lastUpdateFrame = frame;
    m_currentProbabilities = calculate();
    Snapshot snapshot;
    snapshot.frame = frame;
    snapshot.probabilities = m_currentProbabilities;
    m_history.push_back(snapshot);
}

/** Suma el costo de la unidad enemiga destruida al registro del jugador propio. */
void WinPredictor::onUnitDestroyed(BWAPI::Unit unit)
{
    if (!unit || !BWAPI::Broodwar->self() || !unit->getPlayer()) { return; }
    if (!BWAPI::Broodwar->self()->isEnemy(unit->getPlayer())) { return; }
    const BWAPI::UnitType type = unit->getType();
    m_killedValue[BWAPI::Broodwar->self()->getID()] += type.mineralPrice() + 1.5 * type.gasPrice();
}

/** Dibuja barras que comparan visualmente la probabilidad actual por jugador. */
void WinPredictor::draw() const
{
    int y = 12;
    for (auto& player : BWAPI::Broodwar->getPlayers())
    {
        if (!player || player->isNeutral()) { continue; }
        std::map<int, double>::const_iterator it = m_currentProbabilities.find(player->getID());
        const double probability = it == m_currentProbabilities.end() ? 0.0 : it->second;
        const int width = static_cast<int>(probability * 200.0);
        BWAPI::Broodwar->drawBoxScreen(10, y + 14, 210, y + 26, BWAPI::Colors::Grey, true);
        BWAPI::Broodwar->drawBoxScreen(10, y + 14, 10 + width, y + 26, BWAPI::Colors::Green, true);
        BWAPI::Broodwar->drawTextScreen(10, y, "%s: %.1f%%", player->getName().c_str(), probability * 100.0);
        y += 32;
    }
}

/** Exporta la serie temporal en formato largo, incluyendo el ganador real. */
void WinPredictor::saveCsv(const std::string& path, int winnerId) const
{
    std::ofstream out(path.c_str());
    if (!out) { return; }
    out << "frame,player_id,probabilidad,ganador_id\n";
    for (size_t i = 0; i < m_history.size(); ++i)
    {
        for (std::map<int, double>::const_iterator it = m_history[i].probabilities.begin();
            it != m_history[i].probabilities.end(); ++it)
        {
            out << m_history[i].frame << ',' << it->first << ',' << it->second << ',' << winnerId << '\n';
        }
    }
}
