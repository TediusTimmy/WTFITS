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
#include "Forwards/Engine/SpreadSheet.h"

#include "Backwards/Engine/Logger.h"
#include "Backwards/Input/StringInput.h"

#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/Expression.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/StringLogger.h"

#include "Forwards/Types/ValueType.h"
#include "Forwards/Types/StringValue.h"

/*
   This is purposely in Parser because it depends on Parser.
   SpreadSheet creates a circular dependency between Parser and Engine, and I don't like it.
*/

namespace Forwards
 {

namespace Engine
 {

   SpreadSheet::SpreadSheet() : currentSheet(nullptr), c_major(true), top_down(true), left_right(true)
    {
    }

   size_t SpreadSheet::getMaxColumn()
    {
      return currentSheet->getMaxColumn();
    }

   size_t SpreadSheet::getMaxRow()
    {
      return currentSheet->getMaxRow();
    }

   size_t SpreadSheet::getMaxRowForColumn(size_t col)
    {
      return currentSheet->getMaxRowForColumn(col);
    }

   Cell* SpreadSheet::getCellAt(size_t col, size_t row, const std::string& sheet)
    {
      return currentSheet->getCellAt(col, row, sheet);
    }

   bool SpreadSheet::isCellPresent(size_t col, size_t row)
    {
      return currentSheet->isCellPresent(col, row);
    }

   void SpreadSheet::initCellAt(size_t col, size_t row)
    {
      currentSheet->initCellAt(col, row);
    }

   void SpreadSheet::returnCell(Cell* cell)
    {
      currentSheet->returnCell(cell);
    }

   void SpreadSheet::clearCellAt(size_t col, size_t row)
    {
      currentSheet->clearCellAt(col, row);
    }

   void SpreadSheet::clearColumn(size_t col)
    {
      currentSheet->clearColumn(col);
    }

   void SpreadSheet::clearRow(size_t row)
    {
      currentSheet->clearRow(row);
    }

   void SpreadSheet::makeEvergreen(Cell* cell)
    {
      currentSheet->makeEvergreen(cell);
    }

   void SpreadSheet::commitCell(Cell* cell)
    {
      currentSheet->commitCell(cell);
    }

   void SpreadSheet::dispose(Cell* cell)
    {
      currentSheet->dispose(cell);
    }

   void SpreadSheet::stashResult(Cell* cell, size_t generation)
    {
      currentSheet->stashResult(cell, generation);
    }


   std::string SpreadSheet::computeCell(CallingContext& context, std::shared_ptr<Types::ValueType>& OUT, size_t col, size_t row)
    {
      std::string result;
      OUT.reset(); // Ensure to clear OUT variable.

      AutoCell cell (this, getCellAt(col, row, ""));
      if (nullptr == cell.cell)
       {
         return result;
       }
      CellFrame newFrame (cell.cell, col, row);

         // If we have already evaluated this cell this generation, stop.
      if ((context.generation == cell.cell->previousGeneration) && (nullptr != cell.cell->value.get()))
       {
         OUT = cell.cell->previousValue;
         return result;
       }

         // If this is a LABEL, then set the value.
      std::shared_ptr<Expression> value = cell.cell->value;
      if ((LABEL == cell.cell->type) && (nullptr == value.get()))
       {
         value = std::make_shared<Constant>(Input::Token(), std::make_shared<Types::StringValue>(cell.cell->currentInput));
       }
         // Else, this is a VALUE, and we need to parse it.
      if (nullptr == value.get())
       {
         Backwards::Input::StringInput interlinked (cell.cell->currentInput);
         Input::Lexer lexer (interlinked);
         Backwards::Engine::Logger* temp = context.logger;
         Parser::StringLogger newLogger;
         context.logger = &newLogger;
         value = Parser::Parser::ParseFullExpression(lexer, *context.map, *context.logger, col, row);
         context.logger = temp;
         if (newLogger.logs.size() > 0U)
          {
            result = newLogger.logs[0U];
          }
       }

         // If the parse failed, leave. Result will have the first parser message.
      if (nullptr == value.get())
       {
         return result;
       }

         // If this is a regular update, update the cell. Eww....
      if (false == context.inUserInput)
       {
         cell.cell->currentInput = "";
         cell.cell->value = value;
       }

      try
       {
         context.pushCell(&newFrame);
            // Evaluate the new cell.
         context.topCell()->cell->inEvaluation = true;
         context.topCell()->cell->recursed = false;
         OUT = value->evaluate(context);
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (const std::exception& e)
       {
         result = e.what();
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (...)
       {
         context.topCell()->cell->inEvaluation = false;
         context.popCell();
       }

      stashResult(cell.cell, context.generation);

      size_t c = result.find('\n');
      if (std::string::npos != c)
       {
         result.resize(c);
       }
      return result;
    }


   std::shared_ptr<Types::ValueType> SpreadSheet::computeCell(CallingContext& context, size_t col, size_t row, bool rethrow)
    {
      std::shared_ptr<Types::ValueType> OUT;

      AutoCell cell (this, getCellAt(col, row, ""));
      if (nullptr == cell.cell)
       {
         return OUT;
       }
      CellFrame newFrame (cell.cell, col, row);

         // If we have already evaluated this cell this generation, stop.
      if (context.generation == cell.cell->previousGeneration)
       {
         return cell.cell->previousValue;
       }

         // If this is a LABEL, then set the value.
      std::shared_ptr<Expression> value = cell.cell->value;
      if ((LABEL == cell.cell->type) && (nullptr == value.get()))
       {
         value = std::make_shared<Constant>(Input::Token(), std::make_shared<Types::StringValue>(cell.cell->currentInput));
       }
         // Else, this is a VALUE, and we need to parse it.
      if (nullptr == value.get())
       {
         Backwards::Input::StringInput interlinked (cell.cell->currentInput);
         Input::Lexer lexer (interlinked);
         Backwards::Engine::Logger* temp = context.logger;
         Parser::StringLogger newLogger;
         context.logger = &newLogger;
         value = Parser::Parser::ParseFullExpression(lexer, *context.map, *context.logger, col, row);
         context.logger = temp;
       }

         // If the parse failed, leave. Result will have the first parser message.
      if (nullptr == value.get())
       {
         return OUT;
       }

         // If this is a regular update, update the cell. Eww....
      if (false == context.inUserInput)
       {
         cell.cell->currentInput = "";
         cell.cell->value = value;
       }

      try
       {
         context.pushCell(&newFrame);
            // Evaluate the new cell.
         context.topCell()->cell->inEvaluation = true;
         context.topCell()->cell->recursed = false;
         OUT = value->evaluate(context);
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
       }
      catch (...)
       {
         context.topCell()->cell->inEvaluation = false;
         context.topCell()->cell->previousGeneration = context.generation;
         context.topCell()->cell->previousValue = OUT;
         context.popCell();
         if (true == rethrow)
          {
            throw;
          }
       }

      stashResult(cell.cell, context.generation);

      return OUT;
    }


   void SpreadSheet::recalc(CallingContext& context)
    {
      context.inUserInput = false;
      ++context.generation;
      context.names->clear();
      if (c_major) // Going in column-major order
       {
         if (left_right) // Going from left-to-right
          {
            if (top_down) // Going from top-to-bottom
             {
               for (size_t col = 0U; col < getMaxColumn(); ++col)
                {
                  for (size_t row = 0U; row < getMaxRowForColumn(col); ++row)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = 0U; col < getMaxColumn(); ++col)
                {
                  for (size_t row = getMaxRowForColumn(col) - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
          }
         else // Going from right-to-left
          {
            if (top_down) // Going from top-to-bottom
             {
               for (size_t col = getMaxColumn() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                {
                  for (size_t row = 0U; row < getMaxRowForColumn(col); ++row)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from bottom-to-top
             {
               for (size_t col = getMaxColumn() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                {
                  for (size_t row = getMaxRowForColumn(col) - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
          }
       }
      else // Going in row major order
       {
         if (top_down) // Going from top-to-bottom
          {
            if (left_right) // Going from left-to-right
             {
               for (size_t row = 0U; row < getMaxRow(); ++row)
                {
                  for (size_t col = 0U; col < getMaxColumn(); ++col)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = getMaxRow() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = 0U; col < getMaxColumn(); ++col)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
          }
         else // Going from bottom-to-top
          {
            if (left_right) // Going from left-to-right
             {
               for (size_t row = 0U; row < getMaxRow(); ++row)
                {
                  for (size_t col = getMaxColumn() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
            else // Going from right-to-left
             {
               for (size_t row = getMaxRow() - 1U; row != (static_cast<size_t>(0U) - 1U); --row)
                {
                  for (size_t col = getMaxColumn() - 1U; col != (static_cast<size_t>(0U) - 1U); --col)
                   {
                     (void) computeCell(context, col, row, false);
                   }
                }
             }
          }
       }
      ++context.generation;
    }

   AutoCell::AutoCell(SpreadSheet* sheet, Cell* cell) : sheet(sheet), cell(cell)
    {
    }

   AutoCell::~AutoCell()
    {
      sheet->returnCell(cell);
    }

 } // namespace Engine

 } // namespace Forwards
