#pragma once

#include "resource.h"

void EndInstance();
void MinimizeMemory(); 
BOOL SetTransValue(__in HWND hWnd, __in BYTE transValue); 
void SetTopMost(__in HWND hWnd); 
void inline CrossMove(__in int vKey, __inout int* dx, __inout int* dy); 
