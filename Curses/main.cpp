/*
BSD 3-Clause License

Copyright (c) 2023, Thomas DiModica
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <iostream>
#include <filesystem>

#include "Backwards/Engine/Logger.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "Forwards/Types/ValueType.h"

#include "NumberSystem.h"

#include "DBManager.h"
#include "BatchMode.h"
#include "GetAndSet.h"
#include "LibraryLoader.h"
#include "SaveFile.h"

#include "Screen.h"

int main (int argc, char ** argv)
 {
   Forwards::Engine::CallingContext context;
   Backwards::Engine::Scope global;
   context.globalScope = &global;
   Forwards::Parser::StringLogger logger;
   context.logger = &logger;
   Forwards::Engine::SpreadSheet sheet;
   context.theSheet = &sheet;
   Forwards::Engine::GetterMap map;
   context.map = &map;
   Forwards::Engine::NameMap names;
   context.names = &names;

   std::list<std::string> batches;
   std::vector<std::pair<std::string, std::string> > argLibs;

   int file = 1;

   if (file < argc)
    {
      if (std::string("-0") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(BCNUM_NUMBER_SYSTEM);
         ++file;
       }
      else if (std::string("-1") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(LIBDECMATH_NUMBER_SYSTEM);
         ++file;
       }
      else if (std::string("-2") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(SLOWFLOAT_NUMBER_SYSTEM);
         ++file;
       }
      else if (std::string("-3") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(DOUBLE_NUMBER_SYSTEM);
         ++file;
       }
      else if (std::string("-4") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(LIBMPDEC_NUMBER_SYSTEM);
         ++file;
       }
      else if (std::string("-5") == argv[file])
       {
         NumberSystem::setCurrentNumberSystem(MPFR_NUMBER_SYSTEM);
         ++file;
       }
    }
   if (nullptr == &NumberSystem::getCurrentNumberSystem())
    {
      NumberSystem::setCurrentNumberSystem(BCNUM_NUMBER_SYSTEM);
    }

   file = PreLoadLibraries(argc, argv, file, argLibs);
   file = ReadBatches(argc, argv, file, batches);


   SharedData state;

   state.c_row = 0U;
   state.c_col = 0U;
   state.tr_row = 0U;
   state.tr_col = 0U;

   state.inputMode = false;
   state.insertMode = true;
   state.useComma = false;

   std::string fileName = "untitled.wts"; // Untitled Oot Sheet file.
    {
      DBManager manager;
      state.manager = &manager;
      manager.context = &context;

      state.yankedType = Forwards::Engine::ERROR;

      state.context = &context;

      if (file < argc)
       {
         fileName = argv[file];
         ++file;
       }
       {
         std::vector<std::pair<std::string, std::string> > fileLibs;
         LoadFile(fileName, manager, fileLibs, argLibs);
         LoadLibraries(fileLibs, context);
       }

      if (file < argc)
       {
         AttachDB(argv[file], manager);
       }

      sheet.currentSheet = manager.getWorkingSpreadSheet();
      if (nullptr == sheet.currentSheet)
       {
         std::cerr << "Program cannot start as something very bad happened." << std::endl;
         return 1;
       }


      if (0U != sheet.getMaxRow()) // We loaded saved data, so recalculate the sheet.
       {
         sheet.recalc(context);
       }


      if (false == batches.empty())
       {
         RunBatches(batches, context);
         return 0;
       }


      InitScreen();
      UpdateScreen(state);
      while (ProcessInput(state))
       {
         if (sheet.currentSheet != manager.getWorkingSpreadSheet())
          { // Delayed so that cells can be returned.
            sheet.currentSheet = manager.getWorkingSpreadSheet();
          }

         UpdateScreen(state);
       }
      DestroyScreen();

      if (false == logger.logs.empty())
       {
         std::cerr << "These messages were logged:" << std::endl;
         for (const auto& bob : logger.logs)
          {
            std::cerr << bob << std::endl;
          }
         logger.logs.clear();
       }
    }

   std::filesystem::remove(fileName + ".tmp");

   return 0;
 }
