#ifndef GAME_SERVER_GAMEMODES_INFECTWAR_H
#define GAME_SERVER_GAMEMODES_INFECTWAR_H

#include <game/server/gamecontroller.h>

class CGameControllerInfectWar : public IGameController
{
	int m_LastTurretID;

	int m_NeedInfectNum;
public:
	CGameControllerInfectWar(class CGameContext *pGameServer);

	// phase
	bool IsBuildTime();
	bool IsLeaveTime();
	int LastBuildTick();

	int NumPlayers() const;
	int NumInfects() const;
	int NumHumans() const;


	void DoNinjaBar();

	void DoInfection();

	// overrides
	bool OnEntity(int Index, vec2 Pos) override;
	bool IsFriendlyFire(int ClientID1, int ClientID2) override;

	bool PlayerPickable(class CCharacter *pChr) override;

	const char* GetTeamName(int Team) override;

	int ClampTeam(int Team) override;
	int GetShowTeam(int Team) override;
	int GetAutoTeam(int NotThisID) override;
	int OnCharacterDeath(class CCharacter *pVictim, int Killer, int Weapon) override;
	
	void Snap(int SnappingClient) override;
	void SnapEx(int SnappingClient) override;

	void Tick() override;
	void DoWincheck() override;

	void StartRound() override;

	void OnClientConnected(int ClientID) override;
	void OnCharacterSpawn(class CCharacter *pChr) override;
	void OnCharacterDamage(class CCharacter *pChr, int From, int& Dmg) override;
	void OnPlayerInfoChange(class CPlayer *pPlayer) override;
	void OnPlayerEmoticon(class CPlayer *pPlayer, int Emoticon) override;
	void OnPlayerJoinTeam(class CPlayer *pPlayer, int JoinTeam) override;
};

#endif
