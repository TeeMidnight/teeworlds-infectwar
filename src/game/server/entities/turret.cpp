#include <game/server/gamecontext.h>
#include "turret.h"

#include "laser.h"
#include "projectile.h"
#include "pickup.h"

CTurret::CTurret(CGameWorld *pGameWorld, vec2 Pos, int Type, int Owner, int TurretID, bool Placer)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_TURRET)
{
	m_ProximityRadius = 32.0f;
	m_Radius = Type == WEAPON_HAMMER ? m_ProximityRadius * 1.5f : 320.0f;

    m_Pos = Pos;
    m_Type = Type;
    m_Owner = Owner;

    m_AttackTick = Server()->Tick();

	m_TurretID = TurretID;

	m_Health = 5 + Type * 5;

	m_Drop = false;
	m_Placer = Placer;

	GameWorld()->InsertEntity(this);
}

void CTurret::Destroy()
{
	if(m_Drop)
	{
		CPickup *pPickup = new CPickup(&GameServer()->m_World, POWERUP_WEAPON, m_Type);
		pPickup->m_Pos = m_Pos;
		pPickup->m_StartPos = m_Pos;
		pPickup->m_Direction = vec2(0, 1);
		pPickup->m_Gravity = true;
		pPickup->m_OneTime = true; // only pick one time
		pPickup->m_StartTick = Server()->Tick();
	}

	CEntity::Destroy();
}

void CTurret::Fire()
{
	vec2 Direction = normalize(m_TargetPos);
    vec2 ProjStartPos = m_Pos+Direction*28.0f*0.75f;

	switch(m_Type)
	{
		case WEAPON_HAMMER:
		{
			GameServer()->CreateSound(m_Pos, SOUND_HAMMER_FIRE);

			CCharacter *apEnts[MAX_CLIENTS];
			int Hits = 0;
			int Num = GameServer()->m_World.FindEntities(ProjStartPos, m_ProximityRadius*0.5f, (CEntity**)apEnts,
														MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);

			for (int i = 0; i < Num; ++i)
			{
				CCharacter *pTarget = apEnts[i];

				if(pTarget->GetPlayer()->GetTeam() == TEAM_BLUE)
					continue;

				if (GameServer()->Collision()->IntersectLine(ProjStartPos, pTarget->m_Pos, NULL, NULL))
					continue;

				// set his velocity to fast upward (for now)
				if(length(pTarget->m_Pos-ProjStartPos) > 0.0f)
					GameServer()->CreateHammerHit(pTarget->m_Pos-normalize(pTarget->m_Pos-ProjStartPos)*m_ProximityRadius*0.5f);
				else
					GameServer()->CreateHammerHit(ProjStartPos);

				vec2 Dir;
				if (length(pTarget->m_Pos - m_Pos) > 0.0f)
					Dir = normalize(pTarget->m_Pos - m_Pos);
				else
					Dir = vec2(0.f, -1.f);

				pTarget->TakeDamage(vec2(0.f, -1.f) + normalize(Dir + vec2(0.f, -1.1f)) * 10.0f, g_pData->m_Weapons.m_Hammer.m_pBase->m_Damage,
					m_Owner, m_Type);
				Hits++;
			}

		} break;

		case WEAPON_GUN:
		{
			new CProjectile(GameWorld(), WEAPON_GUN,
				m_Owner,
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_GunLifetime),
				1, 0, 0, -1, WEAPON_GUN);

			GameServer()->CreateSound(m_Pos, SOUND_GUN_FIRE);
		} break;

		case WEAPON_SHOTGUN:
		{
			int ShotSpread = 2;

			for(int i = -ShotSpread; i <= ShotSpread; ++i)
			{
				float Spreading[] = {-0.185f, -0.070f, 0, 0.070f, 0.185f};
				float a = GetAngle(Direction);
				a += Spreading[i+2];
				float v = 1-(absolute(i)/(float)ShotSpread);
				float Speed = mix((float)GameServer()->Tuning()->m_ShotgunSpeeddiff, 1.0f, v);
				new CProjectile(GameWorld(), WEAPON_SHOTGUN,
					m_Owner,
					ProjStartPos,
					vec2(cosf(a), sinf(a))*Speed,
					(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_ShotgunLifetime),
					1, 0, 0, -1, WEAPON_SHOTGUN);
			}

			GameServer()->CreateSound(m_Pos, SOUND_SHOTGUN_FIRE);
		} break;

		case WEAPON_GRENADE:
		{
			new CProjectile(GameWorld(), WEAPON_GRENADE,
				m_Owner,
				ProjStartPos,
				Direction,
				(int)(Server()->TickSpeed()*GameServer()->Tuning()->m_GrenadeLifetime),
				1, true, 0, SOUND_GRENADE_EXPLODE, WEAPON_GRENADE);

			GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
		} break;

		case WEAPON_RIFLE:
		{
			new CLaser(GameWorld(), m_Pos, Direction, GameServer()->Tuning()->m_LaserReach, m_Owner);
			GameServer()->CreateSound(m_Pos, SOUND_RIFLE_FIRE);
		} break;

		case WEAPON_NINJA: break; // no ninja, now
	}
    m_AttackTick = Server()->Tick();
}

void CTurret::TakeDamage(int From, int Dmg)
{
	GameServer()->m_pController->OnCharacterDamage(nullptr, From, Dmg);

	m_Health -= Dmg;

	if(m_Health <= 0)
	{
		GameServer()->CreateExplosion(m_Pos, m_Owner, m_Type, false);
		GameWorld()->DestroyEntity(this);

		m_Drop = false;

		GameServer()->SendChatTargetFormat(m_Owner, _("'%s' destroys your turret!"), Server()->ClientName(From));

		if(GameServer()->m_apPlayers[From])
			GameServer()->m_apPlayers[From]->m_Score ++;
	}
}

void CTurret::DoAttacker()
{
	CCharacter *apEnts[MAX_CLIENTS];
    int Num = GameServer()->m_World.FindEntities(m_Pos, m_Radius, (CEntity**)apEnts,
                                                MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER);
    bool Target = false;

    int ReloadTick = g_pData->m_Weapons.m_aId[m_Type].m_Firedelay * Server()->TickSpeed() / 500;

    for (int i = 0; i < Num; ++i)
    {
        CCharacter *pTarget = apEnts[i];
		if(pTarget->GetPlayer()->GetTeam() == TEAM_BLUE)
			continue;

        if(distance(m_Pos, pTarget->m_Pos) < distance(m_Pos, m_TargetPos) || !Target)
            m_TargetPos = pTarget->m_Pos;
        Target = true;
    }
    
    if(Target)
    {
        m_TargetPos -= m_Pos;

        if(m_AttackTick  + ReloadTick <= Server()->Tick())
        {
            Fire();
        }
    }
}

void CTurret::DoPlacer()
{
	int Type = (m_Type == WEAPON_HAMMER) ? random_int(POWERUP_HEALTH, POWERUP_ARMOR) : POWERUP_WEAPON;
	int RespawnTime = g_pData->m_aPickups[Type].m_Respawntime * (m_Type + 1) * Server()->TickSpeed();
	if(Server()->Tick() >= m_AttackTick + RespawnTime)
	{
		CPickup *pPickup = new CPickup(&GameServer()->m_World, Type, m_Type);
		pPickup->m_Pos = m_Pos;
		pPickup->m_StartPos = m_Pos;
		pPickup->m_Direction = vec2(0, 1);
		pPickup->m_Gravity = true;
		pPickup->m_OneTime = true; // only pick one time
		pPickup->m_StartTick = Server()->Tick();

		m_AttackTick = Server()->Tick();
	}
}

void CTurret::Tick()
{
	if(m_MarkedForDestroy)
		return;

	if(m_Owner != -1 && !GameServer()->GetPlayerChar(m_Owner))
	{
		m_Drop = true;
		GameWorld()->DestroyEntity(this);
		return;
	}

	if(GameServer()->m_apPlayers[m_Owner]->GetTeam() != TEAM_BLUE)
	{
		m_Drop = true;
		GameWorld()->DestroyEntity(this);
		return;
	}

	if(m_Placer)
		DoPlacer();
	else
		DoAttacker();
}

void CTurret::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	if(SnapFakeTee(SnappingClient)) // if the fake tee snaped, then return (don't snap pickup when snap fake tee)
		return;
    
	CNetObj_Pickup *pP = static_cast<CNetObj_Pickup *>(Server()->SnapNewItem(NETOBJTYPE_PICKUP, m_ID, sizeof(CNetObj_Pickup)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Type = POWERUP_WEAPON;
	pP->m_Subtype = m_Type;
}

bool CTurret::SnapFakeTee(int SnappingClient)
{
	int id = m_TurretID;

	if(!Server()->Translate(id, SnappingClient))
		return false;

	CNetObj_ClientInfo *pClientInfo = static_cast<CNetObj_ClientInfo *>(Server()->SnapNewItem(NETOBJTYPE_CLIENTINFO, id, sizeof(CNetObj_ClientInfo)));
	if(!pClientInfo)
		return false;

	CTeeInfo Info;
    str_copy(Info.m_SkinName, m_Placer ? "x_ninja" : "x_spec", sizeof(Info.m_SkinName));
    Info.m_UseCustomColor = 0;
    Info.m_ColorBody = 0;
    Info.m_ColorFeet = 0;

	StrToInts(&pClientInfo->m_Name0, 4, "");
	StrToInts(&pClientInfo->m_Clan0, 3, "");
	pClientInfo->m_Country = -1;
	StrToInts(&pClientInfo->m_Skin0, 6, Info.m_SkinName);
	pClientInfo->m_UseCustomColor = Info.m_UseCustomColor;
	pClientInfo->m_ColorBody = Info.m_ColorBody;
	pClientInfo->m_ColorFeet = Info.m_ColorFeet;

	CNetObj_PlayerInfo *pPlayerInfo = static_cast<CNetObj_PlayerInfo *>(Server()->SnapNewItem(NETOBJTYPE_PLAYERINFO, id, sizeof(CNetObj_PlayerInfo)));
	if(!pPlayerInfo)
		return false;

	pPlayerInfo->m_Latency = 999;
	pPlayerInfo->m_Local = 0;
	pPlayerInfo->m_ClientID = id;
	pPlayerInfo->m_Score = 0;
	pPlayerInfo->m_Team = -1; // don't show them on the hud
    
    CNetObj_Character *pCharacter = static_cast<CNetObj_Character *>(Server()->SnapNewItem(NETOBJTYPE_CHARACTER, id, sizeof(CNetObj_Character)));
    if(!pCharacter)
        return false;

    pCharacter->m_AmmoCount = 0;
    pCharacter->m_Armor = 0;
    pCharacter->m_Direction = 0;
    pCharacter->m_Emote = 0;
    pCharacter->m_Health = 0;
    pCharacter->m_HookDx = 0;
    pCharacter->m_HookDy = 0;
    pCharacter->m_HookedPlayer = -1;
    pCharacter->m_HookState = HOOK_IDLE;
    pCharacter->m_HookTick = 0;
    pCharacter->m_HookX = 0;
    pCharacter->m_HookY = 0;
    pCharacter->m_Jumped = 0;

    float a = 0;
    if(m_TargetPos.x == 0)
        a = atanf(m_TargetPos.y);
    else
        a = atanf(m_TargetPos.y/m_TargetPos.x);

    if(m_TargetPos.x < 0)
        a = a+pi;

    int Angle = (int)(a*256.0f);
    pCharacter->m_Angle = Angle;
    pCharacter->m_Weapon = m_Placer ? WEAPON_HAMMER : m_Type;
    pCharacter->m_PlayerFlags = PLAYERFLAG_PLAYING;
    pCharacter->m_Tick = Server()->Tick();
    pCharacter->m_AttackTick = m_AttackTick;
    
    pCharacter->m_X = m_Pos.x;
    pCharacter->m_Y = m_Pos.y;
    pCharacter->m_VelX = 0;
    pCharacter->m_VelY = 0;

    CNetObj_DDNetCharacter *pDDNetCharacter = static_cast<CNetObj_DDNetCharacter *>(Server()->SnapNewItem(NETOBJTYPE_DDNETCHARACTER, id, sizeof(CNetObj_DDNetCharacter)));
    if(!pDDNetCharacter) return false;
	
	pDDNetCharacter->m_Flags = CHARACTERFLAG_COLLISION_DISABLED | CHARACTERFLAG_HOOK_HIT_DISABLED;

	pDDNetCharacter->m_FreezeStart = 0;
	pDDNetCharacter->m_FreezeEnd =	0;

	pDDNetCharacter->m_JumpedTotal = 0;
	pDDNetCharacter->m_Jumps = 0;
	pDDNetCharacter->m_TeleCheckpoint = 0;
	pDDNetCharacter->m_StrongWeakID = 0; // unused

	pDDNetCharacter->m_NinjaActivationTick = 0;

	pDDNetCharacter->m_TargetX = m_TargetPos.x;
	pDDNetCharacter->m_TargetY = m_TargetPos.y;

	return true;
}