#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// funzione main del processo figlio
int main(int argc, char *argv[])
{
    // test loop
    int i = 0;
    for (;;) {
        // test indice per chiusura forzata
        if (i++ > 100) {
            // chiusura forzata
            printf("%s: il processo esce\n", argv[0]);
            return EXIT_FAILURE;
        }

        // thread sleep (100 ms)
        nanosleep((const struct timespec[]){{0, 100000000L}}, NULL);
    }

    // esco con Ok
    return EXIT_SUCCESS;
}
