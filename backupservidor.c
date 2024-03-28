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
char op, key, value1, N_Value2, V_Value2;
char res[256];
int ret, sc;

void tratar_mensaje(void *sc) {
  char cadena[256]; /* mensaje recibido del cliente  */ 
  int longitud;

  /* el thread copia el mensaje a un mensaje local  */
  pthread_mutex_lock(&mutex_mensaje);
  printf("sc tiene valor: %d\n", *(int *)sc);
  if (recvMessage(sc, (char *)&longitud, sizeof(int)) == -1) {
      perror("error al recvMessage");
      return -1;
    }

  //mensaje = (*(struct peticion *)mess);
  if (recvMessage(sc, (char *)&cadena, longitud) == -1) {
      perror("error al recvMessage");
      return -1;
    }
  printf("Mensaje recibido: %s\n", cadena);

  /* ya se puede despertar al servidor*/
  mensaje_no_copiado = false;

  pthread_cond_signal(&cond_mensaje);

  pthread_mutex_unlock(&mutex_mensaje);

  // Tokenizar la cadena
  char *token;
  token = strtok(cadena, "/");
  if (token != NULL) {
      op = token[0]; // La primera letra como operación
  }
  token = strtok(NULL, "/");
  if (token != NULL) {
      strcpy(key, token); // Copiar el segundo token como key
  }
  token = strtok(NULL, "/");
  if (token != NULL) {
      strcpy(value1, token); // Copiar el segundo token como value1
  }

  token = strtok(NULL, "/");
  if (token != NULL) {
      N_Value2 = token; // Convertir el tercer token a entero como N_Value2
  }

  token = strtok(NULL, "/");
  if (token != NULL) {
      // Convertir el cuarto token en un array de números flotantes
      char *subtoken;
      int i = 0;
      subtoken = strtok(token, "-");
      while (subtoken != NULL && i < N_Value2) {
          float V_Value2[N_Value2];
          while (subtoken != NULL && i < N_Value2) {
            V_Value2[i++] = atof(subtoken); // Convert each subtoken to float
            subtoken = strtok(NULL, "-");
          }
          subtoken = strtok(NULL, "-");
      }
  }


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

int main(void) {
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