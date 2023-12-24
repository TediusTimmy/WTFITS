WTFITS
======
Pronounced ootfits, like "outfit", but Dutch.


Spreadsheet Interface For SQLite Databases
------------------------------------------
The only other curses-based interface I know of for SQLite databases is VisiData. This program attempts to put a spreadsheet-style interface to an SQLite database: it tries to be easier to use than the sqlite3 console application for getting information about or from a database. Note that it is not a tool for adding data to a database: it is read-only and is about processing the data already in the database.


Invocation
----------
` WTFITS [-0 | -1 | -2 | -3 | -4 | -5] { -l <library name> } { -b <batch string> } [ <working sheet> [ <db to analyze> ] ] `

The first argument is the number system to use. There are currently six implemented:
* `-0` BC number-like system used in BC-DeciCalc
* `-1` Decimal floating point used in DeciCalc
* `-2` The SlowFloat decimal floating point system used in the Backrooms engine
* `-3` Normal machine double
* `-4` libmpdec used by Python for decimal floating point
* `-5` mpfr

Note for libraries: when you load a library, it gets stored in the working sheet. You only need to load them once.


Database-backed working sheet
-----------------------------
The working sheet is an sqlite database. Every change is immediately commit to the database. The new bottom-right cell is `ZZZZ999999999999`, and I don't recommend using the high-number cells. Also note that the cell table is one table: SQLite advertises that it can handle, maybe, 2 trillion (2e13) rows in a table. Given that you can put one trillion cells in a column: you will run out of backing cells long before you fill up the working table.


Number System notes
-------------------
Half of these (`-1`, `-2`, and `-3`) are fixed precision: changing the working precision has no effect. Most of these lack some of the rounding modes provided by the BC number-like system: `-1` and `-2` lack the double rounding mode; `-4` lacks ties-to-odd; and `-3` and `-5` only have ties-to-even, to-positive-infinity, to-negative-infinity, and to-zero (and `-5` has away-from-zero). Also also, be prepared for strange results when you change the rounding mode in `-3`: most C runtimes are broken when you change the machine rounding mode.

Manual
------
See [Le Manuel](Manuel.md).
