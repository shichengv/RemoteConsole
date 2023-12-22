#pragma once
#include "framework.h"
#include "Console.h"

#define LISTENQ			5

void DebugSocketLog(const TCHAR* LogStr);
void UpdateServerLogFile(HANDLE hFile, const TCHAR szMsg[], SYSTEMTIME* st);
BOOL InitSocket();
void CleanSocket();
SOCKET OpenListenSocket(PCSTR Port);
