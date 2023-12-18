#ifndef GAME_SERVER_GAMEMODES_INFECTWAR_H
#define GAME_SERVER_GAMEMODES_INFECTWAR_H
#include <game/server/gamecontroller.h>

class CGameControllerInfectWar : public IGameController
{
public:
	CGameControllerInfectWar(class CGameContext *pGameServer);

	void DoNinjaBar();
	void Tick() override;
};
#endif
