#include "clavesservidor.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include "comm.h"

/* mutex y variables condicionales para proteger la copia del mensaje*/
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
int ret, longitud;
char op;
char key[256], value1[256]; 
char cadena[256]; 
float V_Value2[256]; 
int N_Value2;
char res[256];

void tratar_mensaje(void *sc) {

  /* el thread copia el mensaje a un mensaje local  */
  pthread_mutex_lock(&mutex_mensaje);
  printf("sc tiene valor: %d\n", *(int *)sc);

  if (recvMessage(sc, (char *)&longitud, sizeof(int)) == -1) {
      perror("error al recvMessage 1");
      return -1;
  }
  printf("Longitud: %d\n", longitud);

  if (recvMessage(sc, (char *)&cadena, longitud) == -1) {
      perror("error al recvMessage 2");
      return -1;
  }

  printf("Mensaje recibido: %s\n", cadena);

  /* ya se puede despertar al servidor*/
  mensaje_no_copiado = false;

  pthread_cond_signal(&cond_mensaje);

  pthread_mutex_unlock(&mutex_mensaje);

  // Tokenizar la cadena
  
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


  /* ejecutar la petición del cliente y preparar respuesta */
  /*Aqui habra que cambiar dependiendo de como pasemos el mensaje*/
  switch (op) {
  case 0:
    init_serv(&res);

    break;
  case 1:
    set_value_serv(key, value1, N_Value2, V_Value2, &res);
    break;

  case 2:
    get_value_serv(key, value1, N_Value2, V_Value2, &res);
    break;
  case 3:
    modify_value_serv(key, value1, N_Value2, V_Value2, &res);
    break;
  case 4:
    delete_value_serv(key, &res);
    break;
  default:
    exists_serv(key, &res);
    break;
  }
  printf("Resultado: %d de la función %d \n", &res, op);
  longitud = strlen(res);
  printf("Resultado: %s de la función %d \n", res, op);
  /* Se devuelve el resultado al cliente */
  /* Para ello se envía el resultado a su cola */
  ret = sendMessage(sc, (char *)&longitud, sizeof(int));
        if (ret == -1) {
            printf("Error en envío\n");
            return -1 ;
        }
        
  ret = sendMessage(sc, (char *)&res, longitud);
        if (ret == -1) {
            printf("Error en envío\n");
            return;
        }
  
  pthread_exit(0);
}

int main() {
  int sd, sc;
  pthread_attr_t t_attr; // atributos de los threads
  pthread_t thid;

  sd = serverSocket(INADDR_ANY, 4200, SOCK_STREAM) ;
        if (sd < 0) {
            printf ("SERVER: Error en serverSocket\n");
            return 0;
        }

  pthread_mutex_init(&mutex_mensaje, NULL);
  pthread_cond_init(&cond_mensaje, NULL);
  pthread_attr_init(&t_attr);

  // atributos de los threads, threads independientes
  pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

  while (1) {
    sc = serverAccept(sd) ;
    printf("sc tiene valor: %d\n", sc);
                if (sc < 0) {
                    printf("Error en serverAccept\n");
                    continue ;
                }

    if (pthread_create(&thid, &t_attr, (void *)tratar_mensaje, (void *)&sc) ==//No se si se debe pasar asi el descriptor sc
        0) {
      // se espera a que el thread copie el mensaje
      pthread_mutex_lock(&mutex_mensaje);
      while (mensaje_no_copiado)
        pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
      mensaje_no_copiado = true;
      pthread_mutex_unlock(&mutex_mensaje);
    }
  }
  return 0;
}