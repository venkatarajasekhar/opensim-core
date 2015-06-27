/* -------------------------------------------------------------------------- *
 *                            OpenSim:  DataTable.h                           *
 * -------------------------------------------------------------------------- *
 * The OpenSim API is a toolkit for musculoskeletal modeling and simulation.  *
 * See http://opensim.stanford.edu and the NOTICE file for more information.  *
 * OpenSim is developed at Stanford University and supported by the US        *
 * National Institutes of Health (U54 GM072970, R24 HD065690) and by DARPA    *
 * through the Warrior Web program.                                           *
 *                                                                            *
 * Copyright (c) 2005-2015 Stanford University and the Authors                *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0.         *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */

/** \file
This file defines the  DataTable class, which is used by OpenSim to provide an 
in-memory container for data access and manipulation.                         */

#ifndef OPENSIM_COMMON_DATA_TABLE_H_
#define OPENSIM_COMMON_DATA_TABLE_H_

// Non-standard headers.
#include "SimTKcommon.h"
#include "OpenSim/Common/Exception.h"

// Standard headers.
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>
#include <string>
#include <type_traits>
#include <limits>


namespace OpenSim {
/** Enum to specify if the InputIterator is traversing data row-wise or
column-wise. clang3.6 crashes if this is turned to a "enum class". Until it 
is fixed, use a pre c++11 enum.                                           */
struct InputItDim {
    enum Dim {
        RowWise, 
        ColumnWise
    };
};

class NotEnoughElements : public Exception {
public:
    NotEnoughElements(const std::string& expl) : Exception(expl) {}
};

class NumberOfColumnsMismatch : public Exception {
public:
    NumberOfColumnsMismatch(const std::string& expl) : Exception(expl) {}
};

class NumberOfRowsMismatch : public Exception {
public:
    NumberOfRowsMismatch(const std::string& expl) : Exception(expl) {}
};

class RowDoesNotExist : public Exception {
public:
    RowDoesNotExist(const std::string& expl) : Exception(expl) {}
};

class ColumnDoesNotExist : public Exception {
public:
    ColumnDoesNotExist(const std::string& expl) : Exception(expl) {}
};

class ColumnHasLabel : public Exception {
public:
    ColumnHasLabel(const std::string& expl) : Exception(expl) {}
};

class ColumnHasNoLabel : public Exception {
public:
    ColumnHasNoLabel(const std::string& expl) : Exception(expl) {}
};

class ZeroElements : public Exception {
public:
    ZeroElements(const std::string& expl) : Exception(expl) {}
};

class InvalidEntry : public Exception {
public:
    InvalidEntry(const std::string& expl) : Exception(expl) {}
};

class MetaDataKeyExists : public Exception {
public:
    MetaDataKeyExists(const std::string& expl) : Exception(expl) {}
};

class MetaDataKeyDoesNotExist : public Exception {
public:
    MetaDataKeyDoesNotExist(const std::string& expl) : Exception(expl) {}
};

class MetaDataTypeMismatch : public Exception {
public:
    MetaDataTypeMismatch(const std::string& expl) : Exception(expl) {}
};


/** AbstractDataTable is the base-class of all DataTable_(templated) allowing 
storage of DataTable_ templated on different types to be stored in a container 
like std::vector. AbstractDataTable_ offers:
- Interface to access columns of DataTable_ through column labels. 
- A heterogeneous container to store metadata associated with the DataTable_ in
  the form of key-value pairs where key is of type std::string and value can be
  of any type.

This class is abstract and cannot be used directly. Create instances of 
DataTable_ instead. See DataTable_ for details on ussage.                     */
class AbstractDataTable {
protected:
    using size_t                = std::size_t;
    using string                = std::string;
    using ColumnLabels          = std::unordered_map<string, size_t>;
    using ColumnLabelsConstIter = ColumnLabels::const_iterator;
    using MetaDataValue         = SimTK::ClonePtr<SimTK::AbstractValue>;
    using MetaData              = std::unordered_map<string, MetaDataValue>;

    // Proxy class pretending to be column labels container.
    class ColumnLabelsContainerProxy {
    public:
        ColumnLabelsContainerProxy(const AbstractDataTable* adt) : adt_{adt} {}
        ColumnLabelsContainerProxy()                                 = delete;
        ColumnLabelsContainerProxy(const ColumnLabelsContainerProxy& adt) 
                                                                     = default;
        ColumnLabelsContainerProxy(ColumnLabelsContainerProxy&& adt) = default;
        ColumnLabelsContainerProxy& operator=(const ColumnLabelsContainerProxy&)
                                                                     = default;
        ColumnLabelsContainerProxy& operator=(ColumnLabelsContainerProxy&&) 
                                                                     = default;

        ColumnLabelsConstIter cbegin() const {
            return adt_->columnLabelsBegin();
        }

        ColumnLabelsConstIter cend() const {
            return adt_->columnLabelsEnd();
        }

        ColumnLabelsConstIter begin() const {
            return cbegin();
        }

        ColumnLabelsConstIter end() const {
            return cend();
        }

    private:
        const AbstractDataTable* adt_;
    };

public:
    AbstractDataTable()                                      = default;
    AbstractDataTable(const AbstractDataTable&)              = default;
    AbstractDataTable(AbstractDataTable&&)                   = default;
    AbstractDataTable& operator=(const AbstractDataTable&)   = default;
    AbstractDataTable& operator=(AbstractDataTable&&)        = default;
    virtual std::unique_ptr<AbstractDataTable> clone() const = 0;
    virtual ~AbstractDataTable() {}

    /** \name Column-labels.
        Column labels accessors & mutators.                                   */
    /**@{*/

    /** Check if a column has label. Time complexity is linear in number 
    of column labels. All columns will have an index (starting at 0). All 
    columns need not have a label.

    \throws ColumnDoesNotExist If column index specified does not exist.      */
    bool columnHasLabel(size_t columnIndex) const {
        using ColumnLabelsValue = typename ColumnLabels::value_type;
        throwIfColumnDoesNotExist(columnIndex);
        auto res = std::find_if(col_ind_.begin(), 
                                col_ind_.end(), 
                                [columnIndex] (const ColumnLabelsValue& kv) {
                                    return kv.second == columnIndex;
                                });
        return res != col_ind_.end();
    }

    /** Check if a column exists using column index.                          */
    virtual bool hasColumn(size_t columnIndex) const = 0;

    /** Check if a column exists using column label. All columns have an index 
    but not all columns may have labels.                                      */
    bool hasColumn(const string& columnLabel) const {
        return col_ind_.find(columnLabel) != col_ind_.end();
    }

    /** Label a column. The column should not have a label already. To update 
    the label of a column that already has a label, use updColumnLabel().

    \throws ColumnDoesNotExist If the column index specified does not exist.
    \throws ColumnHasLabel If the column index specified already has a label. */
    void setColumnLabel(size_t columnIndex, 
                        const string& columnLabel) {
        throwIfColumnHasLabel(columnIndex);
        col_ind_.emplace(columnLabel, columnIndex);
    }

    /** Label a column. The column should not have a label already. To update 
    the label of a column that already has a label, use updColumnLabel().

    \throws ColumnDoesNotExist If the column index specified does not exist.
    \throws ColumnHasLabel If the column index specified already has a label. */
    void setColumnLabel(size_t columnIndex, 
                        string&& columnLabel) {
        throwIfColumnHasLabel(columnIndex);
        col_ind_.emplace(std::move(columnLabel), columnIndex);
    }

    /** Label a set of columns at once using an InputIterator that produces one
    index-label pair (std::pair<std::string, std::size_t>) at a time. The 
    columns referred to by the iterator must not already have a label. See
    <a href="http://en.cppreference.com/w/cpp/concept/InputIterator">this page
    </a> for details on InputIterator.

    \throws ColumnDoesNotExist If the column index specified by an entry 
                               produced by the iterator does not exist.
    \throws ColumnHasLabel If the column index specified by an entry produced
                           by the iterator already has a label. */
    template<typename InputIt>
    void setColumnLabels(InputIt first, InputIt last) {
        while(first != last) {
            throwIfColumnHasLabel(first->second);
            col_ind_.emplace(*first);
            ++first;
        }
    }
  
    /** Get the label of a column. Time complexity is linear in the number of
    column labels. The returned value is a copy of the label. To update the 
    label of a column, use updColumnLabel(). 

    \throws ColumnHasNoLabel If the column does not have a label.
    \throws ColumnDoesNotExist If the column does not exist.                  */
    string getColumnLabel(size_t columnIndex) const {
        using ColumnLabelsValue = typename ColumnLabels::value_type;
        throwIfColumnDoesNotExist(columnIndex);
        auto res = std::find_if(col_ind_.begin(),
                                col_ind_.end(),
                                [columnIndex] (const ColumnLabelsValue& kv) {
                                    return kv.second == columnIndex;
                                });
        if(res == col_ind_.end()) {
            throw ColumnHasNoLabel{"Column " + std::to_string(columnIndex) + 
                                   " has no label."};
        }

        return res->first;
    }

    /** Get all the column labels. Returns an object that can be used in a
    range-for statement. The returned object supports begin() and end() 
    functions to retrieve begin and end iterators respectively. Dereferencing 
    the iterator will produce a pair (std::pair<std::string, std::size_t>) where
    the first element of the pair is the column label and the second element is 
    the column index. Not all columns will have labels. The result is not
    writable. Use updColumnLabel() to update column labels                    */
    ColumnLabelsContainerProxy getColumnLabels() const {
        return ColumnLabelsContainerProxy{this};
    }
    
    /** Update the label of a column with a new label. Time complexity is linear
    in the number of column labels. The column specified must already have a
    label. To label a column that does not yet have a label, use 
    setColumnLabel().

    \throws ColumnHasNoLabel If the column specified by the column index does 
                             not already have a label.
    \throws ColumnDoesNotExist If the column specified by the column index does 
                               not exist.                                     */
    void updColumnLabel(size_t columnIndex, 
                        const string& newColumnLabel) {
        const string old_collabel{getColumnLabel(columnIndex)};
        col_ind_.erase(old_collabel);
        col_ind_.emplace(newColumnLabel, columnIndex);
    }

    /** Update the label of a column with a new label. Time complexity is 
    constant on average and linear in number of column labels in the worst case.

    \throws ColumnDoesNotExist If there is no column with the specified label.*/
    void updColumnLabel(const string& oldColumnLabel, 
                        const string& newColumnLabel) {
        const size_t colind{getColumnIndex(oldColumnLabel)};
        col_ind_.erase(oldColumnLabel);
        col_ind_[newColumnLabel] = colind;
    }

    /** Get the index of a column from its label. Time complexity is constant on
    average and linear in number of column labels on worst case.

    \throws ColumnDoesNotExist If the column label does not exist.            */
    size_t getColumnIndex(const string& columnLabel) const {
        try {
            return col_ind_.at(columnLabel);
        } catch(const std::out_of_range&) {
            throw ColumnDoesNotExist{"No column with label '" + columnLabel + 
                                     "'."};
        }
    }

    /** Remove label for column specified by column index.

    \retval true If the column index had a label and was removed.
    \retval false If the column index did not have a label to remove.

    \throws ColumnDoesNotExist If the column specified by "columnIndex" does
                               not exist.                                     */
    bool removeColumnLabel(size_t columnIndex) {
        using ColumnLabelsValue = typename ColumnLabels::value_type;

        throwIfColumnDoesNotExist(columnIndex);
        auto res = std::find_if(col_ind_.begin(),
                                col_ind_.end(),
                                [columnIndex] (const ColumnLabelsValue& kv) {
                                    return kv.second == columnIndex;
                                });
        if(res != col_ind_.end()) {
            col_ind_.erase(res);
            return true;
        } else
            return false;
    }

    /** Remove label for column specified by column label.

    \retval true if the column label exists and was removed.
    \retval false If the column label does not exist.                         */
    bool removeColumnLabel(const string& columnLabel) {
        return col_ind_.erase(columnLabel) > 0;
    }

    /** Clear all the column labels. Data is not cleared. Only the column labels
    are cleared                                                               */
    void clearColumnLabels() {
        col_ind_.clear();
    }

    /** Get an InputIterator representing the beginning of column labels. Get
    the sentinel iterator using columnLabelsEnd(). Dereferencing the iterator 
    produces a pair (std::pair<std::string, std::size_t>) where the first 
    element is the column label and second element is the column index. The 
    result is not writable. Use updColumnLabel() to update column labels. See
    <a href="http://en.cppreference.com/w/cpp/concept/InputIterator">this page
    </a> for details on InputIterator.                                        */
    ColumnLabelsConstIter columnLabelsBegin() const {
        return col_ind_.cbegin();
    }

    /** Get an InputIterator representing the end of column labels. Get the
    beginning iterator using columnLabelsBegin(). See columnLabelsBegin() on 
    using the iterator.                                                       */
    ColumnLabelsConstIter columnLabelsEnd() const {
        return col_ind_.cend();
    }

    /**@}*/
    /** \name Meta-data.
        Meta-data accessors and mutators                                      */
    /**@{*/

    /** Insert metadata. DataTable_ can hold metadata as an associative array of
    key-value pairs where is key is always of type std::string and value can be 
    of any type(except an array type[eg char[]]). The metadata inserted can 
    later be retrieved using the functions getMetaData() and updMetaData(). This
    function throws if the key is already present.

    \param key A std::string that can be used the later to retrieve the inserted
               metadata.
    \param value An object/value of any type except array types(eg int[]). The 
                 code will fail to compile for array types.

    \throws MetaDataKeyExists If the specified key already exits              */
    template<typename ValueType>
    void insertMetaData(const std::string& key, ValueType&& value) {
        using namespace SimTK;
        using ValueTypeNoRef = typename std::remove_reference<ValueType>::type;

        static_assert(!std::is_array<ValueTypeNoRef>::value,
                      "'value' cannot be of array type. For ex. use std::string"
                      " instead of char[], use std::vector<int> instead of " 
                      "int[].");

        if(hasMetaData(key))
            throw MetaDataKeyExists{"Key '" + std::string{key} + 
                                    "' already exists. Remove the existing " 
                                    "entry before inserting."};

        // Type erased value.
        auto tev = new Value<ValueTypeNoRef>{std::forward<ValueType>(value)};
        metadata_.emplace(key, MetaDataValue{tev});
    }

    /** Get previously inserted metadata using its key and type. The template
    argument has to be exactly the non-reference type of the metadata previously
    stored using insertMetaData(). The return value is a read-only reference to 
    the metadata. Use updMetaData() to obtain a writable reference. Time 
    complexity is constant on average and linear in number of elements in 
    metadata on worst case.

    \throws MetaDataKeyDoesNotExist If the key specified does not exist in 
                                    metadata.
    \throws MetaDataTypeMismatch If the type specified as template argument doe 
                                 not match the type of metadata stored under the
                                 key specified.                               */
    template<typename ValueType>
    const ValueType& getMetaData(const string& key) const {
        static_assert(!std::is_reference<ValueType>::value, 
                      "Template argument 'ValueType' should be exactly the" 
                      " non-reference type of the MetaData value stored.");

        try {
            return metadata_.at(key)->template getValue<ValueType>();
        } catch(std::out_of_range&) {
            throw MetaDataKeyDoesNotExist{"Key '" + key + "' not found."};
        } catch(std::bad_cast&) {
            throw MetaDataTypeMismatch{"Template argument specified for " 
                                       "getMetaData is incorrect."};
        }
    }

    /** Update previously inserted metadata using its key and type. The template
    argument has to be exactly the non-reference type of the metadata previously
    stored using insertMetaData(). The returned value is editable. Time 
    complexity is constant on average and linear in number of elements in 
    metadata on worst case.

    \throws MetaDataKeyDoesNotExist If the key specified does not exist in 
                                    metadata.
    \throws MetaDataTypeMismatch If the type specified as template argument does
                                 not match the type of metadata stored under the
                                 key specified.                               */
    template<typename ValueType>
    ValueType& updMetaData(const string& key) {
        return const_cast<ValueType&>(getMetaData<ValueType>(key));
    }

    /** Pop previously inserted metadata using its key and type. The template
    argument has to be exactly the non-reference type of the metadata previously
    inserted using insertMetaData(). The key-value pair is removed from metadata
    and the value is returned. To simply remove the key-value pair without 
    retrieving the value, use removeMetaData(). Time complexity is constant on
    average and linear in number of elements in the metadata on worst case.

    \throws MetaDataKeyDoesNotExist If the key specified does not exist in 
                                    metadata.
    \throws MetaDataTypeMismatch If the type specified as template argument does
                                 not match the type of metadata stored under the
                                 key specified.                               */
    template<typename ValueType>
    ValueType popMetaData(const string& key) {
        ValueType value{std::move(updMetaData<ValueType>(key))};
        metadata_.erase(key);
        return value;
    }

    /** Remove a metadata key-value pair previously inserted. 

    \retval true If there was a removal. 
    \retval false If the key was not found in metadata. 

    Time complexity is constant on average and linear in number of elements in 
    the metadata on worst case.                                               */
    bool removeMetaData(const string& key) {
        return metadata_.erase(key);
    }

    /** Clear the metadata. All the metadata will be lost with this operation.*/
    void clearMetaData() {
        metadata_.clear();
    }

    /** Check if metadata for a given key exists. Time complexity is constant on
    average and linear in the number of elements in the metadata on worst 
    case.                                                                     */
    bool hasMetaData(const string& key) const {
        return metadata_.find(key) != metadata_.end();
    }

    /** Check if metadata is empty -- if the number of elements is zero.      */
    bool isMetaDataEmpty() const {
        return metadata_.empty();
    }

    /** Get the number of elements in the metadata. Time complexity of other 
    operations on metadata depend on this number.                             */
    size_t getMetaDataSize() const {
        return metadata_.size();
    }

    /**@}*/

protected:
    // Helper function. Check if a column exists and throw an exception if it
    // does not.
    void throwIfColumnDoesNotExist(const size_t columnIndex) const {
        if(!hasColumn(columnIndex)) {
            throw ColumnDoesNotExist{"Column " + std::to_string(columnIndex) + 
                                     " does not exist. Index out of range."};
        }
    }

    // Helper function. Check if a column has label and throw an exception if it
    // does.
    void throwIfColumnHasLabel(const size_t columnIndex) const {
        if(columnHasLabel(columnIndex)) {
            throw ColumnHasLabel{"Column " + std::to_string(columnIndex) + 
                                 " already has a label."};
        }
    }

    // Meta-data.
    MetaData     metadata_;
    // Column label to column index.
    ColumnLabels col_ind_;
}; // AbstractDataTable


/** \brief DataTable_ is a in-memory storage container for data (in the form of 
a matrix with column names) with support for holding metadata.                
                                                                              
- Underlying matrix will have entries of configurable type ET (template 
  param).
- Random-access (constant-time) to specific entries, entire columns and entire 
  rows using their index.
- Average constant-time access to columns through column-labels.
- Add rows and columns to existing DataTable_. 
- Add/concatenate two DataTable_(s) by row and by column. 
- %Set column labels for a subset of columns, update them, remove them etc. 
- Construct DataTable_ emtpy OR with a given shape and default value OR using 
  and iterator pair one entry at a time.
- Heterogeneous metadata container through base class AbstractDataTable. 
  Metadata in the form of key-value pairs where key is a std::string and value 
  is is of any type.
                                                                              
\tparam ET Type of the entries in the underlying matrix. Defaults to         
           SimTK::Real (alias for double).                                    */
template<typename ET = SimTK::Real>
class DataTable_ : public AbstractDataTable {
public:
    using value_type    = ET;
    using size_type     = size_t;

    /** \name Create.
        Constructors.                                                         */
    /**@{*/

    /** Construct empty DataTable.                                            */
    DataTable_() = default;

    /** Construct DataTable_ with dimensions [numRows x numColumns] where each
    entry is initialized with initialValue. Default value for initialValue is 
    NaN.
  
    \param numRows Number of rows.
    \param numColumns Number of columns.
    \param initialValue Value to initialize all the entries to.               */
    DataTable_(size_t numRows,
               size_t numColumns,
               const ET& initialValue = ET{SimTK::NaN}) 
        : data_{static_cast<int>(numRows), 
                static_cast<int>(numColumns), 
                initialValue} {}

    /** Construct DataTable_ using an InputIterator which produces one entry at 
    a time when dereferenced. The entries of DataTable_ are copy initialized 
    using the values produced by the iterator. For example, specifying RowWise 
    for parameter dimension and 10 for parameter numEntries will populate the 
    DataTable_ one row at a time with each row's length taken to be 10. See
    <a href="http://en.cppreference.com/w/cpp/concept/InputIterator">this page
    </a> for details on InputIterator.
      
    \param first Beginning of range covered by the iterator. Both first and last
                 are of same type -- InputIt.
    \param last End of the range covered by the iterator. Both first and last
                are of same type -- InputIt.
    \param numEntries Extent of the dimension specified by parameter dim. 
    \param dimension Dimension to populate the DataTable_. Possible values are:
                     - RowWise -- Populate the DataTable_ one row at a time.
                     - ColumnWise -- Populate the DataTable_ one column at a 
                       time.
    \param allowMissing Allow for missing values. 
                        - false -- NotEnoughElements will be thrown if the input
                          iterator fills up the last row/column only partially. 
                        - true -- No exception thrown if the input iterator
                          fills up the last row/column only partially. Instead,
                          missing elements are set to SimTK::NaN.
  
    \throws ZeroElements When input-iterator does not produce any elements.
                         That is first == last.                                 
    \throws InvalidEntry When the required input argument 'numEntries' is zero.
    \throws NotEnoughElements The argument allowMissing enables/disables this
                              exception. When enabled, if dimension == RowWise, 
                              this exception is thrown when the input iterator 
                              does not produce enough elements to fill up the 
                              last row completely. If dimension == ColumnWise, 
                              this exception is thrown when the input iterator 
                              does not produce enough elements to fill up the 
                              last column completely.                         */
    template<typename InputIt>
    DataTable_(InputIt first,
               typename std::enable_if<!std::is_integral<InputIt>::value,
                                       InputIt>::type last,
               size_t numEntries,
               InputItDim::Dim dimension = InputItDim::RowWise,
               bool allowMissing = false) 
        : data_{static_cast<int>(dimension == InputItDim::RowWise ? 
                                 1 : numEntries), 
                static_cast<int>(dimension == InputItDim::RowWise ? 
                                 numEntries : 1)} {
        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements."};
        if(numEntries == 0)
            throw InvalidEntry{"Input argument 'numEntries' cannot be zero."};

        int row{0};
        int col{0};
        while(first != last) {
            data_.set(row, col, *first);
            ++first;
            if(dimension == InputItDim::RowWise) {
                ++col;
                if(col == static_cast<int>(numEntries)  && first != last) {
                    col = 0;
                    ++row;
                    data_.resizeKeep(data_.nrow() + 1, data_.ncol());
                }
            } else {
                ++row;
                if(row == static_cast<int>(numEntries) && first != last) {
                    row = 0;
                    ++col;
                    data_.resizeKeep(data_.nrow(), data_.ncol() + 1);
                }
            }
        }
        if(!allowMissing) {
            if(dimension == InputItDim::RowWise && 
               col != data_.ncol()) {
                throw NotEnoughElements{"Input iterator did not produce " 
                                        "enough elements to fill the last " 
                                        "row. Expected = " + 
                                        std::to_string(data_.ncol()) + 
                                        " Received = " + 
                                        std::to_string(col)};
            } else if(dimension == InputItDim::ColumnWise && 
                      row != data_.nrow()) {
                throw NotEnoughElements{"Input iterator did not produce enough "
                                        "elements to fill the last column. " 
                                        "Expected = " + 
                                        std::to_string(data_.nrow()) + 
                                        " Received = " + std::to_string(row)};
            }
        }
    }
    
    /**@}*/
    /** \name Copy.
        Copy operations including copy constructor.                           */
    /**@{*/

    /** Copy constructor.                                                     */
    DataTable_(const DataTable_& dt) = default;

    /** Virtual copy constructor.                                             */
    std::unique_ptr<AbstractDataTable> clone() const override {
        return std::unique_ptr<AbstractDataTable>(new DataTable_{*this});
    }

    /** Copy assignment                                                       */
    DataTable_& operator=(const DataTable_& dt) = default;

    /**@}*/
    /** \name Move.
        Move operations.                                                      */
    /**@{*/

    /** Move constructor.                                                     */
    DataTable_(DataTable_&& dt) = default;

    /** Move assignment                                                       */
    DataTable_& operator=(DataTable_&& dt) = default;

    /**@}*/
    /** \name Destroy.
        Destructor.                                                           */
    /**@{*/

    /** Destructor.                                                           */
    ~DataTable_() override = default;

    /**@}*/
    /** \name Data.
        Data accessors & mutators.                                            */
    /**@{*/

    /** Get number of rows in the DataTable_.                                 */
    size_t getNumRows() const {
        return static_cast<size_t>(data_.nrow()); 
    }

    /** Get number of columns in the DataTable_.                              */
    size_t getNumColumns() const {
        return static_cast<size_t>(data_.ncol()); 
    }

    /** Get a sub-matrix (or block) of the DataTable_. Returned object is not
    writable. Use updMatrix() to obtain a writable reference. For more 
    information on using the result, see SimTK::MatrixView_.                  

    \throws RowDoesNotExist If the row specified by either rowStart or 
                            [rowStart + numRows - 1] does not exist.
    \throws ColumnDoesNotExist If the column specified by either columnStart or
                               [columnStart + numColumns - 1] does not exist. */
    SimTK::MatrixView_<ET> getMatrix(size_t rowStart, 
                                     size_t columnStart,
                                     size_t numRows,
                                     size_t numColumns) const {
        throwIfRowDoesNotExist(rowStart);
        throwIfRowDoesNotExist(rowStart + numRows - 1);
        throwIfColumnDoesNotExist(columnStart);
        throwIfColumnDoesNotExist(columnStart + numColumns - 1);
        
        return data_.block(static_cast<int>(rowStart), 
                           static_cast<int>(columnStart), 
                           static_cast<int>(numRows), 
                           static_cast<int>(numColumns));
    }

    /** Get a sub-matrix (or block) of the DataTable_. Returned object is 
    writable. For more information on using the result, see 
    SimTK::MatrixView_.                                                       

    \throws RowDoesNotExist If the row specified by either rowStart or 
                            [rowStart + numRows - 1] does not exist.
    \throws ColumnDoesNotExist If the column specified by either columnStart or
                               [columnStart + numColumns - 1] does not exist. */
    SimTK::MatrixView_<ET> updMatrix(size_t rowStart, 
                                     size_t columnStart,
                                     size_t numRows,
                                     size_t numColumns) {
        throwIfRowDoesNotExist(rowStart);
        throwIfRowDoesNotExist(rowStart + numRows - 1);
        throwIfColumnDoesNotExist(columnStart);
        throwIfColumnDoesNotExist(columnStart + numColumns - 1);

        return data_.updBlock(static_cast<int>(rowStart), 
                              static_cast<int>(columnStart), 
                              static_cast<int>(numRows), 
                              static_cast<int>(numColumns));
    }

    /** Get a row of the DataTable_ by index. Returned row is read-only. Use 
    updRow() to obtain a writable reference. See SimTK::RowVectorView_ for more
    details.
  
    \throws RowDoesNotExist If the row specified by either rowIndex does not 
                            exist.                                            */
    SimTK::RowVectorView_<ET> getRow(size_t rowIndex) const {
        throwIfRowDoesNotExist(rowIndex);
        return data_.row(static_cast<int>(rowIndex));
    }

    /** Get a row of the DataTable_ by index. Returned row is editable. See 
    SimTK::RowVectorView_ for more details.
  
    \throws RowDoesNotExist If the row specified by either rowIndex does not 
                            exist.                                            */
    SimTK::RowVectorView_<ET> updRow(size_t rowIndex) {
        throwIfRowDoesNotExist(rowIndex);
        return data_.updRow(static_cast<int>(rowIndex));
    }

    /** Get a column of the DataTable_ by index. Returned column is read-only. 
    Use updColumn() to obtain a writable reference. See SimTK::VectorView_ for 
    more details.
  
    \throws ColumnDoesNotExist If the column specified by columnIndex does not
                               exist.                                         */
    SimTK::VectorView_<ET> getColumn(size_t columnIndex) const {
        throwIfColumnDoesNotExist(columnIndex);
        return data_.col(static_cast<int>(columnIndex));
    }

    /** Get a column of the DataTable_ by label. Returned column is read-only. 
    Use updColumn to obtain a writable reference. See SimTK::VectorView_ for 
    more details.
      
    \throws ColumnDoesNotExist If the column label specified is not in the 
                               DataTable_.                                    */
    SimTK::VectorView_<ET> getColumn(const string& columnLabel) const {
        return data_.col(static_cast<int>(getColumnIndex(columnLabel)));
    }

    /** Get a column of the DataTable_ by index. Returned column is editable. 
    See SimTK::VectorView_ for more details.
  
    \throws ColumnDoesNotExist If the column specified by columnIndex does not
                               exist.                                         */
    SimTK::VectorView_<ET> updColumn(size_t columnIndex) {
        throwIfColumnDoesNotExist(columnIndex);
        return data_.updCol(static_cast<int>(columnIndex));
    }

    /** Get a column of the DataTable_ by label. Returned column is editable. 
    See SimTK::VectorView_ for more details.
  
    \throws ColumnDoesNotExist If the column label specified is not in the 
                               DataTable_.                                    */
    SimTK::VectorView_<ET> updColumn(const string& columnLabel) {
        return data_.updCol(static_cast<int>(getColumnIndex(columnLabel)));
    }

    /** Get an element of the DataTable_ by its index-pair(row, column). The 
    returned element is read-only. use updElt() to get a writable reference.

    \throws RowDoesNotExist If the row specified by rowIndex does not exist.
    \throws ColumnDoesNotExist If the column specified by columnIndex does not
                               exist                                          */
    const ET& getElt(size_t rowIndex, size_t columnIndex) const {
        throwIfRowDoesNotExist(rowIndex);
        throwIfColumnDoesNotExist(columnIndex);
        return data_.getElt(static_cast<int>(rowIndex), 
                            static_cast<int>(columnIndex));
    }

    /** Get an element of the DataTable_ by (row, column-label) pair. The 
    returned element is read-only. use updElt to get a writable reference.
   
    \throws RowDoesNotExist If the row specified by rowIndex does not exist.
    \throws ColumnDoesNotExist If the column label specified is not in the 
                               DataTable_.                                    */
    const ET& getElt(size_t rowIndex, const string& columnLabel) const {
        throwIfRowDoesNotExist(rowIndex);
        return data_.getElt(static_cast<int>(rowIndex), 
                            static_cast<int>(getColumnIndex(columnLabel)));
    }

    /** Get an element of the DataTable_ by its index-pair(row, column). The 
    returned element is editable.
  
    \throws RowDoesNotExist If the row specified by rowIndex does not exist.
    \throws ColumnDoesNotExist If the column specified by columnIndex does not
                               exist                                          */
    ET& updElt(size_t rowIndex, size_t columnIndex) {
        throwIfRowDoesNotExist(rowIndex);
        throwIfColumnDoesNotExist(columnIndex);
        return data_.updElt(static_cast<int>(rowIndex), 
                            static_cast<int>(columnIndex));
    }

    /** Get an element of the DataTable_ by (row, column-label) pair. The 
    returned element is editable.

    \throws RowDoesNotExist If the row specified by rowIndex does not exist.
    \throws ColumnDoesNotExist If the column label specified is not in the 
                               DataTable_.                                    */
    ET& updElt(size_t rowIndex, const string& columnLabel) {
        throwIfRowDoesNotExist(rowIndex);
        return data_.updElt(static_cast<int>(rowIndex), 
                            static_cast<int>(getColumnIndex(columnLabel)));
    }

    /** Get a *copy* of the underlying matrix of the DataTable_.              */
    SimTK::Matrix_<ET> copyAsMatrix() const {
        return *(new SimTK::Matrix_<ET>{data_});
    }

    /** Add (append) a row to the DataTable_ using a SimTK::RowVector_. If the 
    DataTable is empty, input row will be the first row. This function can be
    used to populate an empty DataTable_.
  
    \param row The row to be added as a SimTK::RowVector_.
  
    \throws ZeroElements If number of elements in the input row is zero.
    \throws NumberOfColsMismatch If number of columns in the input row does not 
                                 match the number of columns of the 
                                 DataTable_.                                  */
    void addRow(const SimTK::RowVector_<ET>& row) {
        if(row.nrow() == 0 || row.ncol() == 0)
            throw ZeroElements{"Input row has zero length."};
        if(data_.ncol() > 0 && row.size() != data_.ncol())
            throw NumberOfColumnsMismatch{"Input row has incorrect number of " 
                                          "columns. Expected = " + 
                                          std::to_string(data_.ncol()) + 
                                          " Received = " + 
                                          std::to_string(row.size())};

        data_.resizeKeep(data_.nrow() + 1, row.ncol());
        data_.updRow(data_.nrow() - 1).updAsRowVector() = row;
    }

    /** Add (append) a row to the DataTable_ using an InputIterator producing 
    one entry at a time. If this function is called on an empty DataTable_ 
    without providing numColumnsHint, it performs <i>allocation + relocation</i>
    for [log2(ncol) + 1] times where ncol is the actual number of elements 
    produced by the InputIterator. To add multiple rows at once using an 
    InputIterator, use addRows().
  
    \param first Beginning of range covered by the iterator.
    \param last End of the range covered by the iterator.
    \param numColumnsHint Hint for the number of columns in the input iterator. 
                          Can be approximate above or below the actual number. 
                          This is only used when this function is called on an 
                          empty DataTable_. Ignored otherwise. Providing a hint 
                          reduces the number of resize operations which involves
                          memory allocation and relocation.
    \param allowMissing Allow for missing values. Enables/disables the exception
                        NotEnoughElements. This is only used when DataTable_ is
                        non-empty. If this function is called on an empty
                        DataTable_, this argument is ignored.
                        - false -- exception is thrown if the input iterator 
                          fills up the row only partially. 
                        - true -- exception is not thrown even if the input
                        iterator fills up the row only partially. Instead,
                        missing values are set to SimTK::NaN.
  
    \throws ZeroElements If the number of elements produced by the input 
                         iterator is zero.
    \throws InvalidEntry The DataTable is empty and required the input argument 
                         'numColumnsHint' is zero.
    \throws NotEnoughElements Argument allowMissing enables/disables this 
                              exception. This exception is applicable only
                              when this function is called on a non-empty 
                              DataTable_. When enabled, this exception is thrown
                              if the InputIterator does not produce enough
                              elements to fill up the entire row.             */
    template<typename InputIt>
    void addRow(InputIt first, 
                InputIt last, 
                size_t numColumnsHint = 2,
                bool allowMissing = false) {
        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if((data_.nrow() == 0 || data_.ncol() == 0) && numColumnsHint == 0)
            throw InvalidEntry{"Input argument 'numColumnsHint' cannot be zero "
                               "when DataTable is empty."};

        if(data_.ncol() > 0) {
            data_.resizeKeep(data_.nrow() + 1, data_.ncol());
            int col{0};
            while(first != last) {
                data_.set(data_.nrow() - 1, col++, *first);
                ++first;
            }
            if(!allowMissing && col != data_.ncol())
                throw NotEnoughElements{"Input iterator did not produce enough "
                                        "elements to fill the row. Expected = " 
                                        + std::to_string(data_.ncol()) + 
                                        " Received = " + std::to_string(col)};
        } else {
            int col{0};
            size_t ncol{numColumnsHint};
            data_.resizeKeep(1, static_cast<int>(ncol));
            while(first != last) {
                data_.set(0, col++, *first);
                ++first;
                if(col == static_cast<int>(ncol) && first != last) {
                    // If ncol is a power of 2, double it. Otherwise round it to
                    // the next power of 2.
                    ncol = (ncol & (ncol - 1)) == 0 ? 
                           ncol << 2 : rndToNextPowOf2(ncol); 
                    data_.resizeKeep(1, static_cast<int>(ncol));
                }
            }
            if(col != static_cast<int>(ncol))
                data_.resizeKeep(1, col);
        }
    }

    /** Add (append) multiple rows to the DataTable_ using an InputIterator 
    producing one entry at a time. If this function is called on an empty 
    DataTable_, numColumns must be provided. Otherwise, numColumns is ignored. 
    To add just one row, use addRow() instead.

    \param first Beginning of range covered by the iterator.
    \param last End of the range covered by the iterator.
    \param numColumns Number of columns to create in the DataTable_. This is 
                      only used (and required) when the function is called on 
                      an empty DataTable_. Ignored otherwise.
    \param allowMissing Allow for missing values. Enables/disables the exception
                        NotEnoughElements.  
                        - false -- exception is thrown if the input iterator 
                          fills up the last row only partially. 
                        - true -- exception is not thrown even if the input
                          iterator fills up the last row only partially. Instead
                          , missing values are set to SimTK::NaN.

    \throws ZeroElements If the number of elements produced by the input 
                         iterator is zero.
    \throws InvalidEntry If the function is called on an empty DataTable_ 
                         -- without providing the argument numColumns or 
                         -- providing a zero for numColumns.
    \throws NotEnoughElements Arguments allowMissing enables/disables this
                              exception. When enables, this exception is thrown
                              if the input iterator does not produce enough 
                              elements to fill up the last row completely.    */
    template<typename InputIt>
    void addRows(InputIt first, 
                 InputIt last, 
                 size_t numColumns = std::numeric_limits<size_t>::max(),
                 bool allowMissing = false) {
        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if((data_.nrow() == 0 || data_.ncol() == 0) && 
           (numColumns == std::numeric_limits<size_t>::max() || 
            numColumns == 0))
            throw InvalidEntry{"DataTable is empty. 'numColumns' argument must" 
                               " be provided and it cannot be zero."};

        data_.resizeKeep(data_.nrow() + 1, 
                          data_.ncol() == 0 ? 
                          static_cast<int>(numColumns) : data_.ncol());
        int row{data_.nrow() - 1};
        int col{0};
        while(first != last) {
            data_.set(row, col, *first);
            ++first; ++col;
            if(col == static_cast<int>(data_.ncol()) && first != last) {
                col = 0;
                ++row;
                data_.resizeKeep(data_.nrow() + 1, data_.ncol());
            }
        }
        if(!allowMissing && col != data_.ncol())
            throw NotEnoughElements{"Input iterator did not produce enough " 
                                    "elements to fill the last row. Expected = "
                                    + std::to_string(data_.ncol()) + 
                                    " Received = " + std::to_string(col)};
    }

    /** Add (append) a column to the DataTable_ using a SimTK::Vector_. If the 
    DataTable is empty, input column will be the first column. This function can
    be used to populate an empty DataTable_.
  
    \param column The column to be added as a SimTK::Vector_.
  
    \throws ZeroElements If number of elements in the input column is zero.
    \throws NumberOfRowsMismatch If number of columns in the input column does 
                                 not match the number of rows of the 
                                 DataTable_.                                  */
    void addColumn(const SimTK::Vector_<ET>& column) {
        if(column.nrow() == 0 || column.ncol() == 0)
            throw ZeroElements{"Input column has zero length."};
        if(data_.nrow() > 0 && column.size() != data_.nrow())
            throw NotEnoughElements{"Input column has incorrect number of rows."
                                    "Expected = " + 
                                    std::to_string(data_.nrow()) + 
                                    " Received = " + 
                                    std::to_string(column.size())};

        data_.resizeKeep(column.size(), data_.ncol() + 1);
        data_.updCol(data_.ncol() - 1).updAsVector() = column;
    }

    /** Add (append) a column to the DataTable_ using an InputIterator producing
    one entry at a time. If this function is called on an empty DataTable_ 
    without providing numRowsHint, it performs <i>allocation + relocation</i> 
    for [log2(nrow) + 1] times where nrow is the actual number of elements 
    produced by the InputIterator. To add multiple columns at once using 
    input-iterator, use addColumns().

    \param first Beginning of range covered by the iterator.
    \param last End of the range covered by the iterator.
    \param numRowsHint Hint for the number of rows in the input iterator. Can 
                       be approximate above or below the actual number. This is 
                       only used when this function is called on an empty 
                       DataTable_. Ignored otherwise. Providing a hint reduces 
                       the number of resize operations which involves memory 
                       allocation + r elocation.
    \param allowMissing Allow for missing values. Enables/disables the exception
                        NotEnoughElements. This is only used when DataTable_ is
                        non-empty. 
                        - false -- exception is thrown if the input iterator 
                          fills up the row only partially. 
                        - true -- exception is not thrown even if the input
                          iterator fills up the row only partially. Instead, 
                          missing values are set to SimTK::NaN.

    \throws ZeroElements If the number of elements produced by the input
                         iterator is zero.                                      
    \throws InvalidEntry DataTable is empty and input argument numRowsHint is
                         zero.
    \throws NotEnoughElements Argument allowMissing enables/disables this
                              exception. This exception is applicable only
                              when this function is called on a non-empty 
                              DataTable_. When enabled, this exception is thrown
                              if the InputIterator does not produce enough 
                              elements to fill up the column completely.      */
    template<typename InputIt>
    void addColumn(InputIt first, 
                   InputIt last, 
                   size_t numRowsHint = 2,
                   bool allowMissing = false) {
        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if((data_.nrow() == 0 || data_.ncol() == 0) && numRowsHint == 0)
            throw InvalidEntry{"Input argument 'numRowsHint' cannot be zero" 
                               " when DataTable is empty."};

        if(data_.nrow() > 0) {
            data_.resizeKeep(data_.nrow(), data_.ncol() + 1);
            int row{0};
            while(first != last) {
                data_.set(row++, data_.ncol() - 1, *first);
                ++first;
            }
            if(!allowMissing && row != data_.nrow()) 
                throw NotEnoughElements{"Input iterator did not produce enough "
                                        "elements to fill the column. " 
                                        "Expected = " + 
                                        std::to_string(data_.nrow()) + 
                                        " Received = " + std::to_string(row)};
        } else {
            int row{0};
            size_t nrow{numRowsHint};
            data_.resizeKeep(static_cast<int>(nrow), 1);
            while(first != last) {
                data_.set(row++, 0, *first);
                ++first;
                if(row == static_cast<int>(nrow) && first != last) {
                    // If nrow is a power of 2, double it. Otherwise round it to
                    //  the next power of 2.
                    nrow = (nrow & (nrow - 1)) == 0 ? 
                           nrow << 2 : rndToNextPowOf2(nrow); 
                    data_.resizeKeep(static_cast<int>(nrow), 1);
                }
            }
            if(row != static_cast<int>(nrow))
                data_.resizeKeep(row, 1);
        }
    }

    /** Add (append) multiple columns to the DataTable_ using an InputIterator 
    producing one entry at a time. If this function is called on an empty 
    DataTable_, numRows must be provided. Otherwise, numRows is ignored. To add 
    just one column, use addColumn() instead.

    \param first Beginning of range covered by the iterator.
    \param last End of the range covered by the iterator.
    \param numRows Number of rows to create in the DataTable_. This is only 
                   used (and required) when the function is called on an empty 
                   DataTable_. Ignored otherwise.
    \param allowMissing Allow for missing values. Enables/disables the exception
                        NotEnoughElements. 
                        - false -- exception is thrown if the input iterator
                          fills up the last column only partially.
                        - true -- exception is not thrown even if the input
                          iterator fills up the last column only partially.
                          Instead, missing values are set to SimTK::NaN.

    \throws ZeroElements If the number of elements produced by the input 
                         iterator is zero.
    \throws InvalidEntry If the function is called on an empty DataTable_ 
                         -- without providing the argument numRows or 
                         -- providing zero for numRows.
    \throws NotEnoughElements Argument allowMissing enables/disables this 
                              exception. When enabled, this exception is thrown
                              if the input iterator does not produce enough 
                              elements to fill up the last column completely. */
    template<typename InputIt>
    void addColumns(InputIt first, 
                    InputIt last, 
                    size_t numRows = std::numeric_limits<size_t>::max(),
                    bool allowMissing = false) {
        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if((data_.nrow() == 0 || data_.ncol() == 0) && 
           (numRows == std::numeric_limits<size_t>::max() || numRows == 0))
            throw InvalidEntry{"DataTable is empty. 'nrow' argument must be" 
                               " provided and it cannot be zero."};

        data_.resizeKeep(data_.nrow() == 0 ? 
                          static_cast<int>(numRows) : data_.nrow(), 
                          data_.ncol() + 1);
        int row{0};
        int col{data_.ncol() - 1};
        while(first != last) {
            data_.set(row, col, *first);
            ++first; ++row;
            if(row == static_cast<int>(data_.nrow()) && first != last) {
                row = 0;
                ++col;
                data_.resizeKeep(data_.nrow(), data_.ncol() + 1);
            }
        }
        if(!allowMissing && row != data_.nrow())
            throw NotEnoughElements{"Input iterator did not produce enough " 
                                    "elements to fill the last column. " 
                                    "Expected = " + 
                                    std::to_string(data_.nrow()) + 
                                    " Received = " + std::to_string(row)};
    }

    /** Add/concatenate rows of another DataTable_ to this DataTable_. The new 
    rows will appear as the last rows of this DataTable_. Only data will be 
    appended. Metadata is not added from the other DataTable_. Columns retain 
    their labels. To create a new DataTable_ by concatenating two existing 
    DataTable_(s), use OpenSim::concatenateRows().

    \throws NumberOfColumsMismatch If input DataTable_ has incorrect number of
                                   columns for concatenation to work.
    \throws InvalidEntry If trying to add a DataTable_ to itself.             */
    void concatenateRows(const DataTable_& table) {
        if(data_.ncol() != table.data_.ncol()) 
            throw NumberOfColumnsMismatch{"Input DataTable has incorrect number"
                                          " of columns. Expected = " + 
                                          std::to_string(data_.ncol()) + 
                                          " Received = " + 
                                          std::to_string(table.data_.ncol())};
        if(&data_ == &table.data_)
            throw InvalidEntry{"Cannot concatenate a DataTable to itself."};

        int old_nrow{data_.nrow()};
        data_.resizeKeep(data_.nrow() + table.data_.nrow(), data_.ncol());
        data_.updBlock(old_nrow, 
                        0, 
                        table.data_.nrow(), 
                        data_.ncol()) = table.data_;
    }

    /** Add/concatenate columns of another DataTable_ to this DataTable_. The 
    new columns will appear as the last columns of this DataTable_. Only data 
    will be appended. Column labels and metadata are not added. Current columns
    retain their labels. To create a new DataTable_ by concatenating two 
    existing DataTable_(s), use OpenSim::concatenateColumns().

    \throws NumberOfRowsMismatch If input DataTable_ has incorrect number of
                                 rows for concatenation to work.
    \throws InvalidEntry If trying to concatenation a DataTable_ to itself.   */
    void concatenateColumns(const DataTable_& table) {
        if(data_.nrow() != table.data_.nrow())
            throw NumberOfRowsMismatch{"Input DataTable has incorrect number of"
                                       " rows. Expected = " + 
                                       std::to_string(data_.nrow()) + 
                                       " Received = " 
                                       + std::to_string(table.data_.nrow())};
        if(&data_ == &table.data_)
            throw InvalidEntry{"Cannot concatenate a DataTable to itself."};

        int old_ncol{data_.ncol()};
        data_.resizeKeep(data_.nrow(), data_.ncol() + table.data_.ncol());
        data_.updBlock(0, 
                        old_ncol,
                        data_.nrow(), 
                        table.data_.ncol()) = table.data_;
    }

    /** Clear the data of this DataTable_. After this operation, the DataTable_
    will be of size 0x0 and all column labels will be cleared as well.        */
    void clearData() {
        data_.clear();
        clearColumnLabels();
    }

    /** Resize the DataTable_, retaining as much of the old data as will fit. 
    New memory will be allocated and the existing entries will be copied over 
    to the extent they will fit. If columns are dropped during this call, the 
    labels for the lost columns will also be lost. *Be careful not to shrink 
    the DataTable unintentionally*.                                           */
    void resizeKeep(size_t numRows, size_t numColumns) {
        if(numRows == 0)
            throw InvalidEntry{"Input argument 'numRows' cannot be zero."
                               "To clear all data, use clearData()."};
        if(numColumns == 0)
            throw InvalidEntry{"Input argument 'numColumns' cannot be zero."
                               "To clear all data, use clearData()."};

        if(static_cast<int>(numColumns) < data_.ncol())
            for(size_t c_ind = numColumns; 
                c_ind < static_cast<size_t>(data_.ncol()); 
                ++c_ind) 
                removeColumnLabel(c_ind);

        data_.resizeKeep(static_cast<int>(numRows), 
                         static_cast<int>(numColumns));
    }

    /** Check if a row exists by its index.                                   */
    bool hasRow(size_t rowIndex) const {
        return (rowIndex >= 0 &&
                rowIndex < static_cast<size_t>(data_.nrow()));
    }

    /** Check if a column exists by its index.                                */
    bool hasColumn(size_t columnIndex) const override {
        return (columnIndex >= 0 && 
                columnIndex < static_cast<size_t>(data_.ncol()));
    }

    using AbstractDataTable::hasColumn;

private:
    // Helper function. Check if a row exists and throw an exception if it does
    // not.
    void throwIfRowDoesNotExist(const size_t rowIndex) const {
        if(!hasRow(rowIndex))
            throw RowDoesNotExist{"Row " + std::to_string(rowIndex) + 
                                  " does not exist. Index out of range."};
    }

    // Helper function. Round to next highest power of 2. Works only for 
    // 32 bits.
    size_t rndToNextPowOf2(size_t num) {
        assert(static_cast<unsigned long long>(num) <= 
               static_cast<unsigned long long>(0xFFFFFFFF));

        --num;
        num |= (num >>  1); // Highest  2 bits are set by end of this.
        num |= (num >>  2); // Highest  4 bits are set by end of this.
        num |= (num >>  4); // Highest  8 bits are set by end of this.
        num |= (num >>  8); // Highest 16 bits are set by end of this.
        num |= (num >> 16); // Highest 32 bits are set by end of this.
        return ++num;
    }

    // Matrix of data. This excludes timestamp column.
    SimTK::Matrix_<ET> data_;
};  // DataTable_


using DataTable = DataTable_<SimTK::Real>;


/** Add/concatenate two DataTable_(s) by row and produce a new DataTable_.    */
template<typename ET>
DataTable_<ET> concatenateRows(const DataTable_<ET>& dt1, 
                               const DataTable_<ET>& dt2) {
    DataTable_<ET> dt{dt1};
    dt.concatenateRows(dt2);
    return dt;
}


/** Add/concatenate two DataTable_(s) by column and produce a new DataTable_. */
template<typename ET>
DataTable_<ET> concatenateColumns(const DataTable_<ET>& dt1, 
                                  const DataTable_<ET>& dt2) {
    DataTable_<ET> dt{dt1};
    dt.concatenateColumns(dt2);
    return dt;
}

} // namespace OpenSim

#endif //OPENSIM_COMMON_DATA_TABLE_H_