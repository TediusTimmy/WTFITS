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
#ifndef GETANDSET_H
#define GETANDSET_H

#define MAX_COLUMN_WIDTH 40
#define MIN_COLUMN_WIDTH 1
#define DEF_COLUMN_WIDTH 9

class WidthGetterSetter
 {
protected:
   int defWidth;
public:
   int getDefWidth() const { return defWidth; }
   void setDefWidth(int newWidth) { defWidth = newWidth; }

   explicit WidthGetterSetter(int defWidth) : defWidth(defWidth) { }

   virtual int getWidth(size_t col) = 0;
   virtual void setWidth(size_t col, int width) = 0;
   virtual void incWidth(size_t col) = 0;
   virtual void decWidth(size_t col) = 0;
 };

class MemoryWidthGS final : public WidthGetterSetter
 {
private:
   std::vector<int> map;
public:
   explicit MemoryWidthGS(int defWidth) : WidthGetterSetter(defWidth) { }

   virtual int getWidth(size_t col) override;
   virtual void setWidth(size_t col, int width) override;
   virtual void incWidth(size_t col) override;
   virtual void decWidth(size_t col) override;
 };

class DBWidthGS final : public WidthGetterSetter
 {
private:
   void *db; // Don't tell the compiler this is a sqlite3* : DBManager owns this object
   std::map<size_t, size_t> lru;
   std::map<size_t, int> cache;
   size_t access;
public:
   DBWidthGS(int defWidth, void *db) : WidthGetterSetter(defWidth), db(db), access(0U) { }

   virtual int getWidth(size_t col) override;
   virtual void setWidth(size_t col, int width) override;
   virtual void incWidth(size_t col) override;
   virtual void decWidth(size_t col) override;

   void precache();
 };

#endif /* GETANDSET_H */
