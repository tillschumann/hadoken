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

#define BOOST_TEST_MODULE geometryTests
#define BOOST_TEST_MAIN

#include <iostream>
#include <sstream>
#include <random>



#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include <hadoken/spmd/spmd_array.hpp>


using namespace hadoken;


typedef boost::mpl::list<char, unsigned char, short, unsigned short, int, unsigned int, float, double> numerical_types;
//typedef boost::mpl::list<float> numerical_types;




// dummy generator of numerical value
template<typename T>
class generator{
public:

    T rand(){
        return T(_dist(_generator))/ T(100);
    }

private:
    std::mt19937_64 _generator;
    typename std::conditional<std::is_integral<T>::value,
            std::uniform_int_distribution<T>,
            std::uniform_real_distribution<T>>::type _dist;
};


template<typename T>
std::array<T, 10> get_base_array(){
    std::array<T, 10> base_numbers;
    std::random_device rd;
    std::mt19937_64 g(rd());

    std::iota(base_numbers.begin(), base_numbers.end(), 1);
    std::shuffle(base_numbers.begin(), base_numbers.end(), g);

    return base_numbers;
}



//
// simple stupid test for 3D distance computation
BOOST_AUTO_TEST_CASE_TEMPLATE( base_add_spmd_operations, T, numerical_types )
{
    namespace spmd = hadoken::spmd;

    generator<T> gen;




    // simple add vec
    {
        T val_inputs1[] = { gen.rand(), gen.rand(), gen.rand(), gen.rand() };
        T val_inputs2[] = { gen.rand(), gen.rand(), gen.rand(), gen.rand() };

        spmd::spmd_array<T,4> my_array_res1, my_array_res2;

        spmd::spmd_array<T,4> my_array_src1({ val_inputs1[0], val_inputs1[1], val_inputs1[2], val_inputs1[3]});

        spmd::spmd_array<T,4> my_array_src2({ val_inputs2[0], val_inputs2[1], val_inputs2[2], val_inputs2[3]});

        // asm mark plus

        my_array_res1 = spmd::plus(my_array_src1, my_array_src2);

        BOOST_CHECK_EQUAL(my_array_res1[1], (val_inputs1[1] + val_inputs2[1]));

        my_array_res2 = spmd::plus(my_array_src2, my_array_src1);


        BOOST_CHECK(my_array_res1 == my_array_res2);


        my_array_res1 = my_array_res2 + T(10);

        for(std::size_t i =0; i < my_array_res1.size(); ++i){
            BOOST_CHECK_EQUAL(my_array_res1[i], my_array_res2[i]+ T(10));
        }


        std::cout << "test_spmd_add " << my_array_res1 << std::endl;

    }






}



//
// simple stupid test for 3D distance computation
BOOST_AUTO_TEST_CASE_TEMPLATE( base_sub_spmd_operations, T, numerical_types )
{
    namespace spmd = hadoken::spmd;



    generator<T> gen;


    std::array<T, 10> base_array = get_base_array<T>();

    {



        spmd::spmd_array<T,4> my_array_res1, my_array_res2;

        spmd::spmd_array<T,4> my_array_src1({ base_array[0], base_array[1], base_array[2], base_array[3]});

        spmd::spmd_array<T,4> my_array_src2({ base_array[4], base_array[5], base_array[6], base_array[7]});

        // asm mark sub

        my_array_res1 = my_array_src1 - my_array_src2;

        BOOST_CHECK_EQUAL(my_array_res1[1], T( base_array[1] - base_array[5]));

        my_array_res2 = my_array_res1 + my_array_src2;

        BOOST_CHECK(spmd::close_to_abs(my_array_src1, my_array_res2, T(0.01)));


        my_array_res1 = my_array_res2 - T(10);

        for(std::size_t i =0; i < my_array_res1.size(); ++i){
            BOOST_CHECK_EQUAL(my_array_res1[i], T(my_array_res2[i]- T(10)));
        }


        // force to string invocation
        std::cout << "test_spmd_sub " << spmd::to_string(my_array_res1) << std::endl;

    }


}


BOOST_AUTO_TEST_CASE_TEMPLATE( base_mul_spmd_operations, T, numerical_types )
{
    namespace spmd = hadoken::spmd;

    generator<T> gen;
    std::array<T, 10> base_array = get_base_array<T>();

    // mul simple
    {

        spmd::spmd_array<T,4> my_array_res1, my_array_res2;

        spmd::spmd_array<T,4> my_array_src1({ base_array[0], base_array[1], base_array[2], base_array[3]});

        spmd::spmd_array<T,4> my_array_src2({ base_array[4], base_array[5], base_array[6], base_array[7]});

        // asm mark mul

        my_array_res1 = my_array_src1 * my_array_src2;

        BOOST_CHECK_EQUAL(my_array_res1[1], (base_array[1] * base_array[5]));

        my_array_res2 = my_array_src2 * my_array_src1;


        BOOST_CHECK(my_array_res1 == my_array_res2);

        my_array_res1 = my_array_res2 * T(10);

        for(std::size_t i =0; i < my_array_res1.size(); ++i){
            BOOST_CHECK_EQUAL(my_array_res1[i], T(my_array_res2[i] * T(10)));
        }

        std::cout << "test_spmd_mul " << my_array_res1 << std::endl;

    }


}





BOOST_AUTO_TEST_CASE_TEMPLATE( base_div_spmd_operations, T, numerical_types )
{
    namespace spmd = hadoken::spmd;

    generator<T> gen;
    std::array<T, 10> base_array = get_base_array<T>();

    // mul simple
    {

        spmd::spmd_array<T,4> my_array_res1;

        spmd::spmd_array<T,4> my_array_src1({ base_array[0], base_array[1], base_array[2], base_array[3]});

        spmd::spmd_array<T,4> my_array_src2({ base_array[4], base_array[5], base_array[6], base_array[7]});

        // asm mark div

        my_array_res1 = my_array_src1 / my_array_src2;

        for(std::size_t i =0; i < 4; ++i ){
            BOOST_CHECK_EQUAL( my_array_res1[i], (base_array[i] / base_array[i+4]));
        }


        const T scalar_div = base_array[6];

        my_array_res1 = my_array_src2 / scalar_div;

        for(std::size_t i =0; i < my_array_res1.size(); ++i){
            BOOST_CHECK_EQUAL(my_array_res1[i], T(my_array_src2[i] / scalar_div));
        }

        std::cout << "test_spmd_div " << my_array_res1 << std::endl;

    }


}
