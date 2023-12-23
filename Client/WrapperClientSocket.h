#pragma once

#include "framework.h"
#include "Client.h"

void UpdateClientLogFile(const TCHAR szMsg[]);
void DebugSocketLog(const TCHAR* LogStr);
BOOL InitSocket();
void CleanSocket();
SOCKET OpenClientSocket(PCSTR HostName, PCSTR Port);
