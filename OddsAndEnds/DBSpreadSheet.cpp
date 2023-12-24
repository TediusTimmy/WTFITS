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
#include "DBSpreadSheet.h"
#include "Forwards/Engine/Cell.h"

#include "DBManager.h"

#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "Forwards/Types/ValueType.h"

#include <sqlite3.h>

DBSpreadSheet::DBSpreadSheet(void *db, void *td, DBManager* mgr) : db(db), td(td), mgr(mgr)
 {
 }

size_t DBSpreadSheet::getMaxColumn()
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT MAX(col) FROM sheet;", 28U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return 0U;
    }

   int col = 0U;
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      col = sqlite3_column_int(messi, 0);
    }

   sqlite3_finalize(messi);
   return col + 1U;
 }

size_t DBSpreadSheet::getMaxRow()
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT MAX(row) FROM sheet;", 28U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return 0U;
    }

   int row = 0U;
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      row = sqlite3_column_int(messi, 0);
    }

   sqlite3_finalize(messi);
   return row + 1U;
 }

size_t DBSpreadSheet::getMaxRowForColumn(size_t col)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT MAX(row) FROM sheet WHERE col = :col;", 45U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return 0U;
    }

   int row = 0U;
   sqlite3_bind_int(messi, 1, col);
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      row = sqlite3_column_int(messi, 0);
    }

   sqlite3_finalize(messi);
   return row + 1U;
 }

Forwards::Engine::Cell* DBSpreadSheet::getCellAtRaw(size_t col, size_t row)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT * FROM sheet WHERE col = :col AND row = :row;", 53U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return nullptr;
    }

   Forwards::Engine::Cell* cell = nullptr;
   sqlite3_bind_int64(messi, 1, col);
   sqlite3_bind_int64(messi, 2, row);
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      int type;
      std::string text;

      type = sqlite3_column_int(messi, 2);
      text = reinterpret_cast<const char*>(sqlite3_column_text(messi, 3));

      cell = new Forwards::Engine::Cell(col, row);
      if (1 == type)
       {
         cell->type = Forwards::Engine::LABEL;
       }
      else if (2 == type)
       {
         cell->type = Forwards::Engine::VALUE;
       }
      cell->currentInput = text;
    }

   sqlite3_finalize(messi);

   if (nullptr != cell)
    {
      errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(td), "SELECT * FROM sheet WHERE col = :col AND row = :row;", 53U, &messi, nullptr);
      if (SQLITE_OK == errorCode)
       {
         sqlite3_bind_int64(messi, 1, col);
         sqlite3_bind_int64(messi, 2, row);
         if (SQLITE_ROW == sqlite3_step(messi))
          {
            size_t generation;
            std::string text;

            generation = sqlite3_column_int64(messi, 2);
            text = reinterpret_cast<const char*>(sqlite3_column_text(messi, 3));

            cell->previousGeneration = generation;
             {
               Backwards::Input::StringInput interlinked (text);
               Forwards::Input::Lexer lexer (interlinked);
               Forwards::Parser::StringLogger newLogger;
               std::shared_ptr<Forwards::Engine::Expression> value = Forwards::Parser::Parser::ParseFullExpression(lexer, *mgr->context->map, newLogger, col, row);
               if (nullptr != value.get()) // Should never happen
                {
                  Forwards::Engine::CellFrame newFrame (cell, col, row);
                  try
                   {
                     mgr->context->pushCell(&newFrame);
                     cell->previousValue = value->evaluate(*mgr->context);
                     mgr->context->popCell();
                   }
                  catch (...)
                   {
                     mgr->context->popCell();
                   }
                }
             }
          }

         sqlite3_finalize(messi);
       }
    }

   if ((nullptr != cell) && (nullptr != cell->previousValue.get()))
    {
      Backwards::Input::StringInput interlinked (cell->currentInput);
      Forwards::Input::Lexer lexer (interlinked);
      Forwards::Parser::StringLogger newLogger;
      cell->value = Forwards::Parser::Parser::ParseFullExpression(lexer, *mgr->context->map, newLogger, col, row);
      if (nullptr != cell->value.get())
       {
         cell->currentInput = "";
       }
    }

   return cell;
 }

size_t makeCellId(size_t col, size_t row)
 {
   return ((col << 44U) | row);
 }

Forwards::Engine::Cell* DBSpreadSheet::getCellAt(size_t col, size_t row, const std::string& sheet)
 {
   if (false == sheet.empty())
    {
      Forwards::Engine::SpreadSheetHolder* sheetHolder = mgr->getSpreadSheet(sheet);
      if (nullptr != sheetHolder)
       {
         return sheetHolder->getCellAt(col, row, sheet);
       }
      return nullptr;
    }

   size_t id = makeCellId(col, row);
   auto needle = cellCache.find(id);
   if (cellCache.end() != needle)
    {
      ++refs[needle->second.get()];
      return needle->second.get();
    }

   Forwards::Engine::Cell* cell = getCellAtRaw(col, row);
   if (nullptr != cell)
    {
      cellCache[id] = std::unique_ptr<Forwards::Engine::Cell>(cell);
      refs[cell] = 1U;
    }

   return cell;
 }

void DBSpreadSheet::returnCell(Forwards::Engine::Cell* cell)
 {
   if (nullptr != cell)
    {
      auto needle = refs.find(cell);
      if (refs.end() != needle)
       {
         --needle->second;
         if ((0U == needle->second) && (false == cell->evergreen))
          {
            cellCache.erase(makeCellId(cell->col, cell->row));
            refs.erase(cell);
          }
       }
      else
       {
         // Oh noes!
       }
    }
 }

void DBSpreadSheet::initCellAt(size_t col, size_t row)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "INSERT OR REPLACE INTO sheet VALUES (:col, :row, :type, :content);", 67U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, col);
      sqlite3_bind_int64(messi, 2, row);
      sqlite3_bind_int(messi, 3, 0);
      sqlite3_bind_text(messi, 4, "", -1, nullptr);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }

bool DBSpreadSheet::isCellPresent(size_t col, size_t row)
 {
   Forwards::Engine::Cell* cell = getCellAtRaw(col, row);
   bool result = nullptr != cell;
   delete cell;
   return result;
 }

void DBSpreadSheet::clearCellAt(size_t col, size_t row)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "DELETE FROM sheet WHERE col = :col AND row = :row;", 51U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, col);
      sqlite3_bind_int64(messi, 2, row);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }

void DBSpreadSheet::clearColumn(size_t col)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "DELETE FROM sheet WHERE col = :col;", 36U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, col);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }

void DBSpreadSheet::clearRow(size_t row)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "DELETE FROM sheet WHERE row = :row;", 36U, &messi, nullptr);

   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, row);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }

void DBSpreadSheet::makeEvergreen(Forwards::Engine::Cell* cell)
 {
   cell->evergreen = true;
 }

void DBSpreadSheet::commitCell(Forwards::Engine::Cell* cell)
 {
   cell->evergreen = false;

   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "INSERT OR REPLACE INTO sheet VALUES (:col, :row, :type, :content);", 67U, &messi, nullptr);

   std::string content;
   int type = 0;
   if (Forwards::Engine::VALUE == cell->type)
    {
      type = 2;
      if (nullptr == cell->value.get())
       {
         content = cell->currentInput;
       }
      else
       {
         content = cell->value->toString(cell->col, cell->row);
       }
    }
   else
    {
      type = 1;
      if (nullptr != cell->previousValue.get())
       {
         content = cell->previousValue->toString(cell->col, cell->row, false);
       }
      else
       {
         content = cell->currentInput; // Hack until cells work good again.
       }
    }

   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, cell->col);
      sqlite3_bind_int64(messi, 2, cell->row);
      sqlite3_bind_int(messi, 3, type);
      sqlite3_bind_text(messi, 4, content.c_str(), -1, nullptr);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }

void DBSpreadSheet::dispose(Forwards::Engine::Cell* cell)
 {
   cell->evergreen = false;
 }

void DBSpreadSheet::stashResult(Forwards::Engine::Cell* cell, size_t generation)
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(td), "INSERT OR REPLACE INTO sheet VALUES (:col, :row, :generation, :content);", 73U, &messi, nullptr);

   std::string content;
   if (nullptr != cell->previousValue.get())
    {
      content = cell->previousValue->toString(cell->col, cell->row, true);
    }
   if (SQLITE_OK == errorCode)
    {
      sqlite3_bind_int64(messi, 1, cell->col);
      sqlite3_bind_int64(messi, 2, cell->row);
      sqlite3_bind_int64(messi, 3, generation);
      sqlite3_bind_text(messi, 4, content.c_str(), -1, nullptr);
      sqlite3_step(messi);
      sqlite3_finalize(messi);
    }
 }
