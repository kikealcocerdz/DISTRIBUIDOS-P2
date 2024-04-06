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

char *IP_TUPLAS;
int PORT_TUPLAS;

int initialize_env_variables() {
    IP_TUPLAS = getenv("IP_TUPLAS");
    if (IP_TUPLAS == NULL) {
        printf("Variable IP_TUPLAS no definida\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Variable IP_TUPLAS definida con valor %s\n", IP_TUPLAS);
    }

    char *port_env = getenv("PORT_TUPLAS");
    if (port_env == NULL) {
        printf("Variable PORT_TUPLAS no definida\n");
        exit(EXIT_FAILURE);
    } else {
        PORT_TUPLAS = atoi(port_env);
        printf("Variable PORT_TUPLAS definida con valor %d\n", PORT_TUPLAS);
    }
}

static void create_message(int op, int key, char *value1, int N_Value2, double *V_Value2, char *cadena) {
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
    return;
}

int init() {
    initialize_env_variables();
    // Creación de la conexión del socket
    sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

    // Relleno del mensaje
    double aux[MAXSIZE] = { 0.0 };
    create_message(0, -1, NULL, 1, aux, request); 


    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2) {
    initialize_env_variables();
    // Creación de la conexión del socket
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

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
    create_message(1, key, value1, N_value2, V_value2, request);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2) {

    initialize_env_variables();
    // Creación de la conexión del socket
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE] = "";

    // Validación de datos
    if (strlen(value1) > 256) {
        close(sock);
        return -1;
    }
    if (*N_value2 > 32 || *N_value2 < 0) {
        close(sock);
        return -1;
    }

    // Relleno del mensaje, este da igual lo que le pasemos en value1, N_value2 y V_value2,
    // ya que queremos que nos lo devuelva el servidor
    create_message(2, key, value1, *N_value2, V_value2, request);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    //Aqui va la funcion que se encarga de separar el mensaje respuesta y almacenar los valores en las variables correspondientes
    char *token;
    char res;
    printf("Respuesta en get value: %s\n", response);
    token = strtok(response, "/");
    if (token != NULL) {
        res = token[0]; 
    }
    printf("Operación: %c\n", res);

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(value1, token); 
    }
    printf("Value1: %s\n", value1);

    token = strtok(NULL, "/");
    if (token != NULL) {
        *N_value2 = atoi(token); 
    }
    printf("N_value2: %d\n", *N_value2);

    token = strtok(NULL, "/");
    for (int i = 0; i < *N_value2; i++) {
    V_value2[i] = 0.0;
}
    printf("V_value2: %s\n", token);
    char *subtoken = strtok(token, "-");
for (int i = 0; i < *N_value2; i++) {
    printf("El subtoken actual es: %s\n", subtoken);
    if (subtoken != NULL) {
        V_value2[i] = atof(subtoken); 
    }
    printf("V_value2[%d]: %lf\n", i, V_value2[i]);
    subtoken = strtok(NULL, "-"); // Avanza al siguiente subtoken
}
    printf("V_value2: [");
    for(int i = 0; i < *N_value2; i++) {
        printf("%f", V_value2[i]);
        if (i < *N_value2 - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    // Cierre del socket
    close(sock);
    return res;
}




int modify_value(int key, char *value1, int N_value2, double *V_value2) {
    
    initialize_env_variables();
    // Creación del socket
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

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
    create_message(3, key, value1, N_value2, V_value2, request);

    // Enviamos el mensaje
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cerramos el socket
    close(sock);
    return 0;
}

int delete_key(int key) {
    initialize_env_variables();

    // Creación de la conexión del socket
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

    // Relleno del mensaje
    create_message(4, key, NULL, 0, NULL, request);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}

int exist(int key) {
    initialize_env_variables();

    // Creación de la conexión del socket
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

    // Relleno del mensaje
    create_message(5, key, NULL, 0, NULL, request);

    // Envío del mensaje
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    // Cierre del socket
    close(sock);
    return 0;
}
