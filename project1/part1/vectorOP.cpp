#include "PPintrin.h"

// implementation of absSerial(), but it is vectorized using PP intrinsics
void absVector(float *values, float *output, int N)
{
  __pp_vec_float x;
  __pp_vec_float result;
  __pp_vec_float zero = _pp_vset_float(0.f);
  __pp_mask maskAll, maskIsNegative, maskIsNotNegative;

  //  Note: Take a careful look at this loop indexing.  This example
  //  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
  //  Why is that the case?
  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {

    // All ones
    maskAll = _pp_init_ones();

    // All zeros
    maskIsNegative = _pp_init_ones(0);

    // Load vector of values from contiguous memory addresses
    _pp_vload_float(x, values + i, maskAll); // x = values[i];

    // Set mask according to predicate
    _pp_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

    // Execute instruction using mask ("if" clause)
    _pp_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;

    // Inverse maskIsNegative to generate "else" mask
    maskIsNotNegative = _pp_mask_not(maskIsNegative); // } else {

    // Execute instruction ("else" clause)
    _pp_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }

    // Write results back to memory
    _pp_vstore_float(output + i, result, maskAll);
  }
}

void clampedExpVector(float *values, int *exponents, float *output, int N)
{
  //
  // PP STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  __pp_vec_float x, result;
  __pp_vec_int y, count;
  __pp_vec_int zero = _pp_vset_int(0);
  __pp_vec_int one = _pp_vset_int(1);
  __pp_vec_float limit = _pp_vset_float(9.999999);
  __pp_mask maskAll, maskIsZero, maskIsNonZero, maskExceed;

  for (int i = 0; i < N; i += VECTOR_WIDTH)
  {
    // Initialize mask for the current chunk
    int width = (i + VECTOR_WIDTH > N) ? (N - i) : VECTOR_WIDTH;
    maskAll = _pp_init_ones(width);

    // Load values and exponents from the input arrays into vector registers
    _pp_vload_float(x, values + i, maskAll);
    _pp_vload_int(y, exponents + i, maskAll);

    // Initialize maskIsZero to identify where exponents are zero
    maskIsZero = _pp_init_ones(0);
    _pp_veq_int(maskIsZero, y, zero, maskAll);

    _pp_vset_float(result, 1.0, maskIsZero);

    // Set result to the base value (x) where exponents are non-zero
    maskIsNonZero = _pp_mask_not(maskIsZero);
    _pp_vmove_float(result, x, maskIsNonZero);

    // Initialize count to the values of the exponents for further processing
    _pp_vmove_int(count, y, maskIsNonZero);
    _pp_vsub_int(count, count, one, maskIsNonZero);

    // Loop to compute the power using repeated multiplication
    while (_pp_cntbits(maskIsNonZero) > 0)
    {
      _pp_vmult_float(result, result, x, maskIsNonZero);
      _pp_vsub_int(count, count, one, maskIsNonZero);
      _pp_vgt_int(maskIsNonZero, count, zero, maskIsNonZero);
    }

    // Clamp results that exceed the limit
    _pp_vgt_float(maskExceed, result, limit, maskAll);
    _pp_vset_float(result, 9.999999, maskExceed);

    _pp_vstore_float(output + i, result, maskAll);
  }
}

// returns the sum of all elements in values
// You can assume N is a multiple of VECTOR_WIDTH
// You can assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{

  //
  // PP STUDENTS TODO: Implement your vectorized version of arraySumSerial here
  //
  __pp_vec_float sumVec = _pp_vset_float(0.0f);
  __pp_mask maskAll = _pp_init_ones();
  
  // Sum up all the elements in the array using vectorized addition
  for (int i = 0; i < N; i += VECTOR_WIDTH) {
    __pp_vec_float x;
    _pp_vload_float(x, values + i, maskAll);
    _pp_vadd_float(sumVec, sumVec, x, maskAll);
  }

  // Reduce the vector register to a single value
  for (int i = VECTOR_WIDTH; i > 1; i /= 2) {
    _pp_hadd_float(sumVec, sumVec);
    _pp_interleave_float(sumVec, sumVec);
  }

  // Return the final sum
  return sumVec.value[0];
}