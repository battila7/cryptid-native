#include <string.h>
#include <stdlib.h>

#include "CryptID_ABE.h"
#include "elliptic/TatePairing.h"
#include "util/RandBytes.h"
#include "util/Utils.h"
#include "util/Validation.h"


// References
//  * [RFC-5091] Xavier Boyen, Luther Martin. 2007. RFC 5091. Identity-Based Cryptography Standard (IBCS) #1: Supersingular Curve Implementations of the BF and BB1 Cryptosystems


static const unsigned int SOLINAS_GENERATION_ATTEMPT_LIMIT = 100;
static const unsigned int POINT_GENERATION_ATTEMPT_LIMIT = 100;

static const unsigned int Q_LENGTH_MAPPING[] = { 160, 224, 256, 384, 512 };
static const unsigned int P_LENGTH_MAPPING[] = { 512, 1024, 1536, 3840, 7680 };

CryptidStatus cryptid_setup_ABE(const SecurityLevel securityLevel, PublicKey_ABE* publickey, MasterKey_ABE* masterkey)
{
    // Construct the elliptic curve and its subgroup of interest
    // Select a random \f$n_q\f$-bit Solinas prime \f$q\f$.
    mpz_t q;
    mpz_init(q);

    CryptidStatus status = random_solinasPrime(q, Q_LENGTH_MAPPING[(int) securityLevel], SOLINAS_GENERATION_ATTEMPT_LIMIT);

    if (status)
    {
        mpz_clear(q);
        return status;
    }

    // Select a random integer \f$r\f$, such that \f$p = 12 \cdot r \cdot q - 1\f$ is an \f$n_p\f$-bit prime.
    unsigned int lengthOfR = P_LENGTH_MAPPING[(int) securityLevel] - Q_LENGTH_MAPPING[(int) securityLevel] - 3;
    mpz_t r, p;
    mpz_inits(r, p, NULL);
    do
    {
        random_mpzOfLength(r, lengthOfR);
        mpz_mul_ui(p, r, 12);
        mpz_mul(p, p, q);
        mpz_sub_ui(p, p, 1);
    }
    while (!validation_isProbablePrime(p));

    mpz_t zero, one;
    mpz_init_set_ui(zero, 0);
    mpz_init_set_ui(one, 1);

    EllipticCurve ec = ellipticCurve_init(zero, one, p);


    // Select a point \f$P\f$ of order \f$q\f$ in \f$E(F_p)\f$.
    AffinePoint pointP;
    do
    {
        AffinePoint pointPprime;
        status = random_affinePoint(&pointPprime, ec, POINT_GENERATION_ATTEMPT_LIMIT);

        if (status)
        {
            mpz_clears(p, q, r, NULL);
            ellipticCurve_destroy(ec);
            return status;
        }

        mpz_t rMul;
        mpz_init_set(rMul, r);
        mpz_mul_ui(rMul, rMul, 12);

        status = AFFINE_MULTIPLY_IMPL(&pointP, rMul, pointPprime, ec);

        if (status)
        {
            mpz_clears(p, q, r, rMul, NULL);
            ellipticCurve_destroy(ec);
            affine_destroy(pointPprime);
            return status;
        }

        mpz_clear(rMul);
        affine_destroy(pointPprime);
    }
    while (affine_isInfinity(pointP));

    mpz_t pMinusOne;
    mpz_init(pMinusOne);
    mpz_sub_ui(pMinusOne, p, 1);

    mpz_t alpha;
    mpz_init(alpha);
    random_mpzInRange(alpha, pMinusOne);

    mpz_t beta;
    mpz_init(beta);
    random_mpzInRange(beta, pMinusOne);

    publickey->ellipticCurve = ec;
    publickey->g = pointP;

    status = AFFINE_MULTIPLY_IMPL(&publickey->h, beta, publickey->g, publickey->ellipticCurve);
    if(status) {
        // TO DO clears
        return status;
    }

    mpz_t beta_inverse;
    mpz_init(beta_inverse);
    mpz_invert(beta_inverse, beta, publickey->ellipticCurve.fieldOrder);

    status = AFFINE_MULTIPLY_IMPL(&publickey->f, beta_inverse, publickey->g, publickey->ellipticCurve);
    if(status) {
        // TO DO clears
        return status;
    }

    mpz_init(masterkey->beta);
    mpz_set(masterkey->beta, beta);

    status = AFFINE_MULTIPLY_IMPL(&masterkey->g_alpha, alpha, publickey->g, publickey->ellipticCurve);
    if(status) {
        // TO DO clears
        return status;
    }

    mpz_init(publickey->q);
    mpz_set(publickey->q, q);

    publickey->hashFunction = hashFunction_initForSecurityLevel(securityLevel);

    Complex pairValue;
    status = tate_performPairing(&pairValue, 2, ec, q, pointP, pointP);
    if(status)
    {
        return status;
    }

    Complex eggalpha = complex_modPow(pairValue, alpha, publickey->ellipticCurve.fieldOrder);
    publickey->eggalpha = eggalpha;

    masterkey->pubkey = publickey;

    mpz_clears(zero, one, NULL);
    mpz_clears(p, q, r, pMinusOne, alpha, beta, beta_inverse, NULL);

    return CRYPTID_SUCCESS;
}

CryptidStatus compute_tree(AccessTree* accessTree, mpz_t s, PublicKey_ABE* publickey)
{
    int d = accessTree->value - 1; // dx = kx-1, degree = threshold-1

    Polynom* q = createPolynom(d, s, publickey);
    //accessTree->polynom = q;
    if(!isLeaf(accessTree))
    {
        int i;
        for(i = 1; i < MAX_CHILDREN; i++)
        {
            if(accessTree->children[i] != NULL)
            {
                mpz_t sum;
                mpz_init(sum);
                polynomSum(q, i, sum);
                compute_tree(accessTree->children[i], sum, publickey);
                mpz_clear(sum);
            }
        }
    }
    else
    {
        AffinePoint Cy;
        CryptidStatus status = AFFINE_MULTIPLY_IMPL(&Cy, s, publickey->g, publickey->ellipticCurve);
        if(status) {
            // TO DO clears
            return status;
        }

        AffinePoint hashedPoint;

        status = hashToPoint(&hashedPoint, publickey->ellipticCurve, publickey->ellipticCurve.fieldOrder,
                                publickey->q, accessTree->attribute, accessTree->attributeLength, publickey->hashFunction);

        if (status) 
        {
            return status;
        }

        AffinePoint CyA;
        status = AFFINE_MULTIPLY_IMPL(&CyA, s, hashedPoint, publickey->ellipticCurve);
        if(status) {
            return status;
        }

        accessTree->Cy = Cy;
        accessTree->CyA = CyA;
        // TO DO AffinePoint destroy
    }

    return CRYPTID_SUCCESS;
}

CryptidStatus cryptid_encrypt_ABE(EncryptedMessage_ABE* encrypted,
                                const char *const message, const size_t messageLength,
                                    PublicKey_ABE* publickey, AccessTree* accessTree)
{
    if(!message)
    {
        return CRYPTID_MESSAGE_NULL_ERROR;
    }

    if(messageLength == 0)
    {
        return CRYPTID_MESSAGE_LENGTH_ERROR;
    }

    if(!publickey) {
        return CRYPTID_MESSAGE_NULL_ERROR;
    }

    if(!accessTree) {
        return CRYPTID_MESSAGE_NULL_ERROR;
    }

    mpz_t pMinusOne;
    mpz_init(pMinusOne);
    mpz_sub_ui(pMinusOne, publickey->ellipticCurve.fieldOrder, 1);

    unsigned long values[messageLength];
    for(size_t i = 0; i < messageLength; i++) 
    {
        values[i] = (long) message[i];
    }

    mpz_t M;
    mpz_init(M);
    mpz_import (M, messageLength, 1, sizeof(values[0]), 0, 0, values);
    gmp_printf("M: %Zd\n", M);
    // TO DO check - M splitting if M.length>=P.length

    mpz_t s;
    mpz_init(s);
    random_mpzInRange(s, pMinusOne);
    compute_tree(accessTree, s, publickey);

    encrypted->tree = accessTree;
    Complex eggalphas = complex_modPow(publickey->eggalpha, s, publickey->ellipticCurve.fieldOrder);
    Complex Ctilde = complex_modMulScalar(eggalphas, M, publickey->ellipticCurve.fieldOrder);
    encrypted->Ctilde = Ctilde;

    CryptidStatus status = AFFINE_MULTIPLY_IMPL(&encrypted->C, s, publickey->h, publickey->ellipticCurve);
    if(status) {
        // TO DO clears
        return status;
    }

    mpz_clears(M, pMinusOne, s, NULL);

    return CRYPTID_SUCCESS;
}

CryptidStatus cryptid_keygen_ABE(MasterKey_ABE* masterkey, char** attributes, SecretKey_ABE* secretkey)
{
    PublicKey_ABE* publickey = masterkey->pubkey;

    AffinePoint Gr;

    mpz_t r;
    mpz_init(r);
    ABE_randomNumber(r, publickey);

    CryptidStatus status = AFFINE_MULTIPLY_IMPL(&Gr, r, publickey->g, publickey->ellipticCurve);
    if(status) {
        return status;
    }

    AffinePoint Gar;
    status = AFFINE_MULTIPLY_IMPL(&Gar, r, masterkey->g_alpha, publickey->ellipticCurve);
    if(status) {
        return status;
    }
    //affine_add(&Gar, masterkey->g_alpha, Gr, publickey->ellipticCurve);

    AffinePoint GarBi;

    mpz_t beta_inverse;
    mpz_init(beta_inverse);
    mpz_invert(beta_inverse, masterkey->beta, publickey->ellipticCurve.fieldOrder);

    status = AFFINE_MULTIPLY_IMPL(&GarBi, beta_inverse, Gar, publickey->ellipticCurve);
    if(status) {
        // TO DO clears
        return status;
    }

    secretkey->D = GarBi;

    for(int i = 0; i < MAX_ATTRIBUTES; i++)
    {
        if(attributes[i][0] != '\0')
        {
            int attributeLength = 0;
            for(int c = 0; c < ATTRIBUTE_LENGTH; c++) {
                if(attributes[i][c] == '\0') {
                    break;
                }
                attributeLength++;
            }

            mpz_t rj;
            mpz_init(rj);
            ABE_randomNumber(rj, publickey);

            AffinePoint Hj;

            status = hashToPoint(&Hj, publickey->ellipticCurve, publickey->ellipticCurve.fieldOrder,
                                    publickey->q, attributes[i], attributeLength, publickey->hashFunction);

            if(status) {
                return status;
            }


            AffinePoint HjRj;

            status = AFFINE_MULTIPLY_IMPL(&HjRj, rj, Hj, publickey->ellipticCurve);
            if(status) {
                return status;
            }

            AffinePoint Dj;
            affine_add(&Dj, HjRj, Gr, publickey->ellipticCurve);
            secretkey->Dj[i] = Dj;

            AffinePoint DjA;
            status = AFFINE_MULTIPLY_IMPL(&DjA, rj, publickey->g, publickey->ellipticCurve);
            if(status) {
                return status;
            }
            secretkey->DjA[i] = DjA;

            secretkey->attributes[i] = malloc(strlen(attributes[i]) + 1);
            strcpy(secretkey->attributes[i], attributes[i]);

            secretkey->pubkey = masterkey->pubkey;

            mpz_clear(rj);
        }
    }

    mpz_clear(beta_inverse);

    return CRYPTID_SUCCESS;
}

int Lagrange_coefficient(int xi, int* S, int sLength, int x)
{
    int result = 1;
    for(int i = 0; i < sLength; i++)
    {
        if(&S[i] != NULL)
        {
            if(S[i] != xi)
            {
                result = result*((x-S[i])/(xi-S[i]));
            }
        }
    }
    return result;
}

CryptidStatus DecryptNode_ABE(EncryptedMessage_ABE* encrypted, SecretKey_ABE* secretkey, AccessTree* node, Complex* result, int* statusCode)
{
    if(isLeaf(node))
    {
        int found = -1;
        for(int i = 0; i < MAX_ATTRIBUTES; i++)
        {
            if(secretkey->attributes[i][0] != '\0')
            {
                if(strcmp(secretkey->attributes[i], node->attribute) == 0)
                {
                    found = i;
                    break;
                }
            }
        }
        if(found >= 0)
        {
            Complex pairValue;
            CryptidStatus status = tate_performPairing(&pairValue, 2, secretkey->pubkey->ellipticCurve, secretkey->pubkey->q, secretkey->Dj[found], node->Cy);
            if(status)
            {
                return status;
            }

            Complex pairValueA;
            status = tate_performPairing(&pairValueA, 2, secretkey->pubkey->ellipticCurve, secretkey->pubkey->q, secretkey->DjA[found], node->CyA);
            if(status)
            {
                return status;
            }

            Complex pairValueA_inverse;
            status = complex_multiplicativeInverse(&pairValueA_inverse, pairValueA, secretkey->pubkey->ellipticCurve.fieldOrder);
            if(status)
            {
                return status;
            }

            *result = complex_modMul(pairValue, pairValueA_inverse, secretkey->pubkey->ellipticCurve.fieldOrder);

            *statusCode = 1;
        }
    }
    else
    {
        Complex* Sx[MAX_CHILDREN];
        int num = 0;
        for(int i = 0; i < MAX_CHILDREN; i++)
        {
            if(node->children[i] != NULL)
            {
                Complex F;
                int code = 0;
                DecryptNode_ABE(encrypted, secretkey, node->children[i], &F, &code);
                if(code == 1)
                {
                    Sx[i] = &F;
                    num++;
                }
            }
        }

        if(num > 0)
        {
            int indexes[num];
            int c = 0;
            for(int i = 0; i < MAX_CHILDREN; i++)
            {
                if(Sx[i] != NULL)
                {
                    indexes[c] = i;
                    c++;
                }
            }

            Complex Fx = complex_initLong(1, 0);
            for(int i = 0; i < num; i++)
            {
                int result = Lagrange_coefficient(i, indexes, num, 0);
                mpz_t resultMpz;
                mpz_init_set_ui(resultMpz, result);
                Complex tmp = complex_modPow(*Sx[indexes[c]], resultMpz, secretkey->pubkey->ellipticCurve.fieldOrder); // Sx[indexes[c]] ^ result
                Fx = complex_modMul(Fx, tmp, secretkey->pubkey->ellipticCurve.fieldOrder);
                mpz_clear(resultMpz);
                //Fx = Fx * Sx[indexes[c]] ^ result
            }
            result = &Fx;
            *statusCode = 1;
        }
    }
    
    return CRYPTID_SUCCESS;
}

CryptidStatus cryptid_decrypt_ABE(char *result, EncryptedMessage_ABE* encrypted, SecretKey_ABE* secretkey)
{
    int satisfy = satisfyValue(encrypted->tree, secretkey->attributes);
    if(satisfy == 0)
    {
        return CRYPTID_ILLEGAL_PRIVATE_KEY_ERROR; // TO DO
    }
    Complex A;
    int code = 0;
    DecryptNode_ABE(encrypted, secretkey, encrypted->tree, &A, &code);
    if(code == 0)
    {
        return CRYPTID_ILLEGAL_PRIVATE_KEY_ERROR; // TO DO
    }

    Complex Ctilde_A = complex_modMul(encrypted->Ctilde, A, secretkey->pubkey->ellipticCurve.fieldOrder);

    Complex eCD;
    CryptidStatus status = tate_performPairing(&eCD, 2, secretkey->pubkey->ellipticCurve, secretkey->pubkey->q, encrypted->C, secretkey->D);
    if(status)
    {
        return status;
    }

    Complex eCD_inverse;
    status = complex_multiplicativeInverse(&eCD_inverse, eCD, secretkey->pubkey->ellipticCurve.fieldOrder);
    if(status)
    {
        return status;
    }

    Complex decrypted = complex_modMul(Ctilde_A, eCD_inverse, secretkey->pubkey->ellipticCurve.fieldOrder);

    /*int zLength;
    unsigned char* z = canonical(&zLength, secretkey->pubkey->ellipticCurve.fieldOrder, decrypted, 1);
    
    
    printf("z: %s\n", z);
    printf("zLength: %d\n", zLength);*/

    printf("result: %s\n", result);

    gmp_printf("real: %Zd\n", decrypted.real);
    gmp_printf("imaginary: %Zd\n", decrypted.imaginary);

    return CRYPTID_SUCCESS;
}