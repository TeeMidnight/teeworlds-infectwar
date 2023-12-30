#ifndef GAME_SERVER_GAMEMODES_INFECTWAR_H
#define GAME_SERVER_GAMEMODES_INFECTWAR_H
#include <game/server/gamecontroller.h>

class CGameControllerInfectWar : public IGameController
{
public:
	CGameControllerInfectWar(class CGameContext *pGameServer);

	// prepare phase
	bool IsBuildTime();
	int LastBuildTick();

	// leave phase
	bool IsLeaveTime();

	void DoNinjaBar();

	// overrides
	int ClampTeam(int Team) override;
	int GetAutoTeam(int NotThisID) override;
	void Snap(int SnappingClient) override;
	void Tick() override;
	void DoWincheck() override;
	void OnClientConnected(int ClientID) override;
};
#endif
