/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/shared/config.h>
#include <engine/console.h>
#include <game/server/player.h>
#include <game/server/entities/character.h>

#include "survtdm.h"

CGameControllerSURVTDM::CGameControllerSURVTDM(class CGameContext *pGameServer) : IGameController(pGameServer)
{
	m_pGameType = "SurvTDM";
	m_GameFlags = GAMEFLAG_TEAMS;
}

void CGameControllerSURVTDM::OnCharacterSpawn(class CCharacter *pChr)
{
	// default health
	pChr->IncreaseHealth(g_Config.m_SvGiveHealth);
	pChr->IncreaseArmor(g_Config.m_SvGiveArmor);

	// give default weapons
	if(g_Config.m_SvGiveWeaponHammer)
		pChr->GiveWeapon(WEAPON_HAMMER, -1);
		
	if(g_Config.m_SvGiveWeaponGun)
		pChr->GiveWeapon(WEAPON_GUN, 10);
	
	if(g_Config.m_SvGiveWeaponShotgun)
		pChr->GiveWeapon(WEAPON_SHOTGUN, g_Config.m_SvGiveWeaponShotgun);
		
	if(g_Config.m_SvGiveWeaponGrenade)
		pChr->GiveWeapon(WEAPON_GRENADE, g_Config.m_SvGiveWeaponGrenade);
	
	if(g_Config.m_SvGiveWeaponLaser)
		pChr->GiveWeapon(WEAPON_RIFLE, g_Config.m_SvGiveWeaponLaser);
}

bool CGameControllerSURVTDM::OnEntity(int Index, vec2 Pos)
{
	if(((Index == ENTITY_ARMOR_1) || (Index == ENTITY_HEALTH_1)) && g_Config.m_SvHidePickUps)
		return true;
		
	if(((Index == ENTITY_WEAPON_SHOTGUN) || (Index == ENTITY_WEAPON_GRENADE) || (Index == ENTITY_WEAPON_RIFLE) || (Index == ENTITY_POWERUP_NINJA)) && g_Config.m_SvHideWeapons)
		return true;
	
	if(IGameController::OnEntity(Index, Pos))
		return true;
	return true;
}

int CGameControllerSURVTDM::OnCharacterDeath(class CCharacter *pVictim, class CPlayer *pKiller, int Weapon)
{
	int ModeSpecial = IGameController::OnCharacterDeath(pVictim, pKiller, Weapon);

	pVictim->GetPlayer()->SetTeamDirect(-1);
	return ModeSpecial;
}

void CGameControllerSURVTDM::Snap(int SnappingClient)
{
	IGameController::Snap(SnappingClient);

	CNetObj_GameData *pGameDataObj = (CNetObj_GameData *)Server()->SnapNewItem(NETOBJTYPE_GAMEDATA, 0, sizeof(CNetObj_GameData));
	if(!pGameDataObj)
		return;

	pGameDataObj->m_TeamscoreRed = m_aTeamscore[TEAM_RED];
	pGameDataObj->m_TeamscoreBlue = m_aTeamscore[TEAM_BLUE];

	pGameDataObj->m_FlagCarrierRed = 0;
	pGameDataObj->m_FlagCarrierBlue = 0;
}

void CGameControllerSURVTDM::PostReset(bool ClearScore)
{
	if(g_Config.m_SvScorelimit > 0 && (m_aTeamscore[TEAM_RED] >= g_Config.m_SvScorelimit || m_aTeamscore[TEAM_BLUE] >= g_Config.m_SvScorelimit))
	{
		m_aTeamscore[TEAM_RED] = 0;
		m_aTeamscore[TEAM_BLUE] = 0;
		IGameController::PostReset();
	}
}

void CGameControllerSURVTDM::StartRound()
{
	ResetGame();
	
	m_RoundStartTick = Server()->Tick();
	m_SuddenDeath = 0;
	m_GameOverTick = -1;
	GameServer()->m_World.m_Paused = false;
	m_ForceBalanced = false;
	Server()->DemoRecorder_HandleAutoStart();
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "start round type='%s' teamplay='%d'", m_pGameType, m_GameFlags&GAMEFLAG_TEAMS);
	GameServer()->Console()->Print(IConsole::OUTPUT_LEVEL_DEBUG, "game", aBuf);
}

void CGameControllerSURVTDM::DoWincheck()
{	
	int AllPlayers = 0, PlayersRed = 0, PlayersBlue = 0, 
	Players_Spec = 0, Winner = 0, 
	Players_SpecRedExplicit = 0, Players_SpecBlueExplicit = 0;	
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			AllPlayers++;
			if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_SPECTATORS)
				Players_Spec++;
			else if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_RED)
				PlayersRed++;
			else if(GameServer()->m_apPlayers[i]->GetTeam() == TEAM_BLUE)
				PlayersBlue++;
				
			if((GameServer()->m_apPlayers[i]->m_SpecExplicit == 1) && (GameServer()->m_apPlayers[i]->m_TempTeam == TEAM_RED))
				Players_SpecRedExplicit++;
			else if((GameServer()->m_apPlayers[i]->m_SpecExplicit == 1) && (GameServer()->m_apPlayers[i]->m_TempTeam == TEAM_BLUE))
				Players_SpecBlueExplicit++;
		}
	}
	
/* 	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(GameServer()->m_apPlayers[i])
		{
			if((AllPlayers == 1) && (GameServer()->m_apPlayers[i]->m_SpecExplicit == 1))
				GameServer()->m_apPlayers[i]->m_SpecExplicit = 0;
		}
	} */

	if(((Players_SpecRedExplicit != 0) || (Players_SpecBlueExplicit != 0)) && (PlayersBlue == 0) && (PlayersRed == 0))
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
	
	if(((PlayersRed > 0) && (PlayersBlue == 0) && (Players_SpecBlueExplicit != 0)) ||
		((PlayersBlue > 0) && (PlayersRed == 0) && (Players_SpecRedExplicit != 0)))
	{					
		if ((PlayersRed > 0) && (PlayersBlue == 0) && (Players_SpecBlueExplicit != 0))
			Winner = TEAM_RED;
		if ((PlayersBlue > 0) && (PlayersRed == 0) && (Players_SpecRedExplicit != 0))
			Winner = TEAM_BLUE;
			
		int TempHealth = 0, TempArmor = 0;
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				if(GameServer()->m_apPlayers[i]->GetTeam() == Winner)
				{
					CCharacter *pChr = GameServer()->m_apPlayers[i]->GetCharacter();
					TempHealth += pChr->GetHealth();
					TempArmor += pChr->GetArmor();
				}
				
				if(GameServer()->m_apPlayers[i]->m_SpecExplicit == 1)
					GameServer()->m_apPlayers[i]->SetTeamDirect(GameServer()->m_apPlayers[i]->m_TempTeam);
			
			}
		}
		
		EndRound();
		
		m_aTeamscore[Winner] += 1;
		char Buf2[256];
		str_format(Buf2, sizeof(Buf2), "%s win with %d health and %d armor!", GetTeamName(Winner), TempHealth, TempArmor );
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, Buf2);		
		return;
	}
	
	IGameController::DoWincheck(); //do also usual wincheck
}