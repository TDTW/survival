/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_SURVTDM_H
#define GAME_SERVER_GAMEMODES_SURVTDM_H
#include <game/server/gamecontroller.h>

class CGameControllerSURVTDM : public IGameController
{
public:
	CGameControllerSURVTDM(class CGameContext *pGameServer);
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void StartRound();
	virtual void DoWincheck();
	virtual void Snap(int SnappingClient);
	virtual void PostReset(bool ClearScore = 0);
};
#endif
