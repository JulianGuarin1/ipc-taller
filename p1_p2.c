{\rtf1\ansi\ansicpg1252\cocoartf2709
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 /* Archivo: p1_p2.c */\
#include <stdio.h>\
#include <stdlib.h>\
#include <unistd.h>\
#include <fcntl.h>\
#include <sys/mman.h>\
#include <sys/types.h>\
#include <semaphore.h>\
#include <string.h>\
#include <errno.h>\
\
#define SHM_NAME "/shared_memory"\
#define SEM_P1_TO_P3 "/sem_p1_to_p3"\
#define SEM_P2_TO_P3 "/sem_p2_to_p3"\
#define SEM_P3_TO_P1 "/sem_p3_to_p1"\
#define SEM_P3_TO_P2 "/sem_p3_to_p2"\
#define MAX_INT_BYTES sizeof(int)\
\
/**\
 * Genera N n\'fameros pares a partir de un valor inicial y los almacena en memoria compartida.\
 * @param start N\'famero inicial.\
 * @param count Cantidad de n\'fameros a generar.\
 * @param shared_mem Puntero al \'e1rea de memoria compartida.\
 */\
void generar_pares(int start, int count, int *shared_mem) \{\
    for (int i = 0; i < count; ++i) \{\
        shared_mem[i] = start + (i * 2);\
    \}\
\}\
\
/**\
 * Genera N n\'fameros impares a partir de un valor inicial y los almacena en memoria compartida.\
 * @param start N\'famero inicial.\
 * @param count Cantidad de n\'fameros a generar.\
 * @param shared_mem Puntero al \'e1rea de memoria compartida.\
 */\
void generar_impares(int start, int count, int *shared_mem) \{\
    for (int i = 0; i < count; ++i) \{\
        shared_mem[i] = start + (i * 2);\
    \}\
\}\
\
int main(int argc, char *argv[]) \{\
    if (argc != 4) \{\
        fprintf(stderr, "Uso: %s N a1 a2\\\\n", argv[0]);\
        return EXIT_FAILURE;\
    \}\
\
    int N = atoi(argv[1]);\
    int a1 = atoi(argv[2]);\
    int a2 = atoi(argv[3]);\
\
    if (N <= 0 || a1 % 2 != 0 || a2 % 2 == 0) \{\
        fprintf(stderr, "Error: par\'e1metros inv\'e1lidos. N debe ser > 0, a1 par, a2 impar.\\\\n");\
        return EXIT_FAILURE;\
    \}\
\
    // Crear memoria compartida\
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);\
    if (shm_fd == -1) \{\
        perror("Error creando memoria compartida");\
        return EXIT_FAILURE;\
    \}\
\
    if (ftruncate(shm_fd, MAX_INT_BYTES * (N + 2)) == -1) \{\
        perror("Error ajustando tama\'f1o de memoria compartida");\
        return EXIT_FAILURE;\
    \}\
\
    int *shared_mem = mmap(NULL, MAX_INT_BYTES * (N + 2), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);\
    if (shared_mem == MAP_FAILED) \{\
        perror("Error mapeando memoria compartida");\
        return EXIT_FAILURE;\
    \}\
\
    // Abrir sem\'e1foros\
    sem_t *sem_p1_to_p3 = sem_open(SEM_P1_TO_P3, O_CREAT, 0666, 0);\
    sem_t *sem_p2_to_p3 = sem_open(SEM_P2_TO_P3, O_CREAT, 0666, 0);\
    sem_t *sem_p3_to_p1 = sem_open(SEM_P3_TO_P1, O_CREAT, 0666, 0);\
    sem_t *sem_p3_to_p2 = sem_open(SEM_P3_TO_P2, O_CREAT, 0666, 0);\
\
    if (sem_p1_to_p3 == SEM_FAILED || sem_p2_to_p3 == SEM_FAILED ||\
        sem_p3_to_p1 == SEM_FAILED || sem_p3_to_p2 == SEM_FAILED) \{\
        perror("Error abriendo sem\'e1foros");\
        return EXIT_FAILURE;\
    \}\
\
    pid_t pid = fork();\
    if (pid == -1) \{\
        perror("Error creando proceso P2");\
        return EXIT_FAILURE;\
    \}\
\
    if (pid == 0) \{\
        // C\'f3digo del proceso P2\
        generar_impares(a2, N, shared_mem);\
        shared_mem[N] = -2; // Notificaci\'f3n de terminaci\'f3n\
        sem_post(sem_p2_to_p3);\
        sem_wait(sem_p3_to_p2);\
        printf("-3 P2 termina\\\\n");\
        return EXIT_SUCCESS;\
    \} else \{\
        // C\'f3digo del proceso P1\
        generar_pares(a1, N, shared_mem);\
        shared_mem[N] = -1; // Notificaci\'f3n de terminaci\'f3n\
        sem_post(sem_p1_to_p3);\
        sem_wait(sem_p3_to_p1);\
        printf("-3 P1 termina\\\\n");\
        return EXIT_SUCCESS;\
    \}\
\}\
}