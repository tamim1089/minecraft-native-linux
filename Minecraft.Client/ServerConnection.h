#pragma once
class PendingConnection;
class PlayerConnection;
class MinecraftServer;
class Socket;
class ServerSettingsChangedPacket;

using namespace std;

class ServerConnection
{
//    public static Logger logger = Logger.getLogger("Minecraft");

private:
//	ServerSocket serverSocket;
//    private Thread listenThread;
public:
	volatile bool running;
private:
	int connectionCounter;
private:
	CRITICAL_SECTION pending_cs;	// 
	vector< shared_ptr<PendingConnection> > pending;
    vector< shared_ptr<PlayerConnection> > players;

	// When the server requests a texture, it should add it to here while we are waiting for it
	vector<wstring> m_pendingTextureRequests;
public:
	MinecraftServer *server;

public:
	ServerConnection(MinecraftServer *server); // removed params InetAddress address, int port);
	~ServerConnection();
	void NewIncomingSocket(Socket *socket);	// added

	void removeSpamProtection(Socket *socket) { }// Not implemented as not required
    void addPlayerConnection(shared_ptr<PlayerConnection> uc);
private:
	void handleConnection(shared_ptr<PendingConnection> uc);
public:
	void stop();
    void tick();

	bool addPendingTextureRequest(const wstring &textureName);
	void handleTextureReceived(const wstring &textureName);
	void handleTextureAndGeometryReceived(const wstring &textureName);
	void handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet);
};
