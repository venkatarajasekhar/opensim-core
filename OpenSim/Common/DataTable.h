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
#include <algorithm>
#include <cstdlib>


namespace OpenSim {
/** Enum to specify dimension of data traversal -- row-wise or olumn-wise.    */
enum class TraverseDir {
    RowMajor, 
    ColumnMajor
};

class EmptyDataTable : public Exception {
public:
    EmptyDataTable(const std::string& expl) : Exception(expl) {}
};

class NotEnoughElements : public Exception {
public:
    NotEnoughElements(const std::string& expl) : Exception(expl) {}
};

class TooManyElements : public Exception {
public:
    TooManyElements(const std::string& expl) : Exception(expl) {}
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

class ColumnLabelExists : public Exception {
public:
    ColumnLabelExists(const std::string& expl) : Exception(expl) {}
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

class IncompatibleIterators : public Exception {
public:
    IncompatibleIterators(const std::string& expl) : Exception(expl) {}
};


namespace internal {
template<typename...>
using void_t = void;

// Compile time check to see if the iterator (template parameter Iter) supports
// dereferencing operator.
template<typename Iter>
using dereference_t = decltype(*std::declval<Iter>());

template<typename Iter, typename = void>
struct is_dereferencable_t 
    : std::false_type {};

template<typename Iter>
struct is_dereferencable_t<Iter, void_t<dereference_t<Iter>>> 
    : std::true_type {};

template<typename Iter>
constexpr bool is_dereferencable = is_dereferencable_t<Iter>::value;


// Compile time checks to see two types are the same after stripping reference 
// and cv qualifiers.
template<typename T>
using rmv_ref_t = typename std::remove_reference<T>::type;

template<typename T>
using rmv_cv_t = typename std::remove_cv<T>::type;

template<typename T>
using rmv_ref_cv_t = rmv_cv_t<rmv_ref_t<T>>;

template<typename A, typename B>
constexpr bool
is_same = std::is_same<rmv_ref_cv_t<A>, rmv_ref_cv_t<B>>::value;

// Compile time check to see if the a type supports equality comparison.
template<typename T>
using eq_expr_t = decltype(std::declval<T>() == std::declval<T>());

template<typename T, typename = void>
struct is_eq_comparable_t
    : std::false_type {};

template<typename T>
struct is_eq_comparable_t<T, void_t<eq_expr_t<T>>>
    : std::true_type {};

template<typename T>
constexpr bool is_eq_comparable = is_eq_comparable_t<T>::value;

// Compile time check to see if the a type supports inequality comparison.
template<typename T>
using neq_expr_t = decltype(std::declval<T>() != std::declval<T>());

template<typename T, typename = void>
struct is_neq_comparable_t
    : std::false_type {};

template<typename T>
struct is_neq_comparable_t<T, void_t<neq_expr_t<T>>>
    : std::true_type {};

template<typename T>
constexpr bool is_neq_comparable = is_neq_comparable_t<T>::value;

// Compile time check to see if a type has member function named begin().
template<typename C>
using mem_begin_t = decltype(std::declval<C>().begin());

template<typename C, typename = void>
struct has_mem_begin_t 
    : std::false_type {};

template<typename C>
struct has_mem_begin_t<C, void_t<mem_begin_t<C>>>
    : std::true_type {};

template<typename C>
constexpr bool
has_mem_begin = has_mem_begin_t<C>::value;

// Compile time check to see if a type has member function named end().
template<typename C>
using mem_end_t = decltype(std::declval<C>().end());

template<typename C, typename = void>
struct has_mem_end_t 
    : std::false_type {};

template<typename C>
struct has_mem_end_t<C, void_t<mem_end_t<C>>>
    : std::true_type {};

template<typename C>
constexpr bool
has_mem_end = has_mem_end_t<C>::value;
}


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
    /** \cond */

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
        ColumnLabelsContainerProxy()                                  = delete;
        ColumnLabelsContainerProxy(const ColumnLabelsContainerProxy&) = default;
        ColumnLabelsContainerProxy(ColumnLabelsContainerProxy&&)      = default;
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

    /** \endcond */
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

    /** Check if the DataTable_ has a column with given string as the label   */
    bool hasColumnLabel(const std::string& columnLabel) const {
        return col_ind_.find(columnLabel) != col_ind_.end();
    }

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

    /** Label a column. The column should not have a label already. Column 
    labels are unique for entire DataTable_. To update the label of a column 
    that already has a label, use updColumnLabel().

    \throws ColumnLabelExists If a column in the DataTable_ already has the 
                              label specified by 'columnLabel'.
    \throws ColumnDoesNotExist If the column index specified does not exist.
    \throws ColumnHasLabel If the column index specified already has a label. */
    void setColumnLabel(size_t columnIndex, const string& columnLabel) {
        throwIfColumnHasLabel(columnIndex);
        throwIfColumnLabelExists(columnLabel);

        col_ind_.emplace(columnLabel, columnIndex);
    }

    /** Label a set of columns at once using an InputIterator that produces one
    index-label pair (std::pair<std::string, std::size_t>) at a time. The 
    columns referred to by the iterator must not already have a label. The 
    column labels have to be unique for the entire DataTable_. See
    <a href="http://en.cppreference.com/w/cpp/concept/InputIterator">this page
    </a> for details on InputIterator.

    \param first InputIterator representing beginning of the range of input
                 values. Both 'first' and 'last' are of same type -- InputIt
                 (type of the input iterator). Don't be confused by the extra
                 code that appears in function signature for 'last'.
    \param last InputIterator representing end of the range of vallues. Both 
                'first' and 'last' are of same type -- InputIt (type of the 
                input iterator). Don't be confused by the extra code that 
                appears in function signature for 'last'.

    \throws ZeroElements If the InputIterator produces zero elements.
    \throws ColumnLabelExists If a column in the DataTable_ already has the 
                              label specified by an entry produced by the 
                              iterator.
    \throws ColumnDoesNotExist If the column index specified by an entry 
                               produced by the iterator does not exist.
    \throws ColumnHasLabel If the column index specified by an entry produced
                           by the iterator already has a label. */
    template<typename InputIt>
    void setColumnLabels(InputIt first, 
    typename std::enable_if<std::is_constructible<ColumnLabels::value_type, 
    typename std::iterator_traits<InputIt>::value_type>::value, 
                         InputIt>::type last) {
        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements."};

        while(first != last) {
            throwIfColumnHasLabel(first->second);
            throwIfColumnLabelExists(first->first);

            col_ind_.emplace(*first);
            ++first;
        }
    }

    /** Label a set of columns at once using an InputIterator that produces one
    label (std::string) at a time. The columns referred to by the iterator must 
    not already have a label. The column labels have to be unique for the entire
    DataTable_. See
    <a href="http://en.cppreference.com/w/cpp/concept/InputIterator">this page
    </a> for details on InputIterator.

    \param first InputIterator representing beginning of the range of input
                 values. Both 'first' and 'last' are of same type -- InputIt
                 (type of the input iterator). Don't be confused by the extra
                 code that appears in function signature for 'last'.
    \param last InputIterator representing end of the range of vallues. Both 
                'first' and 'last' are of same type -- InputIt (type of the 
                input iterator). Don't be confused by the extra code that 
                appears in function signature for 'last'.
    \param startColumnIndex Label the columns starting at the column specified
                            by 'startColumnIndex'.

    \throws ZeroElements If the InputIterator produces zero elements.
    \throws ColumnLabelExists If a column in the DataTable_ already has the 
                              label specified by an entry produced by the 
                              iterator.
    \throws ColumnDoesNotExist If the (1) column index specified by 
                               'startColumnIndex' does not exist OR (2) the 
                               iterator produces more entries than expected in
                               which case it attempts to label a column that 
                               does not exist.
    \throws ColumnHasLabel If the column index specified by an entry produced
                           by the iterator already has a label. */
    template<typename InputIt>
    void setColumnLabels(InputIt first,
     typename std::enable_if<std::is_constructible<std::string,
     typename std::iterator_traits<InputIt>::value_type>::value,
                         InputIt>::type last,
                         size_t startColumnIndex = 0) {
        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements."};

        size_t col{startColumnIndex};
        while(first != last) {
            throwIfColumnHasLabel(col);
            throwIfColumnLabelExists(*first);

            col_ind_.emplace(*first, col);
            ++first; ++col;
        }
    }

    /** Label a set of columns at once using a:
    - sequence container of labels (std::string) OR
    - associative container of label-index pair (std::pair<std::string, 
      std::size_t>). 
    Calling this function is equivalent to:
    \code
    setColumnLabels(container.begin(), container.end());
    \endcode
    See overloads of setColumnLabels() taking InputIterator for exceptions 
    thrown.                                                                   */
    template<typename Container>
    void setColumnLabels(Container container) {
        using ContainerIter = typename Container::iterator;
        using ContainerIterTraits = std::iterator_traits<ContainerIter>;
        using ContainerIterValue = typename ContainerIterTraits::value_type;
        static_assert(std::is_constructible<ColumnLabels::value_type, 
                                            ContainerIterValue>::value ||
                      std::is_constructible<std::string,
                                            ContainerIterValue>::value,
                      "The input container must support an iterator that " 
                      "produces elements of type std::pair<std::string, " 
                      "std::size_t> or of type std::string. See documentation "
                      "for more details.");

        setColumnLabels(container.begin(), container.end());
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
    
    /** Change the label of a column with a new label. Time complexity is linear
    in the number of column labels. The column specified must already have a
    label. Column labels must be unique for the entire DataTable_. To label a 
    column that does not yet have a label, use setColumnLabel().

    \throws ColumnLabelExists If there is already a column with label specified
                              by 'newColumnLabel'.
    \throws ColumnHasNoLabel If the column specified by the column index does 
                             not already have a label.
    \throws ColumnDoesNotExist If the column specified by the column index does 
                               not exist.                                     */
    void changeColumnLabel(size_t columnIndex, 
                           const string& newColumnLabel) {
        const string old_collabel{getColumnLabel(columnIndex)};
        col_ind_.erase(old_collabel);

        throwIfColumnLabelExists(newColumnLabel);

        col_ind_.emplace(newColumnLabel, columnIndex);
    }

    /** Change the label of a column with a new label. Time complexity is 
    constant on average and linear in number of column labels in the worst case.

    \throws ColumnLabelExists If there is already column with the label 
                              specified by 'newColumnLabel'.
    \throws ColumnDoesNotExist If there is no column with the specified label.*/
    void changeColumnLabel(const string& oldColumnLabel, 
                           const string& newColumnLabel) {
        const size_t colind{getColumnIndex(oldColumnLabel)};
        col_ind_.erase(oldColumnLabel);

        throwIfColumnLabelExists(newColumnLabel);

        col_ind_[newColumnLabel] = colind;
    }

    /** Change the labels of a set of columns at once using an InputIterator
    that produces either:
    - A label-index pair (std::pair<std::string, std::size_t>) where label is 
      the new label for the column with given index. OR
    - A new_label-old_label pair (std::pair<std::string, std::string>).     
    See other overloads of changeColumnLabels() for exceptions thrown         */
    template<typename InputIt>
    void changeColumnLabels(InputIt first, InputIt last) {
        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements."};

        while(first != last) {
            changeColumnLabels(first->second, first->first);
            ++first;
        }
    }

    /** Change the labels of a set of columns at once using an associative
    container of:
    - new label to column index where label is of type std::string and index is 
      of type std::size_t.
    - new label to old label where both labels are of type std::string.
    Calling this function is equivalent to:
    \code
    changeColumnLabels(container.begin(), container.end());
    \endcode
    See overload of changeColumnLabels() taking iterators for exceptions
    thrown.                                                                   */
    template<typename Container>
    void changeColumnlabels(Container container) {
        changeColumnLabels(container.begin(), container.end());
    }

    /** Get the index of a column from its label. Time complexity is constant on
    average and linear in number of column labels on worst case.

    \throws ColumnDoesNotExist If the column label does not exist.            */
    size_t getColumnIndex(const string& columnLabel) const {
        throwIfColumnDoesNotExist(columnLabel);

        return col_ind_.at(columnLabel);
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
    /** \cond */
    // Helper function. Check if a column exists and throw an exception if it
    // does not.
    void throwIfColumnDoesNotExist(const size_t columnIndex) const {
        if(!hasColumn(columnIndex)) {
            throw ColumnDoesNotExist{"Column " + std::to_string(columnIndex) + 
                                     " does not exist. Index out of range."};
        }
    }

    // Helper function. Check if a column exists and throw an exception if it
    // does not.
    void throwIfColumnDoesNotExist(const std::string& columnLabel) const {
        if(!hasColumnLabel(columnLabel)) {
            throw ColumnDoesNotExist{"No Column with label '" + columnLabel + 
                                     "'."};
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

    // Helper function. Check if a column label exists and throw an exception if
    // it does.
    void throwIfColumnLabelExists(const std::string& columnLabel) const {
        if(hasColumnLabel(columnLabel))
            throw ColumnLabelExists{"A column with label '" + columnLabel + 
                                    "' already exists. Column labels have to be"
                                    " unique."};
    }

    // Meta-data.
    MetaData     metadata_;
    // Column label to column index.
    ColumnLabels col_ind_;

    /** \endcond */
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
    static_assert(std::is_arithmetic<ET>::value || std::is_class<ET>::value, 
                  "Template parameter ET must be either an arithmetic type " 
                  "(int, float, double etc) or a class/struct type.");
protected:
    template<bool RowOrColIter, bool IsConst>
    class Iterator {
    public:
        using value_type      = std::conditional<RowOrColIter, 
                                                 SimTK::RowVectorView_<ET>,
                                                 SimTK::VectorView_<ET>>;
        using difference_type = int;

        Iterator()                           = delete;
        Iterator(const Iterator&)            = default;
        Iterator(Iterator&&)                 = default;
        Iterator& operator=(const Iterator&) = default;
        Iterator& operator=(Iterator&&)      = default;
        ~Iterator()                          = default;

        Iterator(const DataTable_* dt, size_t index) 
            : dt_{dt}, index_{index} {}

        std::conditional<RowOrColIter, 
                         SimTK::RowVectorView_<ET>,
                         SimTK::VectorView_<ET>>
        operator*() {
            return rowOrCol(index_);
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        bool operator!=(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};
                
            return index_ != rhs.index_;
        }

        bool operator==(const Iterator& rhs) const {
            return !operator==(rhs);
        }

        Iterator& operator+=(int n) {
            index_ += n;
            return *this;
        }

        Iterator operator+(int n) const {
            return Iterator{dt_, index_ + n};
        }

        Iterator& operator-=(int n) {
            return operator+=(-1 * n);
        }

        Iterator operator-(int n) const {
            return operator+(-1 * n);
        }

        int operator-(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};

            return index_ - rhs.index_;
        }

        SimTK::RowVectorView_<ET> operator[](size_t index) const {
            return rowOrCol(index);
        }

        bool operator<(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};

            return index_ < rhs.index_;
        }

        bool operator>(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};
            
            return index_ > rhs.index_;
        }

        bool operator>=(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};
            
            return index_ >= rhs.index_;
        }

        bool operator<=(const Iterator& rhs) const {
            if(dt_ != rhs.dt_)
                throw IncompatibleIterators{"The iterators are for two "
                        "different DataTables."};
            
            return index_ <= rhs.index_;
        }

    private:
        typename std::enable_if<RowOrColIter && IsConst, 
                                SimTK::RowVectorView_<ET>>::type
        rowOrCol(size_t index) const {
            return dt_->getRow(index);
        }
        typename std::enable_if<RowOrColIter && !IsConst, 
                                SimTK::RowVectorView_<ET>>::type
        rowOrCol(size_t index) {
            return dt_->updRow(index);
        }
        typename std::enable_if<!RowOrColIter && IsConst, 
                                SimTK::VectorView_<ET>>::type
        rowOrCol(size_t index, int = 0) const {
            return dt_->getCol(index);
        }
        typename std::enable_if<!RowOrColIter && !IsConst, 
                       SimTK::VectorView_<ET>>::type
        rowOrCol(size_t index, int = 0) {
            return dt_->updCol(index);
        }

        DataTable_* const dt_;
        size_t index_;
    };

    template<bool RowsOrColsContainer, bool IsConst>
    class RowsColsContainerProxy {
    public:
        RowsColsContainerProxy(DataTable_* dt) : dt_{dt} {}
        RowsColsContainerProxy()                                    = delete;
        RowsColsContainerProxy(const RowsColsContainerProxy&)       = default;
        RowsColsContainerProxy(RowsColsContainerProxy&&)            = default;
        RowsColsContainerProxy& operator=(const RowsColsContainerProxy&)
                                                                    = default;
        RowsColsContainerProxy& operator=(RowsColsContainerProxy&&) = default;

        Iterator<RowsOrColsContainer, IsConst> begin() {
            return Iterator<RowsOrColsContainer, IsConst>{dt_, 0};
        }

        Iterator<RowsOrColsContainer, IsConst> end() {
            return Iterator<RowsOrColsContainer, IsConst>{dt_, size()};
        }

    private:
        typename std::enable_if<RowsOrColsContainer, size_t>::type 
        size() {
            dt_->getNumRows();
        }
        typename std::enable_if<!RowsOrColsContainer, size_t>::type 
        size(int = 0) {
            dt_->getNumColumns();
        }

        DataTable_* dt_;
    };

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
               size_t numEntriesInMajor,
               TraverseDir dimension = TraverseDir::RowMajor,
               bool allowMissing     = false,
               size_t numMajors      = 0) {
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<ET, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter ET used to instantiate DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements."};

        if(numEntriesInMajor == 0)
            throw InvalidEntry{"Input argument 'numEntriesInMajor' is required "
                    " cannot be zero."};

        // Optimization. If numMajors is specified, pre-size the data and 
        // avoid having to resize it multiple times later.
        if(numMajors != 0) {
            if(dimension == TraverseDir::RowMajor)
                data_.resize(static_cast<int>(numMajors), 
                             static_cast<int>(numEntriesInMajor));
            else if(dimension == TraverseDir::ColumnMajor)
                data_.resize(static_cast<int>(numEntriesInMajor), 
                             static_cast<int>(numMajors));
        } else {
            if(dimension == TraverseDir::RowMajor)
                data_.resize(1, static_cast<int>(numEntriesInMajor));
            else if(dimension == TraverseDir::ColumnMajor)
                data_.resize(static_cast<int>(numEntriesInMajor), 1);
        }
            
        int row{0};
        int col{0};
        while(first != last) {
            data_.set(row, col, *first);
            ++first;
            if(dimension == TraverseDir::RowMajor) {
                ++col;
                if(col == static_cast<int>(numEntriesInMajor) && 
                   first != last) {
                    col = 0;
                    ++row;
                    if(numMajors == 0)
                        data_.resizeKeep(data_.nrow() + 1, data_.ncol());
                    else if(row == static_cast<int>(numMajors))
                        throw TooManyElements{"Input iterator produced more "
                                "elements than needed to fill " + 
                                std::to_string(numMajors) + " (numMajors) " 
                                "rows."};
                }
            } else {
                ++row;
                if(row == static_cast<int>(numEntriesInMajor) && 
                   first != last) {
                    row = 0;
                    ++col;
                    if(numMajors == 0)
                        data_.resizeKeep(data_.nrow(), data_.ncol() + 1);
                    else if(col == static_cast<int>(numMajors))
                        throw TooManyElements{"Input iterator produced more "
                                "elements than need to fill " + 
                                std::to_string(numMajors) + " (numMajors) "
                                "columns."};
                }
            }
        }

        if(!allowMissing) {
            if(dimension == TraverseDir::RowMajor) {
                if(numMajors != 0 && row != data_.nrow() - 1)
                    throw NotEnoughElements{"Input iterator did not produce "
                            "enough elements to fill all the rows. Total rows ="
                            " " + std::to_string(data_.nrow()) + " Filled rows "
                            "= " + std::to_string(row) + "."};
                if(col != data_.ncol())
                    throw NotEnoughElements{"Input iterator did not produce " 
                            "enough elements to fill the last row. Expected = " 
                            + std::to_string(data_.ncol()) + ", Received = " + 
                            std::to_string(col)};
            } else if(dimension == TraverseDir::ColumnMajor) {
                if(numMajors != 0 && col != data_.ncol() - 1)
                    throw NotEnoughElements{"Input iterator did not produce "
                            "enough elements to fill all the columns. Total "
                            "columns = " + std::to_string(data_.ncol()) + 
                            " Filled columns = " + std::to_string(col) + "."};
                if(row != data_.nrow())
                    throw NotEnoughElements{"Input iterator did not produce "
                            "enough elements to fill the last column. Expected "
                            "= " +  std::to_string(data_.nrow()) + ", "
                            "Received = " + std::to_string(row)};
            }
        }
    }

    template<typename Container>
    DataTable_(const Container& container,
               size_t numEntriesInMajor,
               TraverseDir dimension = TraverseDir::RowMajor,
               bool allowMissing     = false,
               size_t numMajors      = 0) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        DataTable__impl(container, 
                        numEntriesInMajor, 
                        dimension, 
                        allowMissing, 
                        numMajors);
    }
    
    /**@}*/
    /** \name Copy.
        Copy operations including copy constructor.                           */
    /**@{*/

    /** Copy constructor.                                                     */
    DataTable_(const DataTable_&) = default;

    /** Virtual copy constructor.                                             */
    std::unique_ptr<AbstractDataTable> clone() const override {
        return std::unique_ptr<AbstractDataTable>(new DataTable_{*this});
    }

    /** Copy assignment                                                       */
    DataTable_& operator=(const DataTable_&) = default;

    /**@}*/
    /** \name Move.
        Move operations.                                                      */
    /**@{*/

    /** Move constructor.                                                     */
    DataTable_(DataTable_&&) = default;

    /** Move assignment                                                       */
    DataTable_& operator=(DataTable_&&) = default;

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
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<ET, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter ET used to instantiate DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

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

    template<typename Container>
    void addRow(const Container& container, 
                size_t numColumnsHint = 2,
                bool allowMissing = false) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        addRow_impl(container, numColumnsHint, allowMissing);
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
                 size_t numColumns = 0,
                 bool allowMissing = false,
                 size_t numRows    = 0) {
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<ET, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter ET used to instantiate DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if(data_.nrow() == 0 || data_.ncol() == 0) {
            if(numColumns == 0)
                throw InvalidEntry{"DataTable is empty. In order to add rows, "
                        "argument 'numColumns' must be provided and it cannot "
                        "be zero."};
        } else {
            if(numColumns != 0 &&
               static_cast<int>(numColumns) != data_.ncol())
                throw InvalidEntry{"DataTable has " + 
                        std::to_string(data_.nrow()) + " rows and " + 
                        std::to_string(data_.ncol()) + " columns. Argument " 
                        "'numColumns' must be either zero or equal to actual "
                        "number of columns. It is ignored either way but this"
                        " is just to prevent logical errors in the code."};
        }

        int row{0};
        int col{0};
        if(data_.nrow() == 0 || data_.ncol() == 0) {
            data_.resize(static_cast<int>(numRows ? numRows : 1), 
                         static_cast<int>(numColumns));
        } else {
            row = data_.nrow();
            data_.resizeKeep(data_.nrow() + 
                             static_cast<int>(numRows ? numRows : 1), 
                             data_.ncol());
        }

        while(first != last) {
            data_.set(row, col, *first);
            ++first; ++col;
            if(col == static_cast<int>(data_.ncol()) && first != last) {
                col = 0;
                ++row;
                if(numRows == 0)
                    data_.resizeKeep(data_.nrow() + 1, data_.ncol());
                else if(row == static_cast<int>(numRows))
                    throw TooManyElements{"Input iterator produced more "
                            "elements than needed to fill " + 
                            std::to_string(numRows) + " (numRows) rows."};
            }
        }

        if(!allowMissing) { 
            if(row != data_.nrow() - 1)
                throw NotEnoughElements{"Input iterator did not produce "
                        "enough elements to fill all the rows. Total rows = " +
                        std::to_string(data_.nrow()) + ", Filled rows = " +
                        std::to_string(row) + "."};
            if(col != data_.ncol())
                throw NotEnoughElements{"Input iterator did not produce enough" 
                        " elements to fill the last row. Expected = " + 
                        std::to_string(data_.ncol()) + ", Received = " + 
                        std::to_string(col) + "."};
        }
    }

    template<typename Container>
    void addRows(const Container& container,
                 size_t numColumns = 0,
                 bool allowMissing = false,
                 size_t numRows    = 0) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        addRows_impl(container, numColumns, allowMissing, numRows);
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
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<ET, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter ET used to instantiate DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

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

    template<typename Container>
    void addColumn(const Container& container, 
                   size_t numRowsHint = 2,
                   bool allowMissing = false) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        addColumn_impl(container, numRowsHint, allowMissing);
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
                    size_t numRows    = 0,
                    bool allowMissing = false,
                    size_t numColumns = 0) {
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<ET, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter ET used to instantiate DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

        if(first == last)
            throw ZeroElements{"Input iterators produce zero elements."};
        if(data_.nrow() == 0 || data_.ncol() == 0) {
           if(numRows == 0)
               throw InvalidEntry{"DataTable is empty. In order to add columns,"
                       " argument 'numRows' must be provided and it cannot be "
                       "zero."};
        } else {
            if(numRows != 0 &&
               static_cast<int>(numRows) != data_.nrow())
                throw InvalidEntry{"DataTable has " + 
                        std::to_string(data_.nrow()) + " rows and " +
                        std::to_string(data_.ncol()) + " columns. Argument "
                        "'numRows' must be either zero or equal to actual " 
                        "number of rows. It is ignored either way but this is"
                        " just to prevent logical errors in the code."};
        }

        int row{0};
        int col{0};
        if(data_.nrow() == 0 || data_.ncol() == 0) {
            data_.resize(static_cast<int>(numRows), 
                         static_cast<int>(numColumns ? numColumns : 1));
        } else {
            col = data_.ncol();
            data_.resizeKeep(data_.nrow(), 
                             data_.ncol() + 
                             static_cast<int>(numColumns ? numColumns : 1));
        }

        while(first != last) {
            data_.set(row, col, *first);
            ++first; ++row;
            if(row == static_cast<int>(data_.nrow()) && first != last) {
                row = 0;
                ++col;
                if(numColumns == 0)
                    data_.resizeKeep(data_.nrow(), data_.ncol() + 1);
                else if(col == static_cast<int>(numColumns))
                    throw TooManyElements{"Input iterator produced more "
                            "elements than needed to fill " +
                            std::to_string(numColumns) + " (numColumns) "
                            "columns"};
            }
        }
        if(!allowMissing) {
            if(col != data_.ncol() - 1)
                throw NotEnoughElements{"Input iterator did not produce "
                        "enough elements to fill all the columns. Total columns"
                        " = " + std::to_string(data_.ncol()) + ", Filled "
                        "columns = " + std::to_string(col) + "."};
            if(row != data_.nrow())
                throw NotEnoughElements{"Input iterator did not produce enough" 
                        " elements to fill the last column. Expected = " + 
                        std::to_string(data_.nrow()) + ", Received = " + 
                        std::to_string(row) + "."};
        }
    }

    template<typename Container>
    void addColumns(const Container& container,
                    size_t numRows    = 0,
                    bool allowMissing = false,
                    size_t numColumns = 0) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        addColumns_impl(container, numRows, allowMissing, numColumns);
    }

    Iterator<true, true> rowsCBegin() const {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<true, true>{this, 0};
    }

    Iterator<true, true> rowsCEnd() const {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<true, true>{this, data_.nrow()};
    }

    Iterator<true, true> rowsBegin() const {
        return rowsCBegin();
    }

    Iterator<true, true> rowsEnd() const {
        return rowsCEnd();
    }

    Iterator<true, false> rowsBegin() {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<true, false>{this, 0};
    }

    Iterator<true, false> rowsEnd() {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<true, false>{this, data_.nrow()};
    }

    Iterator<false, true> columnsCBegin() const {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<false, true>{this, 0};
    }

    Iterator<false, true> columnsCEnd() const {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<false, true>{this, data_.ncol()};
    }

    Iterator<false, true> columnsBegin() const {
        return columnsCBegin();
    }

    Iterator<false, true> columnsEnd() const {
        return columnsCEnd();
    }

    Iterator<false, false> columnsBegin() {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<false, false>{this, 0};
    }

    Iterator<false, false> columnsEnd() {
        if(data_.nrow() == 0 || data_.ncol() == 0)
            throw EmptyDataTable{"DataTable is empty."};

        return Iterator<false, false>{this, data_.ncol()};
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

protected:
    template<typename Container, 
             typename = decltype(std::declval<Container>().size())>
    void DataTable__impl(const Container& container,
                         size_t numEntriesInMajor,
                         TraverseDir dimension,
                         bool allowMissing,
                         size_t numMajors) {
        if(numMajors == 0) {
            auto res = std::div(container.size(), numEntriesInMajor);

            if(!allowMissing && res.rem != 0) {
                if(dimension == TraverseDir::RowMajor)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add full rows. Last "
                            "row received " + std::string(res.rem) + 
                            " elements. Expected " + 
                            std::to_string(numEntriesInMajor) + " elements " 
                            "(numEntriesInMajor). Missing values are not " 
                            "allowed (allowMissing)."};
                else if(dimension == TraverseDir::ColumnMajor)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add full columns. Last"
                            " column received " + std::to_string(res.rem) +
                            " elements. Expected " + 
                            std::to_string(numEntriesInMajor) + " elements " 
                            "(numEntriesInMajor). Missing values are not " 
                            "allowed (allowMissing)."};
            }

            numMajors = res.rem == 0 ? res.quot : res.quot + 1;
        } else {
            if(numMajors * numEntriesInMajor < container.size()) {
                if(dimension == TraverseDir::RowMajor)
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numMajors) + 
                            " rows (numMajors) with "
                            + std::to_string(numEntriesInMajor) + " columns "
                            "(numEntriesInMajor). Expected = " + 
                            std::to_string(numMajors * numEntriesInMajor) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
                else if(dimension == TraverseDir::ColumnMajor)
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numMajors) + 
                            " columns (numMajors) with " + 
                            std::to_string(numEntriesInMajor) + 
                            " rows (numEntriesInMajor). Expected = " + 
                            std::to_string(numMajors * numEntriesInMajor) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
            } else if(numMajors * numEntriesInMajor > container.size() &&
                      !allowMissing) {
                if(dimension == TraverseDir::RowMajor)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numMajors) + 
                            " rows (numMajors) with " +
                            std::to_string(numEntriesInMajor) + " columns " 
                            "(numEntriesInMajor). Expected = " + 
                            std::to_string(numMajors * numEntriesInMajor) +
                            " elements. Received = " + 
                            std::to_string(container.size()) + " elements."};
                if(dimension == TraverseDir::ColumnMajor)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numMajors) + 
                            " columns (numMajors) with " + 
                            std::to_string(numEntriesInMajor) + " rows " 
                            "(numEntriesInMajor). Expected = " + 
                            std::to_string(numMajors * numEntriesInMajor) +
                            " elements. Received = " + 
                            std::to_string(container.size()) + " elements."};
                
            }
        }

        DataTable_(container.begin(),
                   container.end(),
                   numEntriesInMajor,
                   dimension,
                   allowMissing,
                   numMajors);
    }
    template<typename Container>
    void DataTable__impl(const Container& container,
                         size_t numEntriesInMajor,
                         TraverseDir dimension,
                         unsigned allowMissing,
                         size_t numMajors) {
        DataTable_(container.begin(),
                   container.end(),
                   numEntriesInMajor,
                   dimension,
                   allowMissing,
                   numMajors);
    }

    template<typename Container>
    void addRow_impl(const Container& container, 
                     decltype(std::declval<Container>().size()),
                     bool allowMissing) {
        addRow(container.begin(), 
               container.end(), 
               container.size(), 
               allowMissing);
    }
    template<typename Container>
    void addRow_impl(const Container& container,
                     size_t numColumnsHint,
                     unsigned allowMissing) {
        addRow(container.begin(),
               container.end(),
               numColumnsHint,
               allowMissing);
    }

    template<typename Container,
             typename = decltype(std::declval<Container>().size())>
    void addRows_impl(const Container& container,
                      size_t numColumns,
                      bool allowMissing,
                      size_t numRows) {
        if(data_.nrow() == 0 || data_.ncol() == 0) {
            if(numColumns == 0)
                throw InvalidEntry{"DataTable is empty. Argument 'numColumns' "
                        "must be provided and it cannot be zero."};
            if(numRows == 0) {
                auto res = std::div(container.size(), numColumns);

                if(!allowMissing && res.rem != 0)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements add full rows. Last "
                            "row received " + std::string(res.rem) + 
                            " elements. Expected " + 
                            std::to_string(numColumns) + " elements " 
                            "(numColumns). Missing values are not " 
                            "allowed (allowMissing)."};
                
                numRows = res.rem == 0 ? res.quot : res.quot + 1;
            } else {
                if(numRows * numColumns < container.size())
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numRows) + 
                            " rows (numRows) with "
                            + std::to_string(numColumns) + " columns "
                            "(numColumns). Expected = " + 
                            std::to_string(numRows * numColumns) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
                if(numRows * numColumns > container.size() && !allowMissing)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numRows) + 
                            " rows (numRows) with " + std::to_string(numColumns)
                            + " columns (numColumns). Expected = " + 
                            std::to_string(numRows * numColumns) +
                            " elements. Received = " + 
                            std::to_string(container.size()) + " elements."};
            }
        } else {
            if(numRows == 0) {
                auto res = std::div(container.size(), data_.ncol());
                
                if(!allowMissing && res.rem != 0)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add full rows. Last "
                            "row received " + std::string(res.rem) + 
                            " elements. Expected " + 
                            std::to_string(data_.ncol()) + " elements " 
                            "(getNumColumns()). Missing values are not " 
                            "allowed (allowMissing)."};

                numRows = res.rem = 0 ? res.quot : res.quot + 1;
            } else {
                if(numRows * data_.ncol() < container.size())
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numRows) + 
                            " rows (numRows) with " + 
                            std::to_string(data_.ncol()) + " columns "
                            "(getNumColumns()). Expected = " + 
                            std::to_string(numRows * data_.ncol()) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
                if(numRows * data_.ncol() > container.size() && !allowMissing)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numRows) + 
                            " rows (numRows) with " + 
                            std::to_string(data_.ncol()) +
                            " columns (numColumns). Expected = " + 
                            std::to_string(numRows * data_.ncol()) +
                            " elements. Received = " + 
                            std::to_string(container.size()) + " elements."};
            }
        }

        addRows(container.begin(),
                container.end(),
                numColumns,
                allowMissing,
                numRows);
    }
    template<typename Container>
    void addRows_impl(const Container& container,
                      size_t numColumns,
                      unsigned allowMissing,
                      size_t numRows) {
        addRows(container.begin(),
                container.end(),
                numColumns,
                allowMissing,
                numRows);
    }

    template<typename Container>
    void addColumn_impl(const Container& container, 
                        decltype(std::declval<Container>().size()),
                        bool allowMissing) {
        addColumn(container.begin(), 
                  container.end(), 
                  container.size(), 
                  allowMissing);
    }
    template<typename Container>
    void addColumn_impl(const Container& container,
                        size_t numColumnsHint,
                        unsigned allowMissing) {
        addColumn(container.begin(),
                  container.end(),
                  numColumnsHint,
                  allowMissing);
    }

    template<typename Container,
             typename = decltype(std::declval<Container>().size())>
    void addColumns_impl(const Container& container,
                         size_t numRows,
                         bool allowMissing,
                         size_t numColumns) {
        if(data_.nrow() == 0 || data_.ncol() == 0) {
            if(numRows == 0)
                throw InvalidEntry{"DataTable is empty. Argument 'numRows' "
                        "must be provided and it cannot be zero."};
            if(numColumns == 0) {
                auto res = std::div(container.size(), numRows);

                if(!allowMissing && res.rem != 0)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements add full columns. Last "
                            "column received " + std::string(res.rem) + 
                            " elements. Expected " + 
                            std::to_string(numRows) + " elements " 
                            "(numRows). Missing values are not " 
                            "allowed (allowMissing)."};
                
                numColumns = res.rem == 0 ? res.quot : res.quot + 1;
            } else {
                if(numRows * numColumns < container.size())
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numColumns) + 
                            " columns (numColumns) with "
                            + std::to_string(numRows) + " rows "
                            "(numRows). Expected = " + 
                            std::to_string(numRows * numColumns) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
                if(numRows * numColumns > container.size() && !allowMissing)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numColumns) + 
                            " columns (numColumns) with " + 
                            std::to_string(numRows) + " rows (numRows). "
                            "Expected = " + std::to_string(numRows * numColumns)
                            + " elements, Received = " + 
                            std::to_string(container.size()) + " elements."};
            }
        } else {
            if(numColumns == 0) {
                auto res = std::div(container.size(), data_.ncol());
                
                if(!allowMissing && res.rem != 0)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add full columns. Last "
                            "column received " + std::string(res.rem) + 
                            " elements. Expected " + 
                            std::to_string(data_.nrow()) + " elements " 
                            "(getNumRows()). Missing values are not " 
                            "allowed (allowMissing)."};

                numColumns = res.rem = 0 ? res.quot : res.quot + 1;
            } else {
                if(data_.nrow() * numColumns < container.size())
                    throw TooManyElements{"The container has more elements than"
                            " needed to add " + std::to_string(numColumns) + 
                            " columns (numColumns) with " + 
                            std::to_string(data_.nrow()) + " rows "
                            "(getNumRows()). Expected = " + 
                            std::to_string(data_.nrow() * numColumns) + 
                            " elements,  Received = " + 
                            std::to_string(container.size()) + " elements."};
                if(data_.nrow() * numColumns > container.size() && 
                   !allowMissing)
                    throw NotEnoughElements{"The container does not have enough"
                            " elements to add " + std::to_string(numColumns) + 
                            " columns (numColumns) with " + 
                            std::to_string(data_.nrow()) +
                            " rows (numRows). Expected = " + 
                            std::to_string(data_.nrow() * numColumns) +
                            " elements. Received = " + 
                            std::to_string(container.size()) + " elements."};
            }
        }

        addColumns(container.begin(),
                   container.end(),
                   numRows,
                   allowMissing,
                   numColumns);
    }
    template<typename Container>
    void addColumns_impl(const Container& container,
                         size_t numRows,
                         unsigned allowMissing,
                         size_t numColumns) {
        addColumns(container.begin(),
                   container.end(),
                   numRows,
                   allowMissing,
                   numColumns);
    }

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


class TimestampsEmpty : public OpenSim::Exception {
public:
    TimestampsEmpty(const std::string& expl) : Exception(expl) {}
};


class DataHasZeroRows : public OpenSim::Exception {
public:
    DataHasZeroRows(const std::string& expl) : Exception(expl) {}
};


class TimestampsLengthIncorrect : public OpenSim::Exception {
public:
    TimestampsLengthIncorrect(const std::string& expl) : Exception(expl) {}
};


class TimestampDoesNotExist : public OpenSim::Exception {
public:
    TimestampDoesNotExist(const std::string& expl) : Exception(expl) {}
};


class TimestampBreaksInvariant : public OpenSim::Exception {
public:
    TimestampBreaksInvariant(const std::string& expl) : Exception(expl) {}
};

class TimestampsColumnFull : public OpenSim::Exception {
public:
    TimestampsColumnFull(const std::string& expl) : Exception(expl) {}
};


enum class NearestDir {
    LessOrGreaterThanEqual,
    LessThanEqual,
    GreaterThanEqual
};


template<typename ET = SimTK::Real, typename TS = float>
class TimeSeriesDataTable_ : public DataTable_<ET> {
    static_assert(std::is_arithmetic<TS>::value, "Template argument 'TS' "
                  "representing timestamp must be an arithmetic type (eg. int, "
                  "float, double etc.).");

protected:
    using string = std::string;
    using Timestamps = std::vector<TS>;
    using TimestampsIter = typename Timestamps::iterator;
    using TimestampsConstIter = typename Timestamps::const_iterator;

    class TimestampsContainerProxy {
    public:
        TimestampsContainerProxy(const TimeSeriesDataTable_* tsdt) : 
            tsdt_{tsdt} {}
        TimestampsContainerProxy()                                    = delete;
        TimestampsContainerProxy(const TimestampsContainerProxy&)     = default;
        TimestampsContainerProxy(TimestampsContainerProxy&&)          = default;
        TimestampsContainerProxy& operator=(const TimestampsContainerProxy&)
                                                                      = default;
        TimestampsContainerProxy& operator=(TimestampsContainerProxy&&) 
                                                                      = default;

        TimestampsConstIter cbegin() const {
            return tsdt_->timestampsBegin();
        }

        TimestampsConstIter cend() const {
            return tsdt_->timestampsEnd();
        }

        TimestampsConstIter begin() const {
            return cbegin();
        }

        TimestampsConstIter end() const {
            return cend();
        }

    private:
        const TimeSeriesDataTable_* tsdt_;
    };

public:
    using timestamp_type = TS;

    using DataTable_<ET>::DataTable_;

    TimeSeriesDataTable_& operator=(const TimeSeriesDataTable_&) = default;
    TimeSeriesDataTable_& operator=(TimeSeriesDataTable_&&) = default;

    ~TimeSeriesDataTable_() override = default;

    template<typename Timestamp>
    bool hasTimestamp(Timestamp timestamp) const {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();

        return std::binary_search(timestamps_.cbegin(), 
                                  timestamps_.cend(), 
                                  timestamp);
    }

    void addTimestamp(TS timestamp) {
        throwIfDataHasZeroRows();
        throwIfTimestampsFull();
        throwIfTimestampBreaksInvariantPrev(timestamps_.size(), timestamp);

        timestamps_.push_back(timestamp);
    }

    template<typename InputIt>
    void addTimestamps(InputIt first, 
                       InputIt last) {
        {
        using namespace internal;
        static_assert(is_dereferencable<InputIt>, "Input iterator (InputIt) is "
                      "not dereferencable. It does not support 'operator*()'.");

        static_assert(std::is_constructible<TS, decltype(*first)>::value, 
                      "The type of the value produced by dereferencing the "
                      "input iterator (InputIt) does not match template "
                      "parameter TS (timestamp) used to instantiate "
                      "DataTable.");

        static_assert(is_eq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator==' and so is not comparable for " 
                      "equality.");

        static_assert(is_neq_comparable<InputIt>, "Input iterator does not " 
                      "support 'operator!=' and so is not comparable for " 
                      "inequality.");
        }

        if(first == last)
            throw ZeroElements{"Input iterator produced zero elements"};

        throwIfDataHasZeroRows();

        while(first != last) {
            throwIfTimestampsFull();
            throwIfTimestampBreaksInvariantPrev(timestamps_.size(), *first);

            timestamps_.push_back(*first);
            ++first;
        }
    }

    template<typename Container>
    void addTimestamps(const Container& container) {
        {
        using namespace internal;
        static_assert(has_mem_begin<Container>, "Input container does not have "
                      "a member function named begin(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(has_mem_end<Container>, "Input container does not have "
                      "a member function named end(). Input container is " 
                      "required to have members begin() and end() that return " 
                      "an iterator to the container.");

        static_assert(std::is_same<decltype(container.begin()),
                                   decltype(container.end())>::value,
                      "The member functions begin() and end() of input " 
                      "container do not produce the same type. Input container "
                      "is reuiqred to have members begin() and end() that " 
                      "return an iterator to the container.");
        }

        addTimestamps(container.begin(), container.end());
    }

    template<typename... ArgsToAddRow>
    void addTimestampAndRow(TS timestamp, ArgsToAddRow&&... argsToAddRow) {
        this-addRow(std::forward<ArgsToAddRow>(argsToAddRow)...);
        addTimestamp(timestamp);
    }

    template<typename TimestampInputIt, typename... ArgsToAddRows>
    void addTimestampsAndRows(TimestampInputIt timestampFirst,
                              TimestampInputIt timestampLast,
                              ArgsToAddRows&&... argsToAddRows) {
        this->addRows(std::forward<ArgsToAddRows>(argsToAddRows)...);
        addTimestamps(timestampFirst, timestampLast);
    }

    template<typename TimestampContainer, typename... ArgsToAddRows>
    void addTimestampsAndRows(const TimestampContainer& timestampContainer,
                              ArgsToAddRows&&... argsToAddRows) {
        this->addRows(std::forward<ArgsToAddRows>(argsToAddRows)...);
        addTimestamps(timestampContainer);
    }

    TS getTimestamp(size_t rowIndex) const {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();
        this->throwIfRowDoesNotExist(rowIndex);

        return timestamps_[rowIndex];
    }

    TS getTimestamp(TS timestamp, NearestDir direction) {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();

        auto geq_iter = std::lower_bound(timestamps_.cbegin(), 
                                         timestamps_.cend(),
                                         timestamp);

        if(direction == NearestDir::LessOrGreaterThanEqual) {
            if(geq_iter == timestamps_.cend())
                return timestamps_.back();

            if(*geq_iter == timestamp)
                return timestamp;

            if(geq_iter == timestamps_.cbegin())
                return timestamps_.front();

            if(*geq_iter -  timestamp <= timestamp - *(geq_iter - 1))
                return *geq_iter;
            else
                return *(--geq_iter);
        } else if(direction == NearestDir::LessThanEqual) {
            if(geq_iter == timestamps_.cend())
                return timestamps_.back();

            if(*geq_iter == timestamp)
                return timestamp;

            if(geq_iter == timestamps_.cbegin())
                throw TimestampDoesNotExist{"There is no timestamp less-than/"
                        "equal-to " + std::to_string(timestamp) + "."};

            return *(--geq_iter);
        } else if(direction == NearestDir::GreaterThanEqual) {
            if(geq_iter == timestamps_.cend())
                throw TimestampDoesNotExist{"There is no timestamp " 
                        "greater-than/equal-to " + std::to_string(timestamp) + 
                        "."};

            return *geq_iter;
        }
    }

    TimestampsContainerProxy getTimestamps() const {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();

        return TimestampsContainerProxy{this};
    }

    void changeTimestampOfRow(size_t rowIndex, TS newTimestamp) {
        throwIfDataHasZeroRows();
        this->throwIfRowDoesNotExist(rowIndex);
        throwIfIndexExceedsTimestampLength(rowIndex);
        throwIfBreaksInvariantPrev(rowIndex, newTimestamp);
        throwIfBreaksInvariantNext(rowIndex, newTimestamp);

        timestamps_[rowIndex] = newTimestamp;
    }

    void changeTimestamp(TS oldTimestamp, TS newTimestamp) {
        throwIfDataHasZeroRows();
        throwIfTimestampsEmpty();

        auto iter = std::lower_bound(timestamps_.begin(), 
                                     timestamps_.end(),
                                     oldTimestamp);

        if(*iter != oldTimestamp)
            throw TimestampDoesNotExist{"Timestamp '" + 
                    std::to_string(oldTimestamp) + "' does not exist."};
            

        throwifBreaksInvariantPrev(iter - timestamps_.cbegin(), newTimestamp);
        throwifBreaksInvariantNext(iter - timestamps_.cbegin(), newTimestamp);

        *iter = newTimestamp;
    }

    template<typename InputIt>
    void changeTimestamps(size_t startAtRow, InputIt first, InputIt last) {
        using IterValue = typename std::iterator_traits<InputIt>::value_type;
        static_assert(std::is_constructible<TS, IterValue>::value,
                      "Input iterator must produce values of type that is "
                      "convertible to type of timestamp column (template " 
                      "parameter 'TS').");
        
        throwIfDataHasZeroRows();

        size_t rowIndex{startAtRow};
        while(first != last) {
            this->throwIfRowDoesNotExist(rowIndex);
            throwIfIndexExceedsTimestampLength(rowIndex);
            throwIfBreaksInvariantPrev(rowIndex, *first);
            throwIfBreaksInvariantNext(rowIndex, *first);
            
            timestamps_[rowIndex] = *first;

            ++first;
        }
    }

    template<typename Container>
    void changeTimestamps(Container container) {
        using ContIter = typename Container::iterator;
        using IterValue = typename std::iterator_traits<ContIter>::value_type;
        static_assert(std::is_constructible<TS, IterValue>::value,
                      "Input container must support an iterator that produces " 
                      "values of a type that is convertible to type of " 
                      "timestamp column (template parameter 'TS').");

        changeTimestamps(container.begin(), container.end());
    }

    size_t getRowIndex(TS timestamp) const {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();

        auto iter = std::lower_bound(timestamps_.cbegin(), 
                                     timestamps_.cend(), 
                                     timestamp);

        if(*iter != timestamp)
            throw TimestampDoesNotExist{"Timestamp '" + 
                    std::to_string(timestamp) + "' does not exist."};
            
        return iter - timestamps_.cbegin();
    }

    size_t getRowIndex(TS timestamp, NearestDir direction) const {
        throwIfDataHasZeroRows();
        throwIfTimestampsLengthIncorrect();

        auto geq_iter = std::lower_bound(timestamps_.cbegin(),
                                         timestamps_.cend(),
                                         timestamp);

        if(direction == NearestDir::LessOrGreaterThanEqual) {
            if(geq_iter == timestamps_.cend())
                return timestamps_.size() - 1;
            
            if(*geq_iter == timestamp)
                return geq_iter - timestamps_.cbegin();

            if(geq_iter == timestamps_.cbegin())
                return 0;

            if(*geq_iter - timestamp <= timestamp - *(geq_iter - 1))
                return geq_iter - timestamps_.cbegin();
            else
                return geq_iter - timestamps_.cbegin() - 1;
        } else if(direction == NearestDir::LessThanEqual) {
            if(geq_iter == timestamps_.cend())
                return timestamps_.size() - 1;

            if(*geq_iter == timestamp)
                return geq_iter - timestamps_.cbegin();

            if(geq_iter == timestamps_.cbegin())
                throw TimestampDoesNotExist{"There is no timestamp less-than/"
                        "equal-to " + std::to_string(timestamp) + "."};

            return geq_iter - timestamps_.cbegin() - 1;
        } else if(direction == NearestDir::GreaterThanEqual) {
            if(geq_iter == timestamps_.cend())
                throw TimestampDoesNotExist{"There is no timestamp "
                        "greater-than/equal-to " + std::to_string(timestamp) + 
                        "."};

            return geq_iter - timestamps_.cbegin();
        }
    }

    SimTK::RowVectorView_<ET> getRowOfTimestamp(TS timestamp) const {
        return this->getRow(getRowIndex(timestamp));
    }

    SimTK::RowVectorView_<ET> getRowOfTimestamp(TS timestamp, 
                                                NearestDir direction) const {
        return this->getRow(getRowIndex(timestamp, direction));
    }

    SimTK::RowVectorView_<ET> updRowOfTimestamp(TS timestamp) {
        return this->updRow(getRowIndex(timestamp));
    }

    SimTK::RowVectorView_<ET> updRowOfTimestamp(TS timestamp, 
                                                NearestDir direction) {
        return this->updRow(getRowIndex(timestamp, direction));
    }

    const ET& getEltOfTimestamp(TS timestamp, size_t columnIndex) const {
        this->getElt(getRowIndex(timestamp), columnIndex);
    }

    const ET& getEltOfTimestamp(TS timestamp, const string& columnLabel) const {
        this->getElt(getRowIndex(timestamp), columnLabel);
    }

    const ET& getEltOfTimestamp(TS timestamp, 
                                size_t columnIndex, 
                                NearestDir direction) const {
        this->getElt(getRowIndex(timestamp, direction), columnIndex);
    }

    const ET& getEltOfTimestamp(TS timestamp,
                                const string& columnLabel,
                                NearestDir direction) const {
        this->getElt(getRowIndex(timestamp, direction), columnLabel);
    }

    ET& updEltOfTimestamp(TS timestamp, size_t columnIndex) {
        this->updElt(getRowIndex(timestamp), columnIndex);
    }

    ET& updEltOfTimestamp(TS timestamp, const string& columnLabel) {
        this->updElt(getRowIndex(timestamp), columnLabel);
    }

    ET& updEltOfTimestamp(TS timestamp, 
                          size_t columnIndex, 
                          NearestDir direction) {
        this->updElt(getRowIndex(timestamp, direction), columnIndex);
    }

    ET& updEltOfTimestamp(TS timestamp,
                          const string& columnLabel,
                          NearestDir direction) {
        this->updElt(getRowIndex(timestamp, direction), columnLabel);
    }

    TimestampsConstIter timestampsBegin() const {
        return timestamps_.cbegin();
    }

    TimestampsConstIter timestampsEnd() const {
        return timestamps_.cend();
    }


protected:
    void throwIfTimestampsEmpty() {
        if(timestamps_.empty())
            throw TimestampsEmpty{"Timestamp column is empty. Use setTimestamps"
                                  "() to set the timestamp column."};
    }

    void throwIfDataHasZeroRows() {
        if(this->getNumRows() == 0)
            throw DataHasZeroRows{"DataTable currently has zero rows. There " 
                                  "can be no timestamps without data."};
    }

    void throwIfTimestampsLengthIncorrect() {
        if(this->getNumRows() != timestamps_.size())
            throw TimestampsLengthIncorrect{"Timestamp column length (" + 
                    std::to_string(timestamps_.size()) + ") does not match the "
                    "number of rows (" + std::to_string(this->getNumRows()) + 
                    ") in the DataTable. Add timestamps to fix it."};
    }

    void throwIfTimestampBreaksInvariantPrev(const size_t rowIndex, 
                                     const TS newTimestamp) {
        if(rowIndex > 0 && 
           timestamps_[rowIndex - 1] >= newTimestamp)
            throw TimestampBreaksInvariant{"The input timestamp '" + 
                    std::to_string(newTimestamp) + "' at row " + 
                    std::to_string(rowIndex) + 
                    " is less-than/equal-to previous timestamp '" + 
                    std::to_string(timestamps_[rowIndex - 1]) + "' at row " + 
                    std::to_string(rowIndex - 1) + " and so breaks the " 
                    "invariant that timestamp column must be increasing."};
    }

    void throwIfTimestampBreaksInvariantNext(const size_t rowIndex, 
                                     const TS newTimestamp) {
        if(rowIndex < timestamps_.size() - 1 && 
           timestamps_[rowIndex + 1] <= newTimestamp)
            throw TimestampBreaksInvariant{"The input timestamp '" + 
                    std::to_string(newTimestamp) + "' at row " + 
                    std::to_string(rowIndex) + 
                    " is greater-than/equal-to next timestamp '" + 
                    std::to_string(timestamps_[rowIndex + 1]) + "' at row " + 
                    std::to_string(rowIndex + 1) + " and so breaks the " 
                    "invariant that timestamp column must be increasing."};
    }

    void throwIfIndexExceedsTimestampLength(const size_t rowIndex) {
        if(rowIndex > timestamps_.size() - 1)
            throw TimestampDoesNotExist{"Timestamp column length is " + 
                    std::to_string(timestamps_.size()) + ". There is no " 
                    "timestamp for row " + std::to_string(rowIndex) + ". Use "
                    "addTimestamp(s) to add timestamps."};
    }

    void throwIfTimestampsFull() {
        if(this->getNumRows() == timestamps_.size())
            throw TimestampsColumnFull{"Both timestamp column length and number"
                    " of rows currently are " + 
                    std::to_string(timestamps_.size()) + ". Timestamp column "
                    "length cannot exceed number of rows in DataTable. Add a " 
                    "row before adding another timestamp."};
    }

    Timestamps timestamps_;
};

} // namespace OpenSim

#endif //OPENSIM_COMMON_DATA_TABLE_H_
