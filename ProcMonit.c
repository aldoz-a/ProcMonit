#define _GNU_SOURCE
#include "ProcMonit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>

// prototipi locali
static void *pm_thread(void *arg);

// pm_init - inizializza la struttura del monitor
void pm_init(ProcMonit *pm, const char *pm_file, char* const pm_argv[])
{
    // set/reset dati del monitor
    pm->file = pm_file;             // nome del file da eseguire
    pm->argv = pm_argv;             // lista di argomenti del programma da eseguire
    pm->pid = -1;                   // pid del processo figlio avviato
    pm->t_procmonit = 0;            // descrittore del thread
    pm->stop_t_procmonit = false;   // flag per lo stop del thread
}

// pm_start - avvia il monitor di un processo figlio
void pm_start(ProcMonit *pm)
{
    // test se il thread non è già allocato
    if (pm->t_procmonit == 0) {
        // start del thread di procmonit
        pm->stop_t_procmonit = false;       // reset flag per stop thread
        int error;
        if ((error = pthread_create(&pm->t_procmonit, NULL, &pm_thread, pm)) != 0) {
            // errore: lo mostro e azzero il descrittore del thread
            printf("%s: non posso creare il thread (%s)\n", __func__, strerror(error));
            pm->t_procmonit = 0;
        }
    }
}

// pm_stop - ferma il monitor di un processo figlio
void pm_stop(ProcMonit *pm)
{
    // test se il thread è allocato
    if (pm->t_procmonit) {
        // uccide il processo figlio
        kill(pm->pid, SIGKILL);

        // stop e join del thread di procmonit
        pm->stop_t_procmonit = true;    // set flag per lo stop del thread
        pthread_join(pm->t_procmonit, NULL);
        pm->t_procmonit = 0;            // reset del descrittore del thread
    }
}

// pm_thread - funzione per il thread di procmonit
static void *pm_thread(void *arg)
{
    // estraggo l'argomento con un cast
    ProcMonit *pm = (ProcMonit *)arg;

    // redirigo stdout e stderr su un file di log (questa parte è opzionale)
    char log_fname[64];
    snprintf(log_fname, sizeof(log_fname), "%s.log", pm->file);
    int fd = open(log_fname, O_WRONLY | O_CREAT | O_TRUNC, DEFFILEMODE);
    //dup2(fd, STDOUT_FILENO);  // invia stdout al file fd (opzionale)
    //dup2(fd, STDERR_FILENO);  // invia stderr al file fd (opzionale)
    close(fd);  // posso chiudere fd visto che è già stato duplicato

    // loop del thread
    while (!pm->stop_t_procmonit) {
        // fork del processo
        pid_t parent_pid = getpid();
        pm->pid = fork();

        // test dei pid dei processi
        char errmsg_buf[256];    // buffer per la strerror_r(): 256 è il size raccomandato
        if (pm->pid == 0) {
            // sono il figlio: con prctl() prenoto lo stop (via signal) se termina il padre
            if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1) {
                // errore nella prctl()
                fprintf(stderr, "%s: sono il figlio (%d): errore prtctl(): %s\n",
                                pm->argv[1], getpid(),
                                strerror_r(errno, errmsg_buf, sizeof(errmsg_buf)));
                exit(EXIT_FAILURE);
            }

            // nel caso che il processo padre esca prima di chiamare la prctl()
            if (getppid() != parent_pid) {
                // errore nella prctl()
                fprintf(stderr, "%s: sono il figlio (%d): errore prtctl()\n",
                                pm->argv[1], getpid());
                exit(EXIT_FAILURE);
            }

            // continua l'esecuzione del figlio eseguendo un nuovo processo
            fprintf(stderr, "%s: sono il figlio (%d): eseguo il nuovo processo: %s\n",
                            pm->argv[1], getpid(), pm->file);
            execv(pm->file, pm->argv);
            exit(EXIT_FAILURE);     // exec non ritorna mai
        }
        else if (pm->pid > 0) {
            // sono il padre: attendo l'uscita del figlio
            fprintf(stderr, "%s: sono il padre (%d): "
                            "attendo la terminazione del figlio %s (%d)\n",
                            pm->argv[1], getpid(), pm->file, pm->pid);
            int status;
            if (waitpid(pm->pid, &status, 0) != pm->pid) {
                // errore nella waitpid()
                fprintf(stderr, "%s: sono il padre (%d): "
                                "errore waitpid() figlio %s (%d): %s\n",
                                pm->argv[1], getpid(), pm->file, pm->pid,
                                strerror_r(errno, errmsg_buf, sizeof(errmsg_buf)));
            }
            else {
                // processo terminato: mostro lo status
                if (WIFEXITED(status))
                    fprintf(stderr, "%s: sono il padre (%d): "
                                    "figlio %s (%d) uscito ((status=%d))\n",
                                    pm->argv[1], getpid(), pm->file, pm->pid,
                                    WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    fprintf(stderr, "%s: sono il padre (%d): "
                                    "figlio %s (%d) ucciso dal segnale %d\n",
                                    pm->argv[1], getpid(), pm->file, pm->pid,
                                    WTERMSIG(status));
                else if (WIFSTOPPED(status))
                    fprintf(stderr, "%s: sono il padre (%d): "
                                    "figlio %s (%d) fermato dal segnale %d\n",
                                    pm->argv[1], getpid(), pm->file, pm->pid,
                                    WSTOPSIG(status));
                else
                    fprintf(stderr, "%s: sono il padre (%d): "
                                    "figlio %s (%d) terminato ((status=%d))\n",
                                    pm->argv[1], getpid(), pm->file, pm->pid, status);
            }
        }
        else {
            // errore nella fork(): mostro errore e continuo
            fprintf(stderr, "%s: processo %s: errore fork(): %s\n",
                    pm->argv[1], pm->file,
                    strerror_r(errno, errmsg_buf, sizeof(errmsg_buf)));
        }
    }

    // il thread esce
    pthread_exit(NULL);
}
