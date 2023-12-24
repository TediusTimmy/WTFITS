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
#include "Forwards/Engine/MemorySpreadSheet.h"
#include "Forwards/Engine/Cell.h"

namespace Forwards
 {

namespace Engine
 {

   MemorySpreadSheet::MemorySpreadSheet() : max_row(0U)
    {
    }

   size_t MemorySpreadSheet::getMaxColumn()
    {
      return sheet.size();
    }

   size_t MemorySpreadSheet::getMaxRow()
    {
      return max_row;
    }

   size_t MemorySpreadSheet::getMaxRowForColumn(size_t col)
    {
      if (col < sheet.size())
       {
         return sheet[col].size();
       }
      return 0U;
    }

   Cell* MemorySpreadSheet::getCellAt(size_t col, size_t row, const std::string&)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            return sheet[col][row].get();
          }
       }
      return nullptr;
    }

   void MemorySpreadSheet::initCellAt(size_t col, size_t row)
    {
      if (col >= sheet.size())
       {
         sheet.resize(col + 1U);
       }
      if (row >= sheet[col].size())
       {
         sheet[col].resize(row + 1U);
         if (row >= max_row)
          {
            max_row = row + 1U;
          }
       }
      sheet[col][row] = std::make_unique<Forwards::Engine::Cell>(col, row);
    }

   void MemorySpreadSheet::clearCellAt(size_t col, size_t row)
    {
      if (col < sheet.size())
       {
         if (row < sheet[col].size())
          {
            sheet[col][row].reset();
          }
       }
    }

   void MemorySpreadSheet::clearColumn(size_t col)
    {
      if (col < sheet.size())
       {
         sheet[col].clear();
       }
    }

   void MemorySpreadSheet::clearRow(size_t row)
    {
      for (size_t i = 0U; i < sheet.size(); ++i)
       {
         if (row < sheet[i].size())
          {
            sheet[i][row].reset();
          }
       }
    }

   void MemorySpreadSheet::returnCell(Cell*)
    {
    }

   void MemorySpreadSheet::makeEvergreen(Cell*)
    {
    }

   void MemorySpreadSheet::commitCell(Cell*)
    {
    }

   void MemorySpreadSheet::dispose(Cell*)
    {
    }

   void MemorySpreadSheet::stashResult(Cell*, size_t)
    {
    }

   bool MemorySpreadSheet::isCellPresent(size_t col, size_t row)
    {
      return nullptr != getCellAt(col, row, "");
    }

 } // namespace Engine

 } // namespace Forwards
