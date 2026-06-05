#include "stdafx.h"
#include "PendingConnection.h"
#include "PlayerConnection.h"
#include "ServerConnection.h"
#include "ServerPlayer.h"
#include "ServerPlayerGameMode.h"
#include "ServerLevel.h"
#include "PlayerList.h"
#include "MinecraftServer.h"
#include "../Minecraft.World/net.minecraft.network.h"
#include "../Minecraft.World/Pos.h"
#include "../Minecraft.World/net.minecraft.world.level.dimension.h"
#include "../Minecraft.World/net.minecraft.world.level.storage.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/SharedConstants.h"
#include "Settings.h"
// #ifdef _LINUX
// #endif

Random *PendingConnection::random = new Random();

PendingConnection::PendingConnection(MinecraftServer *server, Socket *socket, const wstring& id)
{
	// added initialisers
	done = false;
    _tick = 0;
    name = L"";
    acceptedLogin = nullptr;
	loginKey = L"";

    this->server = server;
    connection = new Connection(socket, id, this);
    connection->fakeLag = FAKE_LAG;
}

PendingConnection::~PendingConnection()
{
	delete connection;
}

void PendingConnection::tick()
{
    if (acceptedLogin != NULL)
	{
        this->handleAcceptedLogin(acceptedLogin);
        acceptedLogin = nullptr;
    }
    if (_tick++ == MAX_TICKS_BEFORE_LOGIN)
	{
        disconnect(DisconnectPacket::eDisconnect_LoginTooLong);
    }
	else
	{
        connection->tick();
    }
}

void PendingConnection::disconnect(DisconnectPacket::eDisconnectReason reason)
{
 //   try {	// removed try/catch
//        logger.info("Disconnecting " + getName() + ": " + reason);
		app.DebugPrintf("Pending connection disconnect: %d/n", reason );
        connection->send( shared_ptr<DisconnectPacket>( new DisconnectPacket(reason) ) );
        connection->sendAndQuit();
        done = true;
//    } catch (Exception e) {
//        e.printStackTrace();
//    }
}

void PendingConnection::handlePreLogin(shared_ptr<PreLoginPacket> packet)
{
    if (packet->m_netcodeVersion != MINECRAFT_NET_VERSION)
	{
		app.DebugPrintf("Netcode version is %d not equal to %d/n", packet->m_netcodeVersion, MINECRAFT_NET_VERSION);
        if (packet->m_netcodeVersion > MINECRAFT_NET_VERSION)
		{
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        }
		else
		{
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }
//	printf("Server: handlePreLogin/n");
	name = packet->loginKey; // Change from the login packet as we know better on client end during the pre-login packet
	sendPreLoginResponse();
}

void PendingConnection::sendPreLoginResponse()
{
	// Calculate the players with UGC privileges set
	PlayerUID *ugcUids = new PlayerUID[MINECRAFT_NET_MAX_PLAYERS];
	DWORD ugcUidCount = 0;
	DWORD hostIndex = 0;
	BYTE ugcFriendsOnlyBits = 0;
	char szUniqueMapName[14];

	StorageManager.GetSaveUniqueFilename(szUniqueMapName);

	PlayerList *playerList = MinecraftServer::getInstance()->getPlayers();
	for(AUTO_VAR(it, playerList->players.begin()); it != playerList->players.end(); ++it)
	{
		shared_ptr<ServerPlayer> player = *it;
		
		// PADDY - this is failing when a local player with chat restrictions joins an online game

		if( player != NULL && player->connection->m_offlineUID != INVALID_UID && player->connection->m_onlineUID != INVALID_UID )
		{
			if( player->connection->m_friendsOnlyUGC )
			{
				ugcFriendsOnlyBits |= (1<<ugcUidCount);
			}
			ugcUids[ugcUidCount] = player->connection->m_onlineUID;

			if( player->connection->getNetworkPlayer() != NULL && player->connection->getNetworkPlayer()->IsHost() ) hostIndex = ugcUidCount;

			++ugcUidCount;
		}
	}

#if 0
    if (false)//	server->onlineMode) // removed
	{
        loginKey = L"TOIMPLEMENT"; // todo Long.toHexString(random.nextLong());
        connection->send( shared_ptr<PreLoginPacket>( new PreLoginPacket(loginKey, ugcUids, ugcUidCount, ugcFriendsOnlyBits, server->m_ugcPlayersVersion, szUniqueMapName,app.GetGameHostOption(eGameHostOption_All),hostIndex) ) );
    }
	else
#endif
	{
        connection->send( shared_ptr<PreLoginPacket>( new PreLoginPacket(L"-", ugcUids, ugcUidCount, ugcFriendsOnlyBits, server->m_ugcPlayersVersion,szUniqueMapName,app.GetGameHostOption(eGameHostOption_All),hostIndex, server->m_texturePackId) ) );
    }
}

void PendingConnection::handleLogin(shared_ptr<LoginPacket> packet)
{
//	printf("Server: handleLogin/n");
    //name = packet->userName;
    if (packet->clientVersion != SharedConstants::NETWORK_PROTOCOL_VERSION)
	{
		app.DebugPrintf("Client version is %d not equal to %d/n", packet->clientVersion, SharedConstants::NETWORK_PROTOCOL_VERSION);
        if (packet->clientVersion > SharedConstants::NETWORK_PROTOCOL_VERSION)
		{
            disconnect(DisconnectPacket::eDisconnect_OutdatedServer);
        }
		else
		{
            disconnect(DisconnectPacket::eDisconnect_OutdatedClient);
        }
        return;
    }

    //if (true)//  !server->onlineMode)
	bool sentDisconnect = false;

	if( sentDisconnect )
	{
		// Do nothing
	}
	else if( server->getPlayers()->isUidBanned( packet->m_onlineUid ) )
	{
		disconnect(DisconnectPacket::eDisconnect_Banned);
	}
	else
	{
        handleAcceptedLogin(packet);
    }
	//else
	{
		//removed
#if 0 
        new Thread() {
            public void run() {
                try {
                    String key = loginKey;
                    URL url = new URL("http://www.minecraft.net/game/checkserver.jsp?user=" + URLEncoder.encode(packet.userName, "UTF-8") + "&serverId=" + URLEncoder.encode(key, "UTF-8"));
                    BufferedReader br = new BufferedReader(new InputStreamReader(url.openStream()));
                    String msg = br.readLine();
                    br.close();
                    if (msg.equals("YES")) {
                        acceptedLogin = packet;
                    } else {
                        disconnect("Failed to verify username!");
                    }
                } catch (Exception e) {
                    disconnect("Failed to verify username! [internal error " + e + "]");
                    e.printStackTrace();
                }
            }
        }.start();
#endif
    }

}

void PendingConnection::handleAcceptedLogin(shared_ptr<LoginPacket> packet)
{
	if(packet->m_ugcPlayersVersion != server->m_ugcPlayersVersion)
	{
		// Send the pre-login packet again with the new list of players
		sendPreLoginResponse();
		return;
	}

	PlayerUID playerUid = packet->m_offlineUid;
	if(playerUid == INVALID_UID) playerUid = packet->m_onlineUid;

    shared_ptr<ServerPlayer> playerEntity = server->getPlayers()->getPlayerForLogin(this, name, playerUid,packet->m_onlineUid);
    if (playerEntity != NULL)
	{
        server->getPlayers()->placeNewPlayer(connection, playerEntity, packet);
		connection = NULL;	// We've moved responsibility for this over to the new PlayerConnection, NULL so we don't delete our reference to it here in our dtor
    }
    done = true;

}

void PendingConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects)
{
//    logger.info(getName() + " lost connection");
    done = true;
}

void PendingConnection::handleGetInfo(shared_ptr<GetInfoPacket> packet)
{
	//try {
	//String message = server->motd + "�" + server->players->getPlayerCount() + "�" + server->players->getMaxPlayers();
	//connection->send(new DisconnectPacket(message));
	connection->send(shared_ptr<DisconnectPacket>(new DisconnectPacket(DisconnectPacket::eDisconnect_ServerFull) ) );
	connection->sendAndQuit();
	server->connection->removeSpamProtection(connection->getSocket());
	done = true;
	//} catch (Exception e) {
	//	e.printStackTrace();
	//}
}

void PendingConnection::handleKeepAlive(shared_ptr<KeepAlivePacket> packet)
{
	// Ignore
}

void PendingConnection::onUnhandledPacket(shared_ptr<Packet> packet)
{
	disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PendingConnection::send(shared_ptr<Packet> packet)
{
	connection->send(packet);
}

wstring PendingConnection::getName()
{
	return L"Unimplemented";
//        if (name != null) return name + " [" + connection.getRemoteAddress().toString() + "]";
//        return connection.getRemoteAddress().toString();
}

bool PendingConnection::isServerPacketListener()
{
	return true;
}