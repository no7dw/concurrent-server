#include "chat.h"



int listenfd, connfd;

extern int sock_server_init(int &listenfd , uint16_t server_port);



void err_sys(const char *error_string)

{

    printf("%s\n", error_string);    

}



extern void p_read_from_p(int clientfd);

extern int p_write_to_p(int clientfd, const char *msg);



void sig_chld(int signo)

{

    pid_t pid;

    int stat;

    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0 )

    {

        printf("client %d terminated \n", pid);

    }

    return;

}



void chat_server()

{

    printf("chat server start\n");

    void sig_chld(int);

    pid_t childpid;

    socklen_t clilen;

    struct sockaddr_in cliaddr ;

    

    if(sock_server_init(listenfd,(uint16_t) SERVER_PORT) < 0)

    {        

        return ;

    }

    

    signal(SIGCHLD, sig_chld);

    

    char buf[200] = {0};

    

    for(;;)

    {

        clilen = sizeof(cliaddr);

        if((connfd = accept(listenfd, (SA*)&cliaddr, &clilen )) < 0)

        {

            if(errno == EINTR)

                continue;

            else

                err_sys("accept error");

        }

        printf("incoming connection from IP: %s Port: %d\n", 

               inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)), 

               ntohs(cliaddr.sin_port));        

        p_read_from_p(connfd);

        p_write_to_p(connfd, "hi , this is server");

        close(connfd);

    }//for