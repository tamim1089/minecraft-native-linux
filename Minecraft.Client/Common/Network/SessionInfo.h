#pragma once

#if defined(_LINUX) || defined(_LINUX)
#include "../../Common/Network/Sony/SQRNetworkManager.h"
#endif


// A struct that we store in the QoS data when we are hosting the session. Max size 1020 bytes.
#ifdef _LINUX
typedef struct _GameSessionData
{
	unsigned short netVersion;										//   2 bytes
	char hostName[MAX_USERNAME_SIZE];									//  16 bytes ( 16*1 )
	char szPlayers[MINECRAFT_NET_MAX_PLAYERS][MAX_USERNAME_SIZE];		// 128 bytes ( 8*16)
	unsigned int m_uiGameHostSettings;								//   4 bytes
	unsigned int texturePackParentId;										//   4 bytes
	unsigned char subTexturePackId;									//   1 byte

	bool isJoinable;												//   1 byte

	_GameSessionData()
	{
		netVersion = 0;
		memset(hostName,0,MAX_USERNAME_SIZE);
		memset(players,0,MINECRAFT_NET_MAX_PLAYERS*sizeof(players[0]));
		memset(szPlayers,0,MINECRAFT_NET_MAX_PLAYERS*MAX_USERNAME_SIZE);
		isJoinable = true;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
	}
} GameSessionData;
#elif defined _LINUX || defined _LINUX || defined(_LINUX)
typedef struct _GameSessionData
{
	unsigned short netVersion;										//   2 bytes
	unsigned int m_uiGameHostSettings;								//   4 bytes
	unsigned int texturePackParentId;										//   4 bytes
	unsigned char subTexturePackId;									//   1 byte

	bool isJoinable;												//   1 byte

	unsigned char playerCount;										//   1 byte
	bool isReadyToJoin;												//   1 byte

	_GameSessionData()
	{
		netVersion = 0;
		memset(players,0,MINECRAFT_NET_MAX_PLAYERS*sizeof(players[0]));
		isJoinable = true;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
		playerCount = 0;
		isReadyToJoin = false;

	}
} GameSessionData;
#else
typedef struct _GameSessionData
{
	unsigned short netVersion;										//   2 bytes
	unsigned int m_uiGameHostSettings;								//   4 bytes
	unsigned int texturePackParentId;								//   4 bytes
	unsigned char subTexturePackId;									//   1 byte

	bool isReadyToJoin;												//   1 byte

	_GameSessionData()
	{
		netVersion = 0;
		m_uiGameHostSettings = 0;
		texturePackParentId = 0;
		subTexturePackId = 0;
	}
} GameSessionData;
#endif

class FriendSessionInfo
{
public:
	SessionID sessionId;
#ifdef _LINUX
	XSESSION_SEARCHRESULT searchResult;
#elif defined(_LINUX) || defined(_LINUX) || defined (_LINUX)
	SQRNetworkManager::SessionSearchResult searchResult;
#elif defined(_LINUX)
	DQRNetworkManager::SessionSearchResult searchResult;
#endif
	wchar_t *displayLabel;
	unsigned char displayLabelLength;
	unsigned char displayLabelViewableStartIndex;
	GameSessionData data;
	bool hasPartyMember;

	FriendSessionInfo()
	{
		displayLabel = NULL;
		displayLabelLength = 0;
		displayLabelViewableStartIndex = 0;
		hasPartyMember = false;
	}

	~FriendSessionInfo()
	{
		if(displayLabel!=NULL)
			delete displayLabel;
	}
};
