#pragma once
#include "framework.h"
#include "WrapperServerSocket.h"
#include "Console.h"

#define DEFAULT_PORT				"17777"

#define HOSTNAMELEN					(NI_MAXHOST + 2 + NI_MAXSERV)
#define ALIASLEN					64

#define HASHCOUNT				31


class Client {

public:
	SOCKET socket;
	int key;
	TCHAR Hostname[HOSTNAMELEN];
	TCHAR Aliasname[ALIASLEN];
	Client* self;

	void Initilize(SOCKET socket, int key, const TCHAR Hostname[], const TCHAR Aliasname[], Client * self) {
		this->socket = socket;
		this->key = key;
		_tcsncpy_s(this->Hostname, Hostname, HOSTNAMELEN);
		_tcsncpy_s(this->Aliasname, Aliasname, HOSTNAMELEN);
		this->self = self;
	}

	void Destroy() {

		int iResult = shutdown(socket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			DebugSocketLog(TEXT("Client::Destroy()->shutdown()"));
			closesocket(socket);
		}

		delete this->self;
	}
	
};

typedef Client* Item;

typedef struct hash_node {

	Item item;
	int key;
	struct hash_node* prev;
	struct hash_node* next;

}HashNode, * pHashNode;

class HashTable {

public:
	pHashNode Table[HASHCOUNT];

	HashTable() { ZeroMemory(this->Table, HASHCOUNT * sizeof(pHashNode)); }

	UINT32 Hash(int key) {
		return key & HASHCOUNT;
	}

	void Insert(int key, Item item) {
		int index = Hash(key);
		pHashNode tlinkedlist;

		/* 如果下一个节点为空，就插入到该节点的后面 */
		for (tlinkedlist = this->Table[index]; tlinkedlist; tlinkedlist = tlinkedlist->next);

		tlinkedlist = new HashNode;
		tlinkedlist->prev = this->Table[index];
		tlinkedlist->next = NULL;
		tlinkedlist->key = key;
		tlinkedlist->item = item;
	}

	BOOL Delete(int key) {
		int index = Hash(key);
		pHashNode tlinkedlist;
		for (tlinkedlist = this->Table[index]; tlinkedlist->key != key && tlinkedlist; tlinkedlist = tlinkedlist->next);

		if (!tlinkedlist)
			return FALSE;

		if (!tlinkedlist->next) {
			tlinkedlist->prev->next = NULL;
			delete tlinkedlist;
			return TRUE;
		}

		tlinkedlist->prev->next = tlinkedlist->next;
		tlinkedlist->next->prev = tlinkedlist->prev;

		tlinkedlist->item->Destroy();
		delete tlinkedlist;
		return TRUE;
	}

	Item GetItem(int key) {

		int index = Hash(key);
		pHashNode tlinkedlist;

		for (tlinkedlist = this->Table[index]; tlinkedlist->key != key && tlinkedlist; tlinkedlist = tlinkedlist->next);

		if (!tlinkedlist || tlinkedlist->key != key)
			return (Item)0;
		else
			return tlinkedlist->item;

	}

	~HashTable() {
		for (int i = 0; i < HASHCOUNT; i++)
		{
			for (pHashNode phNode = this->Table[i]; phNode; phNode->next) {
				pHashNode phtNode = phNode;
				phtNode->item->Destroy();
				delete phtNode;

			}

		}
	}

};

