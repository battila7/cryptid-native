#ifndef __CRYPTID_BONEH_FRANKLIN_IDENTITY_BASED_ENCRYPTION_CIPHERTEXT_H
#define __CRYPTID_BONEH_FRANKLIN_IDENTITY_BASED_ENCRYPTION_CIPHERTEXT_H

#include <stddef.h>

#include "elliptic/AffinePoint.h"


// References
//  * [RFC-5091] Xavier Boyen, Luther Martin. 2007. RFC 5091. Identity-Based Cryptography Standard (IBCS) #1: Supersingular Curve Implementations of the BF and BB1 Cryptosystems


/**
 * Struct holding the ciphertext data. Refer to {@code BFCiphertextBlock} in [RFC-5091].
 */
typedef struct BonehFranklinIdentityBasedEncryptionCipherText
{
    /**
     * A point in \f$E(F_p)\f$.
     */
    AffinePoint cipherU;

    /**
     * A binary string.
     */
    unsigned char* cipherV;
    size_t cipherVLength;

    /**
     * A binary string.
     */
    unsigned char* cipherW;
    size_t cipherWLength;
} BonehFranklinIdentityBasedEncryptionCipherText;


/**
 * Returns a new BonehFranklinIdentityBasedEncryptionCipherText initialized with the specified values. Note, that {@code cipherV} and {@code cipherw}
 * will be copied.
 * @param cipherU an AffinePointer
 * @param cipherV binary string
 * @param cipherVLength the size of {@code cipherV}
 * @param cipherW binary string
 * @param cipherWLength the size of {@code cipherW}
 * @return a new BonehFranklinIdentityBasedEncryptionCipherText instance
 */
BonehFranklinIdentityBasedEncryptionCipherText bonehFranklinIdentityBasedEncryptionCipherText_init(const AffinePoint cipherU, const unsigned char *const cipherV, const int cipherVLength,
                                     const unsigned char *const cipherW, const int cipherWLength);

/**
 * Frees a BonehFranklinIdentityBasedEncryptionCipherText.
 * @param bonehFranklinIdentityBasedEncryptionCipherText the BonehFranklinIdentityBasedEncryptionCipherText to be destroyed
 */
void bonehFranklinIdentityBasedEncryptionCipherText_destroy(BonehFranklinIdentityBasedEncryptionCipherText bonehFranklinIdentityBasedEncryptionCipherText);

#endif
