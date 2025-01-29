#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>
#define MAXIMA_LONGITUD_buffer 500

//funciones 
void lecturaBuffer(char* buffer);
void lecturaAmpersand(char* buffer, int a);
int contadorAmpersand(char* buffer);
int contadorEspacios(char* buffer);
int cd(char* dir);
char *limpiarBuffer(char* buffer);
char** argumentosComando(char * buffer, int tamano);
int ejecutarComando(char* comando);
int ejecutarComandoEnFichero(char* comando,char* ficheroOut);

int search_exit(const char *line);	
int is_single_word(const char *str);


// Función para contar el número de caracteres '&' en una cadena
int contadorAmpersand(char* buffer) {
    char *aux = buffer; // Puntero auxiliar para recorrer la cadena
    int ampersand = 0; // Contador para los caracteres '&'
    int i = 0; // Variable de índice para iterar sobre la cadena

    // Iterar sobre la cadena hasta alcanzar el final ('\0')
    while (aux[i] != '\0') {
        // Si el carácter actual es '&', incrementar el contador
        if (aux[i++] == '&') {
            ampersand++; 
        }
    }
    // Retornar el número de caracteres '&' encontrados
    return ampersand;
}

/*
Función para contar la cantidad de espacios en una cadena.

Argumentos:
- buffer: Cadena de caracteres en la que se contarán los espacios.

Retorno:
- int: Número de espacios encontrados en la cadena.
*/
int contadorEspacios(char* buffer) {
    char *aux = buffer; // Puntero auxiliar para recorrer la cadena
    int espacio = 0; // Contador para los espacios
    int i = 0; // Variable de índice para iterar sobre la cadena

    // Iterar sobre la cadena hasta alcanzar el final ('\0')
    while (aux[i] != '\0') {
        // Si el carácter actual es un espacio, incrementar el contador
        if (aux[i++] == ' ') {
            espacio++; 
        }
    }
    // Retornar la cantidad de espacios encontrados en la cadena
    return espacio;
}

/*
Función para cambiar el directorio actual.

Argumentos:
- dir: Cadena que contiene el directorio al que se desea cambiar.

Retorno:
- int: 0 si el cambio de directorio fue exitoso, -1 si ocurrió un error.
*/
int cd(char* dir) {
    char* path = dir + 3; // Se extrae la ruta del directorio del comando (se asume que el comando tiene el formato "cd <ruta>")
    //printf("\n\n cd:%s", path); // Se imprime la ruta del directorio (comentario de depuración)
    
    // Intentar cambiar al directorio especificado
    if (chdir(path) != 0) { 
        return -1; // Se retorna -1 si ocurrió un error al cambiar el directorio
    }
    return 0; // Se retorna 0 si el cambio de directorio fue exitoso
}


/*
Función para limpiar un buffer de caracteres.

Argumentos:
- buffer: Puntero a una cadena de caracteres que se desea limpiar.

Retorno:
- char*: Puntero al buffer de caracteres limpio. Retorna NULL si el buffer está vacío.
*/
char *limpiarBuffer(char* buffer) {
    // Verificar si el buffer está vacío o tiene longitud menor o igual a cero
    if (strlen(buffer) <= 0 || buffer[0]=='\0') {
        return NULL; // Retorna NULL si el buffer está vacío
    }

    char* bufferLimpio = strchr(buffer, '\0'); // Puntero al final del buffer ('\0')
    bufferLimpio--; // Retroceder un paso para apuntar al último caracter

    // Iterar hacia atrás mientras el caracter actual no sea imprimible
    while (!(bufferLimpio[0] > 32 && bufferLimpio[0] < 127)) {
        bufferLimpio[0] = '\0'; // Limpiar el caracter actual estableciéndolo como fin de cadena
        bufferLimpio--; // Retroceder al caracter anterior
    }
    
    return buffer; // Retorna el buffer de caracteres limpio
}


/*
Función para dividir una cadena en palabras individuales y almacenarlas en un array de strings.

Argumentos:
- buffer: Cadena de caracteres que se desea dividir.
- tamano: Tamaño máximo esperado para el array de strings resultante.

Retorno:
- char**: Array de strings que contiene las palabras individuales. La última entrada es NULL.
           Retorna NULL si el tamaño especificado es menor que 0 o si ocurre un error durante la asignación de memoria.
*/
char** argumentosComando(char* buffer, int tamano) {
    // Verificar si el tamaño es válido
    if (tamano < 0)
        return NULL;

    // Asignar memoria para el array de strings
    char** argumentos = (char**)malloc((tamano + 2) * sizeof(char*));
    if (argumentos == NULL)
        exit(1);

    // Asignar memoria para cada string del array
    for (int i = 0; i < (tamano + 2); i++) {
        argumentos[i] = (char*)malloc(20 * sizeof(char)); // Se reserva espacio para cada palabra (20 caracteres)
        if (argumentos[i] == NULL)
            exit(1);
    }

    // Dividir la cadena en palabras individuales utilizando el delimitador " "
    char* palabra;
    char* delimiter = " ";
    int contador = 0;
    palabra = strtok(buffer, delimiter);	
    while (palabra != NULL) {
        // Copiar cada palabra al array de strings
        strcpy(argumentos[contador], palabra);
        contador++;
        palabra = strtok(NULL, delimiter);
    }
    argumentos[contador] = NULL; // Establecer el último elemento del array como NULL

    // Imprimir cada palabra (comentario de depuración)
    for (int i = 0; i < contador; i++) {
        //printf("\n\n\n\nargumentosComando: %s\n", argumentos[i]);
    }

    return argumentos; // Retornar el array de strings
}

/*
Función para ejecutar un comando ingresado por el usuario.

Argumentos:
- comando: Cadena que contiene el comando a ejecutar.

Retorno:
- int: 0 si la ejecución del comando fue exitosa, -1 si ocurrió un error.
*/
int ejecutarComando(char* comando) {
    // Verificar si el comando está vacío
    if (limpiarBuffer(comando) == NULL)
        return -1;

    // Contar la cantidad de espacios en el comando
    int i = contadorEspacios(comando);
    //printf("espacios: %d\n", i); // Comentario de depuración

    // Obtener los argumentos del comando
    char** argumentos = argumentosComando(comando, i);

    // Crear un nuevo proceso hijo para ejecutar el comando
    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo
        int status_code = execvp(argumentos[0], argumentos); // Ejecutar el comando con sus argumentos
        if (status_code == -1) {
            // Si ocurre un error durante la ejecución del comando, imprimir un mensaje de error
            char mensaje_error[30] = "An error has occurred\n";
            fprintf(stderr, "%s", mensaje_error);
            free(argumentos); // Liberar la memoria asignada para los argumentos
            exit(1); // Salir con un código de error
        }
        exit(0); // Salir con éxito
    } else {
        // Proceso padre
        int status = 0;
        wait(&status); // Esperar a que el proceso hijo termine
    }

    free(argumentos); // Liberar la memoria asignada para los argumentos
    return 0; // Retornar 0 si la ejecución del comando fue exitosa
}


/*
Función para ejecutar un comando y redirigir la salida a un archivo.

Argumentos:
- comando: Cadena que contiene el comando a ejecutar.
- ficheroOut: Nombre del archivo de salida donde se redirigirá la salida del comando.

Retorno:
- int: 0 si la ejecución del comando fue exitosa, 1 si ocurrió un error.
*/
// Función para ejecutar un comando y redirigir su salida a un archivo
int ejecutarComandoEnFichero(char *comando,char *ficheroOut) {
    // Crear un nuevo proceso hijo para ejecutar el comando
    //printf("comando %s",comando);
    pid_t pid = fork();
    if (pid == -1) {
        // Error al crear el proceso hijo
        fprintf(stderr, "An error has occurred\n");
        exit(1);
    } else if (pid == 0) {
        // Proceso hijo
        // Abrir el archivo de salida para escritura (crearlo si no existe)
        int fd = open(ficheroOut, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            // Error al abrir el archivo de salida
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
        // Redirigir la salida estándar al archivo
        if (dup2(fd, STDOUT_FILENO) == -1) {
            // Error al redirigir la salida estándar al archivo
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }

        // Ejecutar el comando
        execlp("sh", "sh", "-c", comando, NULL);
        // Cerrar el descriptor de archivo que ya no se necesita
        close(fd);
        // Si el comando falla, imprimir un mensaje de error y salir
        //fprintf(stderr, "An error has occurred\n");
        //exit(1);
    } else {
        // Proceso padre
        // Esperar a que el proceso hijo termine
        int status;
        waitpid(pid, &status, 0);
        // Verificar el estado de finalización del proceso hijo
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            // El proceso hijo terminó de manera anormal o con un código de salida distinto de 0
            fprintf(stderr, "An error has occurred\n");
            exit(1);
        }
}

return 0;
}    

/*
Función para leer y ejecutar comandos en paralelo separados por el carácter '&' en una cadena.

Argumentos:
- buffer: Cadena de caracteres que contiene los comandos separados por el carácter '&'.
- a: Cantidad de '&' presentes en la cadena.

Retorno:
- void
*/
void lecturaAmpersand(char* buffer, int a) {
    // Iterar sobre los comandos separados por el carácter '&'
    char *bufferAux = buffer; // Puntero auxiliar para recorrer la cadena
    pid_t pid;
    pid_t wpid;
    int status = 0;
    for (int i = 0; i < a + 1; i++) {
        // Buscar la próxima aparición del carácter '&'
        char* aux = strchr(bufferAux, '&');
        // Si se encuentra, establecerlo como fin de cadena para obtener el comando
        if (aux != NULL)
            aux[0] = '\0';
        
        // Crear un nuevo proceso hijo para ejecutar el comando
        pid = fork();
        if (pid == 0) {
            // Proceso hijo
            // Ejecutar el comando
            lecturaBuffer(bufferAux);
            exit(0); // Salir del proceso hijo
        } else {
            // Proceso padre
            bufferAux = aux + 1; // Mover el puntero al siguiente comando
        }
    }

    // Esperar a que todos los procesos hijos terminen
    while ((wpid = wait(&status)) > 0);
    return;
}

/*
Función para buscar la cadena "exit" en una línea y verificar si el resto de la línea contiene solo espacios.

Argumentos:
- line: Cadena de caracteres en la que se buscará la palabra "exit".

Retorno:
- int: 0 si se encuentra "exit" seguido de solo espacios, 1 en caso contrario.
*/
int search_exit(const char *line) {
    const char *ptr = line; // Puntero para buscar "exit" en la cadena

    // Buscar la posición de "exit" en la cadena
    ptr = strstr(line, "exit");

    // Si "exit" no se encuentra en la cadena, retornar 1
    if (ptr == NULL)
        return 1;

    // Mover el puntero después de "exit"
    ptr += strlen("exit");

    // Verificar si el resto de la cadena contiene solo espacios
    while (*ptr != '\0') {
        if (*ptr != ' ') {
            // Se encontró un carácter que no es espacio
            return 1;
        }
        ptr++;
    }

    // La cadena está vacía o solo contiene espacios después de "exit"
    return 0;
}

/*
Función para verificar si una cadena contiene solo una palabra.

Argumentos:
- str: Cadena de caracteres que se desea verificar.

Retorno:
- int: 1 si la cadena contiene más de una palabra, 0 si contiene solo una palabra o está vacía.
*/
int is_single_word(const char *str) {
    // Verificar si la cadena está vacía o contiene un salto de línea
    if (str[0] == '\0' || str[0] == '\n') {
        return 1; // Cadena vacía o solo un salto de línea
    }

    // Iterar sobre la cadena para verificar si contiene más de una palabra
    while (*str != '\0' && *str != '\n') {
        if (*str == ' ') {
            return 1; // Se encontró un espacio, lo que indica que hay más de una palabra
        }
        str++; // Mover al siguiente carácter
    }

    // La cadena contiene solo una palabra
    return 0;
}

/*
Función para verificar si una cadena no está vacía.

Argumentos:
- str: Cadena de caracteres que se desea verificar.

Retorno:
- int: 1 si la cadena no está vacía, 0 si está vacía.
*/
int is_not_empty(const char *str) {
    // Iterar sobre la cadena para verificar si contiene algún carácter que no sea un espacio en blanco
    while (str[0] != '\0') {
        if (str[0] != ' ') {
            return 1; // La cadena no está vacía
        }
        str++; // Mover al siguiente carácter
    }
    return 0; // La cadena está vacía
}

int verificarCadenaAmpersand(const char *cadena) {
    int contadorAmpersand = 0;
    int esAmpersandAnterior = 1; // Flag para verificar ampersands consecutivos

    // Verificar si la cadena está vacía 
    int i;
    for(i = 0; cadena[i] != '\0'; i++) {
    if(cadena[0] == '&')
      return 1;
        if (cadena[i] == '&') {
            contadorAmpersand++;

            // Verificar si hay ampersands consecutivos
            if (esAmpersandAnterior)
                return 0;
            esAmpersandAnterior = 1;
        } else {
            esAmpersandAnterior = 0;
        }
    }

    // Verificar si hay 0mpersands al principio o al final de la cadena
    if ( cadena[contadorAmpersand - 1] == '&')
        return 1;

    return 0;
}

// Función para procesa1 un buffer de 0ntrada
void lecturaBuffer(char* buffer) {
    // Verificar si el buffer no está vacío y contiene caracteres diferentes de espacios
    if(strlen(buffer) != 0 && is_not_empty(buffer)) {
        //printf("lectura: %s\n",buffer);
        // Verificar si el buffer contiene el comando "PACK"
        if(strncmp(buffer, "PACK", 4) == 0) {
            char bufferComando[MAXIMA_LONGITUD_buffer];
            printf("\ncomando> ");
            fgets(bufferComando, MAXIMA_LONGITUD_buffer, stdin);
            if(strlen(bufferComando) > 0 && !(strlen(bufferComando) == 1 && bufferComando[0] == '\n')) {
                char *comando = (char*)malloc(500 * sizeof(char));
                strcpy(comando, "time -p ");
                strcat(comando, bufferComando);
                lecturaBuffer(comando);
                free(comando);
            }
            return;
        }
        // Verificar si el buffer contiene el carácter '&'
        if(strchr(buffer, '&') != NULL) {
            
            if(verificarCadenaAmpersand(buffer)){
              fprintf(stderr, "An error has occurred\n");
              exit(0);
            }
            int a = contadorAmpersand(buffer);
            lecturaAmpersand(buffer, a);
            return ;
        }
        // Verificar si el buffer contiene la palabra "exit"
        if(strstr(buffer, "exit") != NULL) {
            if(search_exit(buffer)) {
                fprintf(stderr, "An error has occurred\n");
            }
            exit(0);
        }
        // Verificar si el buffer contiene el comando "cd"
        if(strncmp(buffer, "cd", 2) == 0) {
            if(cd(buffer) < 0) {
                fprintf(stderr, "An error has occurred\n");
                exit(0);
            }
        } else {
            // Verificar si el buffer contiene el operador '>'
            char *ficheroOut;
            if((ficheroOut = strchr(buffer, '>')) != NULL) {
                ficheroOut[0] = '\0';
                ficheroOut++;
                while(ficheroOut[0] == ' ')
                    ficheroOut++;
                if(!is_not_empty(buffer) || is_single_word(ficheroOut) ){
                  fprintf(stderr, "An error has occurred\n");
                  exit(0);
                }
                ejecutarComandoEnFichero(buffer,ficheroOut);
            }else
            // Ejecutar el comando del buffer
            if(ejecutarComando(buffer)<0)
              fprintf(stderr, "An error has occurred\n");      
        }
    } else {
        // El buffer está vacío o contiene solo espacios
        //fprintf(stderr, "An error has occurred\n");
    }
}

int main(int argc, char *argv[]) {
    size_t len = 0;
    char *buffer = NULL; // Declaración del buffer
    int intrucciones =0;
    if (argc > 1) {
        // Lectura desde un archivo
        FILE *f;
        if ((f = fopen(argv[1], "r")) == NULL) {
            // Error de apertura del archivo
            fprintf(stderr, "An error has occurred\n");
            return 1;
        }
        while (getline(&buffer, &len, f) != -1) {
            // Procesar cada línea del archivo si no está vacía
            if (is_not_empty(buffer)) {
                buffer[strcspn(buffer, "\n")] = '\0'; // Eliminar el salto de línea
                lecturaBuffer(buffer);
                intrucciones++;
            }
        }
        fclose(f); // Cerrar el archivo después de la lectura
        if(intrucciones==0){
          fprintf(stderr, "An error has occurred\n");
          return 1;
        }
    } else {
        // Lectura desde la entrada estándar (consola)
        while (1) {
            printf("\nUVash> "); // Prompt
            if (getline(&buffer, &len, stdin) != -1) {
                // Procesar la entrada del usuario si no está vacía
                if (is_not_empty(buffer)) {
                    buffer[strcspn(buffer, "\n")] = '\0'; // Eliminar el salto de línea
                    lecturaBuffer(buffer);
                }
            }
        }
    }

    // Liberar la memoria asignada al buffer
    free(buffer);

    return 0;
}
