#pragma once

#include <windows.h>

extern "C" {
   typedef  void (*PGETickFn) ();
   typedef  void (*PGERender3DFn)(void*);
};

extern HINSTANCE gPGEDll;
extern PGETickFn gPGETick;
extern PGERender3DFn gPGERender3D;

void initPGE();