#include "app/pge.h"

#include "console/console.h"

HINSTANCE gPGEDll = NULL;
PGETickFn gPGETick = NULL;
PGERender3DFn gPGERender3D = NULL;

void initPGE()
{
   if (gPGEDll != NULL)
      return;

   static bool once = false;
   if (once)
      return;
   once = true;

   gPGEDll = LoadLibraryA("C:\\Dev\\PGE\\target\\debug\\pge.dll");
   if (!gPGEDll)
   {
      Con::errorf("Failed to load PGE lib");
      return;
   }

   gPGETick = (PGETickFn)GetProcAddress(gPGEDll, "tick");
   if (!gPGETick)
   {
      Con::errorf("Failed to load PGE tick function");
      return;
   }
   gPGERender3D = (PGERender3DFn)GetProcAddress(gPGEDll, "render_3d");
   if (!gPGERender3D)
   {
      Con::errorf("Failed to load PGE render_3d function");
      return;
   }

   Con::warnf("PGE Initialized");
}