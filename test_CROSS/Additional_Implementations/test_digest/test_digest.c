#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "api.h"
#include "csprng_hash.h"

/*
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
void simple_randombytes(unsigned char *x, unsigned long long xlen) {
    int fd = open("/dev/random", O_RDONLY);
    read(fd, x, xlen);
}
*/

void simple_randombytes(unsigned char *x, unsigned long long xlen) {
    for (unsigned long long i = 0; i < xlen; i++) {
        x[i] = (unsigned char) (rand() % 256);
    }
}

static void print_array(const char *name, unsigned char *array, unsigned long long len) {
    printf("%s: ", name);
    for (size_t i = 0; i < len; i++) {
        if(i < 3) printf("%02x", array[i]);
        if(i == len/2) printf(" ... ");
        if(i > (len-4)) printf("%02x", array[i]);
    }
    printf("\n");
}

#define NUM_TESTS 100000
#define DEBUG 100000000000
#define PROGRESS 300

int main() {
    
    unsigned char       *m, *sm, *m1;
    unsigned long long  mlen, smlen, mlen1;
    unsigned char       pk[CRYPTO_PUBLICKEYBYTES], sk[CRYPTO_SECRETKEYBYTES];

    unsigned char       entropy_input[48] = {0};

    mlen = 50;

    m = (unsigned char *)calloc(mlen, sizeof(unsigned char));
    m1 = (unsigned char *)calloc(mlen+CRYPTO_BYTES, sizeof(unsigned char));
    sm = (unsigned char *)calloc(mlen+CRYPTO_BYTES, sizeof(unsigned char));

    setbuf(stdout, NULL);
    
    printf("\nRunning %d keypair+sign+open with MLEN=%lld\n", NUM_TESTS, mlen);

    for(int i=0; i<NUM_TESTS; i++) {

        // TODO: move inside/outside the for loop
        simple_randombytes(entropy_input, 48);
        initialize_csprng(&platform_csprng_state, (const unsigned char *)entropy_input, 48);

        simple_randombytes(m, mlen);

        if ( crypto_sign_keypair(pk, sk) != 0) {
            printf("\n\n **** KEYPAIR ERROR ****\n\n");
            exit(-1);         
        }
        if ( crypto_sign(sm, &smlen, m, mlen, sk) != 0) {
            printf("\n\n **** SIGN ERROR ****\n\n");
            exit(-1);
        }
        if ( crypto_sign_open(m1, &mlen1, sm, smlen, pk) != 0) {
            printf("\n\n **** VERIFY ERROR ****\n\n");
            exit(-1);
        }

        if((i%PROGRESS == 0) && i) {
            printf(".");
            fflush(stdin);
        }

        if((i%DEBUG == 0) && i) {
            printf("\n");
            print_array("m ", m, mlen);
            print_array("m1", m1, mlen);
            print_array("sm", sm, smlen);
            print_array("pk", pk, CRYPTO_PUBLICKEYBYTES);
            print_array("sk", sk, CRYPTO_SECRETKEYBYTES);
        }

    }

    printf("\n");

    free(m);
    free(m1);
    free(sm);

}