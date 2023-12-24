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
#include <filesystem>
#include <sqlite3.h>
#include <cstring>
#include <iostream>

#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Expression.h"
#include "Forwards/Engine/MemorySpreadSheet.h"

#include "Forwards/Parser/Parser.h"

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/StringInput.h"

#include "DBManager.h"
#include "GetAndSet.h"
#include "DBSpreadSheet.h"
#include "TableView.h"

void CreateErrorDatabase(DBManager& manager)
 {
   std::unique_ptr<Forwards::Engine::MemorySpreadSheet> sheet = std::make_unique<Forwards::Engine::MemorySpreadSheet>();
   std::unique_ptr<MemoryWidthGS> getterSetter = std::make_unique<MemoryWidthGS>(DEF_COLUMN_WIDTH);

   manager.attach("", std::move(sheet), std::move(getterSetter));
 }

bool CreateNewTaterBase(const std::string& fileName, DBManager& manager)
 {
   sqlite3 *handel;
   int errorCode;

   errorCode = sqlite3_open(fileName.c_str(), &handel);
   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to create file " + fileName;
      return false;
    }

   errorCode = sqlite3_exec(handel, "CREATE TABLE sheet (col INTEGER, row INTEGER, type INTEGER, content TEXT, PRIMARY KEY (col, row)) WITHOUT ROWID;", nullptr, nullptr, nullptr);
   errorCode |= sqlite3_exec(handel, "CREATE TABLE libs (num INTEGER PRIMARY KEY, name TEXT, lib TEXT);", nullptr, nullptr, nullptr);
   errorCode |= sqlite3_exec(handel, "CREATE TABLE widths (col INTEGER PRIMARY KEY, width INTEGER);", nullptr, nullptr, nullptr);

   sqlite3_close(handel);

   if (SQLITE_OK != errorCode)
    {
      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to create file " + fileName;
      return false;
    }

   return true;
 }

bool IsSchemaValid(sqlite3 *handel)
 {
    /* only validate column names and order: SQLite is wishy-washy with types anyway */
   bool good = true;
   int errorCode;
   sqlite3_stmt *messi;
   errorCode = sqlite3_prepare_v2(handel, "SELECT * FROM sheet LIMIT 0;", 29U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_step(messi);

      good &= 0 == std::strcmp(sqlite3_column_name(messi, 0), "col");
      good &= 0 == std::strcmp(sqlite3_column_name(messi, 1), "row");
      good &= 0 == std::strcmp(sqlite3_column_name(messi, 2), "type");
      good &= 0 == std::strcmp(sqlite3_column_name(messi, 3), "content");

      sqlite3_finalize(messi);
    }
   else
    {
      good = false;
    }

   errorCode = sqlite3_prepare_v2(handel, "SELECT * FROM libs LIMIT 0;", 28U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_step(messi);

      good &= 0 == std::strcmp(sqlite3_column_name(messi, 1), "name");
      good &= 0 == std::strcmp(sqlite3_column_name(messi, 2), "lib");

      sqlite3_finalize(messi);
    }
   else
    {
      good = false;
    }

   errorCode = sqlite3_prepare_v2(handel, "SELECT * FROM widths LIMIT 0;", 30U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_step(messi);

      good &= 0 == std::strcmp(sqlite3_column_name(messi, 0), "col");
      good &= 0 == std::strcmp(sqlite3_column_name(messi, 1), "width");

      sqlite3_finalize(messi);
    }
   else
    {
      good = false;
    }

   return good;
 }

void LoadFile(const std::string& fileName, DBManager& manager, std::vector<std::pair<std::string, std::string> >& allLibs,
   const std::vector<std::pair<std::string, std::string> >& addLibs)
 {
   if (true == std::filesystem::exists(fileName + ".tmp"))
    {
      std::cerr << "Cowardly failing instead of overwriting the file " << fileName << ".tmp" << std::endl;
      return;
    }

   if (false == std::filesystem::exists(fileName))
    {
      if (false == CreateNewTaterBase(fileName, manager))
       {
         return;
       }
    }

   sqlite3 *handel, *ohandel;
   int errorCode;

   errorCode = sqlite3_open(fileName.c_str(), &handel);
   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName;
      return;
    }

   if (false == IsSchemaValid(handel))
    {
      sqlite3_close(handel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "File " + fileName + " is not a valid WTFITS sheet (wts) file.";
      return;
    }

   errorCode = sqlite3_open((fileName + ".tmp").c_str(), &ohandel);
   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);
      sqlite3_close(ohandel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName + ".tmp";
      return;
    }

   errorCode = sqlite3_exec(ohandel, "CREATE TABLE sheet (col INTEGER, row INTEGER, generation INTEGER, content TEXT, PRIMARY KEY (col, row)) WITHOUT ROWID;", nullptr, nullptr, nullptr);
   errorCode |= sqlite3_exec(ohandel, "PRAGMA synchronous = OFF", nullptr, nullptr, nullptr);
   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);
      sqlite3_close(ohandel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName + ".tmp";
      return;
    }

   sqlite3_stmt *messi;
   errorCode = sqlite3_prepare_v2(handel, "SELECT * FROM libs;", 20U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);
      sqlite3_close(ohandel);

      CreateErrorDatabase(manager);
      manager.getWorkingSpreadSheet()->initCellAt(0U, 0U);
      Forwards::Engine::Cell* cell = manager.getWorkingSpreadSheet()->getCellAt(0U, 0U, "");
      cell->type = Forwards::Engine::LABEL;
      cell->currentInput = "Failed to open file " + fileName;
      return;
    }

   while (SQLITE_ROW == sqlite3_step(messi))
    {
      std::string name;
      std::string text;

      name = reinterpret_cast<const char*>(sqlite3_column_text(messi, 1));
      text = reinterpret_cast<const char*>(sqlite3_column_text(messi, 2));

      allLibs.push_back(std::make_pair(name, text));
    }

   sqlite3_finalize(messi);

   if (false == addLibs.empty())
    {
      for (const std::pair<std::string, std::string>& lib : addLibs)
       {
         std::string stripped;
         Backwards::Input::StringInput libText (lib.second);
         Backwards::Input::Lexer lexer (libText, lib.first);
         while (lexer.peekNextToken().lexeme != Backwards::Input::END_OF_FILE)
          {
            if (lexer.peekNextToken().lexeme != Backwards::Input::STRING)
             {
               stripped += lexer.getNextToken().text + " ";
             }
            else
             {
               stripped += "\"" + lexer.getNextToken().text + "\" ";
             }
          }

         errorCode = sqlite3_prepare_v2(handel, "INSERT INTO libs (name, lib) VALUES (:name, :text);", 52U, &messi, nullptr);

         if (SQLITE_OK == errorCode)
          {
            sqlite3_bind_text(messi, 1, lib.first.c_str(), -1, nullptr);
            sqlite3_bind_text(messi, 2, stripped.c_str(), -1, nullptr);

            sqlite3_step(messi);

            sqlite3_finalize(messi);
          }
       }
      allLibs.insert(allLibs.end(), addLibs.begin(), addLibs.end());
    }

   std::unique_ptr<DBSpreadSheet> sheet = std::make_unique<DBSpreadSheet>(reinterpret_cast<void*>(handel), reinterpret_cast<void*>(ohandel), &manager);
   std::unique_ptr<DBWidthGS> getterSetter = std::make_unique<DBWidthGS>(DEF_COLUMN_WIDTH, reinterpret_cast<void*>(handel));
   manager.attachDB(reinterpret_cast<void*>(handel));
   manager.attachDB(reinterpret_cast<void*>(ohandel));
   manager.attach("", std::move(sheet), std::move(getterSetter));
 }

void AttachDB(const std::string& fileName, DBManager& manager)
 {
   sqlite3 *handel;
   int errorCode;

   errorCode = sqlite3_open(fileName.c_str(), &handel);
   if (SQLITE_OK != errorCode)
    {
      sqlite3_close(handel);
      return;
    }

   manager.attachDB(reinterpret_cast<void*>(handel));
    {
      std::unique_ptr<TableView> sheet = std::make_unique<TableView>("sqlite_schema", reinterpret_cast<void*>(handel));
      std::unique_ptr<MemoryWidthGS> getterSetter = std::make_unique<MemoryWidthGS>(DEF_COLUMN_WIDTH);
      manager.attach("sqlite_schema", std::move(sheet), std::move(getterSetter));
    }

   sqlite3_stmt *messi;
   static const std::string query = "SELECT name FROM sqlite_schema WHERE type = 'table';";
   errorCode = sqlite3_prepare_v2(handel, query.c_str(), query.length() + 1U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return;
    }

   while (SQLITE_ROW == sqlite3_step(messi))
    {
      std::string name;
      name = reinterpret_cast<const char*>(sqlite3_column_text(messi, 0));

      std::unique_ptr<TableView> sheet = std::make_unique<TableView>(name, reinterpret_cast<void*>(handel));
      std::unique_ptr<MemoryWidthGS> getterSetter = std::make_unique<MemoryWidthGS>(DEF_COLUMN_WIDTH);

      manager.attach(name, std::move(sheet), std::move(getterSetter));
    }

   sqlite3_finalize(messi);
 }
