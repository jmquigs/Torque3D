#include "app/pge.h"

#include "console/console.h"
#include "platform/platform.h"
#include "core/util/path.h"
#include "console/consoleTypes.h"
#include "console/engineAPI.h"

HINSTANCE gPGEDll = NULL;
PGETickFn gPGETick = NULL;
PGERender3DFn gPGERender3D = NULL;
PGEBeforeUnloadFn gPGEBeforeUnload = NULL;

void unloadPGE()
{
   if (!gPGEDll)
      return;

   Con::warnf("Unloading PGE");

   gPGEBeforeUnload();

   FreeLibrary(gPGEDll);
   gPGEDll = NULL;
   gPGETick = NULL;
   gPGERender3D = NULL;
   gPGEBeforeUnload = NULL;
}

void initPGE(bool reload)
{
   if (reload)
   {
      unloadPGE();
   }

   if (gPGEDll != NULL)
      return;

   static bool once = false;
   if (once && !reload)
      return;
   once = true;

   StringTableEntry wd = Platform::getCurrentDirectory();
   char fullpath[2048];

   // try a few paths
   const char* paths[] = {
      ".",
      "..\\..\\..\\..\\PGE",
   };
   // and a few variants of pge dll paths
   const char* pgeSubpaths[] = {
      ".\\",
      "target\\release\\",
      "target\\debug\\",
   };

   const char* pgeName = "pge.dll";

   S32 numpaths = sizeof(paths) / sizeof(paths[0]);
   S32 numsubpaths = sizeof(pgeSubpaths) / sizeof(pgeSubpaths[0]);

   bool found = false;
   char fullfilepath[2048];
   for (S32 p = 0; !found && p < numpaths; ++p)
   {
      for (S32 sp = 0; !found && sp < numsubpaths; ++sp)
      {
         dSprintf(fullpath, sizeof(fullpath), "%s\\%s\\%s", wd, paths[p], pgeSubpaths[sp]);
         dSprintf(fullfilepath, sizeof(fullfilepath), "%s\\%s", fullpath, pgeName);

         if (Platform::isFile(fullfilepath))
            found = true;
      }
   }
   if (!found)
   {
      Con::warnf("Failed to find PGE");
      return;
   }

   // To support hot reloads, before loading we copy it to a new name.  
   // We also need to move the pdb because VS Debugger might pin it.

   Con::warnf("Found pge: %s; copying to load path", fullfilepath);
   char pathcopy[2048];
   //strcpy_s(pathcopy, sizeof(pathcopy), fullpath);
   sprintf_s(pathcopy, "%s/PGE_load.dll", wd);
   //strcat_s(pathcopy, sizeof(pathcopy), "_load.dll");
   if (!dPathCopy(fullfilepath, pathcopy, false))
   {
      Con::errorf("failed to copy PGE dll to load dll path");
      return;
   }
   // move pdb
   char pdbfile[2048];
   sprintf_s(pdbfile, sizeof(pdbfile), "%s\\deps\\pge.pdb", fullpath);
   if (Platform::isFile(pdbfile))
   {
      char pdbdest[2048];
      sprintf_s(pdbdest, sizeof(pdbdest), "%s/PGE_load.pdb", wd);
      Con::warnf("Moving PGE PDB %s => %s", pdbfile, pdbdest);
      DeleteFileA(pdbdest);
      if (!MoveFileA(pdbfile, pdbdest))
      {
         Con::errorf("Failed to move PGE PDB file");
         return;
      }
      //if (!dPathCopy(pdbfile, pdbdest, false))
      //{
      //   Con::errorf("failed to copy PGE pdb file");
      //   return;
      //}
      //// and we have to delete the source 
      //Con::warnf("Copied PGE PDB file, removing source PDB to allow rebuild");
      //DeleteFileA(pdbfile);
   }

   Con::warnf("Loading PGE: %s", pathcopy);
   gPGEDll = LoadLibraryA(pathcopy);
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
   gPGEBeforeUnload = (PGEBeforeUnloadFn)GetProcAddress(gPGEDll, "before_unload");
   if (!gPGEBeforeUnload)
   {
      Con::errorf("Failed to load PGE before_unload function");
      return;
   }

   Con::warnf("PGE Initialized");
}

DefineConsoleFunction(reloadPGE, void, (), , "()")
{
   initPGE(true);
}