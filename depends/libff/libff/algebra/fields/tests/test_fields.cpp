/**
 *****************************************************************************
 * @author     This file is part of libff, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/algebra/fields/fp12_2over3over2.hpp>
#include <libff/algebra/fields/fp6_3over2.hpp>

using namespace libff;

static void xassert(bool b){
    if(!b)
        throw std::runtime_error("xassert failed");
}
template<typename FieldT>
void test_field()
{
    bigint<1> rand1 = bigint<1>("76749407");
    bigint<1> rand2 = bigint<1>("44410867");
    bigint<1> randsum = bigint<1>("121160274");

    FieldT zero = FieldT::zero();
    FieldT one = FieldT::one();
    FieldT a = FieldT::random_element();
    FieldT a_ser;
    a_ser = reserialize<FieldT>(a);
    xassert(a_ser == a);

    FieldT b = FieldT::random_element();
    FieldT c = FieldT::random_element();
    FieldT d = FieldT::random_element();

    xassert(a != zero);
    xassert(a != one);

    xassert(a * a == a.squared());
    xassert((a + b).squared() == a.squared() + a*b + b*a + b.squared());
    xassert((a + b)*(c + d) == a*c + a*d + b*c + b*d);
    xassert(a - b == a + (-b));
    xassert(a - b == (-b) + a);

    xassert((a ^ rand1) * (a ^ rand2) == (a^randsum));

    xassert(a * a.inverse() == one);
    xassert((a + b) * c.inverse() == a * c.inverse() + (b.inverse() * c).inverse());

}

template<typename FieldT>
void test_sqrt()
{
    for (size_t i = 0; i < 100; ++i)
    {
        FieldT a = FieldT::random_element();
        FieldT asq = a.squared();
        xassert(asq.sqrt() == a || asq.sqrt() == -a);
    }
}

template<typename FieldT>
void test_two_squarings()
{
    FieldT a = FieldT::random_element();
    xassert(a.squared() == a * a);
    xassert(a.squared() == a.squared_complex());
    xassert(a.squared() == a.squared_karatsuba());
}

template<typename FieldT>
void test_Frobenius()
{
    FieldT a = FieldT::random_element();
    xassert(a.Frobenius_map(0) == a);
    FieldT a_q = a ^ FieldT::base_field_char();
    for (size_t power = 1; power < 10; ++power)
    {
        const FieldT a_qi = a.Frobenius_map(power);
        xassert(a_qi == a_q);

        a_q = a_q ^ FieldT::base_field_char();
    }
}

template<typename FieldT>
void test_unitary_inverse()
{
    xassert(FieldT::extension_degree() % 2 == 0);
    FieldT a = FieldT::random_element();
    FieldT aqcubed_minus1 = a.Frobenius_map(FieldT::extension_degree()/2) * a.inverse();
    xassert(aqcubed_minus1.inverse() == aqcubed_minus1.unitary_inverse());
}

template<typename FieldT>
void test_cyclotomic_squaring();


template<typename ppT>
void test_all_fields()
{
    test_field<Fr<ppT> >();
    test_field<Fq<ppT> >();
    test_field<Fqe<ppT> >();
    test_field<Fqk<ppT> >();

    test_sqrt<Fr<ppT> >();
    test_sqrt<Fq<ppT> >();
    test_sqrt<Fqe<ppT> >();

    test_Frobenius<Fqe<ppT> >();
    test_Frobenius<Fqk<ppT> >();

    test_unitary_inverse<Fqk<ppT> >();
}

template<typename Fp4T>
void test_Fp4_tom_cook()
{
    typedef typename Fp4T::my_Fp FieldT;
    for (size_t i = 0; i < 100; ++i)
    {
        const Fp4T a = Fp4T::random_element();
        const Fp4T b = Fp4T::random_element();
        const Fp4T correct_res = a * b;

        Fp4T res;

        const FieldT
            &a0 = a.c0.c0,
            &a1 = a.c1.c0,
            &a2 = a.c0.c1,
            &a3 = a.c1.c1;

        const FieldT
            &b0 = b.c0.c0,
            &b1 = b.c1.c0,
            &b2 = b.c0.c1,
            &b3 = b.c1.c1;

        FieldT
            &c0 = res.c0.c0,
            &c1 = res.c1.c0,
            &c2 = res.c0.c1,
            &c3 = res.c1.c1;

        const FieldT v0 = a0 * b0;
        const FieldT v1 = (a0 + a1 + a2 + a3) * (b0 + b1 + b2 + b3);
        const FieldT v2 = (a0 - a1 + a2 - a3) * (b0 - b1 + b2 - b3);
        const FieldT v3 = (a0 + FieldT(2)*a1 + FieldT(4)*a2 + FieldT(8)*a3) * (b0 + FieldT(2)*b1 + FieldT(4)*b2 + FieldT(8)*b3);
        const FieldT v4 = (a0 - FieldT(2)*a1 + FieldT(4)*a2 - FieldT(8)*a3) * (b0 - FieldT(2)*b1 + FieldT(4)*b2 - FieldT(8)*b3);
        const FieldT v5 = (a0 + FieldT(3)*a1 + FieldT(9)*a2 + FieldT(27)*a3) * (b0 + FieldT(3)*b1 + FieldT(9)*b2 + FieldT(27)*b3);
        const FieldT v6 = a3 * b3;

        const FieldT beta = Fp4T::non_residue;

        c0 = v0 + beta*(FieldT(4).inverse()*v0 - FieldT(6).inverse()*(v1 + v2) + FieldT(24).inverse() * (v3 + v4) - FieldT(5) * v6);
        c1 = - FieldT(3).inverse()*v0 + v1 - FieldT(2).inverse()*v2 - FieldT(4).inverse()*v3 + FieldT(20).inverse() * v4 + FieldT(30).inverse() * v5 - FieldT(12) * v6 + beta * ( - FieldT(12).inverse() * (v0 - v1) + FieldT(24).inverse()*(v2 - v3) - FieldT(120).inverse() * (v4 - v5) - FieldT(3) * v6);
        c2 = - (FieldT(5)*(FieldT(4).inverse()))* v0 + (FieldT(2)*(FieldT(3).inverse()))*(v1 + v2) - FieldT(24).inverse()*(v3 + v4) + FieldT(4)*v6 + beta*v6;
        c3 = FieldT(12).inverse() * (FieldT(5)*v0 - FieldT(7)*v1) - FieldT(24).inverse()*(v2 - FieldT(7)*v3 + v4 + v5) + FieldT(15)*v6;

        xassert(res == correct_res);

        // {v0, v3, v4, v5}
        const FieldT u = (FieldT::one() - beta).inverse();
        xassert(v0 == u * c0 + beta * u * c2 - beta * u * FieldT(2).inverse() * v1 - beta * u * FieldT(2).inverse() * v2 + beta * v6);
        xassert(v3 == - FieldT(15) * u * c0 - FieldT(30) * u * c1 - FieldT(3) * (FieldT(4) + beta) * u * c2 - FieldT(6) * (FieldT(4) + beta) * u * c3 + (FieldT(24) - FieldT(3) * beta * FieldT(2).inverse()) * u * v1 + (-FieldT(8) + beta * FieldT(2).inverse()) * u * v2
               - FieldT(3) * (-FieldT(16) + beta) * v6);
        xassert(v4 == - FieldT(15) * u * c0 + FieldT(30) * u * c1 - FieldT(3) * (FieldT(4) + beta) * u * c2 + FieldT(6) * (FieldT(4) + beta) * u * c3 + (FieldT(24) - FieldT(3) * beta * FieldT(2).inverse()) * u * v2 + (-FieldT(8) + beta * FieldT(2).inverse()) * u * v1
               - FieldT(3) * (-FieldT(16) + beta) * v6);
        xassert(v5 == - FieldT(80) * u * c0 - FieldT(240) * u * c1 - FieldT(8) * (FieldT(9) + beta) * u * c2 - FieldT(24) * (FieldT(9) + beta) * u * c3 - FieldT(2) * (-FieldT(81) + beta) * u * v1 + (-FieldT(81) + beta) * u * v2
               - FieldT(8) * (-FieldT(81) + beta) * v6);

        // c0 + beta c2 - (beta v1)/2 - (beta v2)/ 2 - (-1 + beta) beta v6,
        // -15 c0 - 30 c1 - 3 (4 + beta) c2 - 6 (4 + beta) c3 + (24 - (3 beta)/2) v1 + (-8 + beta/2) v2 + 3 (-16 + beta) (-1 + beta) v6,
        // -15 c0 + 30 c1 - 3 (4 + beta) c2 + 6 (4 + beta) c3 + (-8 + beta/2) v1 + (24 - (3 beta)/2) v2 + 3 (-16 + beta) (-1 + beta) v6,
        // -80 c0 - 240 c1 - 8 (9 + beta) c2 - 24 (9 + beta) c3 - 2 (-81 + beta) v1 + (-81 + beta) v2 + 8 (-81 + beta) (-1 + beta) v6
    }
}

int main()
{
 
    alt_bn128_pp::init_public_params();
    test_field<alt_bn128_Fq6>();
    test_Frobenius<alt_bn128_Fq6>();
    test_all_fields<alt_bn128_pp>();


}

