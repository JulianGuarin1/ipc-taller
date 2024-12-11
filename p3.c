{\rtf1\ansi\ansicpg1252\cocoartf2709
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 /* Archivo: p3.c */\
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
 * Lee n\'fameros de memoria compartida y los imprime por salida est\'e1ndar.\
 * Asegura que no se impriman dos pares o dos impares consecutivos.\
 * @param shared_mem Puntero al \'e1rea de memoria compartida.\
 * @param n Cantidad de n\'fameros a leer.\
 */\
void procesar_numeros(int *shared_mem, int n) \{\
    int ultimo_paridad = -1; // -1 indica que a\'fan no hay n\'famero previo\
    for (int i = 0; i < n; ++i) \{\
        int numero = shared_mem[i];\
        int paridad = numero % 2;\
\
        // Evitar dos n\'fameros con la misma paridad consecutivos\
        if (paridad != ultimo_paridad) \{\
            printf("%d ", numero);\
            fflush(stdout);\
            ultimo_paridad = paridad;\
        \}\
    \}\
    printf("\\\\n");\
    fflush(stdout);\
\}\
\
int main() \{\
    // Abrir memoria compartida\
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);\
    if (shm_fd == -1) \{\
        perror("Error abriendo memoria compartida");\
        return EXIT_FAILURE;\
    \}\
\
    int *shared_mem = mmap(NULL, MAX_INT_BYTES * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);\
    if (shared_mem == MAP_FAILED) \{\
        perror("Error mapeando memoria compartida");\
        return EXIT_FAILURE;\
    \}\
\
    // Abrir sem\'e1foros\
    sem_t *sem_p1_to_p3 = sem_open(SEM_P1_TO_P3, 0);\
    sem_t *sem_p2_to_p3 = sem_open(SEM_P2_TO_P3, 0);\
    sem_t *sem_p3_to_p1 = sem_open(SEM_P3_TO_P1, 0);\
    sem_t *sem_p3_to_p2 = sem_open(SEM_P3_TO_P2, 0);\
\
    if (sem_p1_to_p3 == SEM_FAILED || sem_p2_to_p3 == SEM_FAILED ||\
        sem_p3_to_p1 == SEM_FAILED || sem_p3_to_p2 == SEM_FAILED) \{\
        perror("Error abriendo sem\'e1foros");\
        return EXIT_FAILURE;\
    \}\
\
    printf("Esperando por P1 y P2\\\\n");\
    fflush(stdout);\
\
    // Esperar datos de P1 y P2\
    sem_wait(sem_p1_to_p3);\
    procesar_numeros(shared_mem, 1024); // Procesar datos de P1\
    shared_mem[1024] = -3;\
    sem_post(sem_p3_to_p1);\
\
    sem_wait(sem_p2_to_p3);\
    procesar_numeros(shared_mem, 1024); // Procesar datos de P2\
    shared_mem[1024] = -3;\
    sem_post(sem_p3_to_p2);\
\
    // Finalizar ejecuci\'f3n\
    printf("-3 P3 termina\\\\n");\
    fflush(stdout);\
\
    // Cerrar sem\'e1foros y memoria compartida\
    sem_close(sem_p1_to_p3);\
    sem_close(sem_p2_to_p3);\
    sem_close(sem_p3_to_p1);\
    sem_close(sem_p3_to_p2);\
    munmap(shared_mem, MAX_INT_BYTES * 1024);\
    shm_unlink(SHM_NAME);\
\
    return EXIT_SUCCESS;\
\}\
}