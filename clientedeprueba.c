#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "comm.h"

#define SERVER_ADDRESS "127.0.0.1" // Dirección del servidor (localhost)
#define SERVER_PORT 4200            // Puerto del servidor

int suma_remota(int sd, int *r, int x, int y) {
    int ret;
    char op;
    int32_t a, b, res;

    op = 0; // operación sumar
    ret = sendMessage(sd, (char *)&op, sizeof(char)); // envía operación
    if (ret == -1) {
        printf("Error envío op\n");
        return -1;
    }

    a = htonl(x);                                     // marshalling: a <- host to network long(x)
    ret = sendMessage(sd, (char *)&a, sizeof(int32_t)); // envía a
    if (ret == -1) {
        printf("Error envío a\n");
        return -1;
    }

    b = htonl(y);                                     // marshalling: b <- host to network long(y)
    ret = sendMessage(sd, (char *)&b, sizeof(int32_t)); // envía b
    if (ret == -1) {
        printf("Error envío b\n");
        return -1;
    }
    ret = recvMessage(sd, (char *)&res, sizeof(int32_t)); // recibe respuesta
    if (ret == -1) {
        printf("Error en recepción\n");
        return -1;
    }
    *r = ntohl(res); // unmarshalling: *r <- network to host long(res)

    return 0;
}

int main(int argc, char **argv) {
    int sd, ret, res;

    // Crear socket cliente y conectarse al servidor
    sd = clientSocket(SERVER_ADDRESS, SERVER_PORT);
    if (sd < 0) {
        printf("Error en clientSocket con %s:%d\n", SERVER_ADDRESS, SERVER_PORT);
        return -1;
    }
    char* mensaje = "Hola desde el cliente";
    int longitud = strlen(mensaje);
    ret = sendMessage(sd, (char *)&longitud, sizeof(int)); // envía longitud
    if (ret == -1) {
        printf("Error envío longitud\n");
        return -1;
    }
    printf("Mensaje enviado: %s\n con longitud %d", mensaje,longitud);
    ret = sendMessage(sd, mensaje, strlen(mensaje));
    if (ret == -1) {
        printf("Error envío mensaje\n");
        return -1;
    }
    // Realizar la suma remota
    ret = suma_remota(sd, &res, 5, 2);
    if (ret < 0) {
        closeSocket(sd);
        return -1;
    }

    // Imprimir el resultado
    printf("Resultado de a+b es: %d\n", res);

    // Cerrar el socket
    closeSocket(sd);
    return 0;
}
