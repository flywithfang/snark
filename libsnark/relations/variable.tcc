/** @file
 *****************************************************************************

 Implementation of interfaces for:
 - a variable (i.e., x_i),
 - a linear term (i.e., a_i * x_i), and
 - a linear combination (i.e., sum_i a_i * x_i).

 See variabe.hpp .

 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef VARIABLE_TCC_
#define VARIABLE_TCC_

#include <algorithm>
#include <cassert>

#include <libff/algebra/fields/bigint.hpp>

namespace libsnark {

#include "variable_linear_term.tcc"

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const integer_coeff_t int_coeff)
{
    this->add_term(linear_term<FieldT>(0, int_coeff));
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const FieldT &field_coeff)
{
    this->add_term(linear_term<FieldT>(0, field_coeff));
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const variable<FieldT> &var)
{
    this->add_term(var);
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const linear_term<FieldT> &lt)
{
    this->add_term(lt);
}

template<typename FieldT>
typename std::vector<linear_term<FieldT> >::const_iterator linear_combination<FieldT>::begin() const
{
    return m_terms.begin();
}

template<typename FieldT>
typename std::vector<linear_term<FieldT> >::const_iterator linear_combination<FieldT>::end() const
{
    return m_terms.end();
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &var)
{
    add_term(var,FieldT::one());
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &var, const integer_coeff_t int_coeff)
{
   add_term(var,FieldT(int_coeff));
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const variable<FieldT> &var, const FieldT &coeff)
{
    add_term(linear_term<FieldT>(var.index, coeff));
}

template<typename FieldT>
void linear_combination<FieldT>::add_term(const linear_term<FieldT> &other)
{
    this->m_terms.emplace_back(other);
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator*(const integer_coeff_t int_coeff) const
{
    return (*this) * FieldT(int_coeff);
}

//z ci*x
template<typename FieldT>
FieldT linear_combination<FieldT>::evaluate(const std::vector<FieldT> &assignment) const
{
    FieldT acc = FieldT::zero();
    for (auto &lt : m_terms)
    {
        auto var_v = (lt.index == 0 ? FieldT::one() : assignment[lt.index-1]);
        acc +=  var_v * lt.coeff;
    }
    return acc;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator*(const FieldT &field_coeff) const
{
    linear_combination<FieldT> result;
    result.m_terms.reserve(this->m_terms.size());
    for (const linear_term<FieldT> &lt : this->m_terms)
    {
        result.m_terms.emplace_back(lt * field_coeff);
    }
    return result;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator+(const linear_combination<FieldT> &other) const
{
    linear_combination<FieldT> result;

    auto it1 = this->m_terms.begin();
    auto it2 = other.m_terms.begin();

    /* invariant: it1 and it2 always point to unprocessed items in the corresponding linear combinations */
    while (it1 != this->m_terms.end() && it2 != other.m_terms.end())
    {
        if (it1->index < it2->index)
        {
            result.m_terms.emplace_back(*it1);
            ++it1;
        }
        else if (it1->index > it2->index)
        {
            result.m_terms.emplace_back(*it2);
            ++it2;
        }
        else
        {
            /* it1->index == it2->index */
            result.m_terms.emplace_back(linear_term<FieldT>(variable<FieldT>(it1->index), it1->coeff + it2->coeff));
            ++it1;
            ++it2;
        }
    }

    if (it1 != this->m_terms.end())
    {
        result.m_terms.insert(result.m_terms.end(), it1, this->m_terms.end());
    }
    else
    {
        result.m_terms.insert(result.m_terms.end(), it2, other.m_terms.end());
    }

    return result;
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator-(const linear_combination<FieldT> &other) const
{
    return (*this) + (-other);
}

template<typename FieldT>
linear_combination<FieldT> linear_combination<FieldT>::operator-() const
{
    return (*this) * (-FieldT::one());
}

template<typename FieldT>
bool linear_combination<FieldT>::operator==(const linear_combination<FieldT> &other) const
{
    return (this->m_terms == other.m_terms);
}

template<typename FieldT>
bool linear_combination<FieldT>::is_valid(const size_t num_variables) const
{
    /* check that all m_terms in linear combination are sorted */
    for (size_t i = 1; i < m_terms.size(); ++i)
    {
        if (m_terms[i-1].index >= m_terms[i].index)
        {
            return false;
        }
    }

    /* check that the variables are in proper range. as the variables
       are sorted, it suffices to check the last term */
    if ((--m_terms.end())->index >= num_variables)
    {
        return false;
    }

    return true;
}

template<typename FieldT>
void linear_combination<FieldT>::print(const std::map<size_t, std::string> &variable_annotations) const
{
    for (auto &lt : m_terms)
    {
        if (lt.index == 0)
        {
            printf("    1 * ");
            lt.coeff.print();
        }
        else
        {
            auto it = variable_annotations.find(lt.index);
            printf("    x_%zu (%s) * ", lt.index, (it == variable_annotations.end() ? "no annotation" : it->second.c_str()));
            lt.coeff.print();
        }
    }
}

template<typename FieldT>
void linear_combination<FieldT>::print_with_assignment(const std::vector<FieldT> &full_assignment, const std::map<size_t, std::string> &variable_annotations) const
{
    for (auto &lt : m_terms)
    {
        if (lt.index == 0)
        {
            printf("    1 * ");
            lt.coeff.print();
        }
        else
        {
            printf("    x_%zu * ", lt.index);
            lt.coeff.print();

            auto it = variable_annotations.find(lt.index);
            printf("    where x_%zu (%s) was assigned value ", lt.index,
                   (it == variable_annotations.end() ? "no annotation" : it->second.c_str()));
            full_assignment[lt.index-1].print();
            printf("      i.e. negative of ");
            (-full_assignment[lt.index-1]).print();
        }
    }
}

template<typename FieldT>
std::ostream& operator<<(std::ostream &out, const linear_combination<FieldT> &lc)
{
    //out <<"lc:"<< lc.m_terms.size() << "\n";

    size_t i=0;
    for (const linear_term<FieldT>& lt : lc.m_terms)
    {
        const auto big=lt.coeff.as_bigint();
        if(i>0 && lt.coeff!=FieldT{-1})
            out<< "+";

        if(lt.coeff.is_zero()){
            out<<"0";
        }else{
            if(lt.coeff==FieldT{-1})
                out<<"-v"<<lt.index;
            else if(lt.coeff==FieldT(1)){
                  out<<"v"<<lt.index ;
            }
            else{
                out<< big.to_string()<<"*"<<"v"<<lt.index ;
            }

        }
       
      
        ++i;
    }
    out<<std::endl;
    return out;
}

template<typename FieldT>
std::istream& operator>>(std::istream &in, linear_combination<FieldT> &lc)
{
    lc.m_terms.clear();

    size_t s;
    in >> s;

    libff::consume_newline(in);

    lc.m_terms.reserve(s);

    for (size_t i = 0; i < s; ++i)
    {
        linear_term<FieldT> lt;
        in >> lt.index;
        libff::consume_newline(in);
        in >> lt.coeff;
        libff::consume_OUTPUT_NEWLINE(in);
        lc.m_terms.emplace_back(lt);
    }

    return in;
}

template<typename FieldT>
linear_combination<FieldT> operator*(const integer_coeff_t int_coeff, const linear_combination<FieldT> &lc)
{
    return lc * int_coeff;
}

template<typename FieldT>
linear_combination<FieldT> operator*(const FieldT &field_coeff, const linear_combination<FieldT> &lc)
{
    return lc * field_coeff;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const integer_coeff_t int_coeff, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(int_coeff) + lc;
}

template<typename FieldT>
linear_combination<FieldT> operator+(const FieldT &field_coeff, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(field_coeff) + lc;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const integer_coeff_t int_coeff, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(int_coeff) - lc;
}

template<typename FieldT>
linear_combination<FieldT> operator-(const FieldT &field_coeff, const linear_combination<FieldT> &lc)
{
    return linear_combination<FieldT>(field_coeff) - lc;
}

template<typename FieldT>
linear_combination<FieldT>::linear_combination(const std::vector<linear_term<FieldT> > &all_terms)
{
    if (all_terms.empty())
    {
        return;
    }

    m_terms = all_terms;
    std::sort(m_terms.begin(), m_terms.end(), [](linear_term<FieldT> a, linear_term<FieldT> b) { return a.index < b.index; });

    auto result_it = m_terms.begin();
    for (auto it = ++m_terms.begin(); it != m_terms.end(); ++it)
    {
        if (it->index == result_it->index)
        {
            result_it->coeff += it->coeff;
        }
        else
        {
            *(++result_it) = *it;
        }
    }
    m_terms.resize((result_it - m_terms.begin()) + 1);
}

} // libsnark

#endif // VARIABLE_TCC
