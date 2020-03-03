#ifndef __CRYPTID_CRYPTID_PUBLICKEY_ABE_H
#define __CRYPTID_CRYPTID_PUBLICKEY_ABE_H

#include "gmp.h"

#include "elliptic/AffinePoint.h"
#include "elliptic/EllipticCurve.h"
#include "identity-based/HashFunction.h"
#include "elliptic/TatePairing.h"

typedef struct PublicKey_ABE
{
	EllipticCurve ellipticCurve; // G0
	AffinePoint g; // generator of cyclic group
	AffinePoint h; // g^(beta)
	AffinePoint f; // g^(1/beta)
	Complex eggalpha; // e(g, g)^alpha
	HashFunction hashFunction;
	mpz_t q;
} PublicKey_ABE;

void destroyPublicKey_ABE(PublicKey_ABE* publickey);

#endif
