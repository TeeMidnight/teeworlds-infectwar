#ifndef GAME_SERVER_TURRETS_TURRET_H
#define GAME_SERVER_TURRETS_TURRET_H

#include <game/server/entity.h>

class CTurret : public CEntity
{
    vec2 m_TargetPos;
    bool m_Target;
    int m_AttackTick;
public:
	int m_Type;
    int m_Owner;
    int m_TurretID;

	CTurret(CGameWorld *pGameWorld, vec2 Pos, int Type, int Owner, int TurretID);

    void Fire();
    virtual void Tick();
    
	bool SnapFakeTee(int SnappingClient);
	virtual void Snap(int SnappingClient);
};

#endif
