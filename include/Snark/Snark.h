/*
 * Copyright (C) 2019 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <vector>
#include <string>

typedef std::vector<uint8_t> zbytes;
typedef const zbytes& zbytesConstRef;


struct SnarkExn {
  std::string m_msg;
  SnarkExn(std::string c) : m_msg(c) { }
};

// The input is a sequence of pairs, where each pair
// is a value of size (2 * 32) + (2 * 64).
// In other words, inputs (a1, b1, a2, b2, ..., ak, bk),
// where each ai and bi are points on an elliptic curve 
// and the function will return 1 or 0 (as 32 byte big-endian)
//  depending on whether the pairing check succeeds or fails.
zbytes alt_bn128_pairing_product(zbytesConstRef in);
// p1 is a 64 byte value (representing point (x, y)), and
// s is a scalar, a 32 byte big-endian number
zbytes alt_bn128_G1_mul(zbytesConstRef p1, zbytesConstRef s);
// p1 and p2 are 64 byte values, representing points (x, y),
// where each of x and y are 32 byte big-endian numbers
zbytes alt_bn128_G1_add(zbytesConstRef p1, zbytesConstRef p2);
// p is a 64 byte value (representing point (x, y))
zbytes alt_bn128_G1_neg(zbytesConstRef p);
