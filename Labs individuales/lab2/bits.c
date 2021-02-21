/*
 * CS:APP Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */


#endif
//1
/*
 * bitAnd - x&y using only ~ and |
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  int result = (~x | ~y);
  return (~result);
}
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
  int operation_1 = ~(x & y);
  int operation_2 = ~(~x & ~y);
  return (operation_1 & operation_2);
}
//2
/*
 * negate - return -x
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  int result = (~x) + 1;
  return result;
}
/*
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
  int mask_ff = 0xFF;

  int bits_n = n << 3;
  int bits_m = m << 3;

  int byte_n = (x >> bits_n) & mask_ff;
  int byte_m = (x >> bits_m) & mask_ff;

  int bytes_swapped = (byte_n << bits_m) | (byte_m << bits_n);

  int mask1 = ~( (mask_ff << bits_m) | (mask_ff << bits_n));
  mask1 = x & mask1;

  return mask1 | bytes_swapped;
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int mask = 0xAA;
  mask = (mask << 8) | mask;
  mask = (mask << 16) | mask;
  return (!((x & mask) ^ mask));
}
//3
/*
 * bitMask - Generate a mask consisting of all 1's
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int bitMask(int highbit, int lowbit) {
int all_ones_mask = ~0;// Creo mi mascara de unos, la voy a usar para el corrimiento de bits
int high_bits = (all_ones_mask << highbit) << 1; //Desplazo hasta el bit que me dan del high
int low_bits = all_ones_mask << lowbit; //desplazo hasta el bit de low que me dan
return (high_bits ^ low_bits) & low_bits;// Hago un xor para asegurarme de que los unos que me sobran despues de mi bit
                                         //mas grande sean todos ceros y de que tenga solo unos entre los bits pedidos
                                         //por ultimo con el & me aseguro de que si low es mayor a high devuelvo 0x00
}
/*
 * conditional - same as x ? y : z
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  x = !!x; //Tengo una mascara de unos o ceros dependiendo si x es != 0 o == 0
  x = ~x + 1;// Niego esa mascara
  return ((x & y) | ((~x) & z));//Dependiendo del valor de x, uno de los & hace que esa operacion
                                //se vuelve todo cero y la otra solo sea copiar el valor de y o z
}
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int lower_limit = 0x2F; //47
  int upper_limit = 0x3A; //58

  lower_limit = (~lower_limit) + x; //Si x es menor que 0x2F o igual al realizar el corrimiento obtengo
  lower_limit = lower_limit >> 31;  //una mascara de unos (fuera del rango) o ceros (dentro del rango)

  upper_limit = upper_limit + (~x); //idem arriba, pero pasa si es mayor que 0x39
  upper_limit = upper_limit >> 31;
  return !(lower_limit | upper_limit);
}
//4
/*
 * isNonZero - Check whether x is nonzero using
 *              the legal operators except !
 *   Examples: isNonZero(3) = 1, isNonZero(0) = 0
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int isNonZero(int x) {
  int not_x = (~x + 1);//Cambio todos los ceros por unos y todos los unos por ceros
                       //como en los lugares que ahora tengo ceros o unos significa
                       //tenia unos o ceros respectivamente, por lo tanto con el or
                       //llenaría de unos los lugares con cero (caso que x != 0)
                       //Con el corrimiento de 31 bits me aseguro que sean todos unos o ceros
                       //y hago el and con el 1
  return ((x | not_x) >> 31) & 1;
}
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4
 */
int logicalNeg(int x) {
  int not_x = (~x + 1);//Cambio todos los ceros por unos y todos los unos por ceros
                       //como en los lugares que ahora tengo ceros o unos significa
                       //tenia unos o ceros respectivamente, por lo tanto con el or
                       //llenaría de unos los lugares con cero (caso que x != 0)
                       //Con el corrimiento de 31 bits me aseguro que sean todos unos o ceros
int mask = (~0);       //Creo una mascara de unos y hago un xor para simula la negacion
                       //Por ultimo hago el and con el uno para retornar el valor correcto
  return ((((x | not_x) >> 31) ^ mask) & 1);
}
//float
/*
 * floatNegate - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned floatNegate(unsigned uf) {
  int mask_bit_significant = (1 << 31);
  int mask_frac = uf & (~(mask_bit_significant >> 8));
  int mask_exp = 0xFF;
  mask_exp = mask_exp << 23;
  if(((mask_exp & uf) == mask_exp) && mask_frac){
    return uf;
  }

 return uf ^ mask_bit_significant;
}
/*
 * floatAbsVal - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned floatAbsVal(unsigned uf) {
  int mask_bit_significant = (1 << 31);
  int mask_frac = uf & (~(mask_bit_significant >> 8));
  int mask_exp = 0xFF;
  mask_exp = mask_exp << 23;
  if(((mask_exp & uf) == mask_exp) && mask_frac){
    return uf;
  }

 return uf & (~mask_bit_significant);
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int mask = 0x7F;
  int displacement;

  if(x < -150){
    return 0;
  }
  //caso desnormalizado
  if(x <= (-127) && x >= (-150)){
  displacement = (-x - mask);
  return (1 << (23 - displacement));
  }

  //caso normalizado
  if(x <= (127) && x >= (-126)){
    mask = mask + x;
    return (mask << 23);
  }
  //el resto de los casos, que seria x >= 128
  return (0xFF << 23);
}
