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


void tratar_mensaje(void *mess) {
  struct peticion mensaje; /* mensaje local */
  mqd_t q_cliente;         /* cola del cliente */
  struct respuesta res; 

  /* el thread copia el mensaje a un mensaje local */
  pthread_mutex_lock(&mutex_mensaje);

  mensaje = (*(struct peticion *)mess);

  /* ya se puede despertar al servidor*/
  mensaje_no_copiado = false;

  pthread_cond_signal(&cond_mensaje);

  pthread_mutex_unlock(&mutex_mensaje);

  /* ejecutar la petición del cliente y preparar respuesta */
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
  q_cliente = mq_open(mensaje.q_name, O_WRONLY);
  if (q_cliente == -1) {
    perror("No se puede abrir la cola del cliente");
    mq_close(q_servidor);
    mq_unlink("/SERVIDOR_CLAVES");
  } else {
    if (mq_send(q_cliente, (const char *)&res, sizeof(int), 0) < 0) {
      perror("mq_send");
      mq_close(q_servidor);
      mq_unlink("/SERVIDOR_CLAVES");
      mq_close(q_cliente);
    }
  }
  
  pthread_exit(0);
}

int main(void) {
  struct peticion mess;
  struct mq_attr attr;
  pthread_attr_t t_attr; // atributos de los threads
  pthread_t thid;

  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(struct peticion);

  q_servidor = mq_open("/SERVIDOR_CLAVES", O_CREAT | O_RDONLY, 0700, &attr);
  if (q_servidor == -1) {
    perror("mq_open");
    return -1;
  }

  pthread_mutex_init(&mutex_mensaje, NULL);
  pthread_cond_init(&cond_mensaje, NULL);
  pthread_attr_init(&t_attr);

  // atributos de los threads, threads independientes
  pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

  while (1) {
    if (mq_receive(q_servidor, (char *)&mess, sizeof(mess), 0) < 0) {
      perror("mq_recev");
      return -1;
    }

    if (pthread_create(&thid, &t_attr, (void *)tratar_mensaje, (void *)&mess) ==
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