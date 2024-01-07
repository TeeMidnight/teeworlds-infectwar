#include <engine/shared/config.h>
#include <engine/shared/protocol.h>

#include <game/generated/protocol.h>

#include <game/server/gamecontext.h>
#include <game/server/entities/turret.h>
#include <game/server/entities/pickup.h>

#include "infectwar.h"

#include <string>

#define MIN_PLAYERS 2

CGameControllerInfectWar::CGameControllerInfectWar(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "InfectWar Catch"; // "Catch" make it color gold in DDNet

	m_GameFlags = 0;

	m_NeedInfectNum = 0;

	m_LastTurretID = MAX_CLIENTS;
}

bool CGameControllerInfectWar::IsBuildTime()
{
	// 90 = 1min30s
	return (Server()->Tick() <= m_RoundStartTick + 90 * Server()->TickSpeed());
}

bool CGameControllerInfectWar::IsLeaveTime()
{
	// 90 = 1min30s
	return (Server()->Tick() >= m_RoundStartTick + 60 * g_Config.m_SvTimelimit * Server()->TickSpeed());
}

int CGameControllerInfectWar::LastBuildTick()
{
	return (m_RoundStartTick + 90 * Server()->TickSpeed() - Server()->Tick());
}

int CGameControllerInfectWar::NumPlayers() const
{
	int Num = 0;
	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
			continue;
		Num ++;
	}
	return Num;
}

int CGameControllerInfectWar::NumInfects() const
{
	int Num = 0;
	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_RED)
			Num ++;
	}
	return Num;
}

int CGameControllerInfectWar::NumHumans() const
{
	int Num = 0;
	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_BLUE)
			Num ++;
	}
	return Num;
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
			Buffer.append(LastTime, '#');
			GameServer()->SendBroadcastFormat(i, "[%s]", 2, BCLAYER_NINJABAR, Buffer.c_str());
		}
	}
}

void CGameControllerInfectWar::DoInfection()
{
	if(!m_NeedInfectNum) // first time infection
	{
		int Phase = NumPlayers() / 4;
		switch (Phase)
		{
			case 0: m_NeedInfectNum = 1; break;
			case 1: m_NeedInfectNum = 2; break;
			case 2:
			case 3: 
			case 4: m_NeedInfectNum = 3; break;
			case 5: 
			case 6: m_NeedInfectNum = 4; break;
			case 7: 
			case 8: m_NeedInfectNum = 5; break;
			case 9:
			case 10: 
			case 11: m_NeedInfectNum = 6; break;
			default: m_NeedInfectNum = 7; break;
		}
	}

	int NeedInfect = m_NeedInfectNum; // maybe in build time there are any infects
	std::vector<int> m_HumansID;

	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_RED)
		{
			NeedInfect --;
			continue;
		}
		m_HumansID.push_back(i);
	}

	for(int i = 0; i < NeedInfect; i ++)
	{
		int RandomID = random_int(0, m_HumansID.size() - 1);

		GameServer()->m_apPlayers[m_HumansID[RandomID]]->SetTeam(TEAM_RED, false); // infect
		// send server chat
		GameServer()->SendChatFormat(-1, CGameContext::CHAT_ALL, _("'%s' been a infect!"), Server()->ClientName(m_HumansID[RandomID]));
	}

	return;
}

bool CGameControllerInfectWar::OnEntity(int Index, vec2 Pos)
{
	int Type = -1;
	int SubType = 0;

	if(Index == ENTITY_SPAWN)
		m_aaSpawnPoints[0][m_aNumSpawnPoints[0]++] = Pos;
	else if(Index == ENTITY_SPAWN_RED)
		m_aaSpawnPoints[1][m_aNumSpawnPoints[1]++] = Pos;
	else if(Index == ENTITY_SPAWN_BLUE)
		m_aaSpawnPoints[2][m_aNumSpawnPoints[2]++] = Pos;
	else if(Index == ENTITY_ARMOR_1)
		Type = POWERUP_ARMOR;
	else if(Index == ENTITY_HEALTH_1)
		Type = POWERUP_HEALTH;
	else if(Index == ENTITY_WEAPON_SHOTGUN)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_SHOTGUN;
	}
	else if(Index == ENTITY_WEAPON_GRENADE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_GRENADE;
	}
	else if(Index == ENTITY_WEAPON_RIFLE)
	{
		Type = POWERUP_WEAPON;
		SubType = WEAPON_RIFLE;
	}
	else if(Index == ENTITY_POWERUP_NINJA && g_Config.m_SvPowerups)
	{
		Type = POWERUP_NINJA;
		SubType = WEAPON_NINJA;
	}else if(Index == ENTITY_FLAGSTAND_BLUE)
	{
		new CTurret(&GameServer()->m_World, Pos, rand()%WEAPON_NINJA, -1, m_LastTurretID ++);
		return true;
	}

	if(Type != -1)
	{
		CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, SubType);
		pPickup->m_Pos = Pos;
		pPickup->m_Gravity = false;
		pPickup->m_NextRound = true;
		pPickup->m_OneTime = true; // only pick one time
		return true;
	}
	return false;
}

bool CGameControllerInfectWar::IsFriendlyFire(int ClientID1, int ClientID2)
{
	if(ClientID1 < 0 || ClientID2 < 0)
		return false;

	if(ClientID1 == ClientID2) // true, for turret
		return true;

	if(!GameServer()->m_apPlayers[ClientID1] || !GameServer()->m_apPlayers[ClientID2])
		return false;

	if(GameServer()->m_apPlayers[ClientID1]->GetTeam() == GameServer()->m_apPlayers[ClientID2]->GetTeam())
		return true;

	return false;
}

bool CGameControllerInfectWar::PlayerPickable(class CCharacter *pChr)
{
	if(!IGameController::PlayerPickable(pChr))
		return false;

	if(pChr->GetPlayer()->GetTeam() == TEAM_RED) // infect can't pick up!
		return false;

	return true;
}

const char *CGameControllerInfectWar::GetTeamName(int Team)
{
	if(Team == TEAM_RED)
		return "infects";
	else if(Team == TEAM_BLUE)
		return "humans";

	return "spectators";
}

int CGameControllerInfectWar::ClampTeam(int Team)
{
	if(Team < 0)
		return TEAM_SPECTATORS;
	return Team&1;
}

int CGameControllerInfectWar::GetShowTeam(int Team)
{
	if(Team >= 0)
		return TEAM_RED;
	return TEAM_SPECTATORS;
}

int CGameControllerInfectWar::GetAutoTeam(int NotThisID)
{
	if(IsBuildTime()) // I'd better, build towers so the dead can't eat my mind!~  
		return TEAM_BLUE;
	return TEAM_RED; // Anytime, it's zombie time!~
	// Back and forth in zombie time!~
}

int CGameControllerInfectWar::OnCharacterDeath(class CCharacter *pVictim, int Killer, int Weapon)
{
	if(Killer >= 0)
	{
		CPlayer *pKiller = GameServer()->m_apPlayers[Killer];
		// do scoreing
		if(!pKiller || Weapon == WEAPON_GAME)
			return 0;
		if(pKiller == pVictim->GetPlayer())
			pVictim->GetPlayer()->m_Score--; // suicide
		else
		{
			if(pKiller->GetTeam() == TEAM_RED)
				pKiller->m_Score += 5;
			else
				pKiller->m_Score++;
			pVictim->GetPlayer()->m_DeathNum ++;
		}
		if(Weapon == WEAPON_SELF)
			pVictim->GetPlayer()->m_RespawnTick = Server()->Tick()+Server()->TickSpeed()*3.0f;
	}

	if(!IsBuildTime()) // infect
	{	
		pVictim->GetPlayer()->SetTeamForce(TEAM_RED);
	}

	// drop weapons
	vec2 Direction = vec2(0, 1);
	int DirectionChoose = 0;
	for(int i = WEAPON_SHOTGUN; i < WEAPON_NINJA; i ++)
	{
		if(pVictim->GotWeapon(i))
		{
			int ShotSpread = 2;
			float Spreading[] = {-0.25f, -0.125f, 0, 0.125f, 0.25f};
			float a = GetAngle(Direction);
			a += Spreading[DirectionChoose];
			float v = 1-(absolute(DirectionChoose)/(float)ShotSpread);
			float Speed = mix((float)0.8f, 1.0f, v);
			CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_WEAPON, i);
			pPickup->m_Pos = pVictim->m_Pos;
			pPickup->m_StartPos = pVictim->m_Pos;
			pPickup->m_Direction = vec2(cosf(a), sinf(a)) * Speed;
			pPickup->m_Gravity = true;
			pPickup->m_NextRound = false;
			pPickup->m_OneTime = true; // only pick one time
			pPickup->m_StartTick = Server()->Tick();
			DirectionChoose ++;
		}
	}

	if(pVictim->GetPlayer()->GetTeam() == TEAM_RED)
	{
		CPickup *pPickup = new CPickup(&GameServer()->m_World, random_int(POWERUP_HEALTH, POWERUP_ARMOR));
		pPickup->m_Pos = pVictim->m_Pos;
		pPickup->m_StartPos = pVictim->m_Pos;
		pPickup->m_Direction = vec2(0, 1);
		pPickup->m_Gravity = true;
		pPickup->m_NextRound = false;
		pPickup->m_OneTime = true; // only pick one time
		pPickup->m_StartTick = Server()->Tick();
	}

	return 0;
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
	pGameInfoExObj->m_Flags = GAMEINFOFLAG_ENTITIES_DDRACE | GAMEINFOFLAG_PREDICT_VANILLA | GAMEINFOFLAG_GAMETYPE_VANILLA;
	pGameInfoExObj->m_Flags2 = GAMEINFOFLAG2_NO_SKIN_CHANGE_FOR_FROZEN | GAMEINFOFLAG2_HUD_HEALTH_ARMOR | GAMEINFOFLAG2_HUD_AMMO | GAMEINFOFLAG2_HUD_DDRACE | GAMEINFOFLAG2_NO_WEAK_HOOK | GAMEINFOFLAG2_ALLOW_X_SKINS;
	pGameInfoExObj->m_Version = GAMEINFO_CURVERSION;
}

void CGameControllerInfectWar::Tick()
{
	IGameController::Tick();

	if(NumPlayers() < MIN_PLAYERS)
	{
		m_RoundStartTick ++;
	}

	if(LastBuildTick() > 0) // sleep infect
	{
		for(int i = 0; i < MAX_CLIENTS; i ++)
		{	
			if(!GameServer()->m_apPlayers[i])
				continue;
			if(GameServer()->m_apPlayers[i]->GetTeam() != TEAM_RED)
				continue;
			GameServer()->m_apPlayers[i]->SetTeamForce(TEAM_SPECTATORS); // sleep
		}
	}else if(LastBuildTick() == 0)
	{
		for(int i = 0; i < MAX_CLIENTS; i ++)
		{	
			if(!GameServer()->m_apPlayers[i])
				continue;
			if(GameServer()->m_apPlayers[i]->GetTeam() != TEAM_RED)
				continue;
			GameServer()->m_apPlayers[i]->SetTeamForce(TEAM_RED); // wake up
		}
	}

	DoNinjaBar();

	if(!IsBuildTime())
	{
		DoWincheck();
	}
}

void CGameControllerInfectWar::DoWincheck()
{
	if(m_GameOverTick == -1 && !m_Warmup && !GameServer()->m_World.m_ResetRequested)
	{
		// check humans win
		if((g_Config.m_SvTimelimit > 0 && (Server()->Tick()-m_RoundStartTick) >= g_Config.m_SvTimelimit*Server()->TickSpeed()*60))
		{
			EndRound();
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, _("The humans survived!"));
		}
		else if(!NumHumans())
		{
			EndRound();
			GameServer()->SendChat(-1, CGameContext::CHAT_ALL, _("Every time is zombie time...."));
		}else if(NumInfects() < m_NeedInfectNum)
		{
			DoInfection();
		}
	}
}

void CGameControllerInfectWar::StartRound()
{
	IGameController::StartRound();

	m_NeedInfectNum = 0; // clear infect num

	// destroy all turret
	for(CTurret *pEnt = (CTurret *) GameServer()->m_World.FindFirst(CGameWorld::ENTTYPE_TURRET); pEnt; pEnt = (CTurret *) pEnt->TypeNext())
	{
		GameServer()->m_World.DestroyEntity(pEnt);
	}

	for(int i = 0; i < MAX_CLIENTS; i ++)
	{
		if(!GameServer()->m_apPlayers[i])
			continue;
		if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
			continue;

		GameServer()->m_apPlayers[i]->SetTeamForce(TEAM_BLUE);
		(*Server()->GetIdMap(i)).clear();
	}
	m_LastTurretID = MAX_CLIENTS;
	
	DoInfection();
}

void CGameControllerInfectWar::OnClientConnected(int ClientID)
{
	IGameController::OnClientConnected(ClientID);

	if(NumPlayers() < MIN_PLAYERS)
	{
		GameServer()->SendBroadcastFormat(ClientID, _("At least %d players for start game "), 150, BCLAYER_SYSTEM, MIN_PLAYERS);
	}else if(NumPlayers() == MIN_PLAYERS)
	{
		StartRound();
	}
}

void CGameControllerInfectWar::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	int Health = 10;
	if(pChr->GetPlayer()->GetTeam() == TEAM_RED)
		Health += pChr->GetPlayer()->m_DeathNum * 2; // stronger infect
	else
		pChr->IncreaseArmor(2);
	pChr->IncreaseHealth(Health, true);

	// give default weapons
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	if(pChr->GetPlayer()->GetTeam() == TEAM_BLUE) // give humans gun
		pChr->GiveWeapon(WEAPON_GUN, 10);
}

void CGameControllerInfectWar::OnCharacterDamage(class CCharacter *pChr, int From, int& Dmg)
{
	if(From < 0)
		return;

	CPlayer *pFrom = GameServer()->m_apPlayers[From];

	if(pFrom && pFrom->GetTeam() == TEAM_RED)
	{
		if(pChr)
			Dmg = 10; // kill no armor humans
		else
			Dmg += 1 + pFrom->m_DeathNum / 5; 
	}
}

void CGameControllerInfectWar::OnPlayerInfoChange(CPlayer *pPlayer)
{
	const int InfectColor = 3866368;
	if(pPlayer->GetTeam() == TEAM_RED || pPlayer->GetTeam() == TEAM_SPECTATORS) // red = infected
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 1;
		if(pPlayer->GetTeam() == TEAM_RED || pPlayer->GetCharacter()) // fake spec is infect
		{
			pPlayer->m_TeeInfos.m_ColorBody = InfectColor;
			pPlayer->m_TeeInfos.m_ColorFeet = InfectColor;
		}
		else
		{
			pPlayer->m_TeeInfos.m_ColorBody = 12895054;
			pPlayer->m_TeeInfos.m_ColorFeet = 12895054;
		}
	}else
	{
		pPlayer->m_TeeInfos.m_UseCustomColor = 0;
	}
}

void CGameControllerInfectWar::OnPlayerEmoticon(CPlayer *pPlayer, int Emoticon)
{
	if(pPlayer->GetTeam() != TEAM_BLUE) // only humans can place turret
		return;
	
	if(!pPlayer->GetCharacter()) // where is your character?
		return;

	if(pPlayer->GetCharacter()->ActiveWeapon() == WEAPON_NINJA)
	{
		return; // now don't place your ninja;
	}

	if(Emoticon != EMOTICON_GHOST && Emoticon != EMOTICON_QUESTION)
		return;
	
	// create turret
	int Weapon = pPlayer->GetCharacter()->ActiveWeapon();
	if(pPlayer->GetCharacter()->GotWeapon(Weapon))
	{
		int NeedArmor = Weapon;
		if(Weapon == WEAPON_HAMMER) // build a armor/health/hammer turret
		{
			if(Emoticon == EMOTICON_QUESTION)
				NeedArmor = 4;
			else 
				NeedArmor = 2;
		}
		if(pPlayer->GetCharacter()->GetArmor() < NeedArmor)
		{
			GameServer()->SendChatTargetFormat(pPlayer->GetCID(), _("You need %d armors to build it"), NeedArmor);
			return;
		}

		pPlayer->GetCharacter()->IncreaseArmor(-NeedArmor);

		new CTurret(&GameServer()->m_World, pPlayer->GetCharacter()->m_Pos, 
			Weapon, pPlayer->GetCID(), m_LastTurretID ++, Emoticon == EMOTICON_QUESTION);
		
		if(Weapon != WEAPON_HAMMER && Weapon != WEAPON_GUN)
			pPlayer->GetCharacter()->RemoveWeapon(Weapon);
		pPlayer->GetCharacter()->SetWeapon(WEAPON_HAMMER);
	}
}

void CGameControllerInfectWar::OnPlayerJoinTeam(CPlayer *pPlayer, int JoinTeam)
{
	if(pPlayer->GetTeam() == TEAM_SPECTATORS)
	{
		pPlayer->SetTeam(IsBuildTime() ? TEAM_BLUE : TEAM_RED);
	}else
	{
		pPlayer->SetTeam(TEAM_SPECTATORS);
	}
}