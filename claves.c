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
    double aux[MAXSIZE] = { 1.1, 2.2 };
    strcpy(request, create_message(0, -1, NULL, 3, aux)); 

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);


    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
    printf("Longitud del servidor: %d\n", longitud);
    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2) {
    // Creación de la conexión del socket
    int sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Validación de datos
    if (strlen(value1) > 256) { // 256 en vez de 255 pq incluimos en /0
        close(sock);
        return -1;
    }
    if (N_value2 > 32 || N_value2 < 0) {
        close(sock);
        return -1;
    }

    // Relleno del mensaje
    double aux[MAXSIZE];
    char cadena[MAXSIZE] = " ";
    for (int i = 0; i < N_value2; i++) {
        sprintf(request, "%lf ", V_value2[i]);
        strcat(cadena, request);
    }
    strcpy(request, create_message(1, key, value1, N_value2, V_value2));

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
    printf("Longitud del servidor: %d\n", longitud);

    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2) {
    // Creación de la conexión del socket
    int sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Validación de datos
    if (strlen(value1) > 256) {
        close(sock);
        return -1;
    }
    if (*N_value2 > 32 || *N_value2 < 0) {
        close(sock);
        return -1;
    }

    // Relleno del mensaje,este da igual lo que le pasemos en value1,N_value2 y V_value2,ya que queremos que nos lo devuelva el servidor
    strcpy(request, create_message(2, key, value1, *N_value2, V_value2));

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }
    printf("Longitud del servidor: %d\n", longitud);

    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }

    printf("Resultado recibido del servidor: %s\n", response);
    //Aqui va la funcion que se encarga de separar el mensaje respuesta y almacenar los valores en las variables correspondientes
    //TODO

    // Cierre del socket
    close(sock);
    //Hay que devolver los valores que se necesitan(Creo que lo hicimos mal en el anterior...)

    return 0;
}



int modify_value(int key, char *value1, int N_value2, double *V_value2) {
    // Creación del socket
    int sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Validación de datos
    if (strlen(value1) > 256) {
        close(sock);
        return -1;
    }
    if (N_value2 > 32 || N_value2 < 0) {
        close(sock);
        return -1;
    }

    // Rellenamos el mensaje
    double aux[MAXSIZE];
    char cadena[MAXSIZE] = " ";
    for (int i = 0; i < N_value2; i++) {
        sprintf(request, "%lf ", V_value2[i]);
        strcat(cadena, request);
    }
    strcpy(request, create_message(3, key, value1, N_value2, V_value2));

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);

    // Enviamos el mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado mensaje\n");

    // Recibimos el mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }
    printf("Longitud del servidor: %d\n", longitud);

    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cerramos el socket
    close(sock);
    return 0;
}

int delete_key(int key) {
    // Creación de la conexión del socket
    int sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Relleno del mensaje
    strcpy(request, create_message(4, key, NULL, 0, NULL));

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }
    printf("Longitud del servidor: %d\n", longitud);

    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int exist(int key) {
    // Creación de la conexión del socket
    int sock = clientSocket("127.0.0.1", 4200);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE];

    // Relleno del mensaje
    strcpy(request, create_message(5, key, NULL, 0, NULL));

    printf("Mensaje a enviar: %s\n", request);
    int longitud = strlen(request);
    printf("Longitud: %d\n", longitud);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado longitud\n");

    if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        close(sock);
        return -1;
    }

    printf("Hemos enviado mensaje\n");

    // Recepción del mensaje
    if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }
    printf("Longitud del servidor: %d\n", longitud);

    if (recvMessage(sock, (char *)&response, longitud) < 0) {
        perror("Receive response failed");
        close(sock);
        return -1;
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}
