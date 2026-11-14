#include "WinPredictor.h"

namespace
{
    const double ARMY_WEIGHT = 0.35;
    const double RESOURCE_WEIGHT = 0.20;
    const double BUILDING_WEIGHT = 0.15;
    const double TECH_WEIGHT = 0.10;
    const double MAP_WEIGHT = 0.10;
    const double KILL_WEIGHT = 0.10;

    // Posiciones y tamaños del panel para mantener todas las barras alineadas.
    const int PANEL_X = 8;
    const int PANEL_Y = 8;
    const int PANEL_WIDTH = 300;
    const int PANEL_TOP_HEIGHT = 32;
    const int ROW_HEIGHT = 25;
    const int PLAYER_NAME_X = 18;
    const int BAR_X = 112;
    const int BAR_WIDTH = 145;
    const int BAR_HEIGHT = 10;
    const int PERCENT_X = 265;
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

/** Dibuja el nombre, la barra de color y el porcentaje de un jugador. */
void WinPredictor::drawPlayerBar(BWAPI::Player player, double probability, int y) const
{
    const int barTop = y + 5;
    double boundedProbability = probability;
    if (boundedProbability < 0.0) { boundedProbability = 0.0; }
    if (boundedProbability > 1.0) { boundedProbability = 1.0; }
    const int filledWidth = static_cast<int>(boundedProbability * BAR_WIDTH);

    BWAPI::Broodwar->drawTextScreen(PLAYER_NAME_X, y, "%s", player->getName().c_str());
    BWAPI::Broodwar->drawBoxScreen(BAR_X, barTop, BAR_X + BAR_WIDTH, barTop + BAR_HEIGHT,
        BWAPI::Colors::Grey, true);
    BWAPI::Broodwar->drawBoxScreen(BAR_X, barTop, BAR_X + filledWidth, barTop + BAR_HEIGHT,
        player->getColor(), true);
    BWAPI::Broodwar->drawTextScreen(PERCENT_X, y, "%.1f%%", boundedProbability * 100.0);
}

/** Dibuja el panel de fondo y una barra alineada por cada jugador. */
void WinPredictor::draw() const
{
    int playerCount = 0;
    for (auto& player : BWAPI::Broodwar->getPlayers())
    {
        if (player && !player->isNeutral()) { playerCount++; }
    }
    const int panelHeight = PANEL_TOP_HEIGHT + playerCount * ROW_HEIGHT;
    BWAPI::Broodwar->drawBoxScreen(PANEL_X, PANEL_Y, PANEL_X + PANEL_WIDTH,
        PANEL_Y + panelHeight, BWAPI::Colors::Black, true);
    BWAPI::Broodwar->drawBoxScreen(PANEL_X, PANEL_Y, PANEL_X + PANEL_WIDTH,
        PANEL_Y + panelHeight, BWAPI::Colors::Grey, false);
    BWAPI::Broodwar->drawTextScreen(PANEL_X + 10, PANEL_Y + 5,
        "%cProbabilidad de victoria", BWAPI::Text::White);

    int y = PANEL_Y + PANEL_TOP_HEIGHT;
    for (auto& player : BWAPI::Broodwar->getPlayers())
    {
        if (!player || player->isNeutral()) { continue; }
        std::map<int, double>::const_iterator it = m_currentProbabilities.find(player->getID());
        const double probability = it == m_currentProbabilities.end() ? 0.0 : it->second;
        drawPlayerBar(player, probability, y);
        y += ROW_HEIGHT;
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
