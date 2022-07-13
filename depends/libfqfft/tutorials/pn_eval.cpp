#include <cstdio>
#include <memory>
#include <vector>

#include <libff/common/double.hpp>
#include "../libfqfft/polynomial_arithmetic/basic_operations.hpp"
#include "../libfqfft/polynomial_arithmetic/naive_evaluate.hpp"

using namespace libfqfft;

/* Polynomial Evaluation */
template <typename FieldT>
void pn_eval ()
{

  /* Evaluation vector */
  std::vector<FieldT> a = { 1,1 };
  std::vector<FieldT> b = { 1,0,2};

  std::vector<FieldT>c;
  _polynomial_multiplication_on_fft(c,a,b);

  for (size_t i = 0; i < c.size(); i++)
  {
    printf("%ld: %ld\n", i, c[i].as_ulong());
  }

  FieldT t=1;
  auto v=evaluate_polynomial(c.size(),c,t);
  printf("v=%lu",v.as_ulong());

}

int main()
{
  pn_eval<libff::Double> ();
}
