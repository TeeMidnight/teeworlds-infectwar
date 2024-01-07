#ifndef GAME_SERVER_TURRETS_TURRET_H
#define GAME_SERVER_TURRETS_TURRET_H

#include <game/server/entity.h>

class CTurret : public CEntity
{
    vec2 m_TargetPos;
    bool m_Target;
    int m_AttackTick;

    float m_Radius;

    bool m_Drop;
    bool m_Placer;
public:
	int m_Type;
    int m_Owner;
    int m_TurretID;
    int m_Health;

	CTurret(CGameWorld *pGameWorld, vec2 Pos, int Type, int Owner, int TurretID, bool Placer = false);

    void Destroy() override;
    void Fire();
    void DoAttacker();
    void DoPlacer();
    void TakeDamage(int From, int Dmg);
    void Tick() override;
    
	bool SnapFakeTee(int SnappingClient);
	void Snap(int SnappingClient) override;
};

#endif
