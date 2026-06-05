#pragma once
using namespace std;

#include "Packet.h"
class LevelType;

class LoginPacket : public Packet, public enable_shared_from_this<LoginPacket>
{
public:
	int clientVersion;
	wstring userName;
	__int64 seed;
	char dimension;
	PlayerUID m_offlineUid, m_onlineUid;			// 
	char difficulty;	// 	
	bool m_friendsOnlyUGC; // 
	DWORD m_ugcPlayersVersion; // 
	INT m_multiplayerInstanceId; // for sentient
	BYTE m_playerIndex; // 
	DWORD m_playerSkinId, m_playerCapeId; // 
	bool m_isGuest; // 
	bool m_newSeaLevel; // 
	LevelType *m_pLevelType;
	unsigned int m_uiGamePrivileges;
	int m_xzSize; // 
	int m_hellScale; // 

	// 1.8.2
	int gameType;
	BYTE mapHeight;
	BYTE maxPlayers;

	LoginPacket();
	LoginPacket(const wstring& userName, int clientVersion, LevelType *pLevelType, __int64 seed, int gameType, char dimension, BYTE mapHeight, BYTE maxPlayers, char difficulty, INT m_multiplayerInstanceId, BYTE playerIndex, bool newSeaLevel, unsigned int uiGamePrivileges, int xzSize, int hellScale); // Server -> Client
	LoginPacket(const wstring& userName, int clientVersion, PlayerUID offlineUid, PlayerUID onlineUid, bool friendsOnlyUGC, DWORD ugcPlayersVersion, DWORD skinId, DWORD capeId, bool isGuest); // Client -> Server

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new LoginPacket()); }
	virtual int getId() { return 1; }
};
