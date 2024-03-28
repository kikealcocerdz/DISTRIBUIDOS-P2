#include "claves.h"
#include "mensaje.h"
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <comm.h>
#define MAXSIZE 256


static void create_message(int op,int key, char *value1, int N_Value2, double *V_Value2) {
char cadena[100]; // Tamaño suficiente para almacenar la cadena final

    // Inicializar la cadena
    cadena[0] = '\0';

    // Agregar el valor de op a la cadena
    sprintf(cadena, "%c/", op);
    //Agregar Key
    sprintf(cadena + strlen(cadena), "%d/", key);

    // Agregar el valor de value1 a la cadena
    sprintf(cadena + strlen(cadena), "%s/", value1);

    // Agregar el valor de N_Value2 a la cadena
    sprintf(cadena + strlen(cadena), "%d/", N_Value2);

    // Agregar los valores de V_Value2 a la cadena
    for (int i = 0; i < 4; i++) {
        sprintf(cadena + strlen(cadena), "%.1f", V_Value2[i]);
        if (i < 3) {
            sprintf(cadena + strlen(cadena), "-");
        }
    }
    return cadena;
}

int init() {

  char colalocal[MAXSIZE]; 
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);
  char request [MAXSIZE];
  char response [MAXSIZE];

  // CREACION DE LA COLA DE MENSAJES, PONGO EL PID POR SI HAY VARIOS CLIENTES A LA VEZ

	//sprintf(colalocal,  "/Cola-%d", getpid());

  sock = clientSocket(SERVER_ADDRESS, PORT);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }
  // RELLENAMOS EL MENSAJE,al no tener ninguno yo diria de que sea 0/-1/NULL/-1/-1 o algo por el estilo 
  request = create_message(0,-1,NULL,-1,-1);
  int longitud = strlen(mensaje);

  // ENVIAMOS EL MENSAJE
  //Primero la longitud del mensaje para que el servidor pueda recibir el mensaje completo 
  if (sendMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }
  if (sendMessage(sock, (char *)&request, longitud) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

  // RECIBIMOS EL MENSAJE
  if (recvMessage(sock, (char *)&longitud, sizeof(int)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
 if (recvMessage(sock, (char *)&response, sizeof(struct respuesta)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
  printf("Resultado recibido del servidor: %d\n", response.resultado);
  // CERRAMOS LAS COLAS
  close(sock);
  return 0;
}

int set_value(int key, char *value1, int N_value2, double *V_value2) {
  struct peticion pet;
  struct respuesta res; /* respuesta de la operación */
  struct mq_attr attr;  // Declare the 'attr' variable here
  char colalocal[MAXSIZE]; 

  // Initialize 'attr' before using it
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);


  sprintf(colalocal,  "/Cola-%d", getpid());
  sock = clientSocket(SERVER_ADDRESS, PORT);
    if (sock < 0) {
        perror("Client socket failed");
        exit(EXIT_FAILURE);
    }
  // Validación de datos
  if (strlen(value1) >256){//256 en vez de 255 pq incluimos en /0
    return -1;
  }
  if (N_value2 > 32 || N_value2 < 0){
    return -1;
  }

  // RELLENAMOS EL MENSAJE cambiar a el uso de la funcion estatica
  strcpy(pet.q_name, colalocal);
  pet.op = 1;
  pet.key = key;
  strcpy(pet.value1, value1); 
  pet.N_value2 = N_value2;
  char aux[MAXSIZE];
  char cadena[MAXSIZE] = " ";
  for (int i = 0; i < N_value2; i++) {
    sprintf(aux, "%lf ", V_value2[i]);
    strcat(cadena, aux);
  }
  strcpy(pet.V_value2, cadena);


  // ENVIAMOS EL MENSAJE
  if (sendMessage(sock, (char *)&request, sizeof(struct peticion)) < 0) {
        perror("Send request failed");
        exit(EXIT_FAILURE);
    }

  // RECIBIMOS EL MENSAJE
  i if (recvMessage(sock, (char *)&response, sizeof(struct respuesta)) < 0) {
        perror("Receive response failed");
        exit(EXIT_FAILURE);
    }
  printf("Resultado recibido del servidor: %d\n", response.resultado);
  // CERRAMOS LAS COLAS
  close(sock);
  return 0;
}

int get_value(int key, char *value1, int *N_value2, double *V_value2) {
  mqd_t q_servidor; /* cola de mensajes del servidor */
  mqd_t q_cliente;  /* cola de mensajes del cliente */

  struct peticion pet;
  struct respuesta res; /* respuesta de la operación */
  struct mq_attr attr;  
  char colalocal[MAXSIZE]; 
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);

	sprintf(colalocal,  "/Cola-%d", getpid());
  q_cliente = mq_open(colalocal, O_CREAT|O_RDONLY, 0700, &attr);
  if (q_cliente == -1) {
    perror("mq_open CLIENTE");
    return -1;
  }

  q_servidor = mq_open("/SERVIDOR_CLAVES", O_CREAT | O_WRONLY, 0700, NULL);
  if (q_servidor == -1) {
    mq_close(q_cliente);
    perror("mq_open SERVIDOR");
    return -1;
  }

  // Validación de datos
  if (strlen(value1) >256){
    return -1;
  }
  if ( *N_value2 > 32 || *N_value2 < 0){
    return -1;
  }

  // RELLENAMOS EL MENSAJE
  strcpy(pet.q_name, colalocal);
  pet.op = 2;
  pet.key = key;
  strcpy(pet.value1, value1);
  pet.N_value2 = *N_value2;
  memcpy(pet.V_value2, V_value2, sizeof(V_value2));

  // ENVIAMOS EL MENSAJE
  if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0) {
    perror("mq_send");
    return -1;
  }

  // RECIBIMOS EL MENSAJE
  if (mq_receive(q_cliente, (char *)&res, sizeof(struct respuesta), 0) < 0) {
    perror("mq_recv");
    return -1;
  }

  // CERRAMOS LAS COLAS
  mq_close(q_servidor);
  mq_close(q_cliente);
  mq_unlink(colalocal);
  return 0;
}

int modify_value(int key, char *value1, int N_value2, double *V_value2) {
  mqd_t q_servidor; /* cola de mensajes del servidor */
  mqd_t q_cliente;  /* cola de mensajes del cliente */

  struct peticion pet;
  struct respuesta res; /* respuesta de la operación */
  struct mq_attr attr;  // Declare the 'attr' variable here
  char colalocal[MAXSIZE]; 

  // Initialize 'attr' before using it
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);

  // CREACION DE LA COLA DE MENSAJES,PONGO EL PID POR SI HAY VARIOS CLIENTES A
  // LA VEZ

	sprintf(colalocal,  "/Cola-%d", getpid());
  q_cliente = mq_open(colalocal, O_CREAT|O_RDONLY, 0700, &attr);
  if (q_cliente == -1) {
    perror("mq_open CLIENTE");
    return -1;
  }

  q_servidor = mq_open("/SERVIDOR_CLAVES", O_CREAT | O_WRONLY, 0700, NULL);
  if (q_servidor == -1) {
    mq_close(q_cliente);
    perror("mq_open SERVIDOR");
    return -1;
  }
  
  if (strlen(value1) >256){
    return -1;
  }
  if( N_value2 > 32 || N_value2 < 0){
    return -1;
  }

  // RELLENAMOS EL MENSAJE
  strcpy(pet.q_name, colalocal);
  pet.op = 3;
  pet.key = key;
  strcpy(pet.value1, value1); 
  pet.N_value2 = N_value2;
  char aux[MAXSIZE];
  char cadena[MAXSIZE] = " ";
  for (int i = 0; i < N_value2; i++) {
    sprintf(aux, "%lf ", V_value2[i]);
    strcat(cadena, aux);
  }

  strcpy(pet.V_value2, cadena);

  // ENVIAMOS EL MENSAJE
  if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0) {
    perror("mq_send");
    return -1;
  }
  // RECIBIMOS EL MENSAJE
  if (mq_receive(q_cliente, (char *)&res, sizeof(int), 0) < 0) {
    perror("mq_recv");
    return -1;
  }
  // CERRAMOS LAS COLAS
  mq_close(q_servidor);
  mq_close(q_cliente);
  mq_unlink(colalocal);
  return 0;
}

int delete_key(int key) {
  mqd_t q_servidor; /* cola de mensajes del servidor */
  mqd_t q_cliente;  /* cola de mensajes del cliente */

  struct peticion pet;
  struct respuesta res; /* respuesta de la operación */
  struct mq_attr attr;  
  char colalocal[MAXSIZE]; 
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);

	sprintf(colalocal,  "/Cola-%d", getpid());
  q_cliente = mq_open(colalocal, O_CREAT|O_RDONLY, 0700, &attr);
  if (q_cliente == -1) {
    perror("mq_open CLIENTE");
    return -1;
  }

  q_servidor = mq_open("/SERVIDOR_CLAVES", O_CREAT | O_WRONLY, 0700, NULL);
  if (q_servidor == -1) {
    mq_close(q_cliente);
    perror("mq_open SERVIDOR");
    return -1;
  }
  
  // RELLENAMOS EL MENSAJE
  strcpy(pet.q_name, colalocal);
  pet.op = 4;
  pet.key = key;

  // ENVIAMOS EL MENSAJE
  if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0) {
    perror("mq_send");
    return -1;
  }
  // RECIBIMOS EL MENSAJE
  if (mq_receive(q_cliente, (char *)&res, sizeof(int), 0) < 0) {
    perror("mq_recv");
    return -1;
  }
  // CERRAMOS LAS COLAS
  mq_close(q_servidor);
  mq_close(q_cliente);
  mq_unlink(colalocal);
  return 0;
}

int exist(int key) {
  mqd_t q_servidor; /* cola de mensajes del servidor */
  mqd_t q_cliente;  /* cola de mensajes del cliente */

  struct peticion pet;
  struct respuesta res; /* respuesta de la operación */
  struct mq_attr attr;
  char colalocal[MAXSIZE]; 
  attr.mq_maxmsg = 10;     
	attr.mq_msgsize = sizeof(int);

	sprintf(colalocal,  "/Cola-%d", getpid());
  q_cliente = mq_open(colalocal, O_CREAT|O_RDONLY, 0700, &attr);
  if (q_cliente == -1) {
    perror("mq_open CLIENTE");
    return -1;
  }

  q_servidor = mq_open("/SERVIDOR_CLAVES", O_CREAT | O_WRONLY, 0700, NULL);
  if (q_servidor == -1) {
    mq_close(q_cliente);
    perror("mq_open SERVIDOR");
    return -1;
  }
  // RELLENAMOS EL MENSAJE
  strcpy(pet.q_name, colalocal);
  pet.op = 5;
  pet.key = key;

  // ENVIAMOS EL MENSAJE
  if (mq_send(q_servidor, (const char *)&pet, sizeof(pet), 0) < 0) {
    perror("mq_send");
    return -1;
  }
  // RECIBIMOS EL MENSAJE
  if (mq_receive(q_cliente, (char *)&res, sizeof(int), 0) < 0) {
    perror("mq_recv");
    return -1;
  }
  // CERRAMOS LAS COLAS
  mq_close(q_servidor);
  mq_close(q_cliente);
  mq_unlink(colalocal);
  return 0;
}