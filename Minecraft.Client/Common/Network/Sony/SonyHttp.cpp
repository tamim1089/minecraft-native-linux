#include "stdafx.h"
#include "SonyHttp.h"


#ifdef _LINUX
#include "Linux/Network/SonyHttp_Linux.h"
SonyHttp_Linux g_SonyHttp;

#elif defined _LINUX
#include "Linux/Network/SonyHttp_Orbis.h"
SonyHttp_Orbis g_SonyHttp;

#elif defined _LINUX
#include "Linux/Network/SonyHttp_Vita.h"
SonyHttp_Vita g_SonyHttp;

#endif



bool SonyHttp::init()
{
	return g_SonyHttp.init();
}

void SonyHttp::shutdown()
{
	g_SonyHttp.shutdown();
}

bool SonyHttp::getDataFromURL(const char* szURL, void** ppOutData, int* pDataSize)
{
	return g_SonyHttp.getDataFromURL(szURL, ppOutData, pDataSize);
}
