#pragma once

#include <windows.h>

extern "C" {
   typedef  void (*PGETickFn) ();
   typedef  void (*PGEBeforeUnloadFn) ();
   typedef  void (*PGERender3DFn)(void*);
};

extern HINSTANCE gPGEDll;
extern PGETickFn gPGETick;
extern PGERender3DFn gPGERender3D;
extern PGEBeforeUnloadFn gPGEBeforeUnload;

void initPGE(bool reload = false);