/**
 *****************************************************************************
 * @author     This file is part of libff, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <sstream>

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>

using namespace libff;

static void xassert(bool b){
    if(!b)
        throw std::runtime_error("xassert failed");
}

template<typename GroupT>
void test_mixed_add()
{
    GroupT base, el, result;

    base = GroupT::zero();
    el = GroupT::zero();
    el.to_special();
    result = base.mixed_add(el);
    xassert(result == base + el);

    base = GroupT::zero();
    el = GroupT::random_element();
    el.to_special();
    result = base.mixed_add(el);
    xassert(result == base + el);

    base = GroupT::random_element();
    el = GroupT::zero();
    el.to_special();
    result = base.mixed_add(el);
    xassert(result == base + el);

    base = GroupT::random_element();
    el = GroupT::random_element();
    el.to_special();
    result = base.mixed_add(el);
    xassert(result == base + el);

    base = GroupT::random_element();
    el = base;
    el.to_special();
    result = base.mixed_add(el);
    xassert(result == base.dbl());
}

template<typename GroupT>
void test_group()
{
    bigint<1> rand1 = bigint<1>("76749407");
    bigint<1> rand2 = bigint<1>("44410867");
    bigint<1> randsum = bigint<1>("121160274");

    GroupT zero = GroupT::zero();
    xassert(zero == zero);
    GroupT one = GroupT::one();
    xassert(one == one);
    GroupT two = bigint<1>(2l) * GroupT::one();
    xassert(two == two);
    GroupT five = bigint<1>(5l) * GroupT::one();

    GroupT three = bigint<1>(3l) * GroupT::one();
    GroupT four = bigint<1>(4l) * GroupT::one();

    xassert(two+five == three+four);

    GroupT a = GroupT::random_element();
    GroupT b = GroupT::random_element();

    xassert(one != zero);
    xassert(a != zero);
    xassert(a != one);

    xassert(b != zero);
    xassert(b != one);

    xassert(a.dbl() == a + a);
    xassert(b.dbl() == b + b);
    xassert(one.add(two) == three);
    xassert(two.add(one) == three);
    xassert(a + b == b + a);
    xassert(a - a == zero);
    xassert(a - b == a + (-b));
    xassert(a - b == (-b) + a);

    // handle special cases
    xassert(zero + (-a) == -a);
    xassert(zero - a == -a);
    xassert(a - zero == a);
    xassert(a + zero == a);
    xassert(zero + a == a);

    xassert((a + b).dbl() == (a + b) + (b + a));
    xassert(bigint<1>("2") * (a + b) == (a + b) + (b + a));

    xassert((rand1 * a) + (rand2 * a) == (randsum * a));

    xassert(GroupT::order() * a == zero);
    xassert(GroupT::order() * one == zero);
    xassert((GroupT::order() * a) - a != zero);
    xassert((GroupT::order() * one) - one != zero);

    test_mixed_add<GroupT>();
}

template<typename GroupT>
void test_mul_by_q()
{
    GroupT a = GroupT::random_element();
    xassert((GroupT::base_field_char()*a) == a.mul_by_q());
}

template<typename GroupT>
void test_output()
{
    GroupT g = GroupT::zero();

    for (size_t i = 0; i < 1000; ++i)
    {
        std::stringstream ss;
        ss << g;
        GroupT gg;
        ss >> gg;
        xassert(g == gg);
        /* use a random point in next iteration */
        g = GroupT::random_element();
    }
}

int main(void)
{
   

    alt_bn128_pp::init_public_params();
    test_group<G1<alt_bn128_pp> >();
    test_output<G1<alt_bn128_pp> >();
    test_group<G2<alt_bn128_pp> >();
    test_output<G2<alt_bn128_pp> >();
    test_mul_by_q<G2<alt_bn128_pp> >();


}


