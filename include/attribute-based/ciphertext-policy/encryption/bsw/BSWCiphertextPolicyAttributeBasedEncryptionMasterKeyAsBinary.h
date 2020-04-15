#ifndef __CRYPTID_CRYPTID_MASTERKEY_ABE_AS_BINARY_H
#define __CRYPTID_CRYPTID_MASTERKEY_ABE_AS_BINARY_H

#include "attribute-based/ciphertext-policy/encryption/bsw/BSWCiphertextPolicyAttributeBasedEncryptionMasterKey.h"
#include "attribute-based/ciphertext-policy/encryption/bsw/BSWCiphertextPolicyAttributeBasedEncryptionPublicKeyAsBinary.h"
#include "elliptic/AffinePointAsBinary.h"
#include <stdio.h>

typedef struct BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary
{
	void *beta;
	size_t betaLength;
	AffinePointAsBinary g_alpha;
	BSWCiphertextPolicyAttributeBasedEncryptionPublicKeyAsBinary* publickey;
} BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary;

void BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary_destroy(BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary* masterkey);

void bswChiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary_toBswChiphertextPolicyAttributeBasedEncryptionMasterKey(BSWCiphertextPolicyAttributeBasedEncryptionMasterKey *masterKey, const BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary *masterKeyAsBinary);

void bswChiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary_fromBswChiphertextPolicyAttributeBasedEncryptionMasterKey(BSWCiphertextPolicyAttributeBasedEncryptionMasterKeyAsBinary *masterKeyAsBinary, const BSWCiphertextPolicyAttributeBasedEncryptionMasterKey *masterKey);

#endif
