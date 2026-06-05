// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
#pragma once

#ifdef _LINUX
#else
#define AUTO_VAR(_var, _val) auto _var = _val
#endif

#if ( defined _LINUX || defined _LINUX  || defined _LINUX )
typedef unsigned __int64 __uint64;
#endif

#ifdef _LINUX
#ifndef _LINUX
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
// TODO: reference additional headers your program requires here
#include <d3d11.h>
#else
// (compat/msvc_compat.h + compat/win_types.h). See PORT_LOG.md Phase 0.
#include <malloc.h>
#endif // _LINUX
#endif

#ifdef _LINUX
#include <xdk.h>
#include <wrl.h>
#include <d3d11_x.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <pix.h>
#include "../Minecraft.Client/Linux/DurangoExtras/DurangoStubs.h"
#endif

#if (defined _LINUX || defined _LINUX )
// C RunTime Header Files
#include <stdlib.h>
#endif

#ifdef _LINUX
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <fios2.h>
#include <message_dialog.h>
#endif

#ifdef _LINUX
#include <xtl.h>
#include <xuiapp.h>
#include <xact3.h>
typedef XINVITE_INFO INVITE_INFO;
typedef UID PlayerUID;
typedef XNKID SessionID;
typedef UID GameSessionUID;
#endif

#ifdef _LINUX
#include <cell/l10n.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_sysparam.h>


#include "Ps3Types.h"
#include "Ps3Stubs.h"
#include "PS3Maths.h"

#elif defined _LINUX
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "LinuxTypes.h"
#include "LinuxStubs.h"
#include "LinuxMaths.h"
#elif defined _LINUX
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <touch.h>
#include "LinuxTypes.h"
#include "LinuxStubs.h"
#include "LinuxMaths.h"
#else
#include <unordered_map>
#include <unordered_set>
#ifndef _LINUX
#include <sal.h>
#endif
#include <vector>
#endif //_LINUX

#include <memory>

#include <list>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <math.h>
#include <limits>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

#include <assert.h>
#endif

#ifndef _LINUX
#include "extraX64.h"
#else
#include "../Minecraft.Client/Linux/Network/extra.h"
#endif

#include "Definitions.h"
#include "Class.h"
#include "Exceptions.h"
#include "Mth.h"
#include "StringHelpers.h"
#include "ArrayWithLength.h"
#include "Random.h"
#include "TilePos.h"
#include "ChunkPos.h"
#include "compression.h"
#include "PerformanceTimer.h"


#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif


void MemSect(int sect);

#ifdef _LINUX
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_xtms.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Input.h"
#elif defined (_LINUX)
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Input.h"
#elif defined _LINUX
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Input.h"
#elif defined _LINUX
#include "../Minecraft.Client/Linux64/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux64/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux64/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux64/4JLibs/inc/4J_Input.h"
#elif defined _LINUX
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Input.h"
#else
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Profile.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Render.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Storage.h"
#include "../Minecraft.Client/Linux/4JLibs/inc/4J_Input.h"
#endif

#include "../Minecraft.Client/Common/Network/GameNetworkManager.h"

// #ifdef _LINUX
#include "../Minecraft.Client/Common/UI/UIEnums.h"
#include "../Minecraft.Client/Common/App_Defines.h"
#include "../Minecraft.Client/Common/App_enums.h"
#include "../Minecraft.Client/Common/Tutorial/TutorialEnum.h"
#include "../Minecraft.Client/Common/App_structs.h"
//#endif

#ifdef _LINUX
#include "../Minecraft.Client/Common/XUI/XUI_Helper.h"
#include "../Minecraft.Client/Common/XUI/XUI_Scene_Base.h"
#endif
#include "../Minecraft.Client/Common/Consoles_App.h"
#include "../Minecraft.Client/Common/Minecraft_Macros.h"
#include "../Minecraft.Client/Common/Colours/ColourTable.h"

#include "../Minecraft.Client/Common/BuildVer.h"

#ifdef _LINUX
#include "../Minecraft.Client/Linux/Xbox_App.h"
#include "../Minecraft.Client/GameMedia/strings.h"
#include "../Minecraft.Client/Linux/Sentient/SentientTelemetryCommon.h"
#include "../Minecraft.Client/Linux/Sentient/MinecraftTelemetry.h"

#elif defined (_LINUX)
#include "../Minecraft.Client/Linux/PS3_App.h"
#include "../Minecraft.Client/PS3Media/strings.h"
#include "../Minecraft.Client/Linux/Sentient/SentientTelemetryCommon.h"
#include "../Minecraft.Client/Linux/Sentient/MinecraftTelemetry.h"

#elif defined _LINUX
#include "../Minecraft.Client/Linux/Durango_App.h"
#include "../Minecraft.Client/GameMedia/strings.h"
#include "../Minecraft.Client/Linux/Sentient/SentientTelemetryCommon.h"
#include "../Minecraft.Client/Linux/Sentient/MinecraftTelemetry.h"
#include "../Minecraft.Client/Linux/Sentient/TelemetryEnum.h"

#elif defined _LINUX
#include "../Minecraft.Client/Linux64/Windows64_App.h"
#include "../Minecraft.Client/Linux64/Sentient/SentientTelemetryCommon.h"
#include "../Minecraft.Client/Linux64/Sentient/MinecraftTelemetry.h"

#elif defined _LINUX
#include "../Minecraft.Client/Linux/Linux_App.h"
#include "../Minecraft.Client/Linux/Sentient/SentientManager.h"
#include "../Minecraft.Client/Linux/Sentient/MinecraftTelemetry.h"
#else
#include "../Minecraft.Client/Linux/Orbis_App.h"
#include "../Minecraft.Client/GameMedia/strings.h"
#include "../Minecraft.Client/Linux/Sentient/SentientTelemetryCommon.h"
#include "../Minecraft.Client/Linux/Sentient/MinecraftTelemetry.h"
#endif

#include "../Minecraft.Client/Common/DLC/DLCSkinFile.h"
#include "../Minecraft.Client/Common/Console_Awards_enum.h"
#include "../Minecraft.Client/Common/Potion_Macros.h"
#include "../Minecraft.Client/Common/Console_Debug_enum.h"
#include "../Minecraft.Client/Common/GameRules/ConsoleGameRulesConstants.h"
#include "../Minecraft.Client/Common/GameRules/ConsoleGameRules.h"
#include "../Minecraft.Client/Common/Telemetry/TelemetryManager.h"
