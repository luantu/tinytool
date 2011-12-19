#include "stdafx.h"
#include "ProcessPriorityTable.h"

static int fnameComp(void* s1, void* s2);
BiTreeTable::BiTreeTable(COMPFUNC func)
{
	this->root = NULL;
	this->size = 0;
	this->compFunc = func;
}

BiTreeTable::~BiTreeTable()
{
	this->clear(this->root, TRUE);
}

void BiTreeTable::clear(Entry *p, BOOL bLeft)
{
	if (p->left) {
		clear(p->left, TRUE);
	} else if (p->right) {
		clear(p->right, FALSE);
	} else {
		if (p->parent) {
			if (bLeft) {
				p->parent->left = NULL;
			} else {
				p->parent->right = NULL;
			}
		}
		free(p);
		p = NULL;
	}
	this->size = 0;
	this->root = NULL;
}

void BiTreeTable::put(void* key, void* value)
{
	Entry* p = this->root;
	Entry* pp = NULL;
	bool bLeft = TRUE;
	while (p != NULL) {
		pp = p;
		char comp = this->compFunc(p->key, key);
		if (comp < 0) {
			p = p->left;
			bLeft = TRUE;
		} else if (comp > 0) {
			p = p->right;
			bLeft = FALSE;
		} else {
			p->value = value;
			return;
		}
	}
	p = (Entry*)calloc(1, sizeof(Entry));
	p->key = key;
	p->value = value;
	p->parent = pp;
	if (pp) {
		if (bLeft) {
			pp->left = p;
		} else {
			pp->right = p;
		}
	} else {
		this->root = p;
	}
	this->size++;
}

void* BiTreeTable::get(void* key)
{
	Entry* p = this->root;
	while (p != NULL) {
		char comp = this->compFunc(p->key, key);
		if (comp < 0) {
			p = p->left;
		} else if (comp > 0) {
			p = p->right;
		} else {
			return p->value;
		}
	}
	return NULL;
}

void* BiTreeTable::remove(void* key)
{
	Entry* p = this->root;
	bool bLeft = TRUE;
	void* value = NULL;
	while (p != NULL) {
		char comp = this->compFunc(p->key, key);
		if (comp < 0) {
			p = p->left;
			bLeft = TRUE;
		} else if (comp > 0) {
			p = p->right;
			bLeft = FALSE;
		} else {
			value = p->value;
			if (bLeft) {
				p->parent->left = NULL;
			} else {
				p->parent->right = NULL;
			}
			this->size--;
			free(p);
			p = NULL;
			break;
		}
	}
	return value;
}

unsigned int BiTreeTable::getSize()
{
	return this->size;
}

void BiTreeTable::prepareArray(Entry* p, unsigned int* pindex, BOOL bKey)
{
	if (p->left) {
		prepareArray(p->left, pindex, bKey);
	}
	this->aTmp[*pindex] = bKey ? p->key : p->value;
	(*pindex)++; 
	if (p->right) {
		prepareArray(p->right, pindex, bKey);
	}
}

void** BiTreeTable::getKeys()
{
	this->aTmp = (void**)calloc(this->size, sizeof(void*));
	unsigned int i = 0;
	this->prepareArray(this->root, &i, TRUE);
	void** ret = this->aTmp;
	this->aTmp = NULL;
	return ret;
}

void** BiTreeTable::getValues()
{
	this->aTmp = (void**)calloc(this->size, sizeof(void*));
	unsigned int i = 0;
	this->prepareArray(this->root, &i, FALSE);
	void** ret = this->aTmp;
	this->aTmp = NULL;
	return ret;
}

int fnameComp(void* s1, void* s2)
{
	return lstrcmpi((TCHAR*)s1, (TCHAR*)s2);
}

ProcessPriorityTable::ProcessPriorityTable(void)
{
	this->bProcessing = FALSE;
	this->bInterrupted = FALSE;
	this->processTable = NULL;
}

ProcessPriorityTable::~ProcessPriorityTable(void)
{
	this->clear();
}

void ProcessPriorityTable::loadFromIniFile(TCHAR *fileName)
{
	// get file size.
	OFSTRUCT of = {0};
	of.cBytes = sizeof(OFSTRUCT);
	HANDLE hFile = ::CreateFile(
		fileName, 
		GENERIC_READ, 
		FILE_SHARE_READ, 
		NULL, 
		OPEN_EXISTING, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return;
	}

	DWORD dwSize = ::GetFileSize(hFile, NULL);
	if (dwSize == INVALID_FILE_SIZE) {
		return;
	}

	::CloseHandle(hFile);
	hFile = NULL;

	// create a buffer which size is the file size to ensure enough size.
	TCHAR* buffer = NULL;
	buffer = new TCHAR[dwSize];
	::GetPrivateProfileSectionNames(buffer, dwSize, fileName);

	for (TCHAR* sname = buffer; *sname != '\0'; ) {
		int len = lstrlen(sname); 
		TCHAR* key = (TCHAR*) calloc(len + 1, sizeof(TCHAR));
		lstrcpyn(key, sname, (int)(len + 1));
		int* pvalue = (int*)calloc(1, sizeof(int));
		*pvalue = ::GetPrivateProfileInt(sname, _T(INI_KEY), PRIORITY_UNCHANGE, fileName); 
		if (*pvalue != PRIORITY_UNCHANGE) {
			if (*pvalue > PRIORITY_MAX) {
				*pvalue = PRIORITY_MAX;
			}
			if (*pvalue < PRIORITY_MIN) {
				*pvalue = PRIORITY_MIN;
			}
			if (!this->processTable) {
				this->processTable = new BiTreeTable(fnameComp);
			}
			this->processTable->put(key, pvalue);
		}
		sname += len + 1;
	}

	if (buffer) {
		delete[] buffer;
		buffer = NULL;
	}
}

int ProcessPriorityTable::getPriorityValue(TCHAR* baseName)
{
	if (!this->processTable) {
		return PRIORITY_UNCHANGE;
	}
	int* pvalue = NULL;
	pvalue = (int*)this->processTable->get(baseName);
	if (!pvalue) {
		return PRIORITY_UNCHANGE;
	}

	return *pvalue;
}

unsigned int ProcessPriorityTable::getProcessNum()
{
	return this->processTable ? this->processTable->getSize() : 0;
}

void ProcessPriorityTable::clear()
{
	if (this->processTable) {
		void** keys = this->processTable->getKeys();
		void** values = this->processTable->getValues();
		unsigned int n = this->processTable->getSize();
		for (unsigned int i = 0; i < n; i++) {
			free(keys[i]);
			free(values[i]);
		}
		free(keys);
		keys = NULL;
		free(values);
		values = NULL;
		delete this->processTable;
		this->processTable = NULL;
	}
}