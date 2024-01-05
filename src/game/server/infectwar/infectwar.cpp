#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/entities/turret.h>

#include "infectwar.h"

#include <string>

#define MIN_PLAYERS 3

CGameControllerInfectWar::CGameControllerInfectWar(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "InfectWar Catch"; // "Catch" make it color gold in DDNet

	m_GameFlags = 0;

	m_LastTurretID = MAX_CLIENTS;
}

bool CGameControllerInfectWar::IsBuildTime()
{
	// 90 = 1min30s
	return (Server()->Tick() <= m_RoundStartTick + 90 * Server()->TickSpeed());
}

int CGameControllerInfectWar::LastBuildTick()
{
	return (m_RoundStartTick + 90 * Server()->TickSpeed() - Server()->Tick());
}

bool CGameControllerInfectWar::IsLeaveTime()
{
	// 90 = 1min30s
	return (Server()->Tick() >= m_RoundStartTick + 60 * g_Config.m_SvTimelimit * Server()->TickSpeed());
}

void CGameControllerInfectWar::DoNinjaBar()
{
	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(GameServer()->GetPlayerChar(i) && GameServer()->GetPlayerChar(i)->GotWeapon(WEAPON_NINJA))
		{
			int NinjaTime = g_pData->m_Weapons.m_Ninja.m_Duration * Server()->TickSpeed() / 1000;
			int LastTime = max(1, (NinjaTime + GameServer()->GetPlayerChar(i)->NinjaStatus()->m_ActivationTick - Server()->Tick()) / Server()->TickSpeed());
			
			std::string Buffer;
			Buffer += "[";
			Buffer.append(LastTime, '#');
			Buffer += "]";
			GameServer()->SendBroadcast(Buffer.c_str(), i, 2, BCLAYER_NINJABAR);
		}
	}
}

bool CGameControllerInfectWar::OnEntity(int Index, vec2 Pos)
{
	if(IGameController::OnEntity(Index, Pos))
		return true;

	if(Index == ENTITY_FLAGSTAND_BLUE)
	{
		new CTurret(&GameServer()->m_World, Pos, rand()%WEAPON_NINJA, -1, m_LastTurretID ++);
		return true;
	}

	return false;
}

int CGameControllerInfectWar::ClampTeam(int Team)
{
	if(Team < 0)
		return TEAM_SPECTATORS;
	return Team&1;
}

int CGameControllerInfectWar::GetAutoTeam(int NotThisID)
{
	if(IsBuildTime()) // I'd better, build towers so the dead can't eat my mind!~  
		return TEAM_BLUE;
	return TEAM_RED; // Anytime, it's zombie time!~
	// Back and forth in zombie time!~
}

void CGameControllerInfectWar::Snap(int SnappingClient)
{
	SnapEx(SnappingClient);

	CNetObj_GameInfo *pGameInfoObj = (CNetObj_GameInfo *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
	if(!pGameInfoObj)
		return;

	pGameInfoObj->m_GameFlags = 0;
	pGameInfoObj->m_GameStateFlags = 0;
	if(m_GameOverTick != -1)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if(IsLeaveTime())
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	if(GameServer()->m_World.m_Paused)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	pGameInfoObj->m_RoundStartTick = m_RoundStartTick;
	pGameInfoObj->m_WarmupTimer = IsBuildTime() ? LastBuildTick() : 0;

	pGameInfoObj->m_ScoreLimit = 0;
	pGameInfoObj->m_TimeLimit = g_Config.m_SvTimelimit;

	pGameInfoObj->m_RoundNum = (str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap : 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount+1;
}

void CGameControllerInfectWar::SnapEx(int SnappingClient)
{
	CNetObj_GameInfoEx *pGameInfoExObj = (CNetObj_GameInfoEx *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFOEX, 0, sizeof(CNetObj_GameInfoEx));
	if(!pGameInfoExObj)
		return;
	pGameInfoExObj->m_Flags = GAMEINFOFLAG_ENTITIES_VANILLA | GAMEINFOFLAG_PREDICT_VANILLA | GAMEINFOFLAG_GAMETYPE_VANILLA;
	pGameInfoExObj->m_Flags2 = GAMEINFOFLAG2_NO_SKIN_CHANGE_FOR_FROZEN | GAMEINFOFLAG2_HUD_HEALTH_ARMOR | GAMEINFOFLAG2_HUD_AMMO | GAMEINFOFLAG2_HUD_DDRACE | GAMEINFOFLAG2_NO_WEAK_HOOK | GAMEINFOFLAG2_ALLOW_X_SKINS;
	pGameInfoExObj->m_Version = GAMEINFO_CURVERSION;
}

void CGameControllerInfectWar::Tick()
{
	IGameController::Tick();

	if(GameServer()->NumPlayers() < MIN_PLAYERS)
	{
		m_RoundStartTick ++;
	}

	DoNinjaBar();
	DoWincheck();
}


void CGameControllerInfectWar::DoWincheck()
{
}

void CGameControllerInfectWar::OnClientConnected(int ClientID)
{
	IGameController::OnClientConnected(ClientID);

	if(GameServer()->NumPlayers() < MIN_PLAYERS)
	{
		GameServer()->SendBroadcastFormat(_("At least %d players for start game "), ClientID, 150, 0, MIN_PLAYERS);
	}
}