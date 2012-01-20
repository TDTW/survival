/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/mapitems.h>
#include "../gamecontext.h"
#include "survdm.h"
#include <engine/console.h>

CGameControllerSURVDM::CGameControllerSURVDM(class CGameContext *pGameServer)
: IGameController(pGameServer)
{
	m_pGameType = "SurvDM";
}

bool CGameControllerSURVDM::OnEntity(int Index, vec2 Pos)
{
	if((Index == ENTITY_ARMOR_1) || (Index == ENTITY_HEALTH_1) || (Index == ENTITY_WEAPON_SHOTGUN) || 
	(Index == ENTITY_WEAPON_GRENADE) || (Index == ENTITY_WEAPON_RIFLE) || (Index == ENTITY_POWERUP_NINJA))
		return true;
	
	if(IGameController::OnEntity(Index, Pos))
		return true;
	return true;
}

void CGameControllerSURVDM::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(10);
	pChr->IncreaseArmor(5);

	// give default weapons
	pChr->GiveWeapon(WEAPON_HAMMER, -1);
	pChr->GiveWeapon(WEAPON_GUN, 10);
	pChr->GiveWeapon(WEAPON_SHOTGUN, 8);
	pChr->GiveWeapon(WEAPON_GRENADE, 8);
	pChr->GiveWeapon(WEAPON_RIFLE, 4);
}

int CGameControllerSURVDM::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	int ModeSpecial = IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	pVictim->GetPlayer()->SetTeamDirect(-1);
	return ModeSpecial;
}

void CGameControllerSURVDM::DoWincheck()
{	
	int AllPlayers = 0, Players = 0, Players_Spec = 0, Players_SpecExplicit = 0, Winner = 0;	
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			AllPlayers++;
			if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
				Players_Spec++;
			else			
				Players++;
			if(GameServer()->m_apPlayers[i]->m_SpecExplicit == 1)
				Players_SpecExplicit++;
		}
	}
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if((AllPlayers == 1) && (GameServer()->m_apPlayers[i]->m_SpecExplicit == 1))
				GameServer()->m_apPlayers[i]->m_SpecExplicit = 0;
			
			if((AllPlayers > 1) && (Players == 1) && (Players_SpecExplicit != 0) && (GameServer()->m_apPlayers[i]->GetTeam() != TEAM_SPECTATORS)) 
				Winner = i;			
		}
	}
	
	if((Players_SpecExplicit != 0) && (Players == 0))
	{
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{				
				if(GameServer()->m_apPlayers[i]->m_SpecExplicit == 1)
					GameServer()->m_apPlayers[i]->SetTeamDirect(GameServer()->m_apPlayers[i]->m_TempTeam);			
			}
		}
		EndRound();
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "DRAW!");		
	}	
	
	if((AllPlayers > 1) && (Players == 1) && (Players_SpecExplicit != 0)) 
	{					
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->m_SpecExplicit == 1)
					GameServer()->m_apPlayers[i]->SetTeamDirect(GameServer()->m_apPlayers[i]->m_TempTeam);
			}
		}
		
		EndRound();
		
		CCharacter *pChr = GameServer()->m_apPlayers[Winner]->GetCharacter();
		char Buf2[256];
		str_format(Buf2, sizeof(Buf2), "'%s' win with %d health and %d armor!", Server()->ClientName(Winner), pChr->GetHealth(), pChr->GetArmor());
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, Buf2);		
		return;
	}
	
	IGameController::DoWincheck(); //do also usual wincheck
}

void CGameControllerSURVDM::PostReset()
{

}

void CGameControllerSURVDM::StartRound()
{
	ResetGame();
	
	m_RoundStartTick = Server()->Tick();
	m_SuddenDeath = 0;
	m_GameOverTick = -1;
	GameServer()->m_World.m_Paused = false;
	//m_aTeamscore[TEAM_RED] = 0;
	//m_aTeamscore[TEAM_BLUE] = 0;
	m_ForceBalanced = false;
	Server()->DemoRecorder_HandleAutoStart();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "start round type='%s' teamplay='%d'", m_pGameType, m_GameFlags&GAMEFLAG_TEAMS);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
}