#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include "comm.h"

int servicio(int sc) {
    int ret, longitud;
    char op;
    char key[256], value1[256]; 
    char cadena[256]; 
    float V_Value2[256]; 
    int N_Value2;

    printf("Estamos dentro del servicio\n");

    if (recvMessage(sc, (char *)&longitud, sizeof(int)) == -1) {
        perror("error al recvMessage");
        return -1;
    }
    printf("Longitud: %d\n", longitud);

    if (recvMessage(sc, (char *)&cadena, longitud) == -1) {
        perror("error al recvMessage");
        return -1;
    }

    char *token;
    printf("Mensaje recibido: %s\n", cadena);

    token = strtok(cadena, "/");
    if (token != NULL) {
        op = token[0]; 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(key, token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(value1, token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        N_Value2 = atoi(token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(V_Value2, token); 
    }

    printf("Operación: %c\n", op);
    printf("Key: %s\n", key);
    printf("Value1: %s\n", value1);
    printf("N_Value2: %d\n", N_Value2);
    printf("V_Value2: %s\n", V_Value2);
    printf("\n");

    ret = sendMessage(sc, (char *)&longitud, sizeof(int));
    if (ret == -1) {
        printf("Error en envío\n");
        return -1;
    }

    ret = sendMessage(sc, cadena, longitud);
    if (ret == -1) {
        printf("Error en envío\n");
        return -1;
    }

    return 0;
}
int main ( int argc, char *argv[] )
{
        int sd, sc;

        // crear socket
        sd = serverSocket(INADDR_ANY, 4200, SOCK_STREAM) ;
        if (sd < 0) {
            printf ("SERVER: Error en serverSocket\n");
            return 0;
        }

        while (1)
        {
                // aceptar cliente
                sc = serverAccept(sd) ;
                printf("Cliente conectado\n");
                printf("Socket: %d\n", sc);
                if (sc < 0) {
                    printf("Error en serverAccept\n");
                    continue ;
                }

                // procesar petición
                printf("Vamos a procesar la petición\n");
                servicio(sc) ;
                printf("Hemos procesado la petición\n");
                close(sc);
        }

        close(sd);
        return 0;
}