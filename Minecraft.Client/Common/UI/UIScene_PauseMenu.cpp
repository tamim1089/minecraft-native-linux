#include "stdafx.h"
#include "UI.h"
#include "UIScene_PauseMenu.h"
#include "../../MinecraftServer.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../TexturePackRepository.h"
#include "../../TexturePack.h"
#include "../../DLCTexturePack.h"
#include "../../../Minecraft.World/StringHelpers.h"
#ifdef _LINUX
#include <error_dialog.h>
#endif

#ifdef _LINUX
#include "../../Linux/Leaderboards/DurangoStatsDebugger.h"
#endif

#ifdef _LINUX
#include "Linux/Network/SonyCommerce_Vita.h"
#endif

#if defined _LINUX || defined _LINUX
#define USE_SONY_REMOTE_STORAGE
#endif 

UIScene_PauseMenu::UIScene_PauseMenu(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();
	m_bIgnoreInput=false;
	m_eAction=eAction_None;

	m_buttons[BUTTON_PAUSE_RESUMEGAME].init(app.GetString(IDS_RESUME_GAME),BUTTON_PAUSE_RESUMEGAME);
	m_buttons[BUTTON_PAUSE_HELPANDOPTIONS].init(app.GetString(IDS_HELP_AND_OPTIONS),BUTTON_PAUSE_HELPANDOPTIONS);
	m_buttons[BUTTON_PAUSE_LEADERBOARDS].init(app.GetString(IDS_LEADERBOARDS),BUTTON_PAUSE_LEADERBOARDS);
#ifdef _LINUX
	m_buttons[BUTTON_PAUSE_LINUXHELP].init(app.GetString(IDS_LINUX_HELP_APP), BUTTON_PAUSE_LINUXHELP);
#else
	m_buttons[BUTTON_PAUSE_ACHIEVEMENTS].init(app.GetString(IDS_ACHIEVEMENTS),BUTTON_PAUSE_ACHIEVEMENTS);
#endif
#if defined(_LINUX) || defined(_LINUX)
	m_bTrialTexturePack = false;
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

		m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

		if(!m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{
			m_bTrialTexturePack = true;
		}
	}

	// check for all possible labels being fed into BUTTON_PAUSE_SAVEGAME (Bug 163775)
	// this has to be done before button initialisation!
	wchar_t saveButtonLabels[2][256];
	swprintf( saveButtonLabels[0], 256, L"%ls", app.GetString( IDS_SAVE_GAME ));
	swprintf( saveButtonLabels[1], 256, L"%ls", app.GetString( IDS_DISABLE_AUTOSAVE ));
	m_buttons[BUTTON_PAUSE_SAVEGAME].setAllPossibleLabels(2,saveButtonLabels);

	if(app.GetGameHostOption(eGameHostOption_DisableSaving) || m_bTrialTexturePack)
	{
		m_savesDisabled = true;
		m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_SAVE_GAME),BUTTON_PAUSE_SAVEGAME);
	}
	else
	{
		m_savesDisabled = false;
		m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_DISABLE_AUTOSAVE),BUTTON_PAUSE_SAVEGAME);
	}
#else
	m_buttons[BUTTON_PAUSE_SAVEGAME].init(app.GetString(IDS_SAVE_GAME),BUTTON_PAUSE_SAVEGAME);
#endif
	m_buttons[BUTTON_PAUSE_EXITGAME].init(app.GetString(IDS_EXIT_GAME),BUTTON_PAUSE_EXITGAME);

	if(!ProfileManager.IsFullVersion())
	{
		// hide the trial timer
		ui.ShowTrialTimer(false);
	}

	updateControlsVisibility();

	doHorizontalResizeCheck();

	// get rid of the quadrant display if it's on
	ui.HidePressStart();

#if TO_BE_IMPLEMENTED
	XuiSetTimer(m_hObj,IGNORE_KEYPRESS_TIMERID,IGNORE_KEYPRESS_TIME);
#endif

	if( g_NetworkManager.IsLocalGame() && g_NetworkManager.GetPlayerCount() == 1 )
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)TRUE);
	}

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_PauseMenu, 0);
	TelemetryManager->RecordPauseOrInactive(m_iPad);

	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != NULL && pMinecraft->localgameModes[iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[iPad];

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(false);
	}
	m_bErrorDialogRunning = false;
}

UIScene_PauseMenu::~UIScene_PauseMenu()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft != NULL && pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];

		// This just allows it to be shown
		gameMode->getTutorial()->showTutorialPopup(true);
	}

	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_MenuBackground,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

wstring UIScene_PauseMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"PauseMenuSplit";
	}
	else
	{
		return L"PauseMenu";
	}
}

void UIScene_PauseMenu::tick()
{
	UIScene::tick();

#ifdef _LINUX
	// Need to check for installed DLC here, as we delay the installation of the key file on Vita
	if(!app.DLCInstallProcessCompleted()) app.StartInstallDLCProcess(0);
#endif


#if defined _LINUX || defined _LINUX
	if(!m_bTrialTexturePack && m_savesDisabled != (app.GetGameHostOption(eGameHostOption_DisableSaving) != 0) && ProfileManager.GetPrimaryPad() == m_iPad )
	{
		// We show the save button if saves are disabled as this lets us show a prompt to enable them (via purchasing a texture pack)
		if( app.GetGameHostOption(eGameHostOption_DisableSaving) )
		{
			m_savesDisabled = true;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_SAVE_GAME) );
		}
		else
		{
			m_savesDisabled = false;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_DISABLE_AUTOSAVE) );
		}
	}
#endif

#ifdef _LINUX
	// Process the error dialog (for a patch being available)
	if(m_bErrorDialogRunning)
	{	
		SceErrorDialogStatus stat = sceErrorDialogUpdateStatus();
		if( stat == SCE_ERROR_DIALOG_STATUS_FINISHED ) 
		{
			sceErrorDialogTerminate();
			m_bErrorDialogRunning=false;
		}
	}
#endif
}

void UIScene_PauseMenu::updateTooltips()
{
	bool bUserisClientSide = ProfileManager.IsSignedInLive(m_iPad);
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);

#ifdef _LINUX
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(m_iPad);
#endif

	int iY = -1;
#if defined _LINUX || defined _LINUX
	if(m_iPad == ProfileManager.GetPrimaryPad() ) iY = IDS_TOOLTIPS_GAME_INVITES;
#endif
	int iRB = -1;
	int iX = -1;

	if(ProfileManager.IsFullVersion())
	{
		if(StorageManager.GetSaveDisabled())
		{
			iX = bIsisPrimaryHost?IDS_TOOLTIPS_SELECTDEVICE:-1;
#ifdef _LINUX
			iRB = bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1;
#endif
			if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide )
			{
#ifndef _LINUX
				iY = IDS_TOOLTIPS_SHARE;
#endif
			}		
		}
		else
		{
			iX = bIsisPrimaryHost?IDS_TOOLTIPS_CHANGEDEVICE:-1;
#ifdef _LINUX
			iRB = bDisplayBanTip?IDS_TOOLTIPS_BANLEVEL:-1;
#endif
			if( CSocialManager::Instance()->IsTitleAllowedToPostImages() && CSocialManager::Instance()->AreAllUsersAllowedToPostImages() && bUserisClientSide)
			{
#ifndef _LINUX
				iY = IDS_TOOLTIPS_SHARE;
#endif
			}	
		}
	}
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK,iX,iY, -1,-1,-1,iRB);
}

void UIScene_PauseMenu::updateComponents()
{
	m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);
	m_parentLayer->showComponent(m_iPad,eUIComponent_MenuBackground,true);

	if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
}

void UIScene_PauseMenu::handlePreReload()
{
#if defined _LINUX || defined _LINUX
	if(ProfileManager.GetPrimaryPad() == m_iPad)
	{
		// check for all possible labels being fed into BUTTON_PAUSE_SAVEGAME (Bug 163775)
		// this has to be done before button initialisation!
		wchar_t saveButtonLabels[2][256];
		swprintf( saveButtonLabels[0], 256, L"%ls", app.GetString( IDS_SAVE_GAME ));
		swprintf( saveButtonLabels[1], 256, L"%ls", app.GetString( IDS_DISABLE_AUTOSAVE ));
		m_buttons[BUTTON_PAUSE_SAVEGAME].setAllPossibleLabels(2,saveButtonLabels);
	}
#endif
}

void UIScene_PauseMenu::handleReload()
{
	updateTooltips();
	updateControlsVisibility();	

#if defined _LINUX || defined _LINUX
	if(ProfileManager.GetPrimaryPad() == m_iPad)
	{
		// We show the save button if saves are disabled as this lets us show a prompt to enable them (via purchasing a texture pack)
		if( app.GetGameHostOption(eGameHostOption_DisableSaving) || m_bTrialTexturePack )
		{
			m_savesDisabled = true;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_SAVE_GAME) );
		}
		else
		{
			m_savesDisabled = false;
			m_buttons[BUTTON_PAUSE_SAVEGAME].setLabel( app.GetString(IDS_DISABLE_AUTOSAVE) );
		}
	}
#endif

	doHorizontalResizeCheck();
}

void UIScene_PauseMenu::updateControlsVisibility()
{
	// are we the primary player?
	// fix for 7844 & 7845 - 
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have a Leaderboards option.
	// TCR # 128:  XLA Pause Menu:   When in a multiplayer game as a client the Pause Menu does not have an Achievements option.
	if(ProfileManager.GetPrimaryPad()==m_iPad) // && g_NetworkManager.IsHost()) 
	{
		// are we in splitscreen?
		// how many local players do we have?
		if( app.GetLocalPlayerCount()>1 )
		{
			// Hide the BUTTON_PAUSE_LEADERBOARDS and BUTTON_PAUSE_ACHIEVEMENTS
			removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
#ifndef _LINUX
			removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif
		}
#ifdef _LINUX
		// MGH added - remove leaderboards in adhoc
		if(CGameNetworkManager::usingAdhocMode())
		{
			removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
		}
#endif

		if( !g_NetworkManager.IsHost() )
		{
			// Hide the BUTTON_PAUSE_SAVEGAME
			removeControl( &m_buttons[BUTTON_PAUSE_SAVEGAME], false );
		}
	}
	else
	{
		// Hide the BUTTON_PAUSE_LEADERBOARDS, BUTTON_PAUSE_ACHIEVEMENTS and BUTTON_PAUSE_SAVEGAME
		removeControl( &m_buttons[BUTTON_PAUSE_LEADERBOARDS], false );
#ifndef _LINUX
		removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif
		removeControl( &m_buttons[BUTTON_PAUSE_SAVEGAME], false );
	}

	// is saving disabled?
	if(StorageManager.GetSaveDisabled())
	{
#ifdef _LINUX
		// disable save button
		m_buttons[BUTTON_PAUSE_SAVEGAME].setEnable(false);
#endif
	}

#if defined(_LINUX)  || defined (_LINUX) || defined(_LINUX) 
	removeControl( &m_buttons[BUTTON_PAUSE_ACHIEVEMENTS], false );
#endif

}

void UIScene_PauseMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput)
	{
		return;
	}

	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s/n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

#ifdef _LINUX
	bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==iPad);
	bool bDisplayBanTip = !g_NetworkManager.IsLocalGame() && !bIsisPrimaryHost && !ProfileManager.IsGuest(iPad);
#endif

	switch(key)
	{
#ifdef _LINUX
	case ACTION_MENU_GTC_RESUME:
#endif
	case ACTION_MENU_PAUSEMENU:
#endif
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
#ifdef _LINUX
#endif

			if( iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
			{
				app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
			}

			ui.PlayUISFX(eSFX_Back);
			navigateBack();
			if(!ProfileManager.IsFullVersion())
			{
				ui.ShowTrialTimer(true);
			}
		}
		break;
	case ACTION_MENU_OK:
#ifdef _LINUX
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		if(pressed)
		{
			sendInputToMovie(key, repeat, pressed, released);
		}
		break;

#if TO_BE_IMPLEMENTED
	case VK_PAD_X:
		// Change device
		if(bIsisPrimaryHost)
		{	
			// we need a function to deal with the return from this - if it changes, we need to update the pause menu and tooltips
			// Fix for #12531 - TCR 001: BAS Game Stability: When a player selects to change a storage 
			// device, and repeatedly backs out of the SD screen, disconnects from LIVE, and then selects a SD, the title crashes.
			m_bIgnoreInput=true;

			StorageManager.SetSaveDevice(&UIScene_PauseMenu::DeviceSelectReturned,this,true);
		}
		rfHandled = TRUE;
		break;
#endif

	case ACTION_MENU_Y:
		{
			
#if defined(_LINUX) || defined(_LINUX)
		if(pressed && iPad == ProfileManager.GetPrimaryPad())
		{
#ifdef _LINUX
			// If a patch is available, can't view invites
			if (CheckForPatch()) break;
#endif

			// Are we offline?
			if(!ProfileManager.IsSignedInLive(iPad))
			{
				m_eAction=eAction_ViewInvitesPSN;
#ifdef _LINUX
				int npAvailability = ProfileManager.getNPAvailability(iPad);
				if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					// This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad, NULL, NULL, app.GetStringTable());
				}
				else
					// Determine why they're not "signed in live"
					if (ProfileManager.isSignedInPSN(iPad))
					{
						assert(!ProfileManager.isConnectedToPSN(iPad));

						UINT uiIDA[1];
						uiIDA[0] = IDS_OK;
						ui.RequestMessageBox( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad, NULL, NULL, app.GetStringTable());
					}
					else
					{		
						UINT uiIDA[1];
						uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
						ui.RequestMessageBox( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, this, app.GetStringTable(), NULL, 0, false);
					}				
#else // _LINUX
					// get them to sign in to online
				UINT uiIDA[1];
					uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, this, app.GetStringTable(), NULL, 0, false);
#endif
				}
				else
				{
#ifdef _LINUX
					SQRNetworkManager::RecvInviteGUI();
#else // _LINUX
					int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
					app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )/n", ret, ret);
#endif
				}
			}
#else
#if TO_BE_IMPLEMENTED
			if(bUserisClientSide)
			{			
				// Added check in 1.8.2 bug fix (TU6) to stop repeat key presses
				bool bCanScreenshot = true;
				for(int j=0; j < XUSER_MAX_COUNT;++j)
				{
					if(app.GetXuiAction(j) == eAppAction_SocialPostScreenshot)
					{
						bCanScreenshot = false;
						break;
					}
				}
				if(bCanScreenshot) app.SetAction(pInputData->UserIndex,eAppAction_SocialPost);
			}
			rfHandled = TRUE;
#endif
#endif // _LINUX
		}
		break;
#ifdef _LINUX
	case ACTION_MENU_RIGHT_SCROLL:
		if( bDisplayBanTip )
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestMessageBox(IDS_ACTION_BAN_LEVEL_TITLE, IDS_ACTION_BAN_LEVEL_DESCRIPTION, uiIDA, 2, iPad,&UIScene_PauseMenu::BanGameDialogReturned,this, app.GetStringTable(), NULL, 0, false);

			//rfHandled = TRUE;
		}
		break;
#endif
	}
}

void UIScene_PauseMenu::handlePress(F64 controlId, F64 childId)
{
	if(m_bIgnoreInput) return;

	switch((int)controlId)
	{
	case BUTTON_PAUSE_RESUMEGAME:
		if( m_iPad == ProfileManager.GetPrimaryPad() && g_NetworkManager.IsLocalGame() )
		{
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_PauseServer,(void *)FALSE);
		}
		navigateBack();
		break;
	case BUTTON_PAUSE_LEADERBOARDS:
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;

			//Being used for the leaderboards proper now
			// guests can't look at leaderboards
			if(ProfileManager.IsGuest(m_iPad))
			{
				ui.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable(), NULL, 0, false);
			}
			else if(!ProfileManager.IsSignedInLive(m_iPad))
			{
#ifdef _LINUX
				// If a patch is available, can't show leaderboard
				if (CheckForPatch()) break;

				// Check for content restricted user
				// Update error code
				int errorCode = ProfileManager.getNPAvailability(m_iPad);

				if (errorCode == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					UINT uiIDA[1];
					uiIDA[0] = IDS_CONFIRM_OK;
					ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad, NULL, NULL, app.GetStringTable());

					break;;
				}

#endif

#if defined _LINUX || _LINUX
				// get them to sign in to online
				m_eAction=eAction_ViewLeaderboardsPSN;
				UINT uiIDA[1];
				uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
				ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_LINUXLIVE_NOTIFICATION, uiIDA, 1, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::MustSignInReturnedPSN,this, app.GetStringTable(), NULL, 0, false);
#elif defined(_LINUX)
				m_eAction=eAction_ViewLeaderboardsPSN;
				int npAvailability = ProfileManager.getNPAvailability(m_iPad);
				if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
				{
					// This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
					UINT uiIDA[1];
					uiIDA[0]=IDS_OK;
					ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad, NULL, NULL, app.GetStringTable());
				}
				else
					// Determine why they're not "signed in live"
					if (ProfileManager.isSignedInPSN(m_iPad))
					{

						// Id
						assert(!ProfileManager.isConnectedToPSN(m_iPad));

						UINT uiIDA[1];
						uiIDA[0] = IDS_OK;
						ui.RequestMessageBox( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, m_iPad, NULL, NULL, app.GetStringTable());
					}
					else
					{		
						UINT uiIDA[1];
						uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
						ui.RequestMessageBox( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, m_iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, this, app.GetStringTable(), NULL, 0, false);
					}
#else
			UINT uiIDA[1] = { IDS_OK };
			ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_LINUXLIVE_NOTIFICATION, uiIDA, 1, m_iPad);
#endif
			}
			else
			{	
				bool bContentRestricted=false;
#if defined(_LINUX) || defined(_LINUX)
				ProfileManager.GetChatAndContentRestrictions(m_iPad,true,NULL,&bContentRestricted,NULL);
#endif
				if(bContentRestricted)
				{
#if !(defined(_LINUX) || defined(__LP64__)) // Temp to get the win build running, but so we check this for other platforms
					// you can't see leaderboards
					UINT uiIDA[1];
					uiIDA[0]=IDS_CONFIRM_OK;
					ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad,NULL,this, app.GetStringTable(), NULL, 0, false);
#endif
				}
				else
				{
					ui.NavigateToScene(m_iPad, eUIScene_LeaderboardsMenu);
				}
			}
		}
		break;
#ifdef _LINUX
	case BUTTON_PAUSE_LINUXHELP:
		{
			WXS::User^ user = ProfileManager.GetUser(m_iPad);
			Linux::Linux::ApplicationModel::Help::Show(user);
		}
		break;
#elif TO_BE_IMPLEMENTED
	case BUTTON_PAUSE_ACHIEVEMENTS:

		// guests can't look at achievements
		if(ProfileManager.IsGuest(pNotifyPressData->UserIndex))
		{
			UINT uiIDA[1];
			uiIDA[0]=IDS_OK;
			ui.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable(), NULL, 0, false);
		}
		else
		{
			XShowAchievementsUI( pNotifyPressData->UserIndex );
		}
		break;
#endif

	case BUTTON_PAUSE_HELPANDOPTIONS:
		ui.NavigateToScene(m_iPad,eUIScene_HelpAndOptionsMenu);	
		break;
	case BUTTON_PAUSE_SAVEGAME:
		PerformActionSaveGame();
		break;
	case BUTTON_PAUSE_EXITGAME:
		{
			Minecraft *pMinecraft = Minecraft::GetInstance();
			// Check if it's the trial version
			if(ProfileManager.IsFullVersion())
			{	
				UINT uiIDA[3];

				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[m_iPad]->getSessionTimer();
					}

#if defined(_LINUX) || defined(_LINUX)
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;

					if(g_NetworkManager.IsHost() && StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_EXIT_GAME_SAVE;
						uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

						if(g_NetworkManager.GetPlayerCount()>1)
						{
							ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
						}
						else
						{
							ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
						}
					}
					else if(g_NetworkManager.IsHost() && g_NetworkManager.GetPlayerCount()>1)
					{
						ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, dynamic_cast<IUIScene_PauseMenu*>(this), app.GetStringTable(), NULL, 0, false);
					}
					else
					{
						ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, dynamic_cast<IUIScene_PauseMenu*>(this), app.GetStringTable(), NULL, 0, false);
					}
#else
					if(StorageManager.GetSaveDisabled())
					{
						uiIDA[0]=IDS_CONFIRM_CANCEL;
						uiIDA[1]=IDS_CONFIRM_OK;
						ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,this, app.GetStringTable(), NULL, 0, false);
					}
					else
					{
						if( g_NetworkManager.IsHost() )
						{	
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_EXIT_GAME_SAVE;
							uiIDA[2]=IDS_EXIT_GAME_NO_SAVE;

							if(g_NetworkManager.GetPlayerCount()>1)
							{
								ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_CONFIRM_DISCONNECT_SAVE, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
							}
							else
							{
								ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 3, m_iPad,&UIScene_PauseMenu::ExitGameSaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
							}
						}
						else
						{
							uiIDA[0]=IDS_CONFIRM_CANCEL;
							uiIDA[1]=IDS_CONFIRM_OK;

							ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned,this, app.GetStringTable(), NULL, 0, false);
						}
					}
#endif
				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[m_iPad]->getSessionTimer();
					}

					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Exited);


					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}		
			}
			else
			{
				// is it the primary player exiting?
				if(m_iPad==ProfileManager.GetPrimaryPad())
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[m_iPad]->getSessionTimer();
					}	

					// adjust the trial time played
					ui.ReduceTrialTimerValue();

					// exit the level
					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_CANCEL;
					uiIDA[1]=IDS_CONFIRM_OK;
					ui.RequestMessageBox(IDS_EXIT_GAME, IDS_CONFIRM_EXIT_GAME_PROGRESS_LOST, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::ExitGameDialogReturned, dynamic_cast<IUIScene_PauseMenu*>(this), app.GetStringTable(), NULL, 0, false);

				}
				else
				{
					int playTime = -1;
					if( pMinecraft->localplayers[m_iPad] != NULL )
					{
						playTime = (int)pMinecraft->localplayers[m_iPad]->getSessionTimer();
					}

					TelemetryManager->RecordLevelExit(m_iPad, eSen_LevelExitStatus_Exited);

					// just exit the player
					app.SetAction(m_iPad,eAppAction_ExitPlayer);
				}
			}
		}
		break;
	}
}

void UIScene_PauseMenu::PerformActionSaveGame()
{
	// is the player trying to save in the trial version?
	if(!ProfileManager.IsFullVersion())
	{
#ifdef _LINUX
		// If a patch is available, can't buy full game
		if (CheckForPatch()) return;
#endif

		// Unlock the full version?
		if(!ProfileManager.IsSignedInLive(m_iPad))
		{
#if defined(_LINUX)
			m_eAction=eAction_SaveGamePSN;
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_LINUXLIVE_NOTIFICATION, uiIDA, 2, ProfileManager.GetPrimaryPad(),&UIScene_PauseMenu::MustSignInReturnedPSN,this, app.GetStringTable(), NULL, 0, false);
#elif defined(_LINUX)
			m_eAction=eAction_SaveGamePSN;
			int npAvailability = ProfileManager.getNPAvailability(m_iPad);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				// This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, m_iPad, NULL, NULL, app.GetStringTable());
			}
			else
				// Determine why they're not "signed in live"
				if (ProfileManager.isSignedInPSN(m_iPad))
				{
					assert(!ProfileManager.isConnectedToPSN(m_iPad));

					UINT uiIDA[1];
					uiIDA[0] = IDS_OK;
					ui.RequestMessageBox( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, m_iPad, NULL, NULL, app.GetStringTable());
				}
				else
				{		
					UINT uiIDA[1];
					uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
					ui.RequestMessageBox( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, m_iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, this, app.GetStringTable(), NULL, 0, false);
				}
#endif
		}
		else
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;
			ui.RequestMessageBox(IDS_UNLOCK_TITLE, IDS_UNLOCK_TOSAVE_TEXT, uiIDA, 2,m_iPad,&UIScene_PauseMenu::UnlockFullSaveReturned,this,app.GetStringTable(), NULL, 0, false);
		}

		return;
	}

	// Is the player trying to save but they are using a trial texturepack ?
	if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
	{
		TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
		DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

		m_pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();

		if(!m_pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
		{					
			// upsell
#ifdef _LINUX
			ULONGLONG ullOfferID_Full;
			// get the dlc texture pack
			DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

			app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

			// tell sentient about the upsell of the full version of the texture pack
			TelemetryManager->RecordUpsellPresented(m_iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_OK;
			uiIDA[1]=IDS_CONFIRM_CANCEL;

			// Give the player a warning about the trial version of the texture pack
#ifdef _LINUX
			if(app.DLCInstallProcessCompleted() && !SonyCommerce_Vita::getDLCUpgradePending())  // MGH - devtrack #5861 On vita it can take a bit after the install has finished to register the purchase, so make sure we don't end up asking to purchase again
#endif
			{
				ui.RequestMessageBox(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, m_iPad,&UIScene_PauseMenu::WarningTrialTexturePackReturned,this,app.GetStringTable(), NULL, 0, false);
			}

			return;					
		}
		else
		{
			m_bTrialTexturePack = false;
		}
	}

	// does the save exist?
	bool bSaveExists;
	CStorage::ESaveGameState result=StorageManager.DoesSaveExist(&bSaveExists);

#ifdef _LINUX
	if(result == CStorage::ELoadGame_DeviceRemoved)
	{
		// this will be a tester trying to be clever
		UINT uiIDA[2];
		uiIDA[0]=IDS_SELECTANEWDEVICE;
		uiIDA[1]=IDS_NODEVICE_DECLINE;

		ui.RequestMessageBox(IDS_STORAGEDEVICEPROBLEM_TITLE, IDS_FAILED_TO_LOADSAVE_TEXT, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::DeviceRemovedDialogReturned,this, app.GetStringTable(), NULL, 0, false);
	}
	else
#endif
	{
#if defined(_LINUX) || defined(_LINUX)
		if(!m_savesDisabled)
		{
			UINT uiIDA[2];
			uiIDA[0]=IDS_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestMessageBox(IDS_TITLE_DISABLE_AUTOSAVE, IDS_CONFIRM_DISABLE_AUTOSAVE, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::DisableAutosaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
		}
		else
#endif
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::SaveGameDialogReturned,this, app.GetStringTable(), NULL, 0, false);
			}
			else
			{
#if defined(_LINUX) || defined(_LINUX)
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_TITLE_ENABLE_AUTOSAVE, IDS_CONFIRM_ENABLE_AUTOSAVE, uiIDA, 2, m_iPad,&IUIScene_PauseMenu::EnableAutosaveDialogReturned,this, app.GetStringTable(), NULL, 0, false);
#else
				// flag a app action of save game
				app.SetAction(m_iPad,eAppAction_SaveGame);
#endif
			}
	}
}

void UIScene_PauseMenu::ShowScene(bool show)
{
	app.DebugPrintf("UIScene_PauseMenu::ShowScene is not implemented/n");
}

void UIScene_PauseMenu::HandleDLCInstalled()
{
	// mounted DLC may have changed
	if(app.StartInstallDLCProcess(m_iPad)==false)
	{
		// not doing a mount, so re-enable input
		//m_bIgnoreInput=false;
		app.DebugPrintf("UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput false/n");
	}
	else
	{
		// Somehow, on th edisc build, we get in here, but don't call HandleDLCMountingComplete, so input locks up
		//m_bIgnoreInput=true;
		app.DebugPrintf("UIScene_PauseMenu::HandleDLCInstalled - m_bIgnoreInput true/n");
	}
	// this will send a CustomMessage_DLCMountingComplete when done
}


void UIScene_PauseMenu::HandleDLCMountingComplete()
{	
	// check if we should display the save option

	//m_bIgnoreInput=false;
	app.DebugPrintf("UIScene_PauseMenu::HandleDLCMountingComplete - m_bIgnoreInput false /n");

	// 	if(ProfileManager.IsFullVersion())
	// 	{
	// 		bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);
	// 		if(bIsisPrimaryHost)
	// 		{
	// 			m_buttons[BUTTON_PAUSE_SAVEGAME].setEnable(true);
	// 		}
	// 	}
}

int UIScene_PauseMenu::UnlockFullSaveReturned(void *pParam,int iPad,CStorage::EMessageResult result)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;
	Minecraft *pMinecraft=Minecraft::GetInstance();

	if(result==CStorage::EMessage_ResultAccept)
	{
		if(ProfileManager.IsSignedInLive(pMinecraft->player->GetPadIndex()))
		{
			// need to check this user can access the store
#if defined(_LINUX) || defined(_LINUX)
			bool bContentRestricted;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,NULL,&bContentRestricted,NULL);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,pClass, app.GetStringTable(), NULL, 0, false);
			}
			else
#endif
			{
				ProfileManager.DisplayFullVersionPurchase(false,pMinecraft->player->GetPadIndex(),eSen_UpsellID_Full_Version_Of_Game);
			}
		}
	}
	else
	{
		//SentientManager.RecordUpsellResponded(iPad, eSen_UpsellID_Full_Version_Of_Game, app.m_dwOfferID, eSen_UpsellOutcome_Declined);
	}

	return 0;
}

int UIScene_PauseMenu::SaveGame_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;

	if(bContinue==true)
	{
		pClass->PerformActionSaveGame();
	}

	return 0;
}

#ifdef _LINUX
int UIScene_PauseMenu::BanGameDialogReturned(void *pParam,int iPad,CStorage::EMessageResult result)
{
	// results switched for this dialog
	if(result==CStorage::EMessage_ResultDecline) 
	{
		app.SetAction(iPad,eAppAction_BanLevel);
	}
	return 0;
}
#endif

#if defined(_LINUX)  || defined (_LINUX) || defined(_LINUX)
int UIScene_PauseMenu::MustSignInReturnedPSN(void *pParam,int iPad,CStorage::EMessageResult result)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;

	if(result==CStorage::EMessage_ResultAccept) 
	{
#ifdef _LINUX
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pClass);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pClass);
			break;
		case eAction_SaveGamePSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pClass);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pClass);
			break;
		}
#elif defined _LINUX
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			//Save settings change
			app.SetGameSettings(0, eGameSetting_Linux_NetworkModeAdhoc, 0);
			//Force off
			CGameNetworkManager::setAdhocMode(false);
			//Now Sign-in
			SQRNetworkManager_Vita::AttemptSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pClass);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager_Vita::AttemptSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pClass);
			break;
		case eAction_SaveGamePSN:
			SQRNetworkManager_Vita::AttemptSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pClass);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager_Vita::AttemptSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pClass);
			break;
		}
#else
		switch(pClass->m_eAction)
		{
		case eAction_ViewLeaderboardsPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::ViewLeaderboards_SignInReturned, pClass, false, iPad);
			break;
		case eAction_ViewInvitesPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::ViewInvites_SignInReturned, pClass, false, iPad);
			break;
		case eAction_SaveGamePSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::SaveGame_SignInReturned, pClass, false, iPad);
			break;
		case eAction_BuyTexturePackPSN:
			SQRNetworkManager::AttemptSignIn(&UIScene_PauseMenu::BuyTexturePack_SignInReturned, pClass, false, iPad);
			break;
		}
#endif
	}

	return 0;
}

int UIScene_PauseMenu::ViewLeaderboards_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;

	if(bContinue==true)
	{
		UINT uiIDA[1];
		uiIDA[0]=IDS_CONFIRM_OK;

		// guests can't look at leaderboards
		if(ProfileManager.IsGuest(pClass->m_iPad))
		{
			ui.RequestMessageBox(IDS_PRO_GUESTPROFILE_TITLE, IDS_PRO_GUESTPROFILE_TEXT, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,NULL, app.GetStringTable(), NULL, 0, false);
		}
		else if(ProfileManager.IsSignedInLive(iPad))
		{
#ifndef _LINUX
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(pClass->m_iPad,true,NULL,&bContentRestricted,NULL);
			if(bContentRestricted)
			{
				// you can't see leaderboards
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, ProfileManager.GetPrimaryPad(),NULL,pClass, app.GetStringTable(), NULL, 0, false);
			}
			else
#endif
			{
				ui.NavigateToScene(pClass->m_iPad, eUIScene_LeaderboardsMenu);
			}
		}
	}

	return 0;
}

int UIScene_PauseMenu::WarningTrialTexturePackReturned(void *pParam,int iPad,CStorage::EMessageResult result)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;

#ifdef _LINUX
	// If a patch is available, can't proceed 
	if (pClass->CheckForPatch()) return 0;
#endif

#if defined(_LINUX) || defined(_LINUX) || defined(_LINUX)
	if(result==CStorage::EMessage_ResultAccept)
	{
		if(!ProfileManager.IsSignedInLive(iPad))
		{
			pClass->m_eAction=eAction_SaveGamePSN;
			int npAvailability = ProfileManager.getNPAvailability(iPad);
			if (npAvailability == SCE_NP_ERROR_AGE_RESTRICTION)
			{
				// This is a bit messy and is due to the library incorrectly returning false for IsSignedInLive is the npAvailability isn't SCE_OK
				UINT uiIDA[1];
				uiIDA[0]=IDS_OK;
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad, NULL, NULL, app.GetStringTable());
			}
			else
				// Determine why they're not "signed in live"
				if (ProfileManager.isSignedInPSN(iPad))
				{
					assert(!ProfileManager.isConnectedToPSN(iPad));

					UINT uiIDA[1];
					uiIDA[0] = IDS_OK;
					ui.RequestMessageBox( IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, uiIDA, 1, iPad, NULL, NULL, app.GetStringTable());
				}
				else
				{		
					UINT uiIDA[1];
					uiIDA[0] = IDS_PRO_NOTONLINE_ACCEPT;
					ui.RequestMessageBox( IDS_PRO_NOTONLINE_TITLE, IDS_PRO_NOTONLINE_TEXT, uiIDA, 1, iPad, &UIScene_PauseMenu::MustSignInReturnedPSN, pClass, app.GetStringTable(), NULL, 0, false);
				}
#else // _LINUX
			UINT uiIDA[2];
			uiIDA[0]=IDS_PRO_NOTONLINE_ACCEPT;
			uiIDA[1]=IDS_PRO_NOTONLINE_DECLINE;
			ui.RequestMessageBox(IDS_PRO_NOTONLINE_TITLE, IDS_PRO_LINUXLIVE_NOTIFICATION, uiIDA, 2, iPad,&UIScene_PauseMenu::MustSignInReturnedPSN,pClass, app.GetStringTable(), NULL, 0, false);
#endif
		}
		else
		{
#ifndef _LINUX
			// need to check this user can access the store
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(ProfileManager.GetPrimaryPad(),true,NULL,&bContentRestricted,NULL);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad,NULL,pClass, app.GetStringTable(), NULL, 0, false);
			}
			else
#endif
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s/n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);		

				if(pSONYDLCInfo!=NULL)
				{
					char chName[42];
					char chKeyName[20];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want

					// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a NULL terminating char
					memset(chKeyName, 0, sizeof(chKeyName));
					strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef _LINUX
					strcpy(chName, chKeyName);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);

					// need to check for an empty store
#if defined _LINUX || defined _LINUX || defined _LINUX
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);	
						}
					}
				}
			}
		}
	}
#endif		//	

	return 0;
}

int UIScene_PauseMenu::BuyTexturePack_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	UIScene_PauseMenu* pClass = (UIScene_PauseMenu*)pParam;

	if(bContinue==true)
	{
		// Check if we're signed in to LIVE
		if(ProfileManager.IsSignedInLive(iPad))
		{
#if defined(_LINUX) || defined(_LINUX) || defined(_LINUX)

#ifndef _LINUX
			// need to check this user can access the store
			bool bContentRestricted=false;
			ProfileManager.GetChatAndContentRestrictions(iPad,true,NULL,&bContentRestricted,NULL);
			if(bContentRestricted)
			{
				UINT uiIDA[1];
				uiIDA[0]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1, iPad,NULL,pClass, app.GetStringTable(), NULL, 0, false);
			}
			else
#endif
			{
				// need to get info on the pack to see if the user has already downloaded it
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				// retrieve the store name for the skin pack
				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				const char *pchPackName=wstringtofilename(pDLCPack->getName());
				app.DebugPrintf("Texture Pack - %s/n",pchPackName);
				SONYDLC *pSONYDLCInfo=app.GetSONYDLCInfo((char *)pchPackName);		

				if(pSONYDLCInfo!=NULL)
				{
					char chName[42];
					char chKeyName[20];
					char chSkuID[SCE_NP_COMMERCE2_SKU_ID_LEN];

					memset(chSkuID,0,SCE_NP_COMMERCE2_SKU_ID_LEN);
					// find the info on the skin pack
					// we have to retrieve the skuid from the store info, it can't be hardcoded since Sony may change it.
					// So we assume the first sku for the product is the one we want

					// MGH -  keyname in the DLC file is 16 chars long, but there's no space for a NULL terminating char
					memset(chKeyName, 0, sizeof(chKeyName));
					strncpy(chKeyName, pSONYDLCInfo->chDLCKeyname, 16);

#ifdef _LINUX
					strcpy(chName, chKeyName);
#else
					sprintf(chName,"%s-%s",app.GetCommerceCategory(),chKeyName);
#endif
					app.GetDLCSkuIDFromProductList(chName,chSkuID);

					// need to check for an empty store
#if defined _LINUX || defined _LINUX || defined _LINUX
					if(app.CheckForEmptyStore(iPad)==false)
#endif
					{	
						if(app.DLCAlreadyPurchased(chSkuID))
						{
							app.DownloadAlreadyPurchased(chSkuID);
						}
						else
						{
							app.Checkout(chSkuID);	
						}
					}
				}
			}
#else
#endif
		}
	}
	return 0;
}

int UIScene_PauseMenu::ViewInvites_SignInReturned(void *pParam,bool bContinue, int iPad)
{
	if(bContinue==true)
	{
		// Check if we're signed in to LIVE
		if(ProfileManager.IsSignedInLive(iPad))
		{
#ifdef _LINUX
			SQRNetworkManager::RecvInviteGUI();
#elif defined _LINUX
			int ret = sceNpBasicRecvMessageCustom(SCE_NP_BASIC_MESSAGE_MAIN_TYPE_INVITE, SCE_NP_BASIC_RECV_MESSAGE_OPTIONS_INCLUDE_BOOTABLE, SYS_MEMORY_CONTAINER_ID_INVALID);
			app.DebugPrintf("sceNpBasicRecvMessageCustom return %d ( %08x )/n", ret, ret);
#else // _LINUX
			LINUX_STUBBED;
#endif
		}
	}
	return 0;
}


int UIScene_PauseMenu::ExitGameSaveDialogReturned(void *pParam,int iPad,CStorage::EMessageResult result)
{
	UIScene_PauseMenu *pClass = (UIScene_PauseMenu *)pParam;
	// Exit with or without saving
	// Decline means save in this dialog
	if(result==CStorage::EMessage_ResultDecline || result==CStorage::EMessage_ResultThirdOption) 
	{
		if( result==CStorage::EMessage_ResultDecline ) // Save
		{
			// Is the player trying to save but they are using a trial texturepack ?
			if(!Minecraft::GetInstance()->skins->isUsingDefaultSkin())
			{
				TexturePack *tPack = Minecraft::GetInstance()->skins->getSelected();
				DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

				DLCPack *pDLCPack=pDLCTexPack->getDLCInfoParentPack();//tPack->getDLCPack();
				if(!pDLCPack->hasPurchasedFile( DLCManager::e_DLCType_Texture, L"" ))
				{					
#ifdef _LINUX
					// upsell
					ULONGLONG ullOfferID_Full;
					// get the dlc texture pack
					DLCTexturePack *pDLCTexPack=(DLCTexturePack *)tPack;

					app.GetDLCFullOfferIDForPackID(pDLCTexPack->getDLCParentPackId(),&ullOfferID_Full);

					// tell sentient about the upsell of the full version of the skin pack
					TelemetryManager->RecordUpsellPresented(iPad, eSet_UpsellID_Texture_DLC, ullOfferID_Full & 0xFFFFFFFF);
#endif

					UINT uiIDA[2];
					uiIDA[0]=IDS_CONFIRM_OK;
					uiIDA[1]=IDS_CONFIRM_CANCEL;

					// Give the player a warning about the trial version of the texture pack
					ui.RequestMessageBox(IDS_WARNING_DLC_TRIALTEXTUREPACK_TITLE, IDS_WARNING_DLC_TRIALTEXTUREPACK_TEXT, uiIDA, 2, ProfileManager.GetPrimaryPad() ,&UIScene_PauseMenu::WarningTrialTexturePackReturned, dynamic_cast<IUIScene_PauseMenu*>(pClass),app.GetStringTable(), NULL, 0, false);

					return S_OK;					
				}
			}

			// does the save exist?
			bool bSaveExists;
			StorageManager.DoesSaveExist(&bSaveExists);
			// we check if the save exists inside the libs
			// we need to ask if they are sure they want to overwrite the existing game
			if(bSaveExists)
			{
				UINT uiIDA[2];
				uiIDA[0]=IDS_CONFIRM_CANCEL;
				uiIDA[1]=IDS_CONFIRM_OK;
				ui.RequestMessageBox(IDS_TITLE_SAVE_GAME, IDS_CONFIRM_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameAndSaveReturned, dynamic_cast<IUIScene_PauseMenu*>(pClass), app.GetStringTable(), NULL, 0, false);
				return 0;
			}
			else
			{
#if defined(_LINUX) || defined(_LINUX)
				StorageManager.SetSaveDisabled(false);
#endif
				MinecraftServer::getInstance()->setSaveOnExit( true );
			}
		}
		else
		{
			// been a few requests for a confirm on exit without saving
			UINT uiIDA[2];
			uiIDA[0]=IDS_CONFIRM_CANCEL;
			uiIDA[1]=IDS_CONFIRM_OK;
			ui.RequestMessageBox(IDS_TITLE_DECLINE_SAVE_GAME, IDS_CONFIRM_DECLINE_SAVE_GAME, uiIDA, 2, ProfileManager.GetPrimaryPad(),&IUIScene_PauseMenu::ExitGameDeclineSaveReturned, dynamic_cast<IUIScene_PauseMenu*>(pClass), app.GetStringTable(), NULL, 0, false);
			return 0;
		}

		app.SetAction(iPad,eAppAction_ExitWorld);
	}
	return 0;
}

#endif

void UIScene_PauseMenu::SetIgnoreInput(bool ignoreInput)
{
	m_bIgnoreInput = ignoreInput;
}

#ifdef _LINUX
void UIScene_PauseMenu::HandleDLCLicenseChange()
{	
}
#endif

#ifdef _LINUX
bool UIScene_PauseMenu::CheckForPatch()
{
	int npAvailability = ProfileManager.getNPAvailability(ProfileManager.GetPrimaryPad());

	bool bPatchAvailable;
	switch(npAvailability)
	{
	case SCE_NP_ERROR_LATEST_PATCH_PKG_EXIST:
	case SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED:
		bPatchAvailable=true;
		break;
	default:
		bPatchAvailable=false;
		break;
	}

	if(bPatchAvailable)
	{
		int32_t ret = sceErrorDialogInitialize();
		if (  ret==SCE_OK ) 
		{
			m_bErrorDialogRunning = true;

			SceErrorDialogParam param;
			sceErrorDialogParamInitialize( &param );
			// We want to display the option to get the patch now
			param.errorCode = SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED;//pClass->m_errorCode;
			ret = sceUserServiceGetInitialUser( &param.userId );
			if ( ret == SCE_OK ) 
			{
				ret = sceErrorDialogOpen( &param );
			}
			else 
			{
				sceErrorDialogTerminate();
			}
		}
	}

	return bPatchAvailable;
}
#endif