#!/bin/sh -x

rm -f SlowFloat.o

rm -f SlowFloat.gcda
rm -f SlowFloat.gcno

rm -f SlowFloat_Base.info
rm -f SlowFloat_Run.info
rm -f SlowFloat.info

rm -f SlowFloatTest-SlowFloatTest.gcda
rm -f SlowFloatTest-SlowFloatTest.gcno

rm -f SlowFloatTest.exe

rm -rf cov

if [ "$1" == "clean" ]; then
   exit
fi

g++ -Wall -Wextra -Wpedantic -g --coverage -O0 -c SlowFloat.cpp

g++ -o SlowFloatTest -Wall -Wextra -Wpedantic -g --coverage -O0 -I../Backwards/External/googletest/include SlowFloatTest.cpp SlowFloat.o ../Backwards/External/googletest/lib/libgtest.a ../Backwards/External/googletest/lib/libgtest_main.a
../Backwards/External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --initial --directory . --output-file SlowFloat_Base.info
./SlowFloatTest.exe
../Backwards/External/lcov/bin/lcov --rc lcov_branch_coverage=1 --no-external --capture --directory . --output-file SlowFloat_Run.info
../Backwards/External/lcov/bin/lcov --rc lcov_branch_coverage=1 --add-tracefile SlowFloat_Base.info --add-tracefile SlowFloat_Run.info --output-file SlowFloat.info
mkdir cov
../Backwards/External/lcov/bin/genhtml --rc lcov_branch_coverage=1 --branch-coverage --output-directory cov SlowFloat.info
