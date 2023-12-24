#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>

#include <game/server/gamecontext.h>

#include <string>

#include <engine/shared/config.h>

#include "infectwar.h"

CGameControllerInfectWar::CGameControllerInfectWar(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "InfectWar Catch"; // "Catch" make it color gold in DDNet

	m_GameFlags = 0;
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

void CGameControllerInfectWar::Snap(int SnappingClient)
{
	CNetObj_GameInfo *pGameInfoObj = (CNetObj_GameInfo *)Server()->SnapNewItem(NETOBJTYPE_GAMEINFO, 0, sizeof(CNetObj_GameInfo));
	if(!pGameInfoObj)
		return;

	pGameInfoObj->m_GameFlags = GAMEFLAG_TEAMS;
	pGameInfoObj->m_GameStateFlags = 0;
	if(m_GameOverTick != -1)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_GAMEOVER;
	if(m_SuddenDeath)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_SUDDENDEATH;
	if(GameServer()->m_World.m_Paused)
		pGameInfoObj->m_GameStateFlags |= GAMESTATEFLAG_PAUSED;
	pGameInfoObj->m_RoundStartTick = m_RoundStartTick;
	pGameInfoObj->m_WarmupTimer = GameServer()->m_World.m_Paused ? m_UnpauseTimer : m_Warmup;

	pGameInfoObj->m_ScoreLimit = 0;
	pGameInfoObj->m_TimeLimit = g_Config.m_SvTimelimit;

	pGameInfoObj->m_RoundNum = (str_length(g_Config.m_SvMaprotation) && g_Config.m_SvRoundsPerMap) ? g_Config.m_SvRoundsPerMap : 0;
	pGameInfoObj->m_RoundCurrent = m_RoundCount+1;
}

void CGameControllerInfectWar::Tick()
{
	DoNinjaBar();
	IGameController::Tick();
}
