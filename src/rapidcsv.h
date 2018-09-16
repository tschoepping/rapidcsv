/*
 * rapidcsv.h
 *
 * URL:      https://github.com/d99kris/rapidcsv
 * Version:  3.1
 *
 * Copyright (C) 2017-2018 Kristofer Berggren
 * All rights reserved.
 *
 * rapidcsv is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

namespace rapidcsv
{
  /**
   * @brief     Datastructure holding parameters controlling how invalid numbers (including
   *            empty strings) should be handled.
   */
  struct ConverterParams
  {
    /**
     * @brief   Constructor
     * @param   pHasDefaultConverter  specifies if conversion of non-numerical strings shall be
     *                                converted to a default numerical value, instead of causing
     *                                an exception to be thrown (default).
     * @param   pDefaultFloat         floating-point default value to represent invalid numbers.
     * @param   pDefaultInteger       integer default value to represent invalid numbers.
     */
    explicit ConverterParams(const bool pHasDefaultConverter = false,
                             const long double pDefaultFloat = std::numeric_limits<long double>::signaling_NaN(),
                             const long long pDefaultInteger = 0)
      : mHasDefaultConverter(pHasDefaultConverter)
      , mDefaultFloat(pDefaultFloat)
      , mDefaultInteger(pDefaultInteger)
    {
    }

    /**
     * @brief   specifies if conversion of non-numerical strings shall be converted to a default
     *          numerical value, instead of causing an exception to be thrown (default).
     */
    bool mHasDefaultConverter;

    /**
     * @brief   floating-point default value to represent invalid numbers.
     */
    long double mDefaultFloat;

    /**
     * @brief   integer default value to represent invalid numbers.
     */
    long long mDefaultInteger;
  };

  /**
   * @brief     Exception thrown when attempting to access Document data in a datatype which
   *            is not supported by the Converter class.
   */
  class no_converter : public std::exception
  {
    /**
     * @brief   Provides details about the exception
     * @returns an explanatory string
     */
    virtual const char* what() const throw()
    {
      return "unsupported conversion datatype";
    }
  };

  /**
   * @brief     Class providing conversion to/from numerical datatypes and strings. Only
   *            intended for rapidcsv internal usage, but exposed externally to allow
   *            specialization for custom datatype conversions.
   */
  template<typename T>
  class Converter
  {
  public:
    /**
     * @brief   Constructor
     * @param   pConverterParams      specifies how conversion of non-numerical values to
     *                                numerical datatype shall be handled.
     */
    Converter(const ConverterParams& pConverterParams)
      : mConverterParams(pConverterParams)
    {
    }

    /**
     * @brief   Converts string holding a numerical value to numerical datatype representation.
     * @param   pVal                  numerical value
     * @param   pStr                  output string
     */
    void ToVal(const std::string& pStr, T& pVal) const
    {
      try
      {
        if (typeid(T) == typeid(int))
        {
          pVal = static_cast<T>(std::stoi(pStr));
          return;
        }
        else if (typeid(T) == typeid(long))
        {
          pVal = static_cast<T>(std::stol(pStr));
          return;
        }
        else if (typeid(T) == typeid(long long))
        {
          pVal = static_cast<T>(std::stoll(pStr));
          return;
        }
        else if (typeid(T) == typeid(unsigned))
        {
          pVal = static_cast<T>(std::stoul(pStr));
          return;
        }
        else if (typeid(T) == typeid(unsigned long))
        {
          pVal = static_cast<T>(std::stoul(pStr));
          return;
        }
        else if (typeid(T) == typeid(unsigned long long))
        {
          pVal = static_cast<T>(std::stoull(pStr));
          return;
        }
      }
      catch (...)
      {
        if (!mConverterParams.mHasDefaultConverter)
        {
          throw;
        }
        else
        {
          pVal = static_cast<T>(mConverterParams.mDefaultInteger);
          return;
        }
      }

      try
      {
        if (typeid(T) == typeid(float))
        {
          pVal = static_cast<T>(std::stof(pStr));
          return;
        }
        else if (typeid(T) == typeid(double))
        {
          pVal = static_cast<T>(std::stod(pStr));
          return;
        }
        else if (typeid(T) == typeid(long double))
        {
          pVal = static_cast<T>(std::stold(pStr));
          return;
        }
      }
      catch (...)
      {
        if (!mConverterParams.mHasDefaultConverter)
        {
          throw;
        }
        else
        {
          pVal = static_cast<T>(mConverterParams.mDefaultFloat);
          return;
        }
      }

      if (typeid(T) == typeid(char))
      {
        pVal = static_cast<T>(pStr[0]);
        return;
      }
      else
      {
        throw no_converter();
      }
    }

  private:
    const ConverterParams& mConverterParams;
  };

  /**
   * @brief     Specialized implementation handling string to string conversion.
   * @param     pVal                  string
   * @param     pStr                  string
   */
  template<>
  inline void Converter<std::string>::ToVal(const std::string& pStr, std::string& pVal) const
  {
    pVal = pStr;
  }

  /**
   * @brief     Datastructure holding parameters controlling which row and column should be
   *            treated as labels.
   */
  struct LabelParams
  {
    /**
     * @brief   Constructor
     * @param   pColumnNameIdx        specifies the zero-based row index of the column labels, setting
     *                                it to -1 prevents column lookup by label name, and gives access
     *                                to all rows as document data. Default 0 (enabling access
     *                                using column name)..
     * @param   pRowNameIdx           specifies the zero-based column index of the row labels, setting
     *                                it to >=0 enables row lookup by label name.
     *                                Default -1 (preventing access to data using row name).
     */
    explicit LabelParams(const int pColumnNameIdx = 0, const int pRowNameIdx = -1)
      : mColumnNameIdx(pColumnNameIdx)
      , mRowNameIdx(pRowNameIdx)
    {
    }

    /**
     * @brief   specifies the zero-based row index of the column labels.
     */
    int mColumnNameIdx;

    /**
     * @brief   specifies the zero-based column index of the row labels.
     */
    int mRowNameIdx;
  };

  /**
   * @brief     Datastructure holding parameters controlling how the CSV data fields are separated.
   */
  struct SeparatorParams
  {
    /**
     * @brief   Constructor
     * @param   pSeparator            specifies the column separator (default ',').
     */
    explicit SeparatorParams(const char pSeparator = ',')
      : mSeparator(pSeparator)
    {
    }

    /**
     * @brief   specifies the column separator.
     */
    char mSeparator;
  };

  /**
   * @brief     Datastructure holding parameters controlling how the CSV data shall be buffered.
   */
  struct BufferParams
  {
    /**
     * @brief   Constructor
     * @param   pPreparse             specified whether to load the entire file into memory
     *                                and parse its content when Document is created 
     *                                (default true).
     */
    explicit BufferParams(const bool pPreparse = true)
      : mPreparse(pPreparse)
    {
    }

    /**
     * @brief   specifies whether to preparse and buffer all document data.
     */
    char mPreparse;
  };

  /**
   * @brief     Class representing a CSV document.
   */
  class Document
  {
  public:
    /**
     * @brief   Constructor
     * @param   pPath                 specifies the path of an existing CSV-file to populate the Document
     *                                data with.
     * @param   pLabelParams          specifies which row and column should be treated as labels.
     * @param   pSeparatorParams      specifies which field separator should be used.
     * @param   pConverterParams      specifies how invalid numbers (including empty strings) should be
     *                                handled.
     * @param   pBufferParams         specifies whether to preparse and buffer all document data.
     */
    explicit Document(const std::string& pPath,
                      const LabelParams& pLabelParams = LabelParams(),
                      const SeparatorParams& pSeparatorParams = SeparatorParams(),
                      const ConverterParams& pConverterParams = ConverterParams(),
                      const BufferParams& pBufferParams = BufferParams())
      : mPath(pPath)
      , mLabelParams(pLabelParams)
      , mSeparatorParams(pSeparatorParams)
      , mConverterParams(pConverterParams)
      , mBufferParams(pBufferParams)
    {
      ReadCsv();
    }

    /**
     * @brief   Copy constructor
     * @param   pDocument             specifies the Document instance to copy.
     */
    explicit Document(const Document& pDocument)
      : mPath(pDocument.mPath)
      , mLabelParams(pDocument.mLabelParams)
      , mSeparatorParams(pDocument.mSeparatorParams)
      , mConverterParams(pDocument.mConverterParams)
      , mBufferParams(pDocument.mBufferParams)
      , mData(pDocument.mData)
      , mColumnNames(pDocument.mColumnNames)
      , mRowNames(pDocument.mRowNames)
    {
    }

    /**
     * @brief   Read Document data from file.
     * @param   pPath                 specifies the path of an existing CSV-file to 
     *                                populate the Document data with.
     */
    void Load(const std::string& pPath)
    {
      mPath = pPath;
      ReadCsv();
    }

    /**
     * @brief   Get column by index.
     * @param   pColumnIdx            zero-based column index.
     * @returns vector of column data.
     */
    template<typename T>
    std::vector<T> GetColumn(const ssize_t pColumnIdx) const
    {
      const ssize_t columnIdx = pColumnIdx + GetDataColumnOffset();
      const ssize_t dataRowCount = GetDataRowCount(); 
      Converter<T> converter(mConverterParams);

      std::vector<T> column;
      for (ssize_t rowIdx = GetDataRowOffset(); rowIdx < dataRowCount; ++rowIdx)
      {
        T val;
        converter.ToVal(GetDataCell(columnIdx, rowIdx), val);
        column.push_back(val);
      }

      return column;
    }

    /**
     * @brief   Get column by name.
     * @param   pColumnName           column label name.
     * @returns vector of column data.
     */
    template<typename T>
    std::vector<T> GetColumn(const std::string& pColumnName) const
    {
      const ssize_t columnIdx = GetColumnIdx(pColumnName);
      if (columnIdx < 0)
      {
        throw std::out_of_range("column not found: " + pColumnName);
      }
      return GetColumn<T>(columnIdx);
    }

    /**
     * @brief   Get number of data columns.
     * @returns column count.
     */
    size_t GetColumnCount() const
    {
      const ssize_t columnCount = GetDataColumnCount() - GetDataColumnOffset();
      return (columnCount > 0) ? columnCount : 0;
    }

    /**
     * @brief   Get row by index.
     * @param   pRowIdx               zero-based row index.
     * @returns vector of row data.
     */
    template<typename T>
    std::vector<T> GetRow(const ssize_t pRowIdx) const
    {
      const ssize_t rowIdx = pRowIdx + GetDataRowOffset();
      const ssize_t dataColumnCount = GetDataColumnCount();
      Converter<T> converter(mConverterParams);
      
      std::vector<T> row;
      for (ssize_t columnIdx = GetDataColumnOffset(); columnIdx < dataColumnCount; ++columnIdx)
      {
        T val;
        converter.ToVal(GetDataCell(columnIdx, rowIdx), val);
        row.push_back(val);
      }

      return row;
    }

    /**
     * @brief   Get row by name.
     * @param   pRowName              row label name.
     * @returns vector of row data.
     */
    template<typename T>
    std::vector<T> GetRow(const std::string& pRowName) const
    {
      ssize_t rowIdx = GetRowIdx(pRowName);
      if (rowIdx < 0)
      {
        throw std::out_of_range("row not found: " + pRowName);
      }
      return GetRow<T>(rowIdx);
    }

    /**
     * @brief   Get number of data rows.
     * @returns row count.
     */
    size_t GetRowCount() const
    {
      const ssize_t rowCount = GetDataRowCount() - GetDataRowOffset();
      return (rowCount > 0) ? rowCount : 0;
    }

    /**
     * @brief   Get cell by index.
     * @param   pRowIdx               zero-based row index.
     * @param   pColumnIdx            zero-based column index.
     * @returns cell data.
     */
    template<typename T>
    T GetCell(const ssize_t pColumnIdx, const ssize_t pRowIdx) const
    {
      const ssize_t columnIdx = pColumnIdx + GetDataColumnOffset();
      const ssize_t rowIdx = pRowIdx + GetDataRowOffset();

      T val;
      Converter<T> converter(mConverterParams);
      converter.ToVal(GetDataCell(columnIdx, rowIdx), val);
      return val;
    }

    /**
     * @brief   Get cell by name.
     * @param   pColumnName           column label name.
     * @param   pRowName              row label name.
     * @returns cell data.
     */
    template<typename T>
    T GetCell(const std::string& pColumnName, const std::string& pRowName) const
    {
      const ssize_t columnIdx = GetColumnIdx(pColumnName);
      if (columnIdx < 0)
      {
        throw std::out_of_range("column not found: " + pColumnName);
      }

      const ssize_t rowIdx = GetRowIdx(pRowName);
      if (rowIdx < 0)
      {
        throw std::out_of_range("row not found: " + pRowName);
      }

      return GetCell<T>(columnIdx, rowIdx);
    }

    /**
     * @brief   Get column name
     * @param   pColumnIdx            zero-based column index.
     * @returns column name.
     */
    std::string GetColumnName(const ssize_t pColumnIdx)
    {
      const ssize_t columnIdx = pColumnIdx + GetDataColumnOffset();
      if (mLabelParams.mColumnNameIdx < 0)
      {
        throw std::out_of_range("column name row index < 0: " + std::to_string(mLabelParams.mColumnNameIdx));
      }

      return GetDataCell(columnIdx, mLabelParams.mColumnNameIdx);
    }

    /**
     * @brief   Get column names
     * @returns vector of column names.
     */
    std::vector<std::string> GetColumnNames()
    {
      if (mLabelParams.mColumnNameIdx >= 0)
      {
        return GetRow<std::string>(-1);
      }

      return std::vector<std::string>();
    }

    /**
     * @brief   Get row name
     * @param   pRowIdx               zero-based column index.
     * @returns row name.
     */
    std::string GetRowName(const ssize_t pRowIdx)
    {
      const ssize_t rowIdx = pRowIdx + GetDataRowOffset();
      if (mLabelParams.mRowNameIdx < 0)
      {
        throw std::out_of_range("row name column index < 0: " + std::to_string(mLabelParams.mRowNameIdx));
      }

      return GetDataCell(mLabelParams.mRowNameIdx, rowIdx);
    }

    /**
     * @brief   Get row names
     * @returns vector of row names.
     */
    std::vector<std::string> GetRowNames()
    {
      if (mLabelParams.mRowNameIdx >= 0)
      {
        return GetColumn<std::string>(-1);
      }

      return std::vector<std::string>();
    }

  private:
    inline ssize_t GetDataRowOffset() const
    {
      return (mLabelParams.mColumnNameIdx + 1);
    }
    
    inline ssize_t GetDataColumnOffset() const
    {
      return (mLabelParams.mRowNameIdx + 1);
    }
    
    void ReadCsv()
    {
      std::ifstream file;
      file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      file.open(mPath, std::ios::binary | std::ios::ate);
      std::streamsize fileLength = file.tellg();
      file.seekg(0, std::ios::beg);
      const std::streamsize bufLength = 64 * 1024;
      std::vector<char> buffer(bufLength);
      std::vector<std::string> row;
      std::string cell;
      bool quoted = false;
      int cr = 0;
      int lf = 0;

      while (fileLength > 0)
      {
        long long readLength = std::min(fileLength, bufLength);
        file.read(buffer.data(), readLength);
        for (int i = 0; i < readLength; ++i)
        {
          if (buffer[i] == '"')
          {
            if (cell.empty() || cell[0] == '"')
            {
              quoted = !quoted;
            }
            cell += buffer[i];
          }
          else if (buffer[i] == mSeparatorParams.mSeparator)
          {
            if (!quoted)
            {
              row.push_back(cell);
              cell.clear();
            }
            else
            {
              cell += buffer[i];
            }
          }
          else if (buffer[i] == '\r')
          {
            ++cr;
          }
          else if (buffer[i] == '\n')
          {
            ++lf;
            row.push_back(cell);
            cell.clear();
            mData.push_back(row);
            row.clear();
            quoted = false; // disallow line breaks in quoted string, by auto-unquote at linebreak
          }
          else
          {
            cell += buffer[i];
          }
        }
        fileLength -= readLength;
      }

      // Handle last line without linebreak
      if (!cell.empty() || !row.empty())
      {
        row.push_back(cell);
        cell.clear();
        mData.push_back(row);
        row.clear();
      }

      // Set up column labels
      if ((mLabelParams.mColumnNameIdx >= 0) &&
          (mData.size() > 0))
      {
        int i = 0;
        for (auto& columnName : mData[mLabelParams.mColumnNameIdx])
        {
          mColumnNames[columnName] = i++;
        }
      }

      // Set up row labels
      if ((mLabelParams.mRowNameIdx >= 0) &&
          (static_cast<ssize_t>(mData.size()) >
           GetDataRowOffset()))
      {
        int i = 0;
        for (auto& dataRow : mData)
        {
          mRowNames[dataRow[mLabelParams.mRowNameIdx]] = i++;
        }
      }
    }

    ssize_t GetColumnIdx(const std::string& pColumnName) const
    {
      if (mLabelParams.mColumnNameIdx >= 0)
      {
        if (mColumnNames.find(pColumnName) != mColumnNames.end())
        {
          return mColumnNames.at(pColumnName) - GetDataColumnOffset();
        }
      }
      else
      {
        throw std::out_of_range("column name row index < 0: " + std::to_string(mLabelParams.mColumnNameIdx));
      }

      return -1;
    }

    ssize_t GetRowIdx(const std::string& pRowName) const
    {
      if (mLabelParams.mRowNameIdx >= 0)
      {
        if (mRowNames.find(pRowName) != mRowNames.end())
        {
          return mRowNames.at(pRowName) - GetDataRowOffset();
        }
      }
      else
      {
        throw std::out_of_range("row name column index < 0: " + std::to_string(mLabelParams.mRowNameIdx));
      }

      return -1;
    }

    size_t GetDataRowCount() const
    {
      return mData.size();
    }

    size_t GetDataColumnCount() const
    {
      return (mData.size() > 0) ? mData.at(0).size() : 0;
    }

    std::string GetDataCell(const ssize_t pColumnIdx, const ssize_t pRowIdx) const
    {
      return mData.at(pRowIdx).at(pColumnIdx);
    }

  private:
    std::string mPath;
    LabelParams mLabelParams;
    SeparatorParams mSeparatorParams;
    ConverterParams mConverterParams;
    BufferParams mBufferParams;
    std::vector<std::vector<std::string> > mData;
    std::map<std::string, size_t> mColumnNames;
    std::map<std::string, size_t> mRowNames;
  };
}
