#include "libraries.h"


int main()
{   
    printf("[client] Am deschis sesiunea! \n\n");
    printf("[client] Comenzile pe care le puteti folosi sunt:\n");
    printf("[client] 1) login:username \n");
    printf("[client] 2) get-logged-users\n");
    printf("[client] 3) get-proc-info:pid\n");
    printf("[client] 4) logout \n");
    printf("[client] 6) quit\n\n");
    char s[1024],raspuns[1024], byts_msg1[5], byts_msg2[5];
    int num, cl_to_sv, sv_to_cl, flag_comm_valid=0;
    int nr_byts , nr_byts_int;

    cl_to_sv = open(client_to_server, O_WRONLY);        // deschid ca sa SCRIU
    sv_to_cl = open(server_to_client, O_RDONLY);        // deschid ca sa CITESC


    printf("[client] Introduceti o comanda: ");

    while (gets(s), !feof(stdin)) 
    {   
        
        nr_byts = strlen(s);
        sprintf( byts_msg1 , "%d" , nr_byts );              // in byts_msg1 scriu numarul de byts pe care il trimit cu msg1

        if ((num = write(cl_to_sv, byts_msg1 , strlen(byts_msg1))) == -1)
                perror("Problema la scriere in FIFO!");

        sleep(1);

        if ((num = write(cl_to_sv, s , nr_byts)) == -1)
                perror("Problema la scriere in FIFO!");

        if ((num = read(sv_to_cl, byts_msg2, 5)) == -1)
            perror("Eroare la citirea din FIFO!");
        else 
        {
            byts_msg2[num] = '\0';

            nr_byts_int = char_to_int(byts_msg2);

            if ((num = read(sv_to_cl, raspuns , nr_byts_int)) == -1)
                perror("Eroare la citirea din FIFO!");
            else
            {
                raspuns[num] = '\0';

            if( strcmp( raspuns , "quit" ) == 0 )
            {
                printf("[client] Sesiunea se va inchide! \n");
                close(sv_to_cl);
                close(cl_to_sv);
                return 0;
            }

                printf("[client] S-a primit raspuns de la [server]: %s \n" , raspuns);
            }
        }

        printf("[client] Introduceti o comanda: ");
    }
    
}
