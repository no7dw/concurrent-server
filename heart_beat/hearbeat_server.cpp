#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#if TIME_WITH_SYS_TIME
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#else
#if HAVE_SYS_TIME_H
#include        <sys/time.h>    /* includes <time.h> unsafely */
#else
#include        <time.h>                /* old system? */
#endif
#endif
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <sys/stat.h>    /* for S_xxx file mode constants */
#include        <sys/uio.h>             /* for iovec{} and readv/writev */
#include        <unistd.h>
#include        <sys/wait.h>

#define SERV_PORT 9877
#define LISTENQ 100
#define MAXLINE 512

static int servfd;
static int nsec;
static int maxnalarms;
static int nprobes;
static void sig_urg(int),sig_alrm(int);

void heartbeat_serv(int servfd_arg, int nsec_arg, int maxnalarms_arg)
{
    servfd = servfd_arg;
    if((nsec = nsec_arg) < 1)
        nsec = 1;
    if((maxnalarms = maxnalarms_arg) < nsec)
        maxnalarms = nsec;
    
    signal(SIGURG, sig_urg);//SIGURG ---- 当out-of-band data接收的时候可能发送
    fcntl(servfd, F_SETOWN, getpid());
    
    signal(SIGALRM, sig_alrm);//SIGALRM ----- 用alarm函数设置的timer超时或setitimer函数设置的interval timer超时
    alarm(nsec);
}
void err_sys(const char *error_string)
{
    printf("%s\n", error_string);    
}
static void sig_urg(int signo)
{
    int n;
    char c;
    if(( n = recv(servfd, &c, 1, MSG_OOB)) < 0 )
    {
        if(errno != EWOULDBLOCK)
            err_sys("recv error");
    }
    send(servfd, &c, 1, MSG_OOB);
    nprobes = 0;
    return;
}

static void sig_alrm (int signo)
{
    if(++nprobes > maxnalarms)
    {
        printf("no probs from client\n");
        exit(0);
    }
    alarm(nsec);
    return;
}
void str_echo(int sockfd)
{
    ssize_t n;
    char line[MAXLINE];
    heartbeat_serv(sockfd, 1 , 5);
    
    for(;;)
    {

        if(( n = read(sockfd, line, MAXLINE)) == 0)
        {
            return;//connection closed by other end
        }
        write(sockfd, line, n);
    }
}
void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0 )
    {
        printf("child %d terminated \n", pid);
    }
    return;
}

#define SA struct sockaddr

int main(int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    
    bind(listenfd, (SA*)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    signal(SIGCHLD, sig_chld);
    
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
        if((childpid = fork() ) == 0)
        {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }//for
}
