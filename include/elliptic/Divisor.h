#ifndef __CRYPTID_DIVISOR_H
#define __CRYPTID_DIVISOR_H

#include "complex/Complex.h"
#include "elliptic/AffinePoint.h"
#include "elliptic/ComplexAffinePoint.h"
#include "elliptic/EllipticCurve.h"
#include "util/Status.h"

/**
 * Evaluates the divisor of a vertical line on a Type-1 elliptic curve.
 * @param result the resulting element of \f$F_p^2\f$ that is the divisor of the
 * vertical line going through \f$A\f$ evaluated at \f$B\f$
 * @param a a point in \f$E(F_p)\f$
 * @param b a point \f$E(F_p^2)\f$
 * @param ec the elliptic curve to operate on
 */
void divisor_evaluateVertical(Complex *result, const AffinePoint a,
                              const ComplexAffinePoint b,
                              const EllipticCurve ec);

/**
 * Evaluates the divisor of a tangent on a Type-1 elliptic curve.
 * @param result Out parameter: an element of \f$F_p^2\f$ that is the divisor of
 * the line tangent to \f$A\f$ evaluated at \f$B\f$. On CRYPTID_SUCCESS, this
 * should be destroyed by the caller.
 * @param a a point in \f$E(F_p)\f$
 * @param b a point \f$E(F_p^2)\f$
 * @param ec the elliptic curve to operate on
 * @result CRYPTID_SUCCESS if everything went right
 */
CryptidStatus divisor_evaluateTangent(Complex *result, const AffinePoint a,
                                      const ComplexAffinePoint b,
                                      const EllipticCurve ec);

/**
 * Evaluates the divisor of a line on a Type-1 elliptic curve.
 * @param result Out parameter: an element of \f$F_p^2\f$ that is the divisor of
 * the line going through \f$A^{\prime}\f$ and \f$A^{\prime\prime}\f$ evaluated
 * at \f$B\f$. On CRYPTID_SUCCESS, this should be destroyed by the caller.
 * @param a a point in \f$E(F_p)\f$
 * @param aprime a point in \f$E(F_o)\f$
 * @param b a point in \f$E(F_p^2)\f$
 * @param ec the elliptic curve to operate on
 * @return CRYPTID_SUCCESS if everything went right
 */
CryptidStatus divisor_evaluateLine(Complex *result, const AffinePoint a,
                                   const AffinePoint aprime,
                                   const ComplexAffinePoint b,
                                   const EllipticCurve ec);

#endif
