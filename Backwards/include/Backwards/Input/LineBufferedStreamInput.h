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
#ifndef BACKWARDS_INPUT_LINEBUFFEREDSTREAMINPUT_H
#define BACKWARDS_INPUT_LINEBUFFEREDSTREAMINPUT_H

#include <fstream>
#include <string>
#include "Backwards/Input/GenericInput.h"

namespace Backwards
 {

namespace Input
 {

   class LineBufferedStreamInput : public GenericInput
    {

   private:
      std::istream& input;
      std::string currentLine;
      size_t index;

   public:
      explicit LineBufferedStreamInput(std::istream& input);

      int getNextCharacter() override;

    };

   class ConsoleInput final : public LineBufferedStreamInput
    {

   public:
      ConsoleInput();

    };

   class FileInput final : public LineBufferedStreamInput
    {

   private:
      std::ifstream source;

   public:
      explicit FileInput(const std::string& fileName);

      int getNextCharacter() override;

    };

 } // namespace Input

 } // namespace Backwards

#endif /* BACKWARDS_INPUT_LINEBUFFEREDSTREAMINPUT_H */
