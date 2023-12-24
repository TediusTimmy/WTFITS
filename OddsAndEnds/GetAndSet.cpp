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
#include <map>
#include <set>
#include <limits>
using std::size_t;

#include "GetAndSet.h"

#include <sqlite3.h>

int MemoryWidthGS::getWidth(size_t col)
 {
   if (map.size() <= col)
    {
      return defWidth;
    }
   return map[col];
 }

void MemoryWidthGS::setWidth(size_t col, int width)
 {
   if ((width >= MIN_COLUMN_WIDTH) && (width <= MAX_COLUMN_WIDTH))
    {
      if (map.size() <= col)
       {
         map.resize(col + 1U, defWidth);
       }
      map[col] = width;
    }
 }

void MemoryWidthGS::incWidth(size_t col)
 {
   if (map.size() <= col)
    {
      map.resize(col + 1U, defWidth);
    }
   if (MAX_COLUMN_WIDTH != map[col])
    {
      ++map[col];
    }
 }

void MemoryWidthGS::decWidth(size_t col)
 {
   if (map.size() <= col)
    {
      map.resize(col + 1U, defWidth);
    }
   if (MIN_COLUMN_WIDTH != map[col])
    {
      --map[col];
    }
 }

const size_t MAX_CACHE = 100U;

int DBWidthGS::getWidth(size_t col)
 {
   const auto thing = cache.find(col);
   if (cache.end() != thing)
    {
      lru[col] = ++access;
      return thing->second;
    }

   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT * FROM widths WHERE col = :col;", 39U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return defWidth;
    }

   int width = defWidth;
   sqlite3_bind_int(messi, 1, col);
   if (SQLITE_ROW == sqlite3_step(messi))
    {
      width = sqlite3_column_int(messi, 1);
    }

   sqlite3_finalize(messi);

   cache[col] = width;
   lru[col] = ++access;

   if (cache.size() > MAX_CACHE)
    {
      size_t min = std::numeric_limits<size_t>::max();
      for (auto pair : lru)
       {
         min = std::min(min, pair.second);
       }
      for (auto pair : lru)
       {
         if (pair.second == min)
          {
            cache.erase(pair.first);
            lru.erase(pair.first);
            break;
          }
       }
    }

   return width;
 }

void DBWidthGS::setWidth(size_t col, int width)
 {
   if ((width >= MIN_COLUMN_WIDTH) && (width <= MAX_COLUMN_WIDTH))
    {
      sqlite3_stmt *messi;
      int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "INSERT OR REPLACE INTO widths VALUES (:col, :width);", 53U, &messi, nullptr);

      if (SQLITE_OK == errorCode)
       {
         sqlite3_bind_int(messi, 1, col);
         sqlite3_bind_int(messi, 2, width);
         sqlite3_step(messi);
         sqlite3_finalize(messi);
       }

      auto thing = cache.find(col);
      if (cache.end() != thing)
       {
         thing->second = width;
       }
    }
 }

void DBWidthGS::incWidth(size_t col)
 {
   setWidth(col, getWidth(col) + 1U);
 }

void DBWidthGS::decWidth(size_t col)
 {
   setWidth(col, getWidth(col) - 1U);
 }

void DBWidthGS::precache()
 {
   sqlite3_stmt *messi;
   int errorCode = sqlite3_prepare_v2(reinterpret_cast<sqlite3*>(db), "SELECT * FROM widths;", 22U, &messi, nullptr);

   if (SQLITE_OK != errorCode)
    {
      return;
    }

   while (SQLITE_ROW == sqlite3_step(messi))
    {
      int col;
      int width;

      col = sqlite3_column_int(messi, 0);
      width = sqlite3_column_int(messi, 1);

      cache[col] = width;
      lru[col] = access;
    }

   sqlite3_finalize(messi);
 }
