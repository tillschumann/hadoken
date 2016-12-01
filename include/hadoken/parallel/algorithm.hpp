/**
 * Copyright (c) 2016, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * Boost Software License - Version 1.0
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
*
*/
#ifndef _HADOKEN_PARALLEL_ALGORITHM_HPP_
#define _HADOKEN_PARALLEL_ALGORITHM_HPP_

#include <algorithm>


namespace hadoken{


namespace parallel{


/// sequential execution, no parallelism
class sequential_execution_policy{};

/// parallel execution allowed
class parallel_execution_policy{};

/// parallel execution allowed, vector execution allowed
class parallel_vector_execution_policy{};


/// constexpr for sequential execution
constexpr sequential_execution_policy seq{};

/// constexpr for parallel execution
constexpr parallel_execution_policy par{};

/// constexpr for parallel vector execution
constexpr parallel_vector_execution_policy par_vec{};



/// parallel for_each algorithm with execution specifier
template<typename ExecPolicy, typename Iterator, typename Function>
inline void for_each(ExecPolicy && policy, Iterator begin_it, Iterator end_it, Function fun);

/// parallel fill algorithm
template <typename ExecPolicy, class ForwardIterator, class T>
void fill(ExecPolicy && policy, ForwardIterator first, ForwardIterator last, const T& val);

/// parallel fill_n algorithm
template <typename ExecPolicy, class ForwardIterator, class Size, class T>
void fill_n(ExecPolicy && policy, ForwardIterator first, Size n, const T& val);

/// parallel generate algorithm
template< class ExecutionPolicy, class ForwardIt, class Generator >
void generate( ExecutionPolicy&& policy, ForwardIt first, ForwardIt last, Generator g );

/// parallel generate_n algorithm
template< class ExecutionPolicy, class OutputIt, class Size, class Generator >
OutputIt generate_n( ExecutionPolicy&& policy, OutputIt first, Size count, Generator g );



/// parallel transform algorithm binary
template< class ExecutionPolicy, class InputIterator1, class InputIterator2, class OutputIterator, class BinaryOperation >
OutputIterator transform( ExecutionPolicy&& policy, InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
                    OutputIterator d_first, BinaryOperation binary_op );

/// parallel transform algorithm unary
template< class ExecutionPolicy, class InputIt, class OutputIt, class UnaryOperation >
OutputIt transform( ExecutionPolicy&& policy, InputIt first1, InputIt last1, OutputIt d_first,
                    UnaryOperation unary_op );



/// parallel all_of algorithm
template< class ExecutionPolicy, class InputIterator, class UnaryPredicate >
inline bool all_of( ExecutionPolicy&& policy, InputIterator first, InputIterator last, UnaryPredicate p );

/// parallel any_of algorithm
template< class ExecutionPolicy, class InputIterator, class UnaryPredicate >
inline bool any_of( ExecutionPolicy&& policy, InputIterator first, InputIterator last, UnaryPredicate p );

/// parallel none_of algorithm
template< class ExecutionPolicy, class InputIterator, class UnaryPredicate >
inline bool none_of( ExecutionPolicy&& policy, InputIterator first, InputIterator last, UnaryPredicate p );




/// sort algorithm
template< class ExecutionPolicy, class RandomIt >
void sort( ExecutionPolicy&& policy, RandomIt first, RandomIt last );

/// sort algorithmwith comparator
template< class ExecutionPolicy, class RandomIt, class Compare >
void sort( ExecutionPolicy&& policy, RandomIt first, RandomIt last, Compare comp );





/// Extension: for_range_ algorithm
///
/// for_range is an extension to for_each where the function
/// execute on a subrange, instead of a single element
template<typename ExecPolicy, typename Iterator, typename RangeFunction>
inline void for_range(ExecPolicy && policy, Iterator begin_it, Iterator end_it, RangeFunction fun);

} // parallel



} // hadoken


#ifdef HADOKEN_PARALLEL_USE_TBB

#include <hadoken/parallel/bits/tbb_algorithm_impl.hpp>

#elif (defined HADOKEN_PARALLEL_USE_OMP)

#include <hadoken/parallel/bits/omp_algorithm_impl.hpp>

#elif (defined HADOKEN_PARALLEL_USE_GNU_PSTL)

#include <hadoken/parallel/bits/gnu_pstl_algorithm_impl.hpp>

#else

#include <hadoken/parallel/bits/cxx11_thread_algorithm_impl.hpp>

#endif


#endif
