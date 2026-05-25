#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include "Parser/parser.h"


int main() {
    int pid;
    int * pidHijos;
    int ** pipes;

    tline * entradaParseada;
    char entradaUsuario[1024];

    printf("msh > ");
    while(fgets(entradaUsuario, sizeof(entradaUsuario),stdin)){
        printf("msh > ");
        entradaParseada = tokenize(entradaUsuario);
        if(entradaParseada == NULL){
            continue;
        }
        if (strcmp(entradaParseada->commands[0].argv[0],"cd") == 0){
            if (entradaParseada->commands[0].argv[1] == NULL) {
                chdir(getenv("HOME"));
            }else{
                int fallo = chdir(entradaParseada->commands[0].argv[1]);
                if (fallo == -1){
                    printf("Error no se ha encontrado el directorio especificado");
                }
            }
        }else if (entradaParseada->ncommands == 1){
            pid = fork();

            if(pid < 0){
                fprintf(stderr, "Fallo el fork() %s/n" , strerror(errno));
                exit(-1);
            }else if (pid == 0){
                if (entradaParseada->redirect_output != NULL){
                    int ficheroRedireccion;
                    printf("Se ha redirigido la salida al fichero %s\n ",entradaParseada->redirect_output);
                    ficheroRedireccion = open(entradaParseada->redirect_output,O_WRONLY | O_CREAT | O_TRUNC,0600);
                    if (ficheroRedireccion < 0){
                        printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_output,strerror(errno));
                        return 1;
                    }else{
                        dup2(ficheroRedireccion,1);
                    }
                }

                if (entradaParseada->redirect_input != NULL){
                    int ficheroRedireccion;
                    printf("Se ha redirigido la entrada al fichero %s\n ",entradaParseada->redirect_input);
                    ficheroRedireccion = open(entradaParseada->redirect_input,O_RDONLY);
                    if (ficheroRedireccion < 0){
                        printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_input,strerror(errno));
                        return 1;
                    }else{
                        dup2(ficheroRedireccion,0);
                    }
                }
                if (entradaParseada->redirect_error != NULL){
                    int ficheroRedireccion;
                    printf("Se ha redirigido el error al fichero %s\n ",entradaParseada->redirect_error);
                    ficheroRedireccion = open(entradaParseada->redirect_error,O_WRONLY | O_CREAT | O_TRUNC,0600);
                    if (ficheroRedireccion < 0){
                        printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_error,strerror(errno));
                        return 1;
                    }else{
                        dup2(ficheroRedireccion,2);
                    }
                }
                execvp(entradaParseada->commands[0].filename,entradaParseada->commands[0].argv);
                printf("Error al ejecutar el comando: %s\n", strerror(errno));
            }else{
                wait(NULL);

            }
        }else if (entradaParseada->ncommands > 1){

            pidHijos = malloc(entradaParseada->ncommands * sizeof (int));
            pipes = (int **) malloc((entradaParseada->ncommands-1) * sizeof (int *));


            for (int i =0;i<entradaParseada->ncommands-1;i++){
                pipes[i] = (int *) malloc(2 * sizeof (int));
                if(pipe(pipes[i]) < 0)
                    fprintf(stderr, "Fallo al crear el pipe %s/n" , strerror(errno));
            }



            for (int i =0;i<entradaParseada->ncommands;i++){
                pid = fork();
                if(pid < 0){
                    fprintf(stderr, "Fallo al crear el fork() %s\n" , strerror(errno));
                    exit(-1);
                }else if (pid == 0){
                    if (i == 0){

                        close(pipes[0][0]);
                        for (int cerrar = 1;cerrar < entradaParseada->ncommands-1;cerrar++){
                            close(pipes[cerrar][0]);
                            close(pipes[cerrar][1]);
                        }
                        if (entradaParseada->redirect_input!= NULL){
                            int ficheroRedireccion;
                            printf("Se ha redirigido la entrada al fichero %s\n ",entradaParseada->redirect_input);
                            ficheroRedireccion = open(entradaParseada->redirect_input,O_RDONLY);
                            if (ficheroRedireccion < 0){
                                printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_input,strerror(errno));
                                return 1;
                            }else{
                                dup2(ficheroRedireccion,0);
                            }
                        }
                        dup2(pipes[0][1],1);
                        close(pipes[0][1]);

                    }else if (i == entradaParseada->ncommands -1){

                        close(pipes[i-1][1]);
                        for (int cerrar = 0;cerrar < entradaParseada->ncommands -2;cerrar++){
                            close(pipes[cerrar][0]);
                            close(pipes[cerrar][1]);
                        }
                        if (entradaParseada->redirect_output!= NULL){
                            int ficheroRedireccion;
                            printf("Se ha redirigido la salida al fichero %s\n ",entradaParseada->redirect_output);
                            ficheroRedireccion = open(entradaParseada->redirect_output,O_WRONLY | O_CREAT | O_TRUNC,0600);
                            if (ficheroRedireccion < 0){
                                printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_input,strerror(errno));
                                return 1;
                            }else{
                                dup2(ficheroRedireccion,1);
                            }
                        }
                        if (entradaParseada->redirect_error != NULL){
                            int ficheroRedireccion;
                            printf("Se ha redirigido el error al fichero %s\n ",entradaParseada->redirect_error);
                            ficheroRedireccion = open(entradaParseada->redirect_error,O_WRONLY | O_CREAT | O_TRUNC,0600);
                            if (ficheroRedireccion < 0){
                                printf("Error al abrir el fichero %s.\n%s.\n", entradaParseada->redirect_error,strerror(errno));
                                return 1;
                            }else{
                                dup2(ficheroRedireccion,2);
                            }
                        }
                        dup2(pipes[i-1][0],0);
                        close(pipes[i-1][0]);
                    }else{ //Soy comando intermedio
                        for (int j = 0;j<=i-1;j++){
                            if (j == i-1){
                                close(pipes[j][1]);
                            }else{
                                close(pipes[j][0]);
                                close(pipes[j][1]);
                            }
                        }

                        for(int j = i;j<=entradaParseada->ncommands-2;j++){
                            if (j==i){
                                close(pipes[j][0]);
                            }else{
                                close(pipes[j][0]);
                                close(pipes[j][1]);
                            }
                        }

                        dup2(pipes[i-1][0],0);
                        close(pipes[i-1][0]);
                        dup2(pipes[i][1],1);
                        close(pipes[i][1]);




                    }
                    execvp(entradaParseada->commands[i].filename, entradaParseada->commands[i].argv);
                    fprintf(stderr,"%s: No se encuentra el mandato\n",entradaParseada->commands[i].filename);

                }else{
                    pidHijos[i] = pid;

                }
                }
            for(int k=0; k<entradaParseada->ncommands-1; k++){
                close(pipes[k][1]);
                close(pipes[k][0]);
            }
            for(int k=0; k<entradaParseada->ncommands; k++){
                waitpid(pidHijos[k],NULL,0);
            }
            for(int i=0; i<entradaParseada->ncommands-1; i++){
                free(pipes[i]);
            }
            free(pipes);
            free(pidHijos);
            }
        }
    }

