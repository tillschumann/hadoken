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

#define BOOST_TEST_MODULE parallelTests
#define BOOST_TEST_MAIN

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <future>
#include <algorithm>
#include <random>

#include <chrono>

#include <boost/test/unit_test.hpp>

#include <hadoken/parallel/algorithm.hpp>

#include <parallel/algorithm>


using namespace hadoken;
using cl = std::chrono::system_clock;

BOOST_AUTO_TEST_CASE( parallel_for_each_simple_test)
{



    std::vector<double> values;
    parallel::for_each(parallel::seq, values.begin(), values.end(), [](double & v){ v += 42; });

    parallel::for_each(parallel::parallel_execution_policy(), values.begin(), values.end(), [](double & v){ v += 422; });

    parallel::for_each(parallel::parallel_vector_execution_policy(), values.begin(), values.end(), [](double & v){ v += 4222; });


    values = std::vector<double>(128000, 128);

    auto t1 = cl::now();

	auto fops = [](double & v){
        double res = std::pow(v, 4);
        res = std::sqrt(res);
        v = res;

     };


    parallel::for_each(parallel::seq, values.begin(), values.end(), fops);


    auto t2 = cl::now();

    for(auto i : values){
        BOOST_CHECK_CLOSE(i, 128*128, 0.01);
    }


    std::cout << "sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;

    values = std::vector<double>(128000, 128);

    t1 = cl::now();

    parallel::for_each(parallel::parallel_execution_policy(), values.begin(), values.end(), fops);
    //__gnu_parallel::for_each(values.begin(), values.end(), [](int & v){ v += 100; });

    t2 = cl::now();


    for(auto i : values){
         BOOST_CHECK_CLOSE(i, 128*128, 0.01);
    }

    std::cout << "parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;


    values = std::vector<double>(128000, 128);

    parallel::for_each(parallel::parallel_vector_execution_policy(), values.begin(), values.end(), fops);


 
    for(auto i : values){
         BOOST_CHECK_CLOSE(i, 128*128, 0.01);
    }

}





BOOST_AUTO_TEST_CASE( parallel_fill_test)
{

    using namespace hadoken;

    std::size_t n = 5000;

    std::size_t val = 42;

    std::vector<double> values(n), values_seq(n), values_par(n), values_par_n(n);

    std::fill(values.begin(), values.end(), val);

    parallel::fill(parallel::seq, values_seq.begin(), values_seq.end(), val);

    parallel::fill(parallel::parallel_execution_policy(), values_par.begin(), values_par.end(), val);

    parallel::fill_n(parallel::parallel_vector_execution_policy(), values_par_n.begin(), n, val);


    for(auto & v : values){
        BOOST_CHECK_EQUAL(v, val);
    }

    BOOST_CHECK_EQUAL_COLLECTIONS(values.begin(), values.end(), values_seq.begin(),values_seq.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(values.begin(), values.end(), values_par.begin(),values_par.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(values.begin(), values.end(), values_par_n.begin(),values_par_n.end());

}



BOOST_AUTO_TEST_CASE( parallel_count_test)
{

    using namespace hadoken;

    std::size_t n = 5000000;

    std::size_t num_4, num_2, p_num_4, p_num_2;

    std::vector<std::size_t> values(n, 0);

    std::size_t counter = 0;
    for(auto & v : values){
        v = (counter++) % 5;
    }

    {
        auto t1 = cl::now();

        num_4 =  std::count(values.begin(), values.end(), 4);
        num_2 = std::count_if(values.begin(), values.end(), [&](const std::size_t & v){
            return v == 1;
        });

        auto t2 = cl::now();
        std::cout << " std::count " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }


    {
        auto t1 = cl::now();

        p_num_4 =  parallel::count(parallel::par, values.begin(), values.end(), 4);
        p_num_2 = parallel::count_if(parallel::par, values.begin(), values.end(), [&](const std::size_t & v){
            return v == 1;
        });

        auto t2 = cl::now();
        std::cout << " parallel::count " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }

    std::size_t s_num_4 =  parallel::count(parallel::seq, values.begin(), values.end(), 4);
    std::size_t s_num_2 = parallel::count_if(parallel::seq, values.begin(), values.end(), [&](const std::size_t & v){
        return v == 1;
    });


    BOOST_CHECK_EQUAL(num_4, p_num_4);
    BOOST_CHECK_EQUAL(num_4, s_num_4);

    BOOST_CHECK_EQUAL(num_2, p_num_2);
    BOOST_CHECK_EQUAL(num_2, s_num_2);

    std::cout << " n " << s_num_4 << std::endl;
}


BOOST_AUTO_TEST_CASE( parallel_transform_test)
{

    using namespace hadoken;

    std::size_t n = 800000;

    auto dummy_ops = [](std::size_t i , std::size_t j){
        return (i * 100  + j ) / 7;
    };

    std::vector<std::size_t> v1(n), v2(n), res(n), res2(n), res3(n), res4(n);

    std::size_t counter = 0;
    for(std::size_t i=0; i < n; ++i){
        v1[i] = counter++;
        v2[i] = counter++;
    }




    {
        auto t1 = cl::now();

        hadoken::parallel::transform(hadoken::parallel::par, v1.begin(), v1.end(),
                                     v2.begin(), res.begin(), dummy_ops);

        auto t2 = cl::now();

        std::cout << " transform parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;


        hadoken::parallel::transform(hadoken::parallel::seq, v1.begin(), v1.end(),
                                     res2.begin(), [](const std::size_t & v){
            return v + 100;
        });

    }

    {
        auto t1 = cl::now();

        std::transform(v1.begin(), v1.end(),
                        v2.begin(), res3.begin(), dummy_ops);

        auto t2 = cl::now();

        std::cout << " transform sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;


        std::transform(v1.begin(), v1.end(),
                        res4.begin(), [](const std::size_t & v){
            return v + 100;
        });
    }



    BOOST_CHECK_EQUAL_COLLECTIONS(res.begin(), res.end(), res3.begin(), res3.end());

    BOOST_CHECK_EQUAL_COLLECTIONS(res2.begin(), res2.end(), res4.begin(), res4.end());

}




BOOST_AUTO_TEST_CASE( parallel_all_of_test)
{

    using namespace hadoken;

    std::size_t n = 500000;

    auto is_positive = [](int v){
            return (v >=0);
    };


    std::vector<int> values;

    bool res_seq = parallel::all_of(parallel::seq, values.begin(), values.end(), is_positive);

    bool res_par = parallel::all_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_positive);

    bool res_std = std::all_of(values.begin(), values.end(), is_positive);

    BOOST_CHECK_EQUAL(res_seq, true);
    BOOST_CHECK_EQUAL(res_par, true);
    BOOST_CHECK_EQUAL(res_std, true);


    // fullfil vector with positive values excepted one
    for(std::size_t i= 0; i < n; ++i){
        values.push_back(i);
    }

    values.push_back(-1);

    res_seq = parallel::all_of(parallel::seq, values.begin(), values.end(), is_positive);

    {
        auto t1 = cl::now();
        res_par = parallel::all_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_positive);
        auto t2 = cl::now();

        std::cout << " all_of parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }


    {
        auto t1 = cl::now();
        res_std = std::all_of(values.begin(), values.end(), is_positive);
        auto t2 = cl::now();

        std::cout << " all_of sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }



    BOOST_CHECK_EQUAL(res_seq, false);
    BOOST_CHECK_EQUAL(res_par, false);
    BOOST_CHECK_EQUAL(res_std, false);

    // remove the positive values
    values.pop_back();


    res_seq = parallel::all_of(parallel::seq, values.begin(), values.end(), is_positive);

    res_par = parallel::all_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_positive);

    res_std = std::all_of(values.begin(), values.end(), is_positive);


    BOOST_CHECK_EQUAL(res_seq, true);
    BOOST_CHECK_EQUAL(res_par, true);
    BOOST_CHECK_EQUAL(res_std, true);


}


BOOST_AUTO_TEST_CASE( parallel_none_of_test)
{

    using namespace hadoken;

    std::size_t n = 500000;

    auto is_negative = [](int v){
            return (v <0);
    };

    std::vector<int> values;

    bool res_seq = parallel::none_of(parallel::seq, values.begin(), values.end(), is_negative);

    bool res_par = parallel::none_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);

    bool res_std = std::none_of(values.begin(), values.end(), is_negative);

    BOOST_CHECK_EQUAL(res_seq, true);
    BOOST_CHECK_EQUAL(res_par, true);
    BOOST_CHECK_EQUAL(res_std, true);


    // fullfil vector with positive values excepted one
    for(std::size_t i= 0; i < n; ++i){
        values.push_back(i);
    }

    values.push_back(-1);

    res_seq = parallel::none_of(parallel::seq, values.begin(), values.end(), is_negative);

    {
        auto t1 = cl::now();
        res_par = parallel::none_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);
        auto t2 = cl::now();

        std::cout << " none_of parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }


    {
        auto t1 = cl::now();
        res_std = std::none_of(values.begin(), values.end(), is_negative);
        auto t2 = cl::now();

        std::cout << " none_of sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }



    BOOST_CHECK_EQUAL(res_seq, false);
    BOOST_CHECK_EQUAL(res_par, false);
    BOOST_CHECK_EQUAL(res_std, false);

    // remove the positive values
    values.pop_back();


    res_seq = parallel::none_of(parallel::seq, values.begin(), values.end(), is_negative);

    res_par = parallel::none_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);

    res_std = std::none_of(values.begin(), values.end(), is_negative);


    BOOST_CHECK_EQUAL(res_seq, true);
    BOOST_CHECK_EQUAL(res_par, true);
    BOOST_CHECK_EQUAL(res_std, true);
}




BOOST_AUTO_TEST_CASE( parallel_any_of_test)
{

    using namespace hadoken;

    std::size_t n = 500000;

    auto is_negative = [](int v){
            return (v <0);
    };


    std::vector<int> values;

    bool res_seq = parallel::any_of(parallel::seq, values.begin(), values.end(), is_negative);

    bool res_par = parallel::any_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);

    bool res_std = std::any_of(values.begin(), values.end(), is_negative);

    BOOST_CHECK_EQUAL(res_seq, false);
    BOOST_CHECK_EQUAL(res_par, false);
    BOOST_CHECK_EQUAL(res_std, false);


    // fullfil vector with positive values excepted one
    for(std::size_t i= 0; i < n; ++i){
        values.push_back(i);
    }

    values.push_back(-1);

    res_seq = parallel::any_of(parallel::seq, values.begin(), values.end(), is_negative);

    {
        auto t1 = cl::now();
        res_par = parallel::any_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);
        auto t2 = cl::now();

        std::cout << " any_of parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }


    {
        auto t1 = cl::now();
        res_std = std::any_of(values.begin(), values.end(), is_negative);
        auto t2 = cl::now();

        std::cout << " any_of sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
    }



    BOOST_CHECK_EQUAL(res_seq, true);
    BOOST_CHECK_EQUAL(res_par, true);
    BOOST_CHECK_EQUAL(res_std, true);

    // remove the positive values
    values.pop_back();


    res_seq = parallel::any_of(parallel::seq, values.begin(), values.end(), is_negative);

    res_par = parallel::any_of(parallel::parallel_execution_policy(), values.begin(), values.end(), is_negative);

    res_std = std::any_of(values.begin(), values.end(), is_negative);


    BOOST_CHECK_EQUAL(res_seq, false);
    BOOST_CHECK_EQUAL(res_par, false);
    BOOST_CHECK_EQUAL(res_std, false);


}


template<typename Container>
bool is_ordered(Container & c){
    for(std::size_t i =0; i < c.size()-1; ++i){
        if(c[i] > c[i+1]){
            return false;
        }
    
    }
    return true;
}


BOOST_AUTO_TEST_CASE( parallel_sort)
{

    using namespace hadoken;

    std::size_t n = 500000;

    std::vector<int> values(n);
    
    std::mt19937_64 mt;
    std::uniform_int_distribution<int> dist;
    
    std::generate(values.begin(), values.end(), [&](){
       return dist(mt); 
    });

    BOOST_CHECK( is_ordered(values) == false);

    {
        auto v1 = values;
        parallel::sort(parallel::seq, v1.begin(), v1.end());
        BOOST_CHECK( is_ordered(v1) == true);
        
    }

    {
        auto v2 = values;
        
        auto t1 = cl::now();        
        parallel::sort(parallel::parallel_execution_policy(), v2.begin(), v2.end());
        
        auto t2 = cl::now();

        std::cout << " sort parallel " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;
        
        BOOST_CHECK( is_ordered(v2) == true);
    }

    {

        auto v3 = values;
        
        auto t1 = cl::now();       
              
        std::sort(v3.begin(), v3.end());
        
        auto t2 = cl::now();

        std::cout << " sort sequential " << std::chrono::duration_cast<std::chrono::microseconds>(t2 -t1).count() << std::endl;        
        
        BOOST_CHECK( is_ordered(v3) == true);
    }

  

}

