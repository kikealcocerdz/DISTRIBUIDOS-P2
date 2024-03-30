#include <stdio.h>
#include "clavesservidor.h"
#include <mqueue.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <dirent.h>

int claves;
int key_leida;
char valor1[100];
int N_value2;
double V_value2[100];

void init_serv(char *res) {
    // Eliminar el directorio /claves si existe
    if (access("./claves", F_OK) == 0) {
        DIR *dir = opendir("./claves");
        if (dir) {
            // Si el directorio está lleno, elimina su contenido
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char path[1024];
                    snprintf(path, sizeof(path), "%s/%s", "./claves", entry->d_name);
                    remove(path);
                }
            }
            closedir(dir);
        }
        // Eliminar el directorio /claves
        if (remove("./claves") == -1) {
            printf("Error al eliminar ./claves\n");
            return; 
        }
    }
    // Crear el directorio /claves
    if (mkdir("./claves", 0777) == -1) {
        printf("Error al eliminar ./claves\n");
        return;  
    }

    printf("Init fine");
    return; 
}

void set_value_serv(int key, char *value1, int N_value2, char *V_value2, char *res) {
    char filename[20]; 
    sprintf(filename, "./claves/%d.txt", key);  // Convertimos la clave en string y añadimos el directorio
    FILE *checkExistFile = fopen(filename, "r");
    if (checkExistFile != NULL) {
        fclose(checkExistFile);
        printf("Archivo existía \n");
        return;
    }
    
    FILE *clavesFile = fopen(filename, "w+");
    if (clavesFile == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    // Insertar la nueva clave-valor
    fprintf(clavesFile, "%d %s %d %s", key, value1, N_value2, V_value2); 
    fprintf(clavesFile, "\n"); 
    fclose(clavesFile);
    printf("%d %s %d %s", key, value1, N_value2, V_value2); 
    return;
}

void get_value_serv(int key, char *value1, int *N_value2, char *V_value2, char *res) {
    char filename[20]; 
    sprintf(filename, "./claves/%d.txt", key);
    FILE *clavesFile = fopen(filename, "r");
    if (clavesFile == NULL) {
        printf("Error\n");
        return;
    }

    // Leer el contenido del archivo y almacenarlo en las variables
    if (fscanf(clavesFile, "%d %s %d %s", &key, value1, N_value2, V_value2) != 4) {
        fclose(clavesFile);
        printf("Error\n");
        return;
    }

    fclose(clavesFile);
    printf("All good\n");
    /*
    strcpy(res->value1, value1);
    res->N_value2 = *N_value2;
    strcpy(res->V_value2, V_value2);
    */
    return;
}

void delete_value_serv(int key, char *res) {
    char filename[20]; 
    sprintf(filename, "./claves/%d.txt", key); 

    // Verificar si el archivo existe
    if (access(filename, F_OK) != 0) {
        printf("Error\n");
        return;
    }

    if (unlink(filename) == -1) {
        printf("Error\n");
        return;
    }

    printf("All good\n");
    return;
}

void modify_value_serv(int key, char *value1, int N_value2, char *V_value2, char *res) {
    delete_value_serv(key, res);
    // Llamar a set_value_serv para crear un nuevo archivo con el nuevo valor
    set_value_serv(key, value1, N_value2, V_value2, res);
    printf("All good\n");
    /*
    res->N_value2 = N_value2;
    */
    return;
}

void exists_serv(int key, char*res) {
    char filename[20]; 
    sprintf(filename, "./claves/%d.txt", key); 

    FILE *clavesFile = fopen(filename, "r");
    if (clavesFile == NULL) {
        printf("All good\n");
        return;
    }
    while (fscanf(clavesFile, "%d %s %d", &key_leida, valor1, &N_value2) == 3) {
        if (key_leida == key) {
            fclose(clavesFile);
            printf("Key existe\n");
            return;
        }
    }
    
    fclose(clavesFile);
    printf("Error\n");
    return;
}