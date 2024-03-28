#include "clavesservidor.h"
#include "mensaje.h"
#include <mqueue.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


/* mutex y variables condicionales para proteger la copia del mensaje*/
pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t q_servidor;


void tratar_mensaje(void *sc) {
  char cadena[256]; /* mensaje recibido del cliente  */ 
  int longitud;

  /* el thread copia el mensaje a un mensaje local  */
  pthread_mutex_lock(&mutex_mensaje);
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
      N_Value2 = atoi(token); // Convertir el tercer token a entero como N_Value2
  }

  token = strtok(NULL, "/");
  if (token != NULL) {
      // Convertir el cuarto token en un array de números flotantes
      char *subtoken;
      int i = 0;
      subtoken = strtok(token, "-");
      while (subtoken != NULL && i < 4) {
          V_Value2[i++] = atof(subtoken); // Convertir cada subtoken a flotante
          subtoken = strtok(NULL, "-");
      }
  }


  /* ejecutar la petición del cliente y preparar respuesta */
  /*Aqui habra que cambiar dependiendo de como pasemos el mensaje*/
  switch (mensaje.op) {
  case 0:
    init_serv(&res);

    break;
  case 1:
    set_value_serv(mensaje.key, mensaje.value1, mensaje.N_value2,
                   mensaje.V_value2, &res);
    break;

  case 2:
    get_value_serv(mensaje.key, mensaje.value1, &mensaje.N_value2, mensaje.V_value2, &res);
    break;
  case 3:
    modify_value_serv(mensaje.key, mensaje.value1, mensaje.N_value2,
                   mensaje.V_value2, &res);
    break;
  case 4:
    delete_value_serv(mensaje.key, &res);
    break;
  default:
    exists_serv(mensaje.key, &res);
    break;
  }
  printf("Resultado: %d de la función %d \n", res.resultado, mensaje.op);
  /* Se devuelve el resultado al cliente */
  /* Para ello se envía el resultado a su cola */
  ret = sendMessage(sc, (char *)&res, sizeof(struct respuesta));
        if (ret == -1) {
            printf("Error en envío\n");
            return -1 ;
        }
  
  pthread_exit(0);
}

int main(void) {
  struct peticion mess;
  int sd, sc;
  pthread_attr_t t_attr; // atributos de los threads
  pthread_t thid;

  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(struct peticion);

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