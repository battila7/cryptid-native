#ifndef __CRYPTID_TATEPAIRING_H
#define __CRYPTID_TATEPAIRING_H

#include "gmp.h"

#include "elliptic/AffinePoint.h"
#include "elliptic/ComplexAffinePoint.h"
#include "elliptic/EllipticCurve.h"
#include "complex/Complex.h"
#include "util/Status.h"

/**
 * Computes the Tate pairing over Type-1 elliptic curves.
 * @param result Out parameter to the resulting Complex value. On CRYPTID_SUCCESS, this should be destroyed by the caller.
 * @param p a point of \f$E[r]\f$
 * @param b
 * @param embeddingDegree the embedding degree of the curve
 * @param subgroupOrder the order of the subgroup
 * @param ellipticCurve the elliptic curve to operate on
 * @return CRYPTID_SUCCESS if everything went right
 */
CryptidStatus tate_performPairing(Complex *result, const AffinePoint p, const AffinePoint b, const int embeddingDegree, const mpz_t subgroupOrder, const EllipticCurve ellipticCurve);

#endif
