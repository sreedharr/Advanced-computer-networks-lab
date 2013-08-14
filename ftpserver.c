#include<dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <netdb.h>

#define BACKLOG 5
#define LENGTH 512 

int main ()
{
    /* Defining Variables */
    int sockfd; 
    int nsockfd; 
    int num;
    DIR *d;
    struct dirent *ent;
    int sin_size; 
    struct sockaddr_in addr_local; 
    struct sockaddr_in addr_remote;
    char revbuf[LENGTH]; 

    
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {
        printf("ERROR: Failed to obtain Socket Descriptor\n");
        exit(1);
    }
    else 
        printf("[Server] Obtaining socket descriptor successfully.\n");

   
    addr_local.sin_family = AF_INET; 
    addr_local.sin_port = htons(8000);
    addr_local.sin_addr.s_addr = INADDR_ANY; 
    bzero(&(addr_local.sin_zero), 8); 

   
    if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 )
    {
        printf("ERROR: Failed to bind Port. \n");
        exit(1);
    }
    else 
        printf("[Server] Binded tcp port 8000 in addr 127.0.0.1 sucessfully.\n");

  
    if(listen(sockfd,BACKLOG) == -1)
    {
        printf("ERROR: Failed to listen Port\n");
        exit(1);
    }
    else
        printf ("[Server] Listening the port 8000 successfully.\n");

    int success = 0;
    while(success == 0)
    {
        sin_size = sizeof(struct sockaddr_in);

        
        if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) == -1) 
        {
            printf("ERROR: Obtaining new Socket Despcritor\n");
            exit(1);
        }
        else 
            printf("[Server] Server has got connected from %s.\n", inet_ntoa(addr_remote.sin_addr));
       

	//listing files in current directory of server

        char ls[2000]=""; 
        int i=0,n; 
        char di[1000];
        getcwd(di,sizeof(di));
        printf("\ndirectory:%s\n",di);
        char x[10];
        int q=read(nsockfd,x,sizeof(x));
        if((strcmp(x,"y")==0) || (strcmp(x,"Y")==0))
        {
            if((d=opendir(di))!=NULL)
            {
                while((ent=readdir(d))!=NULL)
                {
                    strcat(ls,ent->d_name);
                    n=strlen(ls);
                    ls[n]='|';
                    //write(nsockfd,ent->d_name,sizeof(ent->d_name));
                    printf("%s\n",ent->d_name);
                }
            n=i;
            printf("\n%s\n",ls);
            }
    
        }
        
        write(nsockfd,ls,sizeof(ls));
            
       

	// Send File to Client
        
            char fs_name[50];
            read(nsockfd,fs_name,sizeof(fs_name));
            char sdbuf[LENGTH]; // Send buffer
            printf("[Server] Sending %s to the Client...", fs_name);
            FILE *fs = fopen(fs_name, "r");
            if(fs == NULL)
            {
                printf("ERROR: File %s not found on server. \n", fs_name);
                exit(1);
            }

            bzero(sdbuf, LENGTH); 
            int fs_block_sz; 
            while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs))>0)
            {
                if(send(nsockfd, sdbuf, fs_block_sz, 0) < 0)
                {
                    printf("ERROR: Failed to send file %s.\n", fs_name);
                    exit(1);
                }
                bzero(sdbuf, LENGTH);
            }
            printf("Ok sent to client!\n");
            

        //Receive File from Client 
        char fr_name[100];
        read(nsockfd,fr_name,sizeof(fr_name));
        printf("%s sending from client",fr_name);
        FILE *fr = fopen(fr_name, "w");
        if(fr == NULL)
            printf("File %s Cannot be opened file on server.\n", fr_name);
        else
        {
            bzero(revbuf, LENGTH); 
            int fr_block_sz = 0;
            while((fr_block_sz = recv(nsockfd, revbuf, LENGTH, 0)) > 0) 
            {
                int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
                if(write_sz < fr_block_sz)
                {
                    error("File write failed on server.\n");
                }
                bzero(revbuf, LENGTH);
                if (fr_block_sz == 0 || fr_block_sz != 512) 
                {
                    break;
                }
            }
            if(fr_block_sz < 0)
            {
                if (errno == EAGAIN)
                {
                    printf("recv() timed out.\n");
                }
                else
                {
                    printf("recv() failed due to errno = %d\n", errno);
                    exit(1);
                }
            }
            printf("Ok received from client!\n");
            fclose(fr); 
        }

            success = 1;
            close(nsockfd);
            printf("[Server] Connection with Client closed. Server will wait now...\n");
            while(waitpid(-1, NULL, WNOHANG) > 0);
      
	
                
    }
}
