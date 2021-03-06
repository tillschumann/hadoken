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
#ifndef _HADOKEN_CXX11_THREAD_ALGORITHM_BITS_HPP_
#define _HADOKEN_CXX11_THREAD_ALGORITHM_BITS_HPP_

#include <type_traits>
#include <future>
#include <thread>
#include <stdexcept>
#include <atomic>

#include <hadoken/containers/small_vector.hpp>
#include <hadoken/parallel/algorithm.hpp>
#include <hadoken/utility/range.hpp>
#include <hadoken/executor/system_executor.hpp>
#include <hadoken/thread/latch.hpp>


#include <hadoken/parallel/bits/parallel_algorithm_generics.hpp>
#include <hadoken/parallel/bits/parallel_none_any_all_generic.hpp>
#include <hadoken/parallel/bits/parallel_transform_generic.hpp>
#include <hadoken/parallel/bits/parallel_sort_generic.hpp>
#include <hadoken/parallel/bits/parallel_numeric_generic.hpp>

namespace hadoken{


namespace parallel{


class sequential_execution_policy;
class parallel_execution_policy;
class parallel_vector_execution_policy;



namespace detail{


std::size_t get_parallel_task(){
    return std::thread::hardware_concurrency();
}


/// for_range algorithm
template<typename Iterator, typename RangeFunction>
inline void _simple_cxx11_for_range(Iterator begin_it, Iterator end_it, RangeFunction fun){
    const std::size_t n_task = get_parallel_task();

    thread::latch latch_task(n_task);

    range<Iterator> global_range(begin_it, end_it);

    system_executor sexec;

    // start task for tasks 1-N on separated executors
    for(std::size_t i = 1; i < n_task; ++i){

         sexec.execute([i, &latch_task, &global_range, &n_task, &fun](){
            auto my_range = take_splice(global_range, i, n_task);
            fun(my_range.begin(), my_range.end());
            latch_task.count_down(1);
        });
    }

    // execute the task 0 locally
    auto my_range = take_splice(global_range, 0, n_task);
    fun(my_range.begin(), my_range.end());

    // wait for the folks
    latch_task.count_down_and_wait(1);
}


} // detail

/// for_range_ algorithm
/// for_range is an extension to for_each where the function
/// execute on a subrange, instead of a single element
template<typename ExecPolicy, typename Iterator, typename RangeFunction>
inline void for_range(ExecPolicy && policy, Iterator begin_it, Iterator end_it, RangeFunction fun){
    if(detail::is_parallel_policy(policy)){
        detail::_simple_cxx11_for_range(begin_it, end_it, fun);
        return;
    }

   fun(begin_it, end_it);
}




} // concurrent



} // hadoken


#endif
