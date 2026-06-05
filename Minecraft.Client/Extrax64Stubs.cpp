#include "stdafx.h"
#ifndef _LINUX
//#include <compressapi.h>
#endif // _LINUX

#ifdef _LINUX
#include "Linux/Sentient/SentientManager.h"
#include "StatsCounter.h"
#include "Linux/Social/SocialManager.h"
#include <libsn.h>
#include <libsntuner.h>
#elif defined _LINUX
#include "Linux/Sentient/SentientManager.h"
#include "StatsCounter.h"
#include "Linux/Social/SocialManager.h"
#include "Linux/Sentient/DynamicConfigurations.h"
#include "Linux/DurangoExtras/xcompress.h"
#elif defined _LINUX
#include "Linux64/Sentient/SentientManager.h"
#include "StatsCounter.h"
#include "Linux64/Social/SocialManager.h"
#include "Linux64/Sentient/DynamicConfigurations.h"
#elif defined _LINUX
#include "Linux/Sentient/SentientManager.h"
#include "StatsCounter.h"
#include "Linux/Social/SocialManager.h"
#include "Linux/Sentient/DynamicConfigurations.h"
#include <libperf.h>
#else
#include "Linux/Sentient/SentientManager.h"
#include "StatsCounter.h"
#include "Linux/Social/SocialManager.h"
#include "Linux/Sentient/DynamicConfigurations.h"
#include <perf.h>
#endif

#if !defined(_LINUX) && !defined(_LINUX) && !defined(_LINUX)
#ifdef _LINUX
//CStorage StorageManager;
C_Profile ProfileManager;
#endif
#endif // _LINUX
CSentientManager SentientManager;
CXuiStringTable StringTable;

#ifndef _LINUX
ATG::XMLParser::XMLParser() {}
ATG::XMLParser::~XMLParser() {}
HRESULT    ATG::XMLParser::ParseXMLBuffer( CONST CHAR* strBuffer, UINT uBufferSize ) { return S_OK; }   
VOID ATG::XMLParser::RegisterSAXCallbackInterface( ISAXCallback *pISAXCallback ) {}
#endif

bool	CSocialManager::IsTitleAllowedToPostAnything() { return false; }
bool	CSocialManager::AreAllUsersAllowedToPostImages() { return false; }
bool	CSocialManager::IsTitleAllowedToPostImages() { return false; }

bool	CSocialManager::PostLinkToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect ) { return false; }
bool	CSocialManager::PostImageToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect ) { return false; }
CSocialManager *CSocialManager::Instance() { return NULL; }
void CSocialManager::SetSocialPostText(LPCWSTR Title, LPCWSTR Caption, LPCWSTR Desc) {};

DWORD XShowPartyUI(DWORD dwUserIndex) { return 0; }
DWORD XShowFriendsUI(DWORD dwUserIndex) { return 0; }
HRESULT XPartyGetUserList(XPARTY_USER_LIST *pUserList) { return S_OK; }
DWORD XContentGetThumbnail(DWORD dwUserIndex, const XCONTENT_DATA *pContentData,  PBYTE pbThumbnail,  PDWORD pcbThumbnail,  PXOVERLAPPED *pOverlapped) { return 0; }
void XShowAchievementsUI(int i) {}
DWORD XBackgroundDownloadSetMode(XBACKGROUND_DOWNLOAD_MODE Mode) { return 0; }

#ifndef _LINUX
void PIXAddNamedCounter(int a, char *b, ...) {}
//#define PS3_USE_PIX_EVENTS 
//#define PS4_USE_PIX_EVENTS 
void PIXBeginNamedEvent(int a, char *b, ...)
{
#ifdef PS4_USE_PIX_EVENTS
	char buf[512];
    va_list args;
    va_start(args,b);
    vsprintf(buf,b,args);
	sceRazorCpuPushMarker(buf, 0xffffffff, SCE_RAZOR_MARKER_ENABLE_HUD);

#endif
#ifdef PS3_USE_PIX_EVENTS
	char buf[256];
	wchar_t wbuf[256];
    va_list args;
    va_start(args,b);
    vsprintf(buf,b,args);
	snPushMarker(buf);

// 	mbstowcs(wbuf,buf,256);
// 	RenderManager.BeginEvent(wbuf);
    va_end(args);
#endif
}
#if 0//_LINUX
	if( PixDepth < 64 )
	{
		char buf[512];
		va_list args;
		va_start(args,b);
		vsprintf(buf,b,args);
		sceRazorCpuPushMarkerWithHud(buf, 0xffffffff, SCE_RAZOR_MARKER_ENABLE_HUD);
	}
	PixDepth += 1;
#endif


void PIXEndNamedEvent()
{
#ifdef PS4_USE_PIX_EVENTS
	sceRazorCpuPopMarker();
#endif
#ifdef PS3_USE_PIX_EVENTS
	snPopMarker();
// 	RenderManager.EndEvent();
#endif
#if 0//_LINUX
	if( PixDepth <= 64 )
	{
		sceRazorCpuPopMarker();
	}
	PixDepth -= 1;
#endif
}
void PIXSetMarkerDeprecated(int a, char *b, ...) {}
#else
// Removed this implementation in favour of a macro that will convert our string format
// conversion at compile time rather than at runtime
//void PIXBeginNamedEvent(int a, char *b, ...)
//{
//	char buf[256];
//	wchar_t wbuf[256];
//	va_list args;
//	va_start(args,b);
//	vsprintf(buf,b,args);
//	mbstowcs(wbuf,buf,256);
//	PIXBeginEvent(a,wbuf);
//}
//void PIXEndNamedEvent()
//{
//	PIXEndEvent();
//}
//void PIXSetMarkerDeprecated(int a, char *b, ...)
//{
//	char buf[256];
//	wchar_t wbuf[256];
//	va_list args;
//	va_start(args,b);
//	vsprintf(buf,b,args);
//	mbstowcs(wbuf,buf,256);
//	PIXSetMarker(a, wbuf);
//}
#endif

// void *D3DXBUFFER::GetBufferPointer() { return NULL; }
// int D3DXBUFFER::GetBufferSize() { return 0; }
// void D3DXBUFFER::Release() {}

// #ifdef _LINUX
// void GetLocalTime(SYSTEMTIME *time) {}
// #endif


bool IsEqualUID(PlayerUID a, PlayerUID b)
{
#if defined(_LINUX) || defined(_LINUX) || defined (_LINUX) || defined(_LINUX)
	return (a == b);
#else
	return false;
#endif
}

void XMemCpy(void *a, const void *b, size_t s) { memcpy(a, b, s); }
void XMemSet(void *a, int t, size_t s) { memset(a, t, s); }
void XMemSet128(void *a, int t, size_t s) { memset(a, t, s); }
void *XPhysicalAlloc(SIZE_T a, ULONG_PTR  b, ULONG_PTR c, DWORD d) { return malloc(a); }
void XPhysicalFree(void *a) { free(a); }

D3DXVECTOR3::D3DXVECTOR3() {}
D3DXVECTOR3::D3DXVECTOR3(float x,float y,float z) : x(x), y(y), z(z) {}
D3DXVECTOR3& D3DXVECTOR3::operator += ( CONST D3DXVECTOR3& add ) { x += add.x; y += add.y; z += add.z; return *this; }

BYTE IQNetPlayer::GetSmallId() { return 0; }
void IQNetPlayer::SendData(IQNetPlayer *player, const void *pvData, DWORD dwDataSize, DWORD dwFlags)
{
	app.DebugPrintf("Sending from 0x%x to 0x%x %d bytes/n",this,player,dwDataSize);
}
bool IQNetPlayer::IsSameSystem(IQNetPlayer *player) { return true; }
DWORD IQNetPlayer::GetSendQueueSize( IQNetPlayer *player, DWORD dwFlags ) { return 0; }
DWORD IQNetPlayer::GetCurrentRtt() { return 0; }
bool IQNetPlayer::IsHost() { return this == &IQNet::m_player[0]; }
bool IQNetPlayer::IsGuest() { return false; }
bool IQNetPlayer::IsLocal() { return true; }
PlayerUID IQNetPlayer::GetUid() { return INVALID_UID; }
LPCWSTR IQNetPlayer::GetUsername() { static const wchar_t *test = L"stub"; return test; }
int IQNetPlayer::GetSessionIndex() { return 0; }
bool IQNetPlayer::IsTalking() { return false; }
bool IQNetPlayer::IsMutedByLocalUser(DWORD dwUserIndex) { return false; }
bool IQNetPlayer::HasVoice() { return false; }
bool IQNetPlayer::HasCamera() { return false; }
int IQNetPlayer::GetUserIndex() { return this - &IQNet::m_player[0]; }
void IQNetPlayer::SetCustomDataValue(ULONG_PTR ulpCustomDataValue) {
	m_customData = ulpCustomDataValue;
}
ULONG_PTR IQNetPlayer::GetCustomDataValue() {
	return m_customData;
}

IQNetPlayer IQNet::m_player[4];

bool _bQNetStubGameRunning = false;

HRESULT IQNet::AddLocalPlayerByUserIndex(DWORD dwUserIndex){ return S_OK; }
IQNetPlayer *IQNet::GetHostPlayer() { return &m_player[0]; }
IQNetPlayer *IQNet::GetLocalPlayerByUserIndex(DWORD dwUserIndex) { return &m_player[dwUserIndex]; } 
IQNetPlayer *IQNet::GetPlayerByIndex(DWORD dwPlayerIndex) { return &m_player[0]; }
IQNetPlayer *IQNet::GetPlayerBySmallId(BYTE SmallId){ return &m_player[0]; }
IQNetPlayer *IQNet::GetPlayerByUid(PlayerUID uid){ return &m_player[0]; }
DWORD IQNet::GetPlayerCount() { return 1; }
QNET_STATE IQNet::GetState() { return _bQNetStubGameRunning ? QNET_STATE_GAME_PLAY : QNET_STATE_IDLE; }
bool IQNet::IsHost() { return true; }
HRESULT IQNet::JoinGameFromInviteInfo(DWORD dwUserIndex, DWORD dwUserMask, const INVITE_INFO *pInviteInfo) { return S_OK; }
void IQNet::HostGame() { _bQNetStubGameRunning = true; }
void IQNet::EndGame() { _bQNetStubGameRunning = false; }

DWORD MinecraftDynamicConfigurations::GetTrialTime() { return DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME; }

void XSetThreadProcessor(HANDLE a, int b) {}
// #if !(defined _LINUX) && !(defined _LINUX)
// BOOL XCloseHandle(HANDLE a) { return CloseHandle(a); }
// #endif // _LINUX

DWORD XUserGetSigninInfo(
         DWORD dwUserIndex,
         DWORD dwFlags,
         PXUSER_SIGNIN_INFO pSigninInfo
)
{
	return 0;
}

LPCWSTR CXuiStringTable::Lookup(LPCWSTR szId) { return szId; }
LPCWSTR CXuiStringTable::Lookup(UINT nIndex) { return L"String"; }
void CXuiStringTable::Clear() {}
HRESULT CXuiStringTable::Load(LPCWSTR szId) { return S_OK; }

DWORD XUserAreUsersFriends( DWORD dwUserIndex, PPlayerUID pUids, DWORD dwUidCount, PBOOL pfResult, void *pOverlapped) { return 0; }

#if defined _LINUX || defined _LINUX || defined _LINUX
#else
HRESULT XMemDecompress(
         XMEMDECOMPRESSION_CONTEXT Context,
         VOID *pDestination,
         SIZE_T *pDestSize,
         CONST VOID *pSource,
         SIZE_T SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
	if( Decompress(
        Decompressor,           //  Decompressor handle
        (void *)pSource,		//  Compressed data
        SrcSize,				//  Compressed data size
        pDestination,			//  Decompressed buffer
        *pDestSize,				//  Decompressed buffer size
        pDestSize) )				//  Decompressed data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCompress(
         XMEMCOMPRESSION_CONTEXT Context,
         VOID *pDestination,
         SIZE_T *pDestSize,
         CONST VOID *pSource,
         SIZE_T SrcSize
)
{
	memcpy(pDestination, pSource, SrcSize);
	*pDestSize = SrcSize;
	return S_OK;

	/*
	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
	if( Compress(
			Compressor,                  //  Compressor Handle
			(void *)pSource,             //  Input buffer, Uncompressed data
			SrcSize,					 //  Uncompressed data size
			pDestination,                //  Compressed Buffer
			*pDestSize,                  //  Compressed Buffer size
			pDestSize)	)				//  Compressed Data size
	{
		return S_OK;
	}
	else
	*/
	{
		return E_FAIL;
	}
}

HRESULT XMemCreateCompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST VOID *pCodecParams,
         DWORD Flags,
         XMEMCOMPRESSION_CONTEXT *pContext
)
{
	/*
	COMPRESSOR_HANDLE Compressor    = NULL;

	HRESULT hr = CreateCompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&Compressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Compressor;
	return hr;
	*/
	return 0;
}

HRESULT XMemCreateDecompressionContext(
         XMEMCODEC_TYPE CodecType,
         CONST VOID *pCodecParams,
         DWORD Flags,
         XMEMDECOMPRESSION_CONTEXT *pContext
)
{
	/*
	DECOMPRESSOR_HANDLE  Decompressor    = NULL;

	HRESULT hr = CreateDecompressor(
		COMPRESS_ALGORITHM_XPRESS_HUFF, //  Compression Algorithm
		NULL,                           //  Optional allocation routine
		&Decompressor);                   //  Handle

	pContext = (XMEMDECOMPRESSION_CONTEXT *)Decompressor;
	return hr;
	*/
	return 0;
}

void XMemDestroyCompressionContext(XMEMCOMPRESSION_CONTEXT Context)
{
//	COMPRESSOR_HANDLE Compressor    = (COMPRESSOR_HANDLE)Context;
//	CloseCompressor(Compressor);
}

void XMemDestroyDecompressionContext(XMEMDECOMPRESSION_CONTEXT Context)
{
//	DECOMPRESSOR_HANDLE Decompressor    = (DECOMPRESSOR_HANDLE)Context;
//	CloseDecompressor(Decompressor);
}
#endif

//#ifndef _LINUX
#if !(defined _LINUX || defined _LINUX || defined _LINUX || defined _LINUX)
DWORD XGetLanguage() { return 1; }
DWORD XGetLocale() { return 0; }
DWORD XEnableGuestSignin(BOOL fEnable) { return 0; }
#endif



/////////////////////////////////////////////// Profile library
#ifdef _LINUX
static void *profileData[4];
static bool s_bProfileIsFullVersion;
void				C_Profile::Initialise( DWORD dwTitleID,
								DWORD dwOfferID,
								unsigned short usProfileVersion,
								UINT uiProfileValuesC,
								UINT uiProfileSettingsC,
								DWORD *pdwProfileSettingsA, 
								int iGameDefinedDataSizeX4,
								unsigned int *puiGameDefinedDataChangedBitmask)
{
	for( int i = 0; i < 4; i++ )
	{
		profileData[i] = new byte[iGameDefinedDataSizeX4/4];
		ZeroMemory(profileData[i],sizeof(byte)*iGameDefinedDataSizeX4/4);

		// Set some sane initial values!
		GAME_SETTINGS *pGameSettings = (GAME_SETTINGS *)profileData[i];
		pGameSettings->ucMenuSensitivity=100; //eGameSetting_Sensitivity_InMenu
		pGameSettings->ucInterfaceOpacity=80; //eGameSetting_Sensitivity_InMenu
		pGameSettings->usBitmaskValues|=0x0200; //eGameSetting_DisplaySplitscreenUsernames - on
		pGameSettings->usBitmaskValues|=0x0400; //eGameSetting_Hints - on
		pGameSettings->usBitmaskValues|=0x1000; //eGameSetting_Autosave - 2
		pGameSettings->usBitmaskValues|=0x8000; //eGameSetting_Tooltips - on
		pGameSettings->uiBitmaskValues=0L; // reset
		pGameSettings->uiBitmaskValues|=GAMESETTING_CLOUDS;					//eGameSetting_Clouds - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_ONLINE;					//eGameSetting_GameSetting_Online - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_FRIENDSOFFRIENDS;		//eGameSetting_GameSetting_FriendsOfFriends - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYUPDATEMSG;		//eGameSetting_DisplayUpdateMessage (counter)
		pGameSettings->uiBitmaskValues&=~GAMESETTING_BEDROCKFOG;			//eGameSetting_BedrockFog - off
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHUD;				//eGameSetting_DisplayHUD - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DISPLAYHAND;			//eGameSetting_DisplayHand - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_CUSTOMSKINANIM;			//eGameSetting_CustomSkinAnim - on
		pGameSettings->uiBitmaskValues|=GAMESETTING_DEATHMESSAGES;			//eGameSetting_DeathMessages - on
		pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE&0x00000800);				// uisize 2
		pGameSettings->uiBitmaskValues|=(GAMESETTING_UISIZE_SPLITSCREEN&0x00004000);	// splitscreen ui size 3
		pGameSettings->uiBitmaskValues|=GAMESETTING_ANIMATEDCHARACTER;		//eGameSetting_AnimatedCharacter - on

		// TU12
		// favorite skins added, but only set in TU12 - set to FFs
		for(int i=0;i<MAX_FAVORITE_SKINS;i++)
		{
			pGameSettings->uiFavoriteSkinA[i]=0xFFFFFFFF;
		}
		pGameSettings->ucCurrentFavoriteSkinPos=0;
		// Added a bitmask in TU13 to enable/disable display of the Mash-up pack worlds in the saves list
		pGameSettings->uiMashUpPackWorldsDisplay = 0xFFFFFFFF;

		// PS3DEC13
		pGameSettings->uiBitmaskValues&=~GAMESETTING_PS3EULAREAD;		//eGameSetting_PS3_EULA_Read - off

		pGameSettings->ucLanguage = MINECRAFT_LANGUAGE_DEFAULT; // use the system language

		// PS Vita - network mode added
		pGameSettings->uiBitmaskValues&=~GAMESETTING_NETWORKMODEADHOC;		//eGameSetting_Linux_NetworkModeAdhoc - off


		// Tutorials for most menus, and a few other things
		pGameSettings->ucTutorialCompletion[0] = 0xFF;
		pGameSettings->ucTutorialCompletion[1] = 0xFF;
		pGameSettings->ucTutorialCompletion[2] = 0xF;

		// Has gone halfway through the tutorial
		pGameSettings->ucTutorialCompletion[28] |= 1<<0;
	}
}
void				C_Profile::SetTrialTextStringTable(CXuiStringTable *pStringTable,int iAccept,int iReject) {}
void				C_Profile::SetTrialAwardText(eAwardType AwardType,int iTitle,int iText) {}
int					C_Profile::GetLockedProfile() { return 0; }
void				C_Profile::SetLockedProfile(int iProf) {}
bool				C_Profile::IsSignedIn(int iQuadrant) { return ( iQuadrant == 0); }
bool				C_Profile::IsSignedInLive(int iProf) { return true; }
bool				C_Profile::IsGuest(int iQuadrant) { return false; }
UINT				C_Profile::RequestSignInUI(bool bFromInvite,bool bLocalGame,bool bNoGuestsAllowed,bool bMultiplayerSignIn,bool bAddUser, int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant) { return 0; }
UINT				C_Profile::DisplayOfflineProfile(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant)  { return 0; }
UINT				C_Profile::RequestConvertOfflineToGuestUI(int( *Func)(LPVOID,const bool, const int iPad),LPVOID lpParam,int iQuadrant) { return 0; }
void				C_Profile::SetPrimaryPlayerChanged(bool bVal) {}
bool				C_Profile::QuerySigninStatus(void) { return true; }
void				C_Profile::GetUID(int iPad, PlayerUID *pUid,bool bOnlineUid) {*pUid = 0xe000d45248242f2e; }
BOOL				C_Profile::AreUIDSEqual(PlayerUID uid1,PlayerUID uid2) { return false; }
BOOL				C_Profile::UIDIsGuest(PlayerUID uid) { return false; }
bool				C_Profile::AllowedToPlayMultiplayer(int iProf) { return true; }

#if defined(_LINUX)
bool				C_Profile::GetChatAndContentRestrictions(int iPad, bool thisQuadrantOnly, bool *pbChatRestricted,bool *pbContentRestricted,int *piAge)
{
	if(pbChatRestricted) *pbChatRestricted = false;
	if(pbContentRestricted) *pbContentRestricted = false;
	if(piAge) *piAge = 100;
	return true;
}
#endif

void				C_Profile::StartTrialGame() {}
void				C_Profile::AllowedPlayerCreatedContent(int iPad, bool thisQuadrantOnly, BOOL *allAllowed, BOOL *friendsAllowed) {}
BOOL				C_Profile::CanViewPlayerCreatedContent(int iPad, bool thisQuadrantOnly, PPlayerUID pUids, DWORD dwUidCount ) { return true; }
bool				C_Profile::GetProfileAvatar(int iPad,int( *Func)(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes), LPVOID lpParam) { return false; }
void				C_Profile::CancelProfileAvatarRequest() {}
int					C_Profile::GetPrimaryPad() { return 0; }
void				C_Profile::SetPrimaryPad(int iPad) {}
#ifdef _LINUX
char fakeUsername[32] = "PlayerName";
void				SetFakeUsername(char *name){ strcpy_s(fakeUsername, name); }
char*				C_Profile::GetUsername(int iPad){ return fakeUsername; }
#else
char*				C_Profile::GetUsername(int iPad){ return "PlayerName"; }
wstring				C_Profile::GetDisplayName(int iPad){ return L"PlayerName"; }
#endif
bool				C_Profile::IsFullVersion() { return s_bProfileIsFullVersion; }
void				C_Profile::SetSignInChangeCallback(void ( *Func)(LPVOID, bool, unsigned int),LPVOID lpParam) {}
void				C_Profile::SetNotificationsCallback(void ( *Func)(LPVOID, DWORD, unsigned int),LPVOID lpParam) {}
bool				C_Profile::RegionIsNorthAmerica(void) { return false; }
bool				C_Profile::LocaleIsUSorCanada(void) { return false; }
HRESULT				C_Profile::GetLiveConnectionStatus() { return S_OK; }
bool				C_Profile::IsSystemUIDisplayed() { return false; }
void				C_Profile::SetProfileReadErrorCallback(void ( *Func)(LPVOID), LPVOID lpParam) {}
int( *defaultOptionsCallback)(LPVOID,C_Profile::PROFILESETTINGS *, const int iPad) = NULL;
LPVOID lpProfileParam = NULL;
int					C_Profile::SetDefaultOptionsCallback(int( *Func)(LPVOID,PROFILESETTINGS *, const int iPad),LPVOID lpParam)
{
	defaultOptionsCallback = Func;
	lpProfileParam = lpParam;
	return 0;
}
int					C_Profile::SetOldProfileVersionCallback(int( *Func)(LPVOID,unsigned char *, const unsigned short,const int),LPVOID lpParam) { return 0; }

// To store the dashboard preferences for controller flipped, etc.
C_Profile::PROFILESETTINGS ProfileSettingsA[XUSER_MAX_COUNT];

C_Profile::PROFILESETTINGS *	C_Profile::GetDashboardProfileSettings(int iPad) { return &ProfileSettingsA[iPad]; }
void				C_Profile::WriteToProfile(int iQuadrant, bool bGameDefinedDataChanged, bool bOverride5MinuteLimitOnProfileWrites) {}
void				C_Profile::ForceQueuedProfileWrites(int iPad) {}
void				*C_Profile::GetGameDefinedProfileData(int iQuadrant)
{
	// Don't reset the options when we call this!!
	//defaultOptionsCallback(lpProfileParam, (C_Profile::PROFILESETTINGS *)profileData[iQuadrant], iQuadrant);
	//pApp->SetDefaultOptions(pSettings,iPad);

#ifdef _LINUX
	// Headless port: profileData[] is never populated (no profile backend). Return a real zeroed
	// per-user GAME_SETTINGS-sized buffer so app.GameSettingsA[] is valid. TODO(port).
	static unsigned char s_profileBuf[4][512] = {{0}};
	if (iQuadrant < 0 || iQuadrant >= 4) iQuadrant = 0;
	return s_profileBuf[iQuadrant];
#endif
	return profileData[iQuadrant];
}
void				C_Profile::ResetProfileProcessState() {}
void				C_Profile::Tick( void ) {}
void				C_Profile::RegisterAward(int iAwardNumber,int iGamerconfigID, eAwardType eType, bool bLeaderboardAffected, 
	CXuiStringTable*pStringTable, int iTitleStr, int iTextStr, int iAcceptStr, char *pszThemeName, unsigned int ulThemeSize) {}
int					C_Profile::GetAwardId(int iAwardNumber) { return 0; }
eAwardType			C_Profile::GetAwardType(int iAwardNumber) { return eAwardType_Achievement; }
bool				C_Profile::CanBeAwarded(int iQuadrant, int iAwardNumber) { return false; }
void				C_Profile::Award(int iQuadrant, int iAwardNumber, bool bForce) {}
bool				C_Profile::IsAwardsFlagSet(int iQuadrant, int iAward) { return false; }
void				C_Profile::RichPresenceInit(int iPresenceCount, int iContextCount) {}
void				C_Profile::RegisterRichPresenceContext(int iGameConfigContextID) {}
void				C_Profile::SetRichPresenceContextValue(int iPad,int iContextID, int iVal) {}
void				C_Profile::SetCurrentGameActivity(int iPad,int iNewPresence, bool bSetOthersToIdle) {}
void				C_Profile::DisplayFullVersionPurchase(bool bRequired, int iQuadrant, int iUpsellParam) {}
void				C_Profile::SetUpsellCallback(void ( *Func)(LPVOID lpParam, eUpsellType type, eUpsellResponse response, int iUserData),LPVOID lpParam) {}
void				C_Profile::SetDebugFullOverride(bool bVal) {s_bProfileIsFullVersion = bVal;}
void				C_Profile::ShowProfileCard(int iPad, PlayerUID targetUid) {}

/////////////////////////////////////////////// Storage library
//#ifdef _LINUX
#if 0
CStorage::CStorage() {}
void								CStorage::Tick() {}
CStorage::EMessageResult			CStorage::RequestMessageBox(UINT uiTitle, UINT uiText, UINT *uiOptionA,UINT uiOptionC, DWORD dwPad, int( *Func)(LPVOID,int,const CStorage::EMessageResult),LPVOID lpParam, CStringTable *pStringTable, WCHAR *pwchFormatString,DWORD dwFocusButton) { return CStorage::EMessage_Undefined; }
CStorage::EMessageResult			CStorage::GetMessageBoxResult()  { return CStorage::EMessage_Undefined; }
bool								CStorage::SetSaveDevice(int( *Func)(LPVOID,const bool),LPVOID lpParam, bool bForceResetOfSaveDevice) { return true; }
void								CStorage::Init(LPCWSTR pwchDefaultSaveName,char *pszSavePackName,int iMinimumSaveSize, int( *Func)(LPVOID, const ESavingMessage, int),LPVOID lpParam) {}
void								CStorage::ResetSaveData() {}
void								CStorage::SetDefaultSaveNameForKeyboardDisplay(LPCWSTR pwchDefaultSaveName) {}
void								CStorage::SetSaveTitle(LPCWSTR pwchDefaultSaveName) {}
LPCWSTR								CStorage::GetSaveTitle() { return L""; }
bool								CStorage::GetSaveUniqueNumber(INT *piVal) { return true; }
bool								CStorage::GetSaveUniqueFilename(char *pszName) { return true; }
void								CStorage::SetSaveUniqueFilename(char *szFilename) { }
void								CStorage::SetState(ESaveGameControlState eControlState,int( *Func)(LPVOID,const bool),LPVOID lpParam) {}
void								CStorage::SetSaveDisabled(bool bDisable) {}
bool								CStorage::GetSaveDisabled(void) { return false; }
unsigned int						CStorage::GetSaveSize() { return 0; }
void								CStorage::GetSaveData(void *pvData,unsigned int *pulBytes) {}
PVOID								CStorage::AllocateSaveData(unsigned int ulBytes) { return new char[ulBytes]; }
void								CStorage::SaveSaveData(unsigned int ulBytes,PBYTE pbThumbnail,DWORD cbThumbnail,PBYTE pbTextData, DWORD dwTextLen) {}
void								CStorage::CopySaveDataToNewSave(PBYTE pbThumbnail,DWORD cbThumbnail,WCHAR *wchNewName,int ( *Func)(LPVOID lpParam, bool), LPVOID lpParam) {}
void								CStorage::SetSaveDeviceSelected(unsigned int uiPad,bool bSelected) {}
bool								CStorage::GetSaveDeviceSelected(unsigned int iPad) { return true; }
CStorage::ELoadGameStatus			CStorage::DoesSaveExist(bool *pbExists) { return CStorage::ELoadGame_Idle; }
bool								CStorage::EnoughSpaceForAMinSaveGame() { return true; }
void								CStorage::SetSaveMessageVPosition(float fY) {}
//CStorage::ESGIStatus				CStorage::GetSavesInfo(int iPad,bool ( *Func)(LPVOID, int, CACHEINFOSTRUCT *, int, HRESULT),LPVOID lpParam,char *pszSavePackName) { return CStorage::ESGIStatus_Idle; }
CStorage::ESaveGameState			CStorage::GetSavesInfo(int iPad,int ( *Func)(LPVOID lpParam,SAVE_DETAILS *pSaveDetails,const bool),LPVOID lpParam,char *pszSavePackName) { return CStorage::ESaveGame_Idle; }

void								CStorage::GetSaveCacheFileInfo(DWORD dwFile,XCONTENT_DATA &xContentData) {}
void								CStorage::GetSaveCacheFileInfo(DWORD dwFile,	PBYTE *ppbImageData, DWORD *pdwImageBytes) {}
CStorage::ESaveGameState			CStorage::LoadSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool, const bool), LPVOID lpParam) {return CStorage::ESaveGame_Idle;}
CStorage::EDeleteGameStatus		CStorage::DeleteSaveData(PSAVE_INFO pSaveInfo,int( *Func)(LPVOID lpParam,const bool), LPVOID lpParam) { return CStorage::EDeleteGame_Idle; }
PSAVE_DETAILS						CStorage::ReturnSavesInfo() {return NULL;}

void								CStorage::RegisterMarketplaceCountsCallback(int ( *Func)(LPVOID lpParam, CStorage::DLC_TMS_DETAILS *, int), LPVOID lpParam ) {}
void								CStorage::SetDLCPackageRoot(char *pszDLCRoot) {}
CStorage::EDLCStatus				CStorage::GetDLCOffers(int iPad,int( *Func)(LPVOID, int, DWORD, int),LPVOID lpParam, DWORD dwOfferTypesBitmaskT) { return CStorage::EDLC_Idle; }
DWORD								CStorage::CancelGetDLCOffers() { return 0; }
void								CStorage::ClearDLCOffers() {}
XMARKETPLACE_CONTENTOFFER_INFO&		CStorage::GetOffer(DWORD dw) { static XMARKETPLACE_CONTENTOFFER_INFO retval = {0}; return retval; }
int									CStorage::GetOfferCount() { return 0; }
DWORD								CStorage::InstallOffer(int iOfferIDC,ULONGLONG *ullOfferIDA,int( *Func)(LPVOID, int, int),LPVOID lpParam, bool bTrial) { return 0; }
DWORD								CStorage::GetAvailableDLCCount( int iPad) { return 0; }
XCONTENT_DATA&						CStorage::GetDLC(DWORD dw) { static XCONTENT_DATA retval = {0}; return retval; }
CStorage::EDLCStatus				CStorage::GetInstalledDLC(int iPad,int( *Func)(LPVOID, int, int),LPVOID lpParam) { return CStorage::EDLC_Idle; }
DWORD								CStorage::MountInstalledDLC(int iPad,DWORD dwDLC,int( *Func)(LPVOID, int, DWORD,DWORD),LPVOID lpParam,LPCSTR szMountDrive) { return 0; }
DWORD								CStorage::UnmountInstalledDLC(LPCSTR szMountDrive) { return 0; }
CStorage::ETMSStatus				CStorage::ReadTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,CStorage::eTMS_FileType eFileType, WCHAR *pwchFilename,BYTE **ppBuffer,DWORD *pdwBufferSize,int( *Func)(LPVOID, WCHAR *,int, bool, int),LPVOID lpParam, int iAction) { return CStorage::ETMSStatus_Idle; }
bool								CStorage::WriteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename,BYTE *pBuffer,DWORD dwBufferSize) { return true; }
bool								CStorage::DeleteTMSFile(int iQuadrant,eGlobalStorage eStorageFacility,WCHAR *pwchFilename) { return true; }
void								CStorage::StoreTMSPathName(WCHAR *pwchName) {}
unsigned int						CStorage::CRC(unsigned char *buf, int len) { return 0; }

struct PTMSPP_FILEDATA;
CStorage::ETMSStatus				CStorage::TMSPP_ReadFile(int iPad,CStorage::eGlobalStorage eStorageFacility,CStorage::eTMS_FILETYPEVAL eFileTypeVal,LPCSTR szFilename,int( *Func)(LPVOID,int,int,PTMSPP_FILEDATA, LPCSTR)/*=NULL*/,LPVOID lpParam/*=NULL*/, int iUserData/*=0*/) {return CStorage::ETMSStatus_Idle;}
#endif // _LINUX

#endif // _LINUX

/////////////////////////////////////////////////////// Sentient manager

HRESULT CSentientManager::Init() { return S_OK; }
HRESULT CSentientManager::Tick() { return S_OK; }
HRESULT CSentientManager::Flush() { return S_OK; }
BOOL CSentientManager::RecordPlayerSessionStart(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordPlayerSessionExit(DWORD dwUserId, int exitStatus) { return true; }
BOOL CSentientManager::RecordHeartBeat(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordLevelStart(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers) { return true; }
BOOL CSentientManager::RecordLevelExit(DWORD dwUserId, ESen_LevelExitStatus levelExitStatus) { return true; }
BOOL CSentientManager::RecordLevelSaveOrCheckpoint(DWORD dwUserId, INT saveOrCheckPointID, INT saveSizeInBytes) { return true; }
BOOL CSentientManager::RecordLevelResume(DWORD dwUserId, ESen_FriendOrMatch friendsOrMatch, ESen_CompeteOrCoop competeOrCoop, int difficulty, DWORD numberOfLocalPlayers, DWORD numberOfOnlinePlayers, INT saveOrCheckPointID)  { return true; }
BOOL CSentientManager::RecordPauseOrInactive(DWORD dwUserId)  { return true; }
BOOL CSentientManager::RecordUnpauseOrActive(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordMenuShown(DWORD dwUserId, INT menuID, INT optionalMenuSubID) { return true; }
BOOL CSentientManager::RecordAchievementUnlocked(DWORD dwUserId, INT achievementID, INT achievementGamerscore) { return true; }
BOOL CSentientManager::RecordMediaShareUpload(DWORD dwUserId, ESen_MediaDestination mediaDestination, ESen_MediaType mediaType) { return true; }
BOOL CSentientManager::RecordUpsellPresented(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID) { return true; }
BOOL CSentientManager::RecordUpsellResponded(DWORD dwUserId, ESen_UpsellID upsellId, INT marketplaceOfferID, ESen_UpsellOutcome upsellOutcome) { return true; }
BOOL CSentientManager::RecordPlayerDiedOrFailed(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
BOOL CSentientManager::RecordEnemyKilledOrOvercome(DWORD dwUserId, INT lowResMapX, INT lowResMapY, INT lowResMapZ, INT mapID, INT playerWeaponID, INT enemyWeaponID, ETelemetryChallenges enemyTypeID) { return true; }
BOOL CSentientManager::RecordSkinChanged(DWORD dwUserId, DWORD dwSkinId) { return true; }
BOOL CSentientManager::RecordBanLevel(DWORD dwUserId) { return true; }
BOOL CSentientManager::RecordUnBanLevel(DWORD dwUserId) { return true; }
INT CSentientManager::GetMultiplayerInstanceID() { return 0; }
INT CSentientManager::GenerateMultiplayerInstanceId() { return 0; }
void CSentientManager::SetMultiplayerInstanceId(INT value) {}

////////////////////////////////////////////////////////  Stats counter

/*
StatsCounter::StatsCounter() {}
void StatsCounter::award(Stat *stat, unsigned int difficulty, unsigned int count) {}
bool StatsCounter::hasTaken(Achievement *ach) { return true; }
bool StatsCounter::canTake(Achievement *ach) { return true; }
unsigned int StatsCounter::getValue(Stat *stat, unsigned int difficulty) { return 0; }
unsigned int StatsCounter::getTotalValue(Stat *stat) { return 0; }
void StatsCounter::tick(int player) {}
void StatsCounter::parse(void* data) {}
void StatsCounter::clear() {}
void StatsCounter::save(int player, bool force) {}
void StatsCounter::flushLeaderboards() {}
void StatsCounter::saveLeaderboards() {}
void StatsCounter::setupStatBoards() {}
#ifdef _DEBUG
void StatsCounter::WipeLeaderboards() {}
#endif
*/
