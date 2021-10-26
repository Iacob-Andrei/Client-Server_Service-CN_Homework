#pragma once

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
#include <utmp.h>
#include <sys/utsname.h>

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
