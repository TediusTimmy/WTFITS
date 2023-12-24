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
#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <string>
#include <map>
#include <memory>

#include "Forwards/Engine/SpreadSheet.h"

namespace Forwards
 {
namespace Engine
 {
   class Cell;
 }
 }

class TableView final : public Forwards::Engine::SpreadSheetHolder
 {
public:
   TableView(const std::string& sheetName, void*);
   TableView(const TableView&) = delete;
   TableView& operator=(const TableView&) = delete;

   std::string sheetName;
   void *db; // Type-pun the db handle.

   virtual size_t getMaxColumn() override;
   virtual size_t getMaxRow() override;
   virtual size_t getMaxRowForColumn(size_t) override;

   virtual Forwards::Engine::Cell* getCellAt(size_t col, size_t row, const std::string& sheet) override;
   virtual void initCellAt(size_t col, size_t row) override; // NOP

   virtual void returnCell(Forwards::Engine::Cell* cell) override; // NOP
   virtual bool isCellPresent(size_t col, size_t row) override;

   virtual void clearCellAt(size_t col, size_t row) override; // NOPe
   virtual void clearColumn(size_t col) override; // NOPe
   virtual void clearRow(size_t row) override; // NOPe

   virtual void makeEvergreen(Forwards::Engine::Cell* cell) override; // NOP
   virtual void commitCell(Forwards::Engine::Cell* cell) override; // NOP
   virtual void dispose(Forwards::Engine::Cell* cell) override; // NOP

   virtual void stashResult(Forwards::Engine::Cell* cell, size_t generation) override; // NOP

private:
   size_t rows, cols;
   size_t last;
   std::map<size_t, std::unique_ptr<Forwards::Engine::Cell> > cellCache;
   std::map<Forwards::Engine::Cell*, size_t> lru;
 };

#endif /* TABLEVIEW_H */
