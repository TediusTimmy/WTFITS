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
#ifndef FORWARDS_ENGINE_MEMORYSPREADSHEET_H
#define FORWARDS_ENGINE_MEMORYSPREADSHEET_H

#include "Forwards/Engine/SpreadSheet.h"

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

   class MemorySpreadSheet final : public SpreadSheetHolder
    {
   public:
      MemorySpreadSheet();
      MemorySpreadSheet(const MemorySpreadSheet&) = delete;
      MemorySpreadSheet& operator=(const MemorySpreadSheet&) = delete;

      std::vector<std::vector<std::unique_ptr<Cell> > > sheet;

      size_t max_row;

      virtual size_t getMaxColumn() override;
      virtual size_t getMaxRow() override;
      virtual size_t getMaxRowForColumn(size_t) override;

      virtual Cell* getCellAt(size_t col, size_t row, const std::string& sheet) override;
      virtual void initCellAt(size_t col, size_t row) override;

      virtual void clearCellAt(size_t col, size_t row) override;
      virtual void clearColumn(size_t col) override;
      virtual void clearRow(size_t row) override;

      virtual void returnCell(Cell* cell) override;
      virtual bool isCellPresent(size_t col, size_t row) override;

      virtual void makeEvergreen(Cell* cell) override;
      virtual void commitCell(Cell* cell) override;
      virtual void dispose(Cell* cell) override;

      virtual void stashResult(Cell* cell, size_t generation) override;
    };

 } // namespace Engine

 } // namespace Forwards

#endif /* FORWARDS_ENGINE_MEMORYSPREADSHEET_H */
