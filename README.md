Rapidcsv
========

| **Linux + Mac** | **Windows** |
|-----------------|-------------|
| [![Build status](https://travis-ci.org/d99kris/rapidcsv.svg?branch=master)](https://travis-ci.org/d99kris/rapidcsv) | [![Build status](https://ci.appveyor.com/api/projects/status/yyc65as5ln6m6i8l/branch/master?svg=true)](https://ci.appveyor.com/project/d99kris/rapidcsv/branch/master) |

Rapidcsv is a simple C++ header-only library for CSV parsing. While the name admittedly
was inspired by the rapidjson project, the objectives are not the same. The goal of
rapidcsv is to be an easy-to-use CSV library enabling rapid development. For optimal
performance (be it CPU or memory usage) a CSV parser implemented for the specific
use-case is likely to be more performant.

Example Usage
=============
Here is a simple example reading a CSV file and getting 'Close' column as a vector of
floats.

```cpp
    #include <iostream>
    #include <vector>
    #include <rapidcsv.h>

    int main()
    {
      rapidcsv::Document doc("../tests/msft.csv");

      std::vector<float> close = doc.GetColumn<float>("Close");
      std::cout << "Read " << close.size() << " values." << std::endl;

      // ...
    }
```

The below section *More Examples* contains more examples(!), and additionally the
[tests](https://github.com/d99kris/rapidcsv/tree/master/tests) directory contains many simple
usage examples as well.

Supported Platforms
===================
Rapidcsv is implemented using C++11 with the intention of being portable. It's been tested on:
- macOS High Sierra
- Ubuntu 18.04
- Windows 10

Installation
============
Simply copy [src/rapidcsv.h](https://raw.githubusercontent.com/d99kris/rapidcsv/master/src/rapidcsv.h) to your project/include directory and include it. 

More Examples
=============

Reading a File with Custom Separator
------------------------------------
For reading of files with custom separator (i.e. not comma), one need to specify the
SeparatorParams argument. The following line reads file.csv using semi-colon as separator.

```cpp
    rapidcsv::Document doc("file.csv", rapidcsv::LabelParams(), rapidcsv::SeparatorParams(';'));
```

Reading a File without Column Labels
------------------------------------
By default rapidcsv treats the first row as column headers, allowing accessing columns
using their labels, for example GetColumn<float>("Close") to get the column labelled
"Close". Sometimes one may prefer to be able to access the entire document as data,
and specify cells by their row and column index. In order to do so one need to use
LabelParams and set pColumnNameIdx and pRowNameIdx to -1 (disabled). Example:

```cpp
    rapidcsv::Document doc("file.csv", rapidcsv::LabelParams(-1, -1));

    std::vector<float> close = doc.GetColumn<float>(4);
```

Reading a File with Column and Row Labels
-----------------------------------------
In order to treat the first row as column labels, and the first item on each row as row
labels, one need to use LabelParams and set pColumnNameIdx and pRowNameIdx to 0.
Example:

```cpp
    rapidcsv::Document doc("file.csv", rapidcsv::LabelParams(0, 0));

    long long volume = doc.GetCell<long long>("Volume", "2011-03-09");
```

Reading a File with Invalid Numbers (e.g. Empty Cells) as Numeric Data
-----------------------------------------------------------------------
By default rapidcsv throws an exception if one tries to access non-numeric data as a
numeric datatype, as it basically propagates the underlying conversion routines'
exceptions to the calling application. The reason for this is to ensure data
correctness. If one wants to be able to read data with invalid numbers as numeric
datatypes, one can use ConverterParams to configure the converter to default to a numeric
value. The value is configurable, but by default it's
std::numeric_limits<long double>::signaling_NaN() for float types, and 0 for integer
types. Example:

```cpp
    rapidcsv::Document doc("file.csv", rapidcsv::LabelParams(), rapidcsv::SeparatorParams(), rapidcsv::ConverterParams(true));
```

Check if a Column Exists
------------------------
Rapidcsv provides the methods GetColumnNames() and GetRowNames() to retrieve the column
and row names. To check whether a particular column name exists one can for example do:

```cpp
    rapidcsv::Document doc("file.csv");
    std::vector<std::string> columnNames = doc.GetColumnNames();
    bool column_A_exists = (std::find(columnNames.begin(), columnNames.end(), "A") != columnNames.end());
```

Custom Data Conversion
----------------------
The internal cell representation in the Document class is using std::string and when
other types are requested, standard conversion routines are used. One may override
conversion routines (or add new ones) by implementing ToVal() and ToStr(). Here is an
example overriding int conversion, to instead provide two decimal fixed-point numbers.
See [tests/test035.cpp](https://github.com/d99kris/rapidcsv/blob/master/tests/test035.cpp)
for a complete program example.

```cpp
    namespace rapidcsv
    {
      template<>
      void Converter<int>::ToVal(const std::string& pStr, int& pVal) const
      {
        pVal = roundf(100.0 * stof(pStr));
      }
    
      template<>
      void Converter<int>::ToStr(const int& pVal, std::string& pStr) const
      {
        std::ostringstream out;
        out << std::fixed << std::setprecision(2) << static_cast<float>(pVal) / 100.0f;
        pStr = out.str();
      }
    }
```

API Documentation
=================
The following classes makes up the Rapidcsv interface:
 - [class rapidcsv::Document](doc/rapidcsv_Document.md)
 - [class rapidcsv::SeparatorParams](doc/rapidcsv_SeparatorParams.md)
 - [class rapidcsv::LabelParams](doc/rapidcsv_LabelParams.md)
 - [class rapidcsv::ConverterParams](doc/rapidcsv_ConverterParams.md)
 - [class rapidcsv::no_converter](doc/rapidcsv_no_converter.md)
 - [class rapidcsv::Converter< T >](doc/rapidcsv_Converter.md)

Technical Details
=================
Rapidcsv uses cmake for its tests. Commands to build and execute the test suite:

    mkdir -p build && cd build && cmake .. && make && ctest -C unit --output-on-failure && ctest -C perf --verbose ; cd -

Rapidcsv uses [doxyman2md](https://github.com/d99kris/doxyman2md) to generate its API
documentation:

    doxyman2md src doc

Rapidcsv uses Uncrustify to ensure consistent code formatting:

    uncrustify -c uncrustify.cfg --no-backup src/rapidcsv.h

Alternatives
============
There are many CSV parsers for C++, for example:
- [CSVparser](https://github.com/MyBoon/CSVparser)
- [Fast C++ CSV Parser](https://github.com/ben-strasser/fast-cpp-csv-parser)

License
=======
Rapidcsv is distributed under the BSD 3-Clause license. See
[LICENSE](https://github.com/d99kris/rapidcsv/blob/master/LICENSE) file.

Contributions
=============
Bugs, PRs, etc are welcome on the GitHub project page https://github.com/d99kris/rapidcsv

Keywords
========
c++, c++11, csv parser, comma separated values, single header library.

