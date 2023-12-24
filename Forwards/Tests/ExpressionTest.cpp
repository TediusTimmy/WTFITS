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
#include "gtest/gtest.h"

#include "Forwards/Engine/Expression.h"
#include "Forwards/Engine/CallingContext.h"
#include "Forwards/Engine/SpreadSheet.h"
#include "Forwards/Engine/Cell.h"
#include "Forwards/Engine/StdLib.h"
#include "Forwards/Engine/MemorySpreadSheet.h"

#include "Forwards/Engine/CellRangeExpand.h"
#include "Forwards/Engine/CellRefEval.h"

#include "Forwards/Parser/Parser.h"
#include "Forwards/Parser/ContextBuilder.h"

#include "Forwards/Types/FloatValue.h"
#include "Forwards/Types/StringValue.h"
#include "Forwards/Types/NilValue.h"
#include "Forwards/Types/CellRefValue.h"
#include "Forwards/Types/CellRangeValue.h"

#include "Backwards/Engine/FatalException.h"
#include "Backwards/Engine/Logger.h"
#include "Backwards/Engine/ProgrammingException.h"
#include "Backwards/Engine/Statement.h"

#include "Backwards/Input/Lexer.h"
#include "Backwards/Input/LineBufferedStreamInput.h"
#include "Backwards/Input/StringInput.h"

#include "Backwards/Parser/SymbolTable.h"
#include "Backwards/Parser/Parser.h"

#include "Backwards/Types/FloatValue.h"
#include "Backwards/Types/StringValue.h"
#include "Backwards/Types/CellRangeValue.h"
#include "Backwards/Types/CellRefValue.h"

#include "NumberSystem.h"

class StringLogger final : public Backwards::Engine::Logger
 {
public:
   std::vector<std::string> logs;
   void log (const std::string& message) { logs.emplace_back(message); }
   std::string get () { return ""; }
 };

static std::shared_ptr<Forwards::Types::FloatValue> makeFloatValue (const char * str)
 {
   return std::make_shared<Forwards::Types::FloatValue>(NumberSystem::getCurrentNumberSystem().fromString(str));
 }

TEST(EngineTests, testFloats)
 {
   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("9"));
   std::shared_ptr<Forwards::Engine::Constant> six = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("0"));
   std::shared_ptr<Forwards::Engine::Constant> sev = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("-2"));
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::Plus plus (Forwards::Input::Token(), one, two);
   res = plus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("15"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6+9", plus.toString(1U, 1U, 0));

   Forwards::Engine::Minus minus (Forwards::Input::Token(), one, two);
   res = minus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("-3"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6-9", minus.toString(1U, 1U, 0));

   Forwards::Engine::Multiply multiply (Forwards::Input::Token(), one, two);
   res = multiply.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("54"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6*9", multiply.toString(1U, 1U, 0));

   NumberSystem::getCurrentNumberSystem().setDefaultPrecision(1U);
   Forwards::Engine::Divide divide (Forwards::Input::Token(), two, one); // Flipped args
   res = divide.evaluate(context);
   NumberSystem::getCurrentNumberSystem().setDefaultPrecision(0U);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1.5"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("9/6", divide.toString(1U, 1U, 0));

   Forwards::Engine::Equals equalsT (Forwards::Input::Token(), one, one);
   res = equalsT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6=6", equalsT.toString(1U, 1U, 0));

   Forwards::Engine::Equals equalsF (Forwards::Input::Token(), one, two);
   res = equalsF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6=9", equalsF.toString(1U, 1U, 0));

   Forwards::Engine::NotEqual notEqualT (Forwards::Input::Token(), one, two);
   res = notEqualT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6<>9", notEqualT.toString(1U, 1U, 0));

   Forwards::Engine::NotEqual notEqualF (Forwards::Input::Token(), one, one);
   res = notEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6<>6", notEqualF.toString(1U, 1U, 0));

   Forwards::Engine::Greater greaterT (Forwards::Input::Token(), two, one);
   res = greaterT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("9>6", greaterT.toString(1U, 1U, 0));

   Forwards::Engine::Greater greaterF (Forwards::Input::Token(), one, two);
   res = greaterF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6>9", greaterF.toString(1U, 1U, 0));

   Forwards::Engine::Less lessT (Forwards::Input::Token(), one, two);
   res = lessT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6<9", lessT.toString(1U, 1U, 0));

   Forwards::Engine::Less lessF (Forwards::Input::Token(), two, one);
   res = lessF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("9<6", lessF.toString(1U, 1U, 0));

   Forwards::Engine::GEQ geqT (Forwards::Input::Token(), two, one);
   res = geqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("9>=6", geqT.toString(1U, 1U, 0));

   Forwards::Engine::GEQ geqF (Forwards::Input::Token(), one, two);
   res = geqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6>=9", geqF.toString(1U, 1U, 0));

   Forwards::Engine::LEQ leqT (Forwards::Input::Token(), one, two);
   res = leqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("6<=9", leqT.toString(1U, 1U, 0));

   Forwards::Engine::LEQ leqF (Forwards::Input::Token(), two, one);
   res = leqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("9<=6", leqF.toString(1U, 1U, 0));

   Forwards::Engine::Negate neg (Forwards::Input::Token(), one);
   res = neg.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("-6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("-6", neg.toString(1U, 1U, 0));

   Forwards::Engine::Cat cat (Forwards::Input::Token(), one, two);
   res = cat.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("69", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value); // Nice
   EXPECT_EQ("6&9", cat.toString(1U, 1U, 0));


   std::shared_ptr<Forwards::Engine::Constant> three = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>());

   Forwards::Engine::Plus plusNil (Forwards::Input::Token(), one, three);
   res = plusNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Minus minusNil (Forwards::Input::Token(), one, three);
   res = minusNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Multiply multiplyNil (Forwards::Input::Token(), one, three);
   res = multiplyNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Divide divideNil (Forwards::Input::Token(), one, three);
   res = divideNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_TRUE(std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value->isInf());

   Forwards::Engine::Equals equalsTNil (Forwards::Input::Token(), six, three);
   res = equalsTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals equalsFNil (Forwards::Input::Token(), one, three);
   res = equalsFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualTNil (Forwards::Input::Token(), one, three);
   res = notEqualTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualFNil (Forwards::Input::Token(), six, three);
   res = notEqualFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterTNil (Forwards::Input::Token(), two, three);
   res = greaterTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterFNil (Forwards::Input::Token(), six, three);
   res = greaterFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessTNil (Forwards::Input::Token(), sev, three);
   res = lessTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessFNil (Forwards::Input::Token(), two, three);
   res = lessFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqTNil (Forwards::Input::Token(), six, three);
   res = geqTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqFNil (Forwards::Input::Token(), sev, three);
   res = geqFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqTNil (Forwards::Input::Token(), sev, three);
   res = leqTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqFNil (Forwards::Input::Token(), two, three);
   res = leqFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Cat catNil (Forwards::Input::Token(), one, three);
   res = catNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("6", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);



   Forwards::Engine::Plus nilplus (Forwards::Input::Token(), three, one);
   res = nilplus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Minus nilminus (Forwards::Input::Token(), three, one);
   res = nilminus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("-6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Multiply nilmultiply (Forwards::Input::Token(), three, one);
   res = nilmultiply.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Divide nildivide (Forwards::Input::Token(), three, one);
   res = nildivide.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals nilequalsT (Forwards::Input::Token(), three, six);
   res = nilequalsT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals nilequalsF (Forwards::Input::Token(), three, one);
   res = nilequalsF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual nilnotEqualT (Forwards::Input::Token(), three, one);
   res = nilnotEqualT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual nilnotEqualF (Forwards::Input::Token(), three, six);
   res = nilnotEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater nilgreaterT (Forwards::Input::Token(), three, sev);
   res = nilgreaterT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater nilgreaterF (Forwards::Input::Token(), three, six);
   res = nilgreaterF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less nillessT (Forwards::Input::Token(), three, one);
   res = nillessT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less nillessF (Forwards::Input::Token(), three, sev);
   res = nillessF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ nilgeqT (Forwards::Input::Token(), three, six);
   res = nilgeqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ nilgeqF (Forwards::Input::Token(), three, one);
   res = nilgeqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ nilleqT (Forwards::Input::Token(), three, one);
   res = nilleqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ nilleqF (Forwards::Input::Token(), three, sev);
   res = nilleqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Cat nilcat (Forwards::Input::Token(), three, one);
   res = nilcat.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("6", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);


      // fc?? Constant refuses to return a CellRefValue. If any these functions get a CellRefValue, that is probably a programming error.
   std::shared_ptr<Forwards::Engine::Constant> fa = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("Hi"));
   std::shared_ptr<Forwards::Engine::Constant> fc = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(1, 1, 1, 1, ""));

   Forwards::Engine::Plus plusFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Plus plusFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Plus Faplus (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Plus Fcplus (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(plusFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(plusFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Faplus.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcplus.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Minus minusFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Minus minusFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Minus Faminus (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Minus Fcminus (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(minusFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(minusFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Faminus.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcminus.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Multiply mulitplyFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Multiply mulitplyFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Multiply Famulitply (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Multiply Fcmulitply (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(mulitplyFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(mulitplyFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Famulitply.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcmulitply.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Divide divideFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Divide divideFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Divide Fadivide (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Divide Fcdivide (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(divideFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(divideFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fadivide.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcdivide.evaluate(context), Backwards::Types::TypedOperationException);

      // Cat with string is valid, to be handled by string. Why? Because this function is YUGE as it is.
   Forwards::Engine::Cat catFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Cat Fccat (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(catFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fccat.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Equals equalsFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Equals equalsFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Equals Faequals (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Equals Fcequals (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(equalsFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(equalsFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Faequals.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcequals.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::NotEqual notequalFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::NotEqual notequalFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::NotEqual Fanotequal (Forwards::Input::Token(), fa, one);
   Forwards::Engine::NotEqual Fcnotequal (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(notequalFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(notequalFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fanotequal.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcnotequal.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Greater greaterFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Greater greaterFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Greater Fagreater (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Greater Fcgreater (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(greaterFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(greaterFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fagreater.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgreater.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Less lessFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::Less lessFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Less Faless (Forwards::Input::Token(), fa, one);
   Forwards::Engine::Less Fcless (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(lessFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(lessFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Faless.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcless.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::GEQ geqFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::GEQ geqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::GEQ Fageq (Forwards::Input::Token(), fa, one);
   Forwards::Engine::GEQ Fcgeq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(geqFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(geqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fageq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgeq.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::LEQ leqFa (Forwards::Input::Token(), one, fa);
   Forwards::Engine::LEQ leqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::LEQ Faleq (Forwards::Input::Token(), fa, one);
   Forwards::Engine::LEQ Fcleq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(leqFa.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(leqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Faleq.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcleq.evaluate(context), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testStrings)
 {
   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("F"));
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("U"));
   std::shared_ptr<Forwards::Engine::Constant> six = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>(""));
   std::shared_ptr<Forwards::Engine::Constant> sev = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("A"));
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::Equals equalsT (Forwards::Input::Token(), one, one);
   res = equalsT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals equalsF (Forwards::Input::Token(), one, two);
   res = equalsF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualT (Forwards::Input::Token(), one, two);
   res = notEqualT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualF (Forwards::Input::Token(), one, one);
   res = notEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterT (Forwards::Input::Token(), two, one);
   res = greaterT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterF (Forwards::Input::Token(), one, two);
   res = greaterF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessT (Forwards::Input::Token(), one, two);
   res = lessT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessF (Forwards::Input::Token(), two, one);
   res = lessF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqT (Forwards::Input::Token(), two, one);
   res = geqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqF (Forwards::Input::Token(), one, two);
   res = geqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqT (Forwards::Input::Token(), one, two);
   res = leqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqF (Forwards::Input::Token(), two, one);
   res = leqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Cat cat (Forwards::Input::Token(), one, two);
   res = cat.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("FU", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value); // Nice


   std::shared_ptr<Forwards::Engine::Constant> three = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>());

   Forwards::Engine::Equals equalsTNil (Forwards::Input::Token(), six, three);
   res = equalsTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals equalsFNil (Forwards::Input::Token(), one, three);
   res = equalsFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualTNil (Forwards::Input::Token(), one, three);
   res = notEqualTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualFNil (Forwards::Input::Token(), six, three);
   res = notEqualFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterTNil (Forwards::Input::Token(), two, three);
   res = greaterTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterFNil (Forwards::Input::Token(), six, three);
   res = greaterFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

//   Forwards::Engine::Less lessTNil (Forwards::Input::Token(), sev, three);
//   res = lessTNil.evaluate(context);

//   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
//   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessFNil (Forwards::Input::Token(), two, three);
   res = lessFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqTNil (Forwards::Input::Token(), six, three);
   res = geqTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

//   Forwards::Engine::GEQ geqFNil (Forwards::Input::Token(), sev, three);
//   res = geqFNil.evaluate(context);

//   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
//   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqTNil (Forwards::Input::Token(), six, three);
   res = leqTNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqFNil (Forwards::Input::Token(), two, three);
   res = leqFNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Cat catNil (Forwards::Input::Token(), one, three);
   res = catNil.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("F", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);



   Forwards::Engine::Equals nilequalsT (Forwards::Input::Token(), three, six);
   res = nilequalsT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Equals nilequalsF (Forwards::Input::Token(), three, one);
   res = nilequalsF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual nilnotEqualT (Forwards::Input::Token(), three, one);
   res = nilnotEqualT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual nilnotEqualF (Forwards::Input::Token(), three, six);
   res = nilnotEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

//   Forwards::Engine::Greater nilgreaterT (Forwards::Input::Token(), three, sev);
//   res = nilgreaterT.evaluate(context);

//   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
//   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater nilgreaterF (Forwards::Input::Token(), three, six);
   res = nilgreaterF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less nillessT (Forwards::Input::Token(), three, one);
   res = nillessT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less nillessF (Forwards::Input::Token(), three, six);
   res = nillessF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ nilgeqT (Forwards::Input::Token(), three, six);
   res = nilgeqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ nilgeqF (Forwards::Input::Token(), three, one);
   res = nilgeqF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ nilleqT (Forwards::Input::Token(), three, one);
   res = nilleqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

//   Forwards::Engine::LEQ nilleqF (Forwards::Input::Token(), three, sev);
//   res = nilleqF.evaluate(context);

//   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
//   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Cat nilcat (Forwards::Input::Token(), three, one);
   res = nilcat.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("F", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);


   std::shared_ptr<Forwards::Engine::Constant> floatCat = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));

   Forwards::Engine::Cat catfs (Forwards::Input::Token(), one, floatCat);
   res = catfs.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("F6", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);

   Forwards::Engine::Cat catsf (Forwards::Input::Token(), floatCat, one);
   res = catsf.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("6F", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);



   std::shared_ptr<Forwards::Engine::Constant> fc = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(1, 1, 1, 1, ""));

      // Cat with string is valid, to be handled by string. Why? Because this function is YUGE as it is.
   Forwards::Engine::Cat catFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Cat Fccat (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(catFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fccat.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Equals equalsFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Equals Fcequals (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(equalsFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcequals.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::NotEqual notequalFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::NotEqual Fcnotequal (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(notequalFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcnotequal.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Greater greaterFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Greater Fcgreater (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(greaterFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgreater.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Less lessFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Less Fcless (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(lessFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcless.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::GEQ geqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::GEQ Fcgeq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(geqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgeq.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::LEQ leqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::LEQ Fcleq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(leqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcleq.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Negate negone (Forwards::Input::Token(), one);
   EXPECT_THROW(negone.evaluate(context), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testOtherNils)
 {
   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>());
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>());
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::Plus plus (Forwards::Input::Token(), one, two);
   res = plus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   Forwards::Engine::Minus minus (Forwards::Input::Token(), one, two);
   res = minus.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   Forwards::Engine::Multiply multiply (Forwards::Input::Token(), one, two);
   res = multiply.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   Forwards::Engine::Divide divide (Forwards::Input::Token(), one, two);
   res = divide.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   Forwards::Engine::Equals equalsT (Forwards::Input::Token(), one, one);
   res = equalsT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::NotEqual notEqualF (Forwards::Input::Token(), one, one);
   res = notEqualF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Greater greaterF (Forwards::Input::Token(), one, two);
   res = greaterF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Less lessF (Forwards::Input::Token(), two, one);
   res = lessF.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("0"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::GEQ geqT (Forwards::Input::Token(), two, one);
   res = geqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::LEQ leqT (Forwards::Input::Token(), one, two);
   res = leqT.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   Forwards::Engine::Negate neg (Forwards::Input::Token(), one);
   res = neg.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   Forwards::Engine::Cat cat (Forwards::Input::Token(), one, two);
   res = cat.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));


   std::shared_ptr<Forwards::Engine::Constant> fc = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(1, 1, 1, 1, ""));

   Forwards::Engine::Plus plusFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Plus Fcplus (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(plusFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcplus.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Minus minusFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Minus Fcminus (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(minusFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcminus.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Multiply mulitplyFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Multiply Fcmulitply (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(mulitplyFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcmulitply.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Divide divideFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Divide Fcdivide (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(divideFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcdivide.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Cat catFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Cat Fccat (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(catFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fccat.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Equals equalsFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Equals Fcequals (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(equalsFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcequals.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::NotEqual notequalFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::NotEqual Fcnotequal (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(notequalFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcnotequal.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Greater greaterFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Greater Fcgreater (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(greaterFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgreater.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Less lessFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::Less Fcless (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(lessFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcless.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::GEQ geqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::GEQ Fcgeq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(geqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcgeq.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::LEQ leqFc (Forwards::Input::Token(), one, fc);
   Forwards::Engine::LEQ Fcleq (Forwards::Input::Token(), fc, one);

   EXPECT_THROW(leqFc.evaluate(context), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Fcleq.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::Negate negfc (Forwards::Input::Token(), fc);
   EXPECT_THROW(negfc.evaluate(context), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testVariousCellRanges)
 {
   std::shared_ptr<Forwards::Engine::Constant> A1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 0, ""));
   std::shared_ptr<Forwards::Engine::Constant> A2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, true, 0, ""));
   std::shared_ptr<Forwards::Engine::Constant> A3 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 0, false, 0, ""));
   std::shared_ptr<Forwards::Engine::Constant> A4 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 0, true, 0, ""));

   std::shared_ptr<Forwards::Engine::Constant> B1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, false, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> B2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, true, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> B3 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, false, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> B4 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, true, 1, ""));

   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::CellFrame frame (nullptr, 0U, 0U);
   context.pushCell(&frame);

   Forwards::Engine::MakeRange range1 (Forwards::Input::Token(), A1, B1);
   res = range1.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col1);
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row1);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col2);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row2);
   EXPECT_EQ("A0:B1", range1.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range2 (Forwards::Input::Token(), A2, B1);
   res = range2.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("A$0:B1", range2.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range3 (Forwards::Input::Token(), A3, B1);
   res = range3.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("$A0:B1", range3.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range4 (Forwards::Input::Token(), A4, B1);
   res = range4.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("$A$0:B1", range4.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range5 (Forwards::Input::Token(), A1, B2);
   res = range5.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("A0:B$1", range5.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range6 (Forwards::Input::Token(), A1, B3);
   res = range6.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("A0:$B1", range6.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range7 (Forwards::Input::Token(), A1, B4);
   res = range7.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("A0:$B$1", range7.toString(0U, 0U, 0));

   Forwards::Engine::MakeRange range8 (Forwards::Input::Token(), B1, A1);
   res = range8.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col1);
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row1);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col2);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row2);
   EXPECT_EQ("B1:A0", range8.toString(0U, 0U, 0));

   std::shared_ptr<Forwards::Engine::Constant> F1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, -1, true, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> F2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, false, -1, ""));

   Forwards::Engine::MakeRange rangeF1 (Forwards::Input::Token(), F1, B1);
   EXPECT_THROW(rangeF1.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::MakeRange rangeF2 (Forwards::Input::Token(), F2, B1);
   EXPECT_THROW(rangeF2.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::MakeRange rangeF3 (Forwards::Input::Token(), B1, F1);
   EXPECT_THROW(rangeF3.evaluate(context), Backwards::Types::TypedOperationException);

   Forwards::Engine::MakeRange rangeF4 (Forwards::Input::Token(), B1, F2);
   EXPECT_THROW(rangeF4.evaluate(context), Backwards::Types::TypedOperationException);

   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));

   Forwards::Engine::MakeRange rangeF5 (Forwards::Input::Token(), one, B1);
   EXPECT_THROW(rangeF5.evaluate(context), Backwards::Engine::ProgrammingException);

   Forwards::Engine::MakeRange rangeF6 (Forwards::Input::Token(), B1, one);
   EXPECT_THROW(rangeF6.evaluate(context), Backwards::Engine::ProgrammingException);

   std::shared_ptr<Forwards::Engine::Plus> plus = std::make_shared<Forwards::Engine::Plus>(Forwards::Input::Token(), one, B1);

   Forwards::Engine::MakeRange rangeF7 (Forwards::Input::Token(), plus, B1);
   EXPECT_THROW(rangeF7.evaluate(context), Backwards::Engine::ProgrammingException);

   Forwards::Engine::MakeRange rangeF8 (Forwards::Input::Token(), B1, plus);
   EXPECT_THROW(rangeF8.evaluate(context), Backwards::Engine::ProgrammingException);
 }

TEST(EngineTests, testParens)
 {
   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("9"));

   std::shared_ptr<Forwards::Engine::Plus> plus = std::make_shared<Forwards::Engine::Plus>(Forwards::Input::Token(), one, two);
   std::shared_ptr<Forwards::Engine::Multiply> mult = std::make_shared<Forwards::Engine::Multiply>(Forwards::Input::Token(), one, two);

   Forwards::Engine::Multiply multiply (Forwards::Input::Token(), one, plus);
   EXPECT_EQ("6*(6+9)", multiply.toString(1U, 1U, 0));

   Forwards::Engine::Minus minus (Forwards::Input::Token(), one, plus);
   EXPECT_EQ("6-(6+9)", minus.toString(1U, 1U, 0));

   Forwards::Engine::Minus minus2 (Forwards::Input::Token(), one, mult);
   EXPECT_EQ("6-6*9", minus2.toString(1U, 1U, 0));
 }

TEST(EngineTests, testFinalConst)
 {
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::SpreadSheet shet;
   context.theSheet = &shet;
   Forwards::Engine::MemorySpreadSheet backing;
   shet.currentSheet = &backing;

   context.inUserInput = true;

   shet.initCellAt(0U, 2U);
   shet.initCellAt(1U, 2U);
   shet.initCellAt(2U, 2U);

   shet.initCellAt(0U, 0U);
   shet.initCellAt(1U, 1U);
   shet.getCellAt(1U, 1U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));

   Forwards::Engine::CellFrame frame (shet.getCellAt(0U, 0U, ""), 0U, 0U);
   context.pushCell(&frame);

   std::shared_ptr<Forwards::Engine::Constant> A1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, false, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> A2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, true, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> A3 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, false, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> A4 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, true, 1, ""));

   res = A1->evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   res = A2->evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   res = A3->evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   res = A4->evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   std::shared_ptr<Forwards::Engine::Constant> F1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, -1, false, 0, ""));
   EXPECT_NO_THROW(F1->evaluate(context));

   std::shared_ptr<Forwards::Engine::Constant> F2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, -1, ""));
   EXPECT_NO_THROW(F2->evaluate(context));

   std::shared_ptr<Forwards::Engine::Constant> B1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 4, false, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> B2 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, false, 4, ""));
   std::shared_ptr<Forwards::Engine::Constant> B3 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 2, false, 2, ""));
   std::shared_ptr<Forwards::Engine::Constant> B4 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 0, ""));

   res = B1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   res = B2->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   res = B3->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   res = B4->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   shet.getCellAt(1U, 1U, "")->previousValue.reset();
   shet.getCellAt(1U, 1U, "")->inEvaluation = true;
   res = A1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   shet.getCellAt(1U, 1U, "")->previousValue = makeFloatValue("9");
   res = A1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("9"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   shet.getCellAt(1U, 1U, "")->inEvaluation = false;
   shet.getCellAt(1U, 1U, "")->previousGeneration = context.generation;
   res = A1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("9"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   context.inUserInput = false;
   ++context.generation;
   res = A1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

      // Ensure that a cell that references a cell doesn't return a cell reference.
   shet.initCellAt(0U, 2U);
   shet.getCellAt(0U, 2U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, true, 1, ""));
   std::shared_ptr<Forwards::Engine::Constant> A9 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 0, true, 2, ""));
   res = A9->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("U"));
   std::shared_ptr<Forwards::Engine::Plus> plus = std::make_shared<Forwards::Engine::Plus>(Forwards::Input::Token(), one, two);
   shet.initCellAt(1U, 2U);
   shet.getCellAt(1U, 2U, "")->value = plus;
   std::shared_ptr<Forwards::Engine::Constant> F9 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, true, 2, ""));
   EXPECT_THROW(F9->evaluate(context), Backwards::Types::TypedOperationException);
 }

TEST(EngineTests, testFunctionsAndRanges)
 {
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::SpreadSheet shet;
   context.theSheet = &shet;
   Forwards::Engine::MemorySpreadSheet backing;
   shet.currentSheet = &backing;

   shet.initCellAt(0U, 0U);
   shet.initCellAt(0U, 1U);
   shet.initCellAt(0U, 2U);
   shet.initCellAt(1U, 0U);
   shet.initCellAt(1U, 1U);
   shet.initCellAt(1U, 2U);
   shet.initCellAt(2U, 0U);
   shet.initCellAt(2U, 1U);
   shet.initCellAt(2U, 2U);
   shet.getCellAt(0U, 0U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("1"));
   shet.getCellAt(0U, 1U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("2"));
   shet.getCellAt(0U, 2U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("3"));
   shet.getCellAt(1U, 0U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("4"));
   shet.getCellAt(1U, 1U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("5"));
   shet.getCellAt(1U, 2U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   shet.getCellAt(2U, 0U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("7"));
   shet.getCellAt(2U, 1U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("8"));
   shet.getCellAt(2U, 2U, "")->value = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("9"));

   Forwards::Engine::CellFrame frame (shet.getCellAt(0U, 0U, ""), 0U, 0U);
   EXPECT_EQ(nullptr, context.topCell());
   context.pushCell(&frame);


   Forwards::Engine::GetterMap map;
   //context.map = &map;

   Backwards::Engine::Scope global;
   context.globalScope = &global;
   Forwards::Parser::ContextBuilder::createGlobalScope(global); // Create the global scope before the table.
   Backwards::Parser::GetterSetter gs;
   Backwards::Parser::SymbolTable table (gs, global);
   Backwards::Input::FileInput console ("../Tests/StdLib.txt");
   Backwards::Input::Lexer lexer (console, "StdLib.txt");

   std::shared_ptr<Backwards::Engine::Statement> stdLib = Backwards::Parser::Parser::ParseFunctions(lexer, table, logger);
   stdLib->execute(context);

   Backwards::Input::StringInput willynilly ("set ARG to function (x) is return x[0] end set BARG to function (x) is return x end");
   Backwards::Input::Lexer lexer2 (willynilly, "NewProgram");

   std::shared_ptr<Backwards::Engine::Statement> stdLib2 = Backwards::Parser::Parser::ParseFunctions(lexer2, table, logger);
   stdLib2->execute(context);

   for (const std::string& name : global.names)
    {
      std::string temp = name;
      std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c){ return std::toupper(c); });
      if (name == temp)
       {
         map.insert(std::make_pair(name, table.getVariableGetter(name)));
       }
    }

   ASSERT_TRUE(map.end() != map.find("SUM"));
   ASSERT_TRUE(map.end() != map.find("ARG"));
   ASSERT_TRUE(map.end() != map.find("BARG"));

   std::vector<std::shared_ptr<Forwards::Engine::Expression> > args;
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(0, 0, 1, 1, "")));
   std::shared_ptr<Forwards::Engine::FunctionCall> fun1 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "SUM", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["SUM"]),
      args);

   EXPECT_EQ("@SUM(A0:B1)", fun1->toString(0U, 0U, 5));

   res = fun1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("12"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(0, 0, 1, 0, "")));
   std::shared_ptr<Forwards::Engine::FunctionCall> fun2 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "SUM", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["SUM"]),
      args);

   EXPECT_EQ("@SUM(A0:B1;A0:B0)", fun2->toString(0U, 0U, 5));

   res = fun2->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("17"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);

   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(0, 0, 0, 0, "")));
   std::shared_ptr<Forwards::Engine::FunctionCall> fun3 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "SUM", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["SUM"]),
      args);

   EXPECT_EQ("@SUM(A0:A0)", fun3->toString(0U, 0U, 5));

   res = fun3->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("1"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);


   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("Hi")));
   std::shared_ptr<Forwards::Engine::FunctionCall> funA1 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "ARG", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["ARG"]),
      args);

   res = funA1->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::StringValue) == typeid(*res.get()));
   EXPECT_EQ("Hi", std::dynamic_pointer_cast<Forwards::Types::StringValue>(res)->value);

   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>()));
   std::shared_ptr<Forwards::Engine::FunctionCall> funA2 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "ARG", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["ARG"]),
      args);

   res = funA2->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));

   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRangeValue>(0, 1, 1, 2, "")));
   std::shared_ptr<Forwards::Engine::FunctionCall> funA3 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "ARG", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["ARG"]),
      args);

   res = funA3->evaluate(context);
   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col1);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row1);
   EXPECT_EQ(1U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->col2);
   EXPECT_EQ(2U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(res)->row2);

   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::NilValue>()));
   std::shared_ptr<Forwards::Engine::FunctionCall> funB1 = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "BARG", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["BARG"]),
      args);

   EXPECT_THROW(funB1->evaluate(context), Backwards::Engine::ProgrammingException);

   std::shared_ptr<Backwards::Engine::CallingContext> copied = context.duplicate();
   std::shared_ptr<Forwards::Engine::CallingContext> casted = std::dynamic_pointer_cast<Forwards::Engine::CallingContext>(copied);
   ASSERT_TRUE(nullptr != casted.get());
   EXPECT_EQ(context.topCell(), casted->topCell());
   EXPECT_EQ(context.theSheet, casted->theSheet);
   EXPECT_EQ(context.logger, casted->logger);


   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Forwards::Engine::Constant> two = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("9"));
   args.clear();
   args.emplace_back(std::make_shared<Forwards::Engine::Plus>(Forwards::Input::Token(), one, two));
   std::shared_ptr<Forwards::Engine::FunctionCall> funTestParens = std::make_shared<Forwards::Engine::FunctionCall>(
      Forwards::Input::Token(Forwards::Input::IDENTIFIER, "ARG", 1U),
      std::make_shared<Backwards::Engine::Variable>(Backwards::Input::Token(), map["ARG"]),
      args);
   EXPECT_EQ("@ARG(6+9)", funTestParens->toString(0U, 0U, 5));
 }

TEST(EngineTests, testCellRangeExpand)
 {
   Backwards::Types::CellRangeValue defaulted (std::make_shared<Forwards::Engine::CellRangeExpand>());
   Backwards::Types::CellRangeValue low (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 1U, 1U, "")));
   Backwards::Types::CellRangeValue high (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(2U, 3U, 4U, 5U, "")));

   EXPECT_FALSE(low.equal(high));
   EXPECT_TRUE(low.notEqual(high));
   EXPECT_FALSE(low.sort(high));
   EXPECT_TRUE(high.sort(low));
   EXPECT_FALSE(low.sort(low));

   EXPECT_NE(0U, low.hash());
   EXPECT_NE(0U, high.hash());

      // All operations on defaulted will core.
   //EXPECT_THROW(low.equal(defaulted), Backwards::Engine::ProgrammingException);
   //EXPECT_THROW(low.sort(defaulted), Backwards::Engine::ProgrammingException);

   Backwards::Types::CellRangeValue smallest (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 0U, 0U, "")));
   Backwards::Types::CellRangeValue med (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(3U, 5U, 7U, 9U, "")));
   Backwards::Types::CellRangeValue also1 (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(3U, 4U, 3U, 9U, "")));
   Backwards::Types::CellRangeValue also2 (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(3U, 4U, 7U, 4U, "")));

   EXPECT_EQ(1U, smallest.getSize());
   EXPECT_EQ(2U, low.getSize());
   EXPECT_EQ(6U, also1.getSize());
   EXPECT_EQ(5U, also2.getSize());

   std::shared_ptr<Backwards::Types::ValueType> res;
   std::shared_ptr<Forwards::Engine::CellRefEval> temp1;
   std::shared_ptr<Forwards::Engine::CellRangeExpand> temp2;
   std::shared_ptr<Forwards::Types::ValueType> ras;

   res = smallest.getIndex(0U);
   ASSERT_TRUE(typeid(Backwards::Types::CellRefValue) == typeid(*res.get()));
   ASSERT_TRUE(typeid(Forwards::Engine::CellRefEval) == typeid(*std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value.get()));
   temp1 = std::dynamic_pointer_cast<Forwards::Engine::CellRefEval>(std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value);
   ASSERT_TRUE(typeid(Forwards::Engine::Constant) == typeid(*temp1->value.get()));
   ASSERT_TRUE(typeid(Forwards::Types::CellRefValue) == typeid(*std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value.get()));
   ras = std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value;
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colAbsolute);
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colRef);
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowAbsolute);
   EXPECT_EQ(0U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowRef);

   res = med.getIndex(1U);
   ASSERT_TRUE(typeid(Backwards::Types::CellRangeValue) == typeid(*res.get()));
   ASSERT_TRUE(typeid(Forwards::Engine::CellRangeExpand) == typeid(*std::dynamic_pointer_cast<Backwards::Types::CellRangeValue>(res)->value.get()));
   temp2 = std::dynamic_pointer_cast<Forwards::Engine::CellRangeExpand>(std::dynamic_pointer_cast<Backwards::Types::CellRangeValue>(res)->value);
   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*temp2->value.get()));
   ras = temp2->value;
   EXPECT_EQ(4U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(ras)->col1);
   EXPECT_EQ(5U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(ras)->row1);
   EXPECT_EQ(4U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(ras)->col2);
   EXPECT_EQ(9U, std::dynamic_pointer_cast<Forwards::Types::CellRangeValue>(ras)->row2);

   res = also1.getIndex(2U);
   ASSERT_TRUE(typeid(Backwards::Types::CellRefValue) == typeid(*res.get()));
   ASSERT_TRUE(typeid(Forwards::Engine::CellRefEval) == typeid(*std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value.get()));
   temp1 = std::dynamic_pointer_cast<Forwards::Engine::CellRefEval>(std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value);
   ASSERT_TRUE(typeid(Forwards::Engine::Constant) == typeid(*temp1->value.get()));
   ASSERT_TRUE(typeid(Forwards::Types::CellRefValue) == typeid(*std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value.get()));
   ras = std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value;
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colAbsolute);
   EXPECT_EQ(3U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colRef);
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowAbsolute);
   EXPECT_EQ(6U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowRef);

   res = also2.getIndex(2U);
   ASSERT_TRUE(typeid(Backwards::Types::CellRefValue) == typeid(*res.get()));
   ASSERT_TRUE(typeid(Forwards::Engine::CellRefEval) == typeid(*std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value.get()));
   temp1 = std::dynamic_pointer_cast<Forwards::Engine::CellRefEval>(std::dynamic_pointer_cast<Backwards::Types::CellRefValue>(res)->value);
   ASSERT_TRUE(typeid(Forwards::Engine::Constant) == typeid(*temp1->value.get()));
   ASSERT_TRUE(typeid(Forwards::Types::CellRefValue) == typeid(*std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value.get()));
   ras = std::dynamic_pointer_cast<Forwards::Engine::Constant>(temp1->value)->value;
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colAbsolute);
   EXPECT_EQ(5U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->colRef);
   EXPECT_EQ(true, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowAbsolute);
   EXPECT_EQ(4U, std::dynamic_pointer_cast<Forwards::Types::CellRefValue>(ras)->rowRef);

   EXPECT_THROW(smallest.getIndex(2U), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(med.getIndex(9U), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(also1.getIndex(8U), Backwards::Engine::ProgrammingException);
   EXPECT_THROW(also2.getIndex(7U), Backwards::Engine::ProgrammingException);
 }

TEST(EngineTests, testCellRefEval)
 {
   Backwards::Types::CellRefValue defaulted (std::make_shared<Forwards::Engine::CellRefEval>());
   Backwards::Types::CellRefValue low (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 1, ""))));
   Backwards::Types::CellRefValue high (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 1, true, 2, ""))));
   Backwards::Types::CellRefValue med (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::StringValue>("Hi"))));

   EXPECT_FALSE(low.equal(high));
   EXPECT_TRUE(low.notEqual(high));
   EXPECT_FALSE(low.equal(med));
   EXPECT_TRUE(low.notEqual(med));
   EXPECT_TRUE(low.sort(high) | high.sort(low));
   EXPECT_TRUE(low.sort(med) | med.sort(low));
   EXPECT_FALSE(low.sort(low));

   EXPECT_NE(0U, low.hash());
   EXPECT_NE(0U, high.hash());
   EXPECT_NE(0U, med.hash());

      // All operations on defaulted will core.
   //EXPECT_THROW(low.equal(defaulted), Backwards::Engine::ProgrammingException);
   //EXPECT_THROW(low.sort(defaulted), Backwards::Engine::ProgrammingException);
 }

TEST(EngineTests, testCellRefEval_EqualCases)
 {
   Backwards::Types::CellRefValue un (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 0, ""))));
   Backwards::Types::CellRefValue deux (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(true, 0, false, 0, ""))));
   Backwards::Types::CellRefValue trois (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, true, 0, ""))));
   Backwards::Types::CellRefValue quatre (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, false, 0, ""))));
   Backwards::Types::CellRefValue cinq (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 1, ""))));
   Backwards::Types::CellRefValue six (std::make_shared<Forwards::Engine::CellRefEval>(
      std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 0, ""))));

   EXPECT_FALSE(un.equal(deux));
   EXPECT_FALSE(un.equal(trois));
   EXPECT_FALSE(un.equal(quatre));
   EXPECT_FALSE(un.equal(cinq));
   EXPECT_TRUE(un.equal(six));

   EXPECT_TRUE(un.sort(deux) | deux.sort(un));
   EXPECT_TRUE(un.sort(trois) | trois.sort(un));
   EXPECT_TRUE(un.sort(quatre) | quatre.sort(un));
   EXPECT_TRUE(un.sort(cinq) | cinq.sort(un));
   EXPECT_FALSE(un.sort(six) | six.sort(un));
 }

TEST(EngineTests, testCellRangeExpand_EqualCases)
 {
   Backwards::Types::CellRangeValue un (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 1U, 1U, "")));
   Backwards::Types::CellRangeValue deux (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(1U, 0U, 1U, 1U, "")));
   Backwards::Types::CellRangeValue trois (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 1U, 1U, 1U, "")));
   Backwards::Types::CellRangeValue quatre (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 2U, 1U, "")));
   Backwards::Types::CellRangeValue cinq (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 1U, 2U, "")));
   Backwards::Types::CellRangeValue six (std::make_shared<Forwards::Engine::CellRangeExpand>(std::make_shared<Forwards::Types::CellRangeValue>(0U, 0U, 1U, 1U, "")));

   EXPECT_FALSE(un.equal(deux));
   EXPECT_FALSE(un.equal(trois));
   EXPECT_FALSE(un.equal(quatre));
   EXPECT_FALSE(un.equal(cinq));
   EXPECT_TRUE(un.equal(six));

   EXPECT_TRUE(un.sort(deux) | deux.sort(un));
   EXPECT_TRUE(un.sort(trois) | trois.sort(un));
   EXPECT_TRUE(un.sort(quatre) | quatre.sort(un));
   EXPECT_TRUE(un.sort(cinq) | cinq.sort(un));
   EXPECT_FALSE(un.sort(six) | six.sort(un));
 }

TEST(EngineTests, testCellEval)
 {
   std::shared_ptr<Backwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::SpreadSheet shet;
   context.theSheet = &shet;
   Forwards::Engine::MemorySpreadSheet backing;
   shet.currentSheet = &backing;

   shet.initCellAt(0U, 0U);

   Forwards::Engine::CellFrame frame (shet.getCellAt(0U, 0U, ""), 0U, 0U);
   EXPECT_EQ(nullptr, context.topCell());
   context.pushCell(&frame);

   Forwards::Engine::GetterMap map;
   context.map = &map;

   res = Forwards::Engine::CellEval(context, std::make_shared<Backwards::Types::StringValue>("2 + 3"));

   ASSERT_TRUE(typeid(Backwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("5"), *std::dynamic_pointer_cast<Backwards::Types::FloatValue>(res)->value);

   EXPECT_THROW(Forwards::Engine::CellEval(context, std::make_shared<Backwards::Types::FloatValue>(NumberSystem::getCurrentNumberSystem().fromString("23"))), Backwards::Types::TypedOperationException);
   EXPECT_THROW(Forwards::Engine::CellEval(context, std::make_shared<Backwards::Types::StringValue>("Hello")), Backwards::Types::TypedOperationException);

   Backwards::Engine::CallingContext badContext;
   EXPECT_THROW(Forwards::Engine::CellEval(badContext, std::make_shared<Backwards::Types::StringValue>("2 + 3")), Backwards::Engine::ProgrammingException);
 }

TEST(EngineTests, testName)
 {
   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;
   Forwards::Engine::NameMap names;
   context.names = &names;
   names.insert(std::make_pair("Billy", one));

   Forwards::Engine::Name name (Forwards::Input::Token(Forwards::Input::NAME, "Billy", 0U));
   res = name.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::FloatValue) == typeid(*res.get()));
   EXPECT_EQ(*NumberSystem::getCurrentNumberSystem().fromString("6"), *std::dynamic_pointer_cast<Forwards::Types::FloatValue>(res)->value);
   EXPECT_EQ("_Billy", name.toString(1U, 1U, 0));


   Forwards::Engine::Name nameBad (Forwards::Input::Token(Forwards::Input::NAME, "Johnny", 0U));
   res = nameBad.evaluate(context);

   EXPECT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));
   EXPECT_EQ("_Johnny", nameBad.toString(1U, 1U, 0));
 }

TEST(EngineTests, testMOVE)
 {
   std::shared_ptr<Forwards::Engine::Constant> A1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 0, false, 0, ""));
   std::shared_ptr<Forwards::Engine::Constant> B1 = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), std::make_shared<Forwards::Types::CellRefValue>(false, 1, false, 1, ""));

   std::shared_ptr<Forwards::Types::ValueType> res;
   Forwards::Engine::CallingContext context;
   StringLogger logger;
   context.logger = &logger;

   Forwards::Engine::SpreadSheet shet;
   context.theSheet = &shet;
   Forwards::Engine::MemorySpreadSheet backing;
   shet.currentSheet = &backing;

   Forwards::Engine::CellFrame frame (nullptr, 0U, 0U);
   context.pushCell(&frame);

   std::shared_ptr<Forwards::Engine::MakeRange> range = std::make_shared<Forwards::Engine::MakeRange>(Forwards::Input::Token(), A1, B1);
   Forwards::Engine::MoveReference bob (Forwards::Input::Token(Forwards::Input::NAME, "bob", 0U), range);
   res = bob.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::CellRangeValue) == typeid(*res.get()));
   EXPECT_EQ("A0:B1!bob", bob.toString(0U, 0U, 0));
   EXPECT_EQ("A0:B1!bob", res->toString(0U, 0U, false));

   Forwards::Engine::MoveReference sam (Forwards::Input::Token(Forwards::Input::NAME, "sam", 0U), A1);
   res = sam.evaluate(context);

   ASSERT_TRUE(typeid(Forwards::Types::NilValue) == typeid(*res.get()));
   EXPECT_EQ("A0!sam", sam.toString(0U, 0U, 0));
      // Had to make MoveReference dereference the cell.
   // EXPECT_EQ("A0!sam", res->toString(0U, 0U, false));

   std::shared_ptr<Forwards::Engine::Constant> one = std::make_shared<Forwards::Engine::Constant>(Forwards::Input::Token(), makeFloatValue("6"));
   Forwards::Engine::MoveReference bad (Forwards::Input::Token(Forwards::Input::NAME, "bad", 0U), one);
   EXPECT_THROW(bad.evaluate(context), Backwards::Types::TypedOperationException);
 }
