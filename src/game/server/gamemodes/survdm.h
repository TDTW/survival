/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_GAMEMODES_SURVDM_H
#define GAME_SERVER_GAMEMODES_SURVDM_H
#include <game/server/gamecontroller.h>

class CGameControllerSURVDM : public IGameController
{
public:
	CGameControllerSURVDM(class CGameContext *pGameServer);
	virtual bool OnEntity(int Index, vec2 Pos);
	virtual void OnCharacterSpawn(class CCharacter *pChr);
	virtual int OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon);
	virtual void StartRound();
	virtual void DoWincheck();
	virtual void PostReset();
};
#endif
