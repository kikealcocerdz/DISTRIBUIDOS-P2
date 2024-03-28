#include "claves.h"
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include "comm.h"

#define MAXSIZE 256

int sock;

static char *create_message(int op, int key, char *value1, int N_Value2, double *V_Value2) {
    char *cadena = (char *)malloc(MAXSIZE * sizeof(char)); // Reservar memoria para la cadena

    // Verificar si se pudo reservar memoria
    if (cadena == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Inicializar la cadena
    cadena[0] = '\0';

    // Agregar el valor de op a la cadena
    sprintf(cadena, "%d/", op);

    // Agregar Key
    sprintf(cadena + strlen(cadena), "%d/", key);

    // Agregar el valor de value1 a la cadena
    if (value1 == NULL) {
        strcat(cadena, "NULL/");
    } else {
        strcat(cadena, value1);
        strcat(cadena, "/");
    }

    // Agregar el valor de N_Value2 a la cadena
    sprintf(cadena + strlen(cadena), "%d/", N_Value2);

    // Agregar los valores de V_Value2 a la cadena
    if (V_Value2 == NULL) {
        strcat(cadena, "NULL");
    } else {
        for (int i = 0; i < N_Value2; i++) {
            sprintf(cadena + strlen(cadena), "%.1f", V_Value2[i]);
            if (i < N_Value2 - 1) {
                strcat(cadena, "-");
            }
        }
    }

    return cadena;
}

int init() {
    // Creación de la conexión del socket
    sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Relleno del mensaje
    double aux[MAXSIZE] = { 1.1 };
    strcpy(request, create_message(0, -1, NULL, 1, aux)); 

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);


    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, request, longitud) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    if (recvMessage(sock, response, longitud) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}
