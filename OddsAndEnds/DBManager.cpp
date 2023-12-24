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
#include <vector>
#include <memory>
#include <map>
#include <list>
#include <string>

#include <sqlite3.h>

#include "DBManager.h"
#include "GetAndSet.h"

#include "Forwards/Engine/SpreadSheet.h"

class DBManagerImpl
 {
public:
   DBManagerImpl() { }
   ~DBManagerImpl()
    {
      for (auto db : dbs)
       {
         sqlite3_close(db);
       }
    }

   std::list<sqlite3*> dbs;
   std::map<std::string, std::unique_ptr<Forwards::Engine::SpreadSheetHolder> > spreadSheets;
   std::map<std::string, std::unique_ptr<WidthGetterSetter> > getterSetters;

   std::string sheetName;
   Forwards::Engine::SpreadSheetHolder* workingSpreadSheet;
   WidthGetterSetter* workingGS;
 };


DBManager::DBManager()
 {
   impl = std::make_unique<DBManagerImpl>();
 }

DBManager::~DBManager()
 {
 }

std::string DBManager::getSheetName()
 {
   return impl->sheetName;
 }

bool DBManager::isSheetEdible()
 {
   return true == impl->sheetName.empty();
 }

bool DBManager::changeTo(const std::string& sheetName)
 {
   std::map<std::string, std::unique_ptr<Forwards::Engine::SpreadSheetHolder> >::iterator iter = impl->spreadSheets.find(sheetName);
   if (impl->spreadSheets.end() != iter)
    {
      impl->sheetName = sheetName;
      impl->workingSpreadSheet = iter->second.get();
      impl->workingGS = impl->getterSetters[sheetName].get();
      return true;
    }
   return false;
 }

Forwards::Engine::SpreadSheetHolder* DBManager::getWorkingSpreadSheet()
 {
   return impl->workingSpreadSheet;
 }

WidthGetterSetter* DBManager::getWorkingWidthGS()
 {
   return impl->workingGS;
 }

Forwards::Engine::SpreadSheetHolder* DBManager::getSpreadSheet(const std::string& sheetName)
 {
   std::map<std::string, std::unique_ptr<Forwards::Engine::SpreadSheetHolder> >::iterator iter = impl->spreadSheets.find(sheetName);
   if (impl->spreadSheets.end() != iter)
    {
      return iter->second.get();
    }
   return nullptr;
 }

void DBManager::attachDB (void* handel)
 {
   impl->dbs.push_back(reinterpret_cast<sqlite3*>(handel));
 }

void DBManager::attach(const std::string& sheetName, std::unique_ptr<Forwards::Engine::SpreadSheetHolder>&& ss, std::unique_ptr<WidthGetterSetter>&& gs)
 {
   impl->spreadSheets[sheetName] = std::move(ss);
   impl->getterSetters[sheetName] = std::move(gs);

   if (true == sheetName.empty())
    {
      impl->workingSpreadSheet = impl->spreadSheets[sheetName].get();
      impl->workingGS = impl->getterSetters[sheetName].get();
    }
 }
