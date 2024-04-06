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
    // Inicialización de las variables de entorno en IP_TUPLAS y PORT_TUPLAS
    IP_TUPLAS = getenv("IP_TUPLAS");
    if (IP_TUPLAS == NULL) {
        fprintf(stderr, "Variable IP_TUPLAS no definida\n");
        exit(EXIT_FAILURE);
    }

    char *port_env = getenv("PORT_TUPLAS");
    if (port_env == NULL) {
        fprintf(stderr, "Variable PORT_TUPLAS no definida\n");
        exit(EXIT_FAILURE);
    } else {
        PORT_TUPLAS = atoi(port_env);
    }
}

static void create_message(int op, int key, char *value1, int N_Value2, double *V_Value2, char *cadena) {
    
    if (cadena == NULL) {
        exit(EXIT_FAILURE);
    }

    // Inicializar la cadena
    cadena[0] = '\0';

    sprintf(cadena, "%d/", op);

    sprintf(cadena + strlen(cadena), "%d/", key);

    if (value1 == NULL) {
        strcat(cadena, "NULL/");
    } else {
        strcat(cadena, value1);
        strcat(cadena, "/");
    }

    // Añadimos el número de elementos de V_Value2
    sprintf(cadena + strlen(cadena), "%d/", N_Value2);

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

    // Relleno por defecto del mensaje al ser init
    double aux[MAXSIZE] = { 0.0 };
    create_message(0, -1, NULL, 1, aux, request); 


    // Envío de la cadena de caracteres
    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    // Recepción de la respuesta del servidor
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

    // Validación de datos, si es mayor de 256 o contiene un espacio
    if (strlen(value1) > 256 || strchr(value1, ' ') != NULL) {
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
    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE] = "";

    if (strlen(value1) > 256 || strchr(value1, ' ') != NULL) {
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
    token = strtok(response, "/");
    if (token != NULL) {
        res = token[0]; 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        strcpy(value1, token); 
    }

    token = strtok(NULL, "/");
    if (token != NULL) {
        *N_value2 = atoi(token); 
    }

    token = strtok(NULL, "/");

    // Inicializamos el vector a 0 para su depuración
    for (int i = 0; i < *N_value2; i++) {
        V_value2[i] = 0.0;
    }

    char *subtoken = strtok(token, "-");
    for (int i = 0; i < *N_value2; i++) {
        if (subtoken != NULL) {
            V_value2[i] = atof(subtoken); 
        }
        subtoken = strtok(NULL, "-"); 
        // Avanza al siguiente subtoken
    }

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

    if (strlen(value1) > 256 || strchr(value1, ' ') != NULL) {
        perror("El valor de value1 no es válido\n");
        close(sock);
        return -1;
    }

    if (N_value2 > 32 || N_value2 < 0) {
        close(sock);
        return -1;
    }

    double aux[MAXSIZE];
    char cadena[MAXSIZE] = " ";
    for (int i = 0; i < N_value2; i++) {
        sprintf(request, "%lf ", V_value2[i]);
        strcat(cadena, request);
    }
    create_message(3, key, value1, N_value2, V_value2, request);

    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    close(sock);
    return 0;
}

int delete_key(int key) {
    initialize_env_variables();

    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";
    
    create_message(4, key, NULL, 0, NULL, request);

    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    close(sock);
    return 0;
}

int exist(int key) {
    initialize_env_variables();

    int sock = clientSocket(IP_TUPLAS, PORT_TUPLAS);
    if (sock < 0) {
        perror("Client socket failed");
        return -1;
    }

    char request[MAXSIZE];
    char response[MAXSIZE]="";

    create_message(5, key, NULL, 0, NULL, request);

    if (sendMessage(sock, (char *)&request, strlen(request) + 1) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

    
    if (readLine(sock, (char *)&response, MAXSIZE) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }

    printf("Resultado recibido del servidor: %s\n", response);

    close(sock);
    return 0;
}
