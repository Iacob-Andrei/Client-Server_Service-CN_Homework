#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h> 
#include <sys/wait.h>
#include <dirent.h>           


#define client_to_server "client_to_server"
#define server_to_client "server_to_client"


int char_to_int( char nr[] )
{
    int numar;
    int power = 1;

    for(int i = strlen(nr) - 1 ; i >= 0 ; i-- )
    {
        numar = numar + ( nr[i] - '0' ) * power;
        power = power * 10;
    }    

    return numar;
}


int main()
{

    printf("[server] Am deschis serverul! \n\n");

    char s[300], nr_byts_str[5], username[20], username_lista[20], buff[5];
    int num, cl_to_sv, sv_to_cl, nr_byts_int;
    int flag_logged = 0;
    
    mknod(client_to_server, S_IFIFO | 0666, 0);     // am creat FIFO in care clientul scrie pt ca sv sa citeasca
    mknod(server_to_client, S_IFIFO | 0666, 0);     // am creat FIFO in care sv scrie iar client citeste

    cl_to_sv = open(client_to_server, O_RDONLY);          // deschid ca sa CITESC
    sv_to_cl = open(server_to_client, O_WRONLY);          // deschid ca sa SCRIU
    
    
    do {
        if ((num = read(cl_to_sv, nr_byts_str, 5)) == -1)                  // citesc numarul de octeti pe care ii are comanda care se va livra
            perror("Eroare la citirea din FIFO!");
        else 
        {
            nr_byts_str[num] = '\0';                                        
            nr_byts_int = char_to_int( nr_byts_str );                       // transform din char in int nr de octeti

            if ((num = read(cl_to_sv, s, nr_byts_int)) == -1)               // citesc comanda propriu zisa
                perror("Eroare la citirea din FIFO!");
            else
            {
                s[num] = '\0';
                printf("[server] Am primit comanda: %s \n" , s);            // In s am comanda
            
                if( strncmp( s, "login:" , 6 ) == 0 )
                {
                    strcpy( username , s + 6 );
                    printf("[server] Utilizatorul %s vrea sa se logheze \n" , username );

                    if( flag_logged == 0 )
                    {
                        
                        int fd[2];                                                            // bucata de cod ( partea cu pipe ) adaptata de la :
                        pipe(fd);                                                             // https://www.geeksforgeeks.org/pass-the-value-from-child-process-to-parent-process/
                        pid_t pid = fork();
                        
                        if( pid == 0 ) 
                        {
                            close(fd[0]);                                                     // nu avem nevoie de citire din pipe
                            close(1);                                                         // inchidem standart output
                            dup(fd[1]);                                                       // duplicam fd[1] in standard output ( 0 )

                            FILE *fp = fopen("usernames.txt", "r");
                            if (fp == NULL)
                                perror("eroare");

                            char buffer[20];
                            while (fgets(buffer, 20, fp))                                       // bucla de cod ( citire linii din fisier) adaptata de la:
                                {                                                               // https://www.learnc.net/c-tutorial/c-read-text-file/?fbclid=IwAR0FuYYq4mAmXTf8k9lphUFGfSd-99ubbYY6MdZ51O6fd9Ygm2MHU4TKtCU
                                    buffer[strlen(buffer)-1] = '\0';
                                    if( strcmp( username , buffer ) == 0 )
                                        {
                                            fclose(fp);
                                            write(1 , "DA" , 2 );
                                            exit(1);
                                        }
                                }

                            fclose(fp);
                            write(1, "NU", 2 );
                            exit(1);
                        } 

                        wait(NULL);                                                   // asteptam ca fiul sa verifice fisierul usernames.txt si sa trimita raspuns

                        close(0);                                                     // inchidem standart input 
                        dup(fd[0]);                                                   // duplicam fd[o] in standart input ( 0 )
                        close(fd[1]);                                                 // nu avem nevoie de scriere din pipe

                        char raspuns[2];
                        int n = read(fd[0], raspuns, 2);

                        if( raspuns[0] == 'D' )
                        {
                            flag_logged = 1;
                            strcpy( buff , "24" );

                            if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                                perror("Problema la scriere in FIFO!");

                            sleep(1);

                            if ((num = write(sv_to_cl, "User conectat cu succes.", 24 )) == -1)
                                perror("Problema la scriere in FIFO!");
                        }
                        else
                        {
                            strcpy( buff , "29" );

                            if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                                perror("Problema la scriere in FIFO!");

                            sleep(1);

                            if ((num = write(sv_to_cl, "User inexistent. Reincercati!", 29 )) == -1)
                                perror("Problema la scriere in FIFO!");
                        }
                    } 
                    else
                    {
                        strcpy( buff , "49" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Este deja conectat un utilizator pe acest client.", 49 )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }
                }
                else if( strcmp( s , "get-logged-users" ) == 0 )
                {
                    printf("[server] Utilizatorul vrea sa se vada lista \n") ;

                    if( flag_logged == 0 )
                    {
                        strcpy( buff , "48" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Utilizatorul nu este logat pentru a vedea lista." , 48 )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }
                    else
                    {
                        strcpy( buff , "40" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Informatii despre utilizatorii logati...", strlen("Informatii despre utilizatorii logati...") )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }

                }
                else if( strcmp( s , "logout" ) == 0 )                                                              
                {
                    printf("[server] Utilizatorul vrea sa se delogheze. \n");

                    if( flag_logged == 0 )
                    {      
                        strcpy( buff , "46" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Utilizatorul nu este logat pentru a se deloga." , 46 )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }
                    else
                    {
                        flag_logged = 0;

                        strcpy( buff , "29" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Utilizator delogat cu succes." , 29 )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }
                }
                else if( strncmp( s , "get-proc-info:" , 14) == 0 )
                {
                    printf("[server] Utilizatorul vrea sa afle informatii despre un pid \n");

                    if( flag_logged == 0 )
                    {
                        strcpy( buff , "47" );

                        if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                            perror("Problema la scriere in FIFO!");

                        sleep(1);

                        if ((num = write(sv_to_cl, "Utilizatorul nu este logat pentru a vedea info." , 47 )) == -1)
                            perror("Problema la scriere in FIFO!");
                    }
                    else
                    {
                        int sockp[2], child; 

                        if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockp) < 0) 
                        { 
                            perror("Err... socketpair"); 
                            exit(1); 
                        }

                        if ((child = fork()) == -1) perror("Err...fork"); 
                        else if (child == 0 )   //copil 
                        { 
                            close(sockp[1]); 

                            char cale_pid[25]= "/proc/";
                            char stats[1000]="\0", stat_line[300];
                            char pid_necesar[10];
                            int octeti_socket = strlen(stats);
                            char octeti_socket_str[5];

                            strcpy( pid_necesar , s +  14 );

                            strcat(cale_pid , pid_necesar);

                            DIR* dir = opendir(cale_pid);                               // bucata de cod adaptata dupa: https://stackoverflow.com/questions/12510874/how-can-i-check-if-a-directory-exists
                            if(dir)
                            {
                                printf("[server] Acest director pentru pid exista! \n");
                                closedir(dir);
                            }
                            else if (ENOENT == errno)
                            {
                                octeti_socket = strlen("Pid-ul nu exista.");
                                sprintf( octeti_socket_str , "%d" , octeti_socket ); 

                                if( write( sockp[0], octeti_socket_str, sizeof(octeti_socket_str) ) < 0 ) perror("[copil]Err...write");
                                sleep(1);
                                if( write( sockp[0], "Pid-ul nu exista.", sizeof("Pid-ul nu exista.") ) < 0 ) perror("[copil]Err...write"); 
                                close(sockp[0]);                                                                                 // trimite mesaj la tata ca nu exista pid
                                exit(2);
                            }
                            else
                            {
                                perror("eroare la deschidere");
                            }

                            strcat( cale_pid , "/status");
                            FILE *stats_fd = fopen( cale_pid , "r" );

                            if( stats_fd == NULL )
                            {
                                perror("eroare la deschidere");
                            }

                            while( fgets( stat_line , 300 , stats_fd ) )                                  // preiau doar info necesare
                            {
                                if ( strncmp( stat_line , "Name:" , 5) == 0 )
                                strcat( stats , stat_line );
                                else if( strncmp ( stat_line , "State:" , 6 ) == 0 )
                                strcat( stats , stat_line );
                                else if( strncmp ( stat_line , "Pid:" , 4 ) == 0 )
                                strcat( stats , stat_line );
                                else if( strncmp ( stat_line , "PPid:" , 5 ) == 0 )
                                strcat( stats , stat_line );
                                else if( strncmp ( stat_line , "Uid:" , 4 ) == 0 )
                                strcat( stats , stat_line );
                                else if( strncmp ( stat_line , "VmSize:" , 7 ) == 0 )
                                {
                                strcat( stats , stat_line );
                                break;
                                }
                            } 

                            stats[strlen(stats)-1]='\0';
                            
                            fclose(stats_fd);
                            
                            octeti_socket = strlen(stats);
                            sprintf( octeti_socket_str , "%d" , octeti_socket ); 

                            if( write( sockp[0], octeti_socket_str, sizeof(octeti_socket_str) ) < 0 ) perror("[copil]Err...write");
                            sleep(1);
                            if( write( sockp[0], stats, sizeof(stats) ) < 0 ) perror("[copil]Err...write"); 
                            close(sockp[0]);
                            exit(1);
                        } 

                            close(sockp[0]); 
                            int nr_octeti_socket;
                            char nr_octeti[5];
                            char raspuns_socket[1024];

                            if (read(sockp[1], nr_octeti, 5) < 0) perror("[parinte]Err...read"); 
                            nr_octeti_socket = char_to_int(nr_octeti);
                            if (read(sockp[1], raspuns_socket, nr_octeti_socket) < 0) perror("[parinte]Err...read"); 

                            close(sockp[1]); 
                            wait(NULL);
                            
                            if ((num = write(sv_to_cl, nr_octeti, strlen(nr_octeti))) == -1)
                                perror("Problema la scriere in FIFO!");

                            sleep(1);

                            if ((num = write(sv_to_cl, raspuns_socket , strlen(raspuns_socket) )) == -1)
                                perror("Problema la scriere in FIFO!");
                    }
                }
                else if( strcmp( s , "quit" ) == 0 )
                {
                    strcpy( buff , "4" );

                    if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                        perror("Problema la scriere in FIFO!");

                    sleep(1);

                    if ((num = write(sv_to_cl, "quit" , 4 )) == -1)
                        perror("Problema la scriere in FIFO!");

                    sleep(1);

                    printf("[server] Am inchis serverul! \n");
                    close(cl_to_sv);
                    close(sv_to_cl);

                    return 0;
                } 
                else
                {
                    printf("[server] Comanda invalida \n");

                    strcpy( buff , "17" );

                    if ((num = write(sv_to_cl, buff, strlen(buff))) == -1)
                        perror("Problema la scriere in FIFO!");

                    sleep(1);

                    if ((num = write(sv_to_cl, "Comanda invalida!" , 17 )) == -1)
                        perror("Problema la scriere in FIFO!");
                }

            }
        }
    } while ( num > 0 );

}
