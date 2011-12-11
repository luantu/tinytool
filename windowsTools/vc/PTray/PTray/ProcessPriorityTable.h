#pragma once

#define INI_KEY				"Priority"
#define PRIORITY_UNCHANGE	9
#define PRIORITY_MIN		-2
#define PRIORITY_MAX		3

typedef int (*COMPFUNC)(void* v1, void* v2);

typedef struct _tagEntry {
	void* key;
	void* value;
	struct _tagEntry* parent;
	struct _tagEntry* left;
	struct _tagEntry* right;
} Entry;

class BiTreeTable
{
protected:
	Entry* root;
	unsigned int size;
	void** aTmp;
	COMPFUNC compFunc;
	void clear(Entry* p, BOOL bLeft);
	void prepareArray(Entry* p, unsigned int* pindex, BOOL bKey);
public:
	BiTreeTable(COMPFUNC);
	~BiTreeTable(void);
	void put(void* key, void* value);
	void* get(void* key);
	void* remove(void* key);
	void** getKeys();
	void** getValues();
	unsigned int getSize();
};

class ProcessPriorityTable
{
private:
	BiTreeTable* processTable;
public:
	BOOL bProcessing;
	ProcessPriorityTable(void);
	~ProcessPriorityTable(void);
	void loadFromIniFile(TCHAR* fileName);
	int getPriorityValue(TCHAR* baseName);
	unsigned int getProcessNum();
	void clear();
};
