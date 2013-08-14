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


#define LENGTH 512 

int main()
{
    /* Variable Definition */
    int sockfd; 
    int nsockfd;
    char revbuf[LENGTH]; 
    struct sockaddr_in remote_addr;
    char list[LENGTH];
    /* Get the Socket file descriptor */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("ERROR: Failed to obtain Socket Descriptor!\n");
        exit(1);
    }

    /* Fill the socket address struct */
    remote_addr.sin_family = AF_INET; 
    remote_addr.sin_port = htons(8000); 
    inet_pton(AF_INET, "127.0.0.1", &remote_addr.sin_addr); 
    bzero(&(remote_addr.sin_zero), 8);

    /* Try to connect the remote */
    if (connect(sockfd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("ERROR: Failed to connect to the host! \n");
        exit(1);
    }
    else 
        printf("[Client] Connected to server at port 8000...ok!\n");

    //listing files in server
    char ls[10];
    printf("\nDo you want to list files in server: Enter [y/n]\n");
    scanf("%s",ls);
    write(sockfd,ls,sizeof(ls));
    int bz,n,tmp;
   read(sockfd,list,sizeof(list));
    n=strlen(list);
    int i=0;
    for(i=0;i<n;i++)
    {
        if(list[i]=='|')
            printf("\n");
        else
            printf("%c",list[i]);
    }
    printf("\nList received from server\n");
    
	    


     /* Receive File from Server */
    char gtfile[20];
    printf("\nEnter the file to get:");
    scanf("%s",gtfile);
    write(sockfd,gtfile,sizeof(gtfile));
    printf("[Client] Receiveing file from Server and saving it as %s...",gtfile);
    FILE *fr = fopen(gtfile, "w");
    if(fr == NULL)
        printf("File %s Cannot be opened.\n", gtfile);
    else
    {
        bzero(revbuf, LENGTH); 
        int fr_block_sz = 0;
        while((fr_block_sz = recv(sockfd, revbuf, LENGTH, 0)) > 0)
        {
            int write_sz = fwrite(revbuf, sizeof(char), fr_block_sz, fr);
            if(write_sz < fr_block_sz)
            {
                printf("File write failed.\n");
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
                printf("recv() failed \n");
            }
        }
        printf("Ok received from server!\n");
        fclose(fr);
    }
  


  
    //sending file to server
    char fs_name[100];
    printf("Enter the file to be transfered to server\n");
    scanf("%s",fs_name);
    write(sockfd,fs_name,sizeof(fs_name));
    char sdbuf[LENGTH]; 
    printf("[Client] Sending %s to the Server... ", fs_name);
    FILE *fs = fopen(fs_name, "r");
    if(fs == NULL)
    { 
        printf("ERROR: File %s not found.\n", fs_name);
        exit(1);
    }

        bzero(sdbuf, LENGTH); 
        int fs_block_sz; 
        while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0)
        {
            if(send(sockfd, sdbuf, fs_block_sz, 0) < 0)
            {
                printf("ERROR: Failed to send file %s.\n");
                break;
            }
            bzero(sdbuf, LENGTH);
        }
        printf("Ok File %s from Client was Sent!\n", fs_name);
    //}

    close (sockfd);
    printf("[Client] Connection lost.\n");
    return (0);
}
