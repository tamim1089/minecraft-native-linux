#pragma once
using namespace std;

#include "Packet.h"
#include "SynchedEntityData.h"

class Player;

class AddPlayerPacket : public Packet, public enable_shared_from_this<AddPlayerPacket>
{

private:
	shared_ptr<SynchedEntityData> entityData;
    vector<shared_ptr<SynchedEntityData::DataItem> > *unpack;

public:
	int id;
    wstring name;
    int x, y, z;
    char yRot, xRot;
    int carriedItem;
	PlayerUID uid; // 
	PlayerUID OnlineUid; // 
	BYTE m_playerIndex; // 
	DWORD m_skinId; // 
	DWORD m_capeId; // 
	unsigned int m_uiGamePrivileges; // 
	byte yHeadRot; // 

	AddPlayerPacket();
	~AddPlayerPacket();
	AddPlayerPacket(shared_ptr<Player> player, PlayerUID uid, PlayerUID OnlineUid,int xp, int yp, int zp, int yRotp, int xRotp, int yHeadRotp);

	virtual void read(DataInputStream *dis);
	virtual void write(DataOutputStream *dos);
	virtual void handle(PacketListener *listener);
	virtual int getEstimatedSize();

	vector<shared_ptr<SynchedEntityData::DataItem> > *getUnpackedData();
public:
	static shared_ptr<Packet> create() { return shared_ptr<Packet>(new AddPlayerPacket()); }
	virtual int getId() { return 20; }
};
