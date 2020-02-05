#ifndef __SIGNID_H
#define __SIGNID_H

#include "gmp.h"

#include "elliptic/AffinePoint.h"
#include "identity-based/Signature.h"
#include "identity-based/PublicParameters.h"
#include "identity-based/SecurityLevel.h"
#include "util/Status.h"

/**
 * Establishes a master secret and public parameters for a given security level. The master secret (as its name suggests)
 * should be kept secret, while the public parameters can be distributed among the clients.
 * @param securityLevel the desired security level
 * @param publicParameters Pointer in which the public parameters will be stored. If the return value is CRYPTID_SUCCESS
 *                         then it will point to correctly filled PublicParameters instance. Note, that the
 *                         q field must be mpz_init'd and thus mpz_clear'd be the caller.
 * @param masterSecret Out parameter which will hold the master secret. Must be mpz_init'd and mpz_clear'd by the caller.
 * @return CRYPTID_SUCCESS if everything went right
 */
CryptidStatus signid_setup(const SecurityLevel securityLevel, PublicParameters* publicParameters, mpz_t masterSecret);

/**
 * Extracts the private key corresponding to a given identity string.
 * @param result Out parameter holding the private key (which is an AffinePoint). On CRYPTID_SUCCESS, it must be destroyed by 
 *               the caller. Initialization is done by this function.
 * @param identity the identity string we're extracting the private key for
 * @param identityLength the length of the identity string
 * @param publicParameters the IBS public parameters
 * @param masterSecret the master secret corresponding to the public parameters
 * @return CRYPTID_SUCCESS if everything went right
 */
CryptidStatus signid_extract(AffinePoint* result, const char *const identity, const size_t identityLength,
                    const PublicParameters publicParameters, const mpz_t masterSecret);

/**
 * Signs a message with the given identity string.
 * @param result Out parameter storing the signature. If the return value is CRYPTID_SUCCESS
 *               then it will point to a Signature instance, that must be destroyed by the caller.
 *               Initialization is done by this function.
 * @param privateKey the private key to sign with
 * @param message the string to sign
 * @param messageLength the length of the message
 * @param identity the identity string to sign with
 * @param identityLength the length of the identity
 * @param publicParameters the IBS public parameters
 * @return CRYPTID_SUCCESS if everything went right
 */
CryptidStatus signid_sign(Signature *result, const AffinePoint privateKey, const char *const message, const size_t messageLength,
                    const char *const identity, const size_t identityLength, const PublicParameters publicParameters);

/**
 * Verifies the signature to a previously signed message.
 * @param message the string which was signed
 * @param messageLength the length of the message
 * @param signature the digital signature of the message, which needs to be verified
 * @param identity the identity string the message was signed with
 * @param identityLength the length of the identity
 * @param publicParameters the IBS public parameters
 * @return CRYPTID_SUCCESS if the signature was valid
 */
CryptidStatus signid_verify(const char *const message, const size_t messageLength, const Signature signature,
                    const char *const identity, const size_t identityLength, const PublicParameters publicParameters);

#endif
