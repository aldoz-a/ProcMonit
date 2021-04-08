#ifndef PROCMONIT_H_
#define PROCMONIT_H_

#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>

// struttura dati per ProcMonit
typedef struct {
    // dati del processo figlio da monitorare
    const char*  file;              // nome del file da eseguire
    char* const* argv;              // lista di argomenti del programma da eseguire
    pid_t        pid;               // pid del processo figlio avviato

    // dati del thread del monitor
    pthread_t    t_procmonit;       // descrittore del thread
    bool         stop_t_procmonit;  // flag per lo stop del thread
} ProcMonit;

// prototipi globali
void pm_init(ProcMonit *pm, const char* pm_file, char* const pm_argv[]);
void pm_start(ProcMonit *pm);
void pm_stop(ProcMonit *pm);

#endif  // PROCMONIT_H_
