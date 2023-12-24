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
#include "TableView.h"
#include "Forwards/Engine/Cell.h"

#include "Forwards/Engine/Expression.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/FloatValue.h"

#include "NumberSystem.h"

#include <sqlite3.h>
#include <limits>
#include <numeric>
#include <algorithm>

TableView::TableView(const std::string& sheetName, void *db) : sheetName(sheetName), db(db), rows(~0U), cols(~0U), last(0U)
 {
 }

size_t TableView::getMaxColumn()
 {
   if (~0U != cols)
    {
      return cols;
    }

   sqlite3_stmt *messi;
   static const std::string query = "SELECT COUNT(name) FROM pragma_table_info(:sheet);";
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), query.c_str(), query.length() + 1U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return 0U;
    }

   sqlite3_bind_text(messi, 1, sheetName.c_str(), -1, nullptr);
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      cols = sqlite3_column_int64(messi, 0);
    }

   sqlite3_finalize(messi);
   return cols;
 }

size_t TableView::getMaxRow()
 {
   if (~0U != rows)
    {
      return rows;
    }

   sqlite3_stmt *messi;
   std::string query = "SELECT COUNT(*) FROM " + sheetName + ";";
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), query.c_str(), query.length() + 1U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return 0U;
    }

   sqlite3_bind_text(messi, 1, sheetName.c_str(), -1, nullptr);
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      rows = sqlite3_column_int64(messi, 0) + 1U; // for headers
    }

   sqlite3_finalize(messi);
   return rows;
 }

size_t TableView::getMaxRowForColumn(size_t)
 {
   return getMaxRow();
 }

static size_t maxCacheSize = 5000U;
static size_t makeCellId(size_t col, size_t row)
 {
   return ((col << 44U) | row);
 }

static Forwards::Engine::Cell* makeCellString(const std::string& text, size_t col, size_t row)
 {
   Forwards::Engine::Cell* cell = new Forwards::Engine::Cell(col, row);
   cell->type = Forwards::Engine::LABEL;
   std::shared_ptr<Forwards::Types::StringValue> str = std::make_shared<Forwards::Types::StringValue>(text);
   cell->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), str);
   cell->previousValue = str;
   return cell;
 }

static Forwards::Engine::Cell* makeCellNumber(const char* text, size_t col, size_t row)
 {
   Forwards::Engine::Cell* cell = new Forwards::Engine::Cell(col, row);
   cell->type = Forwards::Engine::VALUE;
   std::shared_ptr<Forwards::Types::FloatValue> str = std::make_shared<Forwards::Types::FloatValue>(NumberSystem::getCurrentNumberSystem().fromString(text));
   cell->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), str);
   cell->previousValue = str;
   return cell;
 }

Forwards::Engine::Cell* TableView::getCellAt(size_t col, size_t row, const std::string&)
 {
   if (col >= (getMaxColumn() - 1U))
    {
      return nullptr;
    }
   if (row >= getMaxRow())
    {
      return nullptr;
    }

   size_t id = makeCellId(col, row);
   auto needle = cellCache.find(id);
   if (cellCache.end() != needle)
    {
      lru[needle->second.get()] = ++last;
      return needle->second.get();
    }

   sqlite3_stmt *messi;

   std::string query1 = "SELECT * FROM " + sheetName + " LIMIT 1 OFFSET :off ;";
   static const std::string query2 = "SELECT name FROM pragma_table_info(:sheet) LIMIT 1 OFFSET :off ;";
   int errorCode;
   if (0U != row)
    {
      errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), query1.c_str(), query1.length() + 1U, &messi, nullptr);
    }
   else
    {
      errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), query2.c_str(), query2.length() + 1U, &messi, nullptr);
    }

   if (SQLITE_OK != errorCode)
    {
      return nullptr;
    }

   if (0U != row)
    {
      sqlite3_bind_int64(messi, 1, row - 1U);
    }
   else
    {
      sqlite3_bind_text(messi, 1, sheetName.c_str(), -1, nullptr);
      sqlite3_bind_int64(messi, 2, col);
    }
   Forwards::Engine::Cell* cell = nullptr;
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      if (0U != row)
       {
         switch (sqlite3_column_type(messi, col))
          {
         case SQLITE_INTEGER:
         case SQLITE_FLOAT:
          {
            const char * temp = reinterpret_cast<const char*>(sqlite3_column_text(messi, col));
            cell = makeCellNumber(temp, col, row);
          }
            break;
         case SQLITE3_TEXT:
          {
            std::string text = reinterpret_cast<const char*>(sqlite3_column_text(messi, col));
            for (size_t i = 0U; i < text.length(); ++i)
             {
               if ((text[i] < ' ') || (text[i] > '~')) text[i] = ' ';
             }
            cell = makeCellString(text, col, row);
          }
            break;
         case SQLITE_BLOB:
         case SQLITE_NULL:
            break;
          }
       }
      else
       {
         std::string text = reinterpret_cast<const char*>(sqlite3_column_text(messi, 0));
         cell = makeCellString(text, col, row);
       }
    }

   sqlite3_finalize(messi);

   if (nullptr != cell)
    {
      cellCache[id] = std::unique_ptr<Forwards::Engine::Cell>(cell);
      lru[cell] = ++last;

      if (cellCache.size() > maxCacheSize)
       {
         size_t min = std::accumulate(lru.begin(), lru.end(), std::numeric_limits<size_t>::max(), [](size_t x, auto y){ return std::min(x, y.second);});
         auto iter = std::find_if(lru.begin(), lru.end(), [=](auto x){ return min == x.second; });
         cellCache.erase(makeCellId(iter->first->col, iter->first->row));
         lru.erase(iter->first);
       }
    }

   return cell;
 }

void TableView::returnCell(Forwards::Engine::Cell*)
 {
 }

void TableView::initCellAt(size_t, size_t)
 {
 }

bool TableView::isCellPresent(size_t col, size_t row)
 {
   return (col < getMaxColumn()) && (row < getMaxRow());
 }

void TableView::clearCellAt(size_t, size_t)
 {
 }

void TableView::clearColumn(size_t)
 {
 }

void TableView::clearRow(size_t)
 {
 }

void TableView::makeEvergreen(Forwards::Engine::Cell*)
 {
 }

void TableView::commitCell(Forwards::Engine::Cell*)
 {
 }

void TableView::dispose(Forwards::Engine::Cell*)
 {
 }

void TableView::stashResult(Forwards::Engine::Cell*, size_t)
 {
 }
