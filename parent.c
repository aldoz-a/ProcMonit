#include "ProcMonit.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// nome del processo figlio da avviare
#define CHILD_PROC  "child"

// funzione main del processo padre
int main(int argc, char *argv[])
{
    // init struttura procmonit
    char* argv_pm[] = { (char*)CHILD_PROC, argv[0], (char*)NULL };
    ProcMonit procmonit;
    pm_init(&procmonit, CHILD_PROC, argv_pm);

    // avvio il processo figlio usando procmonit
    printf("%s: prenoto lo start del processo %s\n", argv[0], argv_pm[0]);
    pm_start(&procmonit);

    // main loop di test
    for (int i = 0; i < 10; i++) {
        printf("%s: loop interno di test: %d\n", argv[0], i);
        sleep(3);
    }

    // fermo il processo figlio usando procmonit
    printf("%s: prenoto lo stop del processo %s\n", argv[0], argv_pm[0]);
    pm_stop(&procmonit);

    // esco con Ok
    return EXIT_SUCCESS;
}
