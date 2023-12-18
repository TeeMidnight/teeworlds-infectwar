#include <engine/shared/protocol.h>
#include <game/generated/protocol.h>

#include <game/server/gamecontext.h>

#include <string>

#include "infectwar.h"

CGameControllerInfectWar::CGameControllerInfectWar(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "InfectWar Catch"; // "Catch" make it color gold in DDNet

	m_GameFlags = GAMEFLAG_TEAMS;
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

void CGameControllerInfectWar::Tick()
{
	DoNinjaBar();
	IGameController::Tick();
}
