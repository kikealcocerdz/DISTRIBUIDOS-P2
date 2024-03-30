#include "clavesservidor.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "comm.h"

pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;

void tratar_mensaje(void *arg) {
    int sc = *(int *)arg;
    int ret, longitud;
    char op;
    char value1[256], cadena[256], res[256];
    float V_Value2[256];
    int N_Value2, key;

    pthread_mutex_lock(&mutex_mensaje);

    if (recvMessage(sc, (char *)&longitud, sizeof(int)) == -1) {
        perror("error al recvMessage 1 ");
        pthread_mutex_unlock(&mutex_mensaje);
        return;
    }
    printf("Longitud: %d\n", longitud);

    if (recvMessage(sc, (char *)&cadena, longitud) == -1) {
        perror("error al recvMessage 2");
        pthread_mutex_unlock(&mutex_mensaje);
        return;
    }

    printf("Mensaje recibido: %s\n", cadena);

    mensaje_no_copiado = false;
    pthread_cond_signal(&cond_mensaje);
    pthread_mutex_unlock(&mutex_mensaje);

    char *token;
    printf("Mensaje recibido: %s\n", cadena);

    token = strtok(cadena, "/");
    if (token != NULL) {
        op = token[0]; 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        key = atoi(token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(value1, token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        N_Value2 = atoi(token); 
    }

    for (int i = 0; i < N_Value2; i++) {
        token = strtok(NULL, "/");
        if (token != NULL) {
            V_Value2[i] = atof(token); 
        }
    }

    switch (op) {
        case '0':
            init_serv(res);
            break;
        case '1':
            set_value_serv(key, *value1, N_Value2, V_Value2, res);
            break;
        case '2':
            get_value_serv(atoi(key), *value1, &N_Value2, V_Value2, res);
            break;
        case '3':
            modify_value_serv(atoi(key), value1, N_Value2, V_Value2, res);
            break;
        case '4':
            delete_value_serv(atoi(key), res);
            break;
        case '5':
            exists_serv(atoi(key), res);
            break;
        default:
            strcpy(res, "Operación no válida");
            break;
    }

    printf("Resultado: %s de la función %c\n", res, op);

    longitud = strlen(res);

    ret = sendMessage(sc, (char *)&longitud, sizeof(int));
    if (ret == -1) {
        printf("Error en envío\n");
        pthread_mutex_unlock(&mutex_mensaje);
        return;
    }

    ret = sendMessage(sc, res, longitud);
    if (ret == -1) {
        printf("Error en envío\n");
        pthread_mutex_unlock(&mutex_mensaje);
        return;
    }

    pthread_mutex_unlock(&mutex_mensaje);
    close(sc); // Cerrar el socket después de enviar la respuesta

    pthread_exit(NULL);
}

int main() {
    int sd, sc;
    pthread_attr_t t_attr;
    pthread_t thid;

    sd = serverSocket(INADDR_ANY, 4200, SOCK_STREAM);
    if (sd < 0) {
        printf("SERVER: Error en serverSocket\n");
        return 0;
    }

    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&t_attr);
    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    while (1) {
        sc = serverAccept(sd);
        if (sc < 0) {
            printf("Error en serverAccept\n");
            continue;
        }

        if (pthread_create(&thid, &t_attr, (void *)tratar_mensaje, (void *)&sc) == 0) {
            pthread_mutex_lock(&mutex_mensaje);
            while (mensaje_no_copiado)
                pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
            mensaje_no_copiado = true;
            pthread_mutex_unlock(&mutex_mensaje);
        }
    }

    return 0;
}
