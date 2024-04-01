#include "claves.h"
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h> 
#include <string.h>



int main() {
  int num_clients = 3;
  init();
  sleep(1);
  int N_value2 = 3;
  double V_value2[] = {1.0, 2.0, 3.0}; 
  char aux[20];
  
  for (int i = 0; i < num_clients; i++) {
    pid_t pid = fork();

    if (pid == 0) {
      // Código del cliente
      char value1[20]; 
      sprintf(value1, "%d", getpid()); 
      int N_value2 = 3;
      double V_value2[] = {1.0, 2.0, 3.0};
      double V_value3[] = {1.0, 5.0, 159.0};
      init();
      printf("Cliente %d\n", getpid());
      // Ejecución diferente para cada cliente
      if (i == 0) {
        set_value(1, "buenos días", N_value2, V_value2);
        sleep(3);
        get_value(1, value1, &N_value2, V_value2);
        printf("value1: %s, N_value2: %d, ", value1, N_value2);
        printf("V_value2: [");
        for(int i = 0; i < N_value2; i++) {
            printf("%f", V_value2[i]);
            if (i < N_value2 - 1) {
                printf(", ");
            }
        }
        printf("]\n");
        sleep(1); 
        return 0;
      }
      if (i == 1) {
        set_value(15, "buenos días", N_value2, V_value3);
        sleep(3);
        modify_value(15, "ModificadoPorCliente", N_value2, V_value3);
        return 0; 
      }
      if ( i == 2) {
        // Ejecución para el tercer cliente
        set_value(4, value1, N_value2, V_value2);
        exist(4);
        return 0; 
      }
    }
  }
  // Esperar a que todos los clientes terminen
  for (int i = 0; i < num_clients; i++) {
    wait(NULL);
  }
  return 0;
}