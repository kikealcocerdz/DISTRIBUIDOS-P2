#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include "comm.h"

int servicio ( int sc )
{
        int ret ;
        char op;
        int32_t a, b, res;
        char mensaje[100];
        int longitud;
        ret = recvMessage(sc, (char *) &longitud, sizeof(int)); // recibe longitud
        if (ret == -1) {
            printf("Error en recepción longitud\n");
            return -1 ;
        }

        ret = recvMessage(sc, (char *) &mensaje, longitud); // recibe mensaje
        if (ret == -1) {
            printf("Error en recepción mensaje\n");
            return -1 ;
        }
        printf("Mensaje recibido: %s\n", mensaje);

        ret = recvMessage(sc, (char *) &op, sizeof(char)); // operación
        if (ret < 0) {
            printf("Error en recepción op\n");
            return -1 ;
        }

        ret = recvMessage(sc, (char *) &a, sizeof(int32_t)); // recibe a
        if (ret == -1) {
            printf("Error en recepción a\n");
            return -1 ;
        }

        ret = recvMessage(sc, (char *) &b, sizeof(int32_t)); // recibe b
        if (ret == -1) {
            printf("Error en recepción b\n");
            return -1 ;
        }
        printf("Operación: %d\n", op);
        a = ntohl(a);
        b = ntohl(b);
        if (op == 0)
             res = a + b;
        else res = a - b;
        res = htonl(res);

        ret = sendMessage(sc, (char *)&res, sizeof(int32_t));
        if (ret == -1) {
            printf("Error en envío\n");
            return -1 ;
        }

        return 0 ;
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
                if (sc < 0) {
                    printf("Error en serverAccept\n");
                    continue ;
                }

                // procesar petición
                servicio(sc) ;
                close(sc);
        }

        close(sd);
        return 0;
}