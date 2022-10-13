// Aleth: Ethereum C++ client, tools and libraries.
// Copyright 2017-2019 Aleth Authors.
// Licensed under the GNU General Public License, Version 3.

#include <libff/algebra/curves/alt_bn128/alt_bn128_g1.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_g2.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pairing.hpp>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/common/profiling.hpp>

#include <Snark/Snark.h>

namespace
{

/// Concatenate the contents of a container onto a vector
zbytes operator+(zbytesConstRef _a, zbytesConstRef _b)
{
  zbytes c(_a.begin(), _a.end());
  c.insert(c.end(), _b.begin(), _b.end());
  return c;
}

// Create a copy of b from "from" to "to".
// TODO: Use a custom data structure that can create reference for the sub array.
//       (that's what the aleth implementation's zbytesConstRef::cropped() does).
zbytes splice(zbytesConstRef b, size_t from, size_t to)
{
  if (to > b.size() || from > to)
    throw SnarkExn ("Snark: splice: Invalid arguments");
  zbytes r (b.begin() + from, b.begin() + to);
  return r;
}

void initLibSnark() noexcept
{
  static bool s_initialized = []() noexcept
  {
    libff::inhibit_profiling_info = true;
    libff::inhibit_profiling_counters = true;
    libff::alt_bn128_pp::init_public_params();
    return true;
  }();
  (void)s_initialized;
}

libff::bigint<libff::alt_bn128_q_limbs> toLibsnarkBigint(zbytesConstRef _x)
{
  libff::bigint<libff::alt_bn128_q_limbs> b;
  auto const N = b.N;
  constexpr size_t L = sizeof(b.data[0]);
  static_assert(sizeof(mp_limb_t) == L, "Unexpected limb size in libff::bigint.");
  for (size_t i = 0; i < N; i++)
    for (size_t j = 0; j < L; j++)
      b.data[N - 1 - i] |= mp_limb_t(_x[i * L + j]) << (8 * (L - 1 - j));
  return b;
}

zbytes fromLibsnarkBigint(libff::bigint<libff::alt_bn128_q_limbs> const& _b)
{
  static size_t const N = static_cast<size_t>(_b.N);
  static size_t const L = sizeof(_b.data[0]);
  static_assert(sizeof(mp_limb_t) == L, "Unexpected limb size in libff::bigint.");
  zbytes x(32);
  for (size_t i = 0; i < N; i++)
    for (size_t j = 0; j < L; j++)
      x[i * L + j] = uint8_t(_b.data[N - 1 - i] >> (8 * (L - 1 - j)));
  return x;
}

libff::alt_bn128_Fq decodeFqElement(zbytesConstRef _data)
{
  if (_data.size() != 32)
    throw SnarkExn ("Snark: decodeFqElement: Invalid input");

#if 0 // TODO: 256 bit arithmetic leads to a dependency on boost.
  // TODO: Consider using a compiler time constant for comparison.
  if (u256(xbin) >= u256(fromLibsnarkBigint(libff::alt_bn128_Fq::mod)))
    throw SnarkExn ("Snark: decodeFqElement: Invalid encoding");
#endif

  return toLibsnarkBigint(_data);
}

libff::alt_bn128_G1 decodePointG1(zbytesConstRef _data)
{
  libff::alt_bn128_Fq x = decodeFqElement(splice(_data,0, 32));
  libff::alt_bn128_Fq y = decodeFqElement(splice(_data, 32, 64));
  if (x == libff::alt_bn128_Fq::zero() && y == libff::alt_bn128_Fq::zero())
    return libff::alt_bn128_G1::zero();
  libff::alt_bn128_G1 p(x, y, libff::alt_bn128_Fq::one());
  if (!p.is_well_formed())
    throw SnarkExn ("Snark: decodePointG1: Invalid input");
  return p;
}

zbytes encodePointG1(libff::alt_bn128_G1 _p)
{
  if (_p.is_zero())
    return zbytes(64, 0);
  _p.to_affine_coordinates();
  return
    fromLibsnarkBigint(_p.X.as_bigint()) + fromLibsnarkBigint(_p.Y.as_bigint());
}

libff::alt_bn128_Fq2 decodeFq2Element(zbytesConstRef _data)
{
  // Encoding: c1 (256 bits) c0 (256 bits)
  // "Big endian", just like the numbers
  return libff::alt_bn128_Fq2(
    decodeFqElement(splice(_data, 32, 64)),
    decodeFqElement(splice(_data, 0, 32))
  );
}

libff::alt_bn128_G2 decodePointG2(zbytesConstRef _data)
{
  libff::alt_bn128_Fq2 const x = decodeFq2Element(splice(_data, 0, 64));
  libff::alt_bn128_Fq2 const y = decodeFq2Element(splice(_data, 64, 128));
  if (x == libff::alt_bn128_Fq2::zero() && y == libff::alt_bn128_Fq2::zero())
    return libff::alt_bn128_G2::zero();
  libff::alt_bn128_G2 p(x, y, libff::alt_bn128_Fq2::one());
  if (!p.is_well_formed())
    throw SnarkExn ("Snark: decodePointG2: Invalid input");
  return p;
}

}

// _in is a list of pairs. Each pair is of size 2*32 + 2*64.
zbytes alt_bn128_pairing_product(zbytesConstRef _in)
{
  // Input: list of pairs of G1 and G2 points
  // Output: 1 if pairing evaluates to 1, 0 otherwise (left-padded to 32 bytes)

  size_t constexpr pairSize = 2 * 32 + 2 * 64;
  size_t const pairs = _in.size() / pairSize;
  if (pairs * pairSize != _in.size())
    // Invalid length.
    throw SnarkExn ("Snark: alt_bn128_pairing_product: Invalid input length");

  initLibSnark();
  libff::alt_bn128_Fq12 x = libff::alt_bn128_Fq12::one();
  for (size_t i = 0; i < pairs; ++i)
  {
    zbytesConstRef pair = splice(_in, i * pairSize, (i * pairSize) + pairSize);
    libff::alt_bn128_G1 const g1 = decodePointG1(splice(pair, 0, 2 * 32));
    libff::alt_bn128_G2 const p = decodePointG2(splice(pair, 2 * 32, 2 * 32 + 2 * 64));
    if (-libff::alt_bn128_G2::scalar_field::one() * p + p != libff::alt_bn128_G2::zero())
      // p is not an element of the group (has wrong order)
      throw SnarkExn ("Snark: alt_bn128_pairing_product: incorrect order");
    if (p.is_zero() || g1.is_zero())
      continue; // the pairing is one
    x = x * libff::alt_bn128_miller_loop(
      libff::alt_bn128_precompute_G1(g1),
      libff::alt_bn128_precompute_G2(p)
    );
  }

  zbytes ret(32, 0);
  if (libff::alt_bn128_final_exponentiation(x) == libff::alt_bn128_GT::one()) {
    ret[31] = 1; // big-endian 1.
    return ret;
  } else {
    return ret;  // big-endian 0.
  }
}

zbytes alt_bn128_G1_add(zbytesConstRef _p1, zbytesConstRef _p2)
{
  if (_p1.size() != 64 || _p2.size() != 64)
    throw SnarkExn("Snark: alt_bn128_G1_add: Invalid input");

  initLibSnark();
  libff::alt_bn128_G1 const p1 = decodePointG1(_p1);
  libff::alt_bn128_G1 const p2 = decodePointG1(_p2);
  return encodePointG1(p1 + p2);
}

zbytes alt_bn128_G1_mul(zbytesConstRef _p1, zbytesConstRef _s)
{
  if (_p1.size() != 64 || _s.size() != 32)
    throw SnarkExn("Snark: alt_bn128_G1_mul: Invalid input");

  initLibSnark();
  libff::alt_bn128_G1 const p = decodePointG1(_p1);
  libff::alt_bn128_G1 const result = toLibsnarkBigint(_s) * p;
  return encodePointG1(result);
}

zbytes alt_bn128_G1_neg(zbytesConstRef _p)
{
  if (_p.size() != 64)
    throw SnarkExn("Snark: alt_bn128_G1_neg: Invalid input");

  initLibSnark();
  libff::alt_bn128_G1 const p = decodePointG1(_p);
  return encodePointG1(-p);
}
