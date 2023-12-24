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
#ifndef FORWARDS_ENGINE_SPREADSHEET_H
#define FORWARDS_ENGINE_SPREADSHEET_H

#include <vector>
#include <memory>

namespace Forwards
 {

namespace Types
 {
   class ValueType;
 }

namespace Engine
 {

   class CallingContext;
   class Cell;

   class SpreadSheetHolder
    {
   public:
      virtual size_t getMaxColumn() = 0;
      virtual size_t getMaxRow() = 0;
      virtual size_t getMaxRowForColumn(size_t) = 0;

      virtual Cell* getCellAt(size_t col, size_t row, const std::string& sheet) = 0;
      virtual void initCellAt(size_t col, size_t row) = 0;

      virtual void clearCellAt(size_t col, size_t row) = 0;
      virtual void clearColumn(size_t col) = 0;
      virtual void clearRow(size_t row) = 0;

      virtual void returnCell(Cell* cell) = 0;
      virtual bool isCellPresent(size_t col, size_t row) = 0;

      virtual void makeEvergreen(Cell* cell) = 0;
      virtual void commitCell(Cell* cell) = 0;
      virtual void dispose(Cell* cell) = 0;

      virtual void stashResult(Cell* cell, size_t generation) = 0;
    };

   class SpreadSheet final
    {
   public:
      SpreadSheet();
      SpreadSheet(const SpreadSheet&) = delete;
      SpreadSheet& operator=(const SpreadSheet&) = delete;

      SpreadSheetHolder* currentSheet;

      size_t getMaxColumn();
      size_t getMaxRow();
      size_t getMaxRowForColumn(size_t);

      bool c_major;
      bool top_down;
      bool left_right;

      Cell* getCellAt(size_t col, size_t row, const std::string& sheet);
      bool isCellPresent(size_t col, size_t row);
      void initCellAt(size_t col, size_t row);
      void returnCell(Cell* cell);

      void makeEvergreen(Cell* cell);
      void commitCell(Cell* cell);
      void dispose(Cell* cell);
      
      void stashResult(Cell* cell, size_t generation);

      void clearCellAt(size_t col, size_t row);
      void clearColumn(size_t col);
      void clearRow(size_t row);

      std::string computeCell(CallingContext&, std::shared_ptr<Types::ValueType>& OUT, size_t col, size_t row);
      std::shared_ptr<Types::ValueType> computeCell(CallingContext&, size_t col, size_t row, bool rethrow);
      void recalc(CallingContext&);
    };

   class AutoCell final
    {
   public:
      SpreadSheet* sheet;
      Cell* cell;
      AutoCell(SpreadSheet*, Cell*);
      ~AutoCell();
    };

 } // namespace Engine

 } // namespace Forwards

#endif /* FORWARDS_ENGINE_SPREADSHEET_H */
