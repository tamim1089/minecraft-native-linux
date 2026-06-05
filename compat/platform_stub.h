// NOT already defined by Minecraft.World/x64headers/extraX64.h (which defines the bulk of the
// XCONTENT/XMARKETPLACE/XUSER surface itself). Keep this minimal to avoid redefinition clashes.
#pragma once
#ifndef PORT_LINUX_STUB_H
#define PORT_LINUX_STUB_H

#include <cstdint>

typedef unsigned long long UID;

typedef struct { unsigned char ab[8]; }  XNKID;
typedef struct { unsigned char ab[16]; } XNKEY;
typedef struct {
    unsigned char  ina[4];
    unsigned char  inaOnline[4];
    unsigned short wPortOnline;
    unsigned char  abEnet[6];
    unsigned char  abOnline[20];
} XNADDR;

// Headless build never reports presence; provide them as benign 0 so the app/network/server
#ifndef CONTEXT_GAME_STATE
#define CONTEXT_GAME_STATE                       0
#define CONTEXT_GAME_STATE_BLANK                 0
#define CONTEXT_GAME_STATE_ANVIL                 0
#define CONTEXT_GAME_STATE_BREWING               0
#define CONTEXT_GAME_STATE_CRAFTING              0
#define CONTEXT_GAME_STATE_ENCHANTING            0
#define CONTEXT_GAME_STATE_FORGING               0
#define CONTEXT_GAME_STATE_TRADING               0
#define CONTEXT_PRESENCE_IDLE                    0
#define CONTEXT_PRESENCE_MENUS                   0
#define CONTEXT_PRESENCE_MULTIPLAYER             0
#define CONTEXT_PRESENCE_MULTIPLAYER_1P          0
#define CONTEXT_PRESENCE_MULTIPLAYER_1POFFLINE   0
#define CONTEXT_PRESENCE_MULTIPLAYEROFFLINE      0
#endif

#endif // PORT_LINUX_STUB_H
