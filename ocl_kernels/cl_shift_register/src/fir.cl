/**
* Copyright (C) 2019-2021 Xilinx, Inc
*
* Licensed under the Apache License, Version 2.0 (the "License"). You may
* not use this file except in compliance with the License. A copy of the
* License is located at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
* WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
* License for the specific language governing permissions and limitations
* under the License.
*/

/*
  Finite Impulse Response(FIR) Filter

  This example demonstrates how to perform a shift register operation to
  implement a Finite Impulse Response(FIR) filter.

  Shift register operation is the cascading of values of an array by one or more
  places. Here is an example of what a shift register operation looks like on an
  array of length four:

                     ___       ___      ___      ___
  1. shift_reg[N] : | A |  <- | B | <- | C | <- | D |
                     ---       ---      ---      ---
                     ___       ___      ___      ___
  2. shift_reg[N] : | B |  <- | C | <- | D | <- | D |
                     ---       ---      ---      ---

  Here each of the values are copied into the register on the left. This type of
  operation is useful when you want to work on a sliding window of data or when
  the data is being streamed into the kernel.

  The Xilinx compiler can recognize this type of operation into the appropriate
  hardware. For example, the previous illustration can be coded using the
  following loop:

  #define N 4

  __attribute__((opencl_unroll_hint))
  for(int i = 0; i < N-1; i++) {
      shift_reg[i] = shift_reg[i+1];
  }

  The compiler needs to know the number of registers at compile time so the
  definition of N must be a compile time variable.

*/

// Number of coefficient components
#define N_COEFF 11

#define SIGNAL_SIZE (1024 * 1024)

// Tripcount identifiers
__constant int c_n = N_COEFF;
__constant int c_size = SIGNAL_SIZE;

// A naive implementation of the Finite Impulse Response filter.
__kernel __attribute__((reqd_work_group_size(1, 1, 1))) void fir_naive(__global int* restrict output_r,
                                                                       __global int* restrict signal_r,
                                                                       __global int* restrict coeff,
                                                                       int signal_length) {
    int coeff_reg[N_COEFF];
    __attribute__((xcl_loop_tripcount(c_n, c_n))) read_coef : for (int i = 0; i < N_COEFF; i++) coeff_reg[i] = coeff[i];

    __attribute__((xcl_loop_tripcount(c_size, c_size))) outer_loop : for (int j = 0; j < signal_length; j++) {
        int acc = 0;
    shift_loop:
        __attribute__((xcl_pipeline_loop(1))) for (int i = min(j, N_COEFF - 1); i >= 0; i--) {
            acc += signal_r[j - i] * coeff_reg[i];
        }
        output_r[j] = acc;
    }
}

// FIR using shift register
__kernel __attribute__((reqd_work_group_size(1, 1, 1))) void fir_shift_register(__global int* restrict output_r,
                                                                                __global int* restrict signal_r,
                                                                                __global int* restrict coeff,
                                                                                int signal_length) {
    int coeff_reg[N_COEFF];

    // Partitioning of this array is required because the shift register
    // operation will need access to each of the values of the array in
    // the same clock. Without partitioning the operation will need to
    // be performed over multiple cycles because of the limited memory
    // ports available to the array.
    int shift_reg[N_COEFF] __attribute__((xcl_array_partition(complete, 0)));

    __attribute__((xcl_loop_tripcount(c_n, c_n))) init_loop : for (int i = 0; i < N_COEFF; i++) {
        shift_reg[i] = 0;
        coeff_reg[i] = coeff[i];
    }

outer_loop:
    __attribute__((xcl_pipeline_loop(1)))
    __attribute__((xcl_loop_tripcount(c_size, c_size))) for (int j = 0; j < signal_length; j++) {
        int acc = 0;
        int x = signal_r[j];

        // This is the shift register operation. The N_COEFF variable is defined
        // at compile time so the compiler knows the number of operations
        // performed by the loop. This loop does not require the unroll
        // attribute because the outer loop will be pipelined so
        // the compiler will unroll this loop in the process.
        __attribute__((xcl_loop_tripcount(c_n, c_n))) shift_loop : for (int i = N_COEFF - 1; i >= 0; i--) {
            if (i == 0) {
                acc += x * coeff_reg[0];
                shift_reg[0] = x;
            } else {
                shift_reg[i] = shift_reg[i - 1];
                acc += shift_reg[i] * coeff_reg[i];
            }
        }
        output_r[j] = acc;
    }
}
