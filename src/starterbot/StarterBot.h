#pragma once

#include "MapTools.h"
#include "MineralTracker.h"
#include "WinPredictor.h"

#include <BWAPI.h>

/** Coordina los eventos del juego, las mediciones y las acciones actuales del bot. */
class StarterBot
{
    MapTools m_mapTools;
    MineralTracker m_mineralTracker;
    WinPredictor m_winPredictor;

public:

    /** Prepara las herramientas que el bot usa durante la partida. */
    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    /** Envía trabajadores inactivos al mineral más cercano para mantenerlos ocupados. */
    void sendIdleWorkersToMinerals();
    /** Produce trabajadores hasta alcanzar la cantidad objetivo configurada. */
    void trainAdditionalWorkers();
    /** Construye suministro cuando el disponible está por agotarse. */
    void buildAdditionalSupply();
    /** Dibuja información útil para observar el estado actual del bot. */
    void drawDebugInformation();

    // functions that are triggered by various BWAPI events from main.cpp
    /** Configura opciones de BWAPI y prepara las herramientas al comenzar. */
	void onStart();
    /** Actualiza métricas, predicciones y comportamiento una vez por frame. */
	void onFrame();
    /** Guarda los CSV y registra el resultado conocido al terminar la partida. */
	void onEnd(bool isWinner);
    /** Informa al predictor cuando se destruye una unidad enemiga. */
	void onUnitDestroy(BWAPI::Unit unit);
    /** Recibe el cambio de tipo que BWAPI notifica para una unidad. */
	void onUnitMorph(BWAPI::Unit unit);
    /** Procesa comandos escritos durante la partida, como mostrar el mapa. */
	void onSendText(std::string text);
    /** Recibe el evento de creación de una unidad. */
	void onUnitCreate(BWAPI::Unit unit);
    /** Recibe el evento de finalización de una unidad. */
	void onUnitComplete(BWAPI::Unit unit);
    /** Recibe el evento de aparición de una unidad en el mapa. */
	void onUnitShow(BWAPI::Unit unit);
    /** Recibe el evento de ocultamiento de una unidad por la niebla. */
	void onUnitHide(BWAPI::Unit unit);
    /** Recibe el evento de cambio de dueño de una unidad. */
	void onUnitRenegade(BWAPI::Unit unit);
};
