

#include "chat.h"



void p_read_from_p(int clientfd)

{

    if(0 == clientfd)

        return ;

        

    Message msg_receive;

    int read_size = 0;//fd read_size    

    if(( read_size = read(clientfd, &msg_receive.msg_head, sizeof(msg_receive.msg_head))) == 0)

    {

        return;//connection closed by other end

    }

    

    if(( read_size = read(clientfd, &msg_receive.msg_content, msg_receive.msg_head.msg_length)) == 0)

    {

        return;//connection closed by other end

    }

    printf(" ----> receive msg_content:\t %s \n", msg_receive.msg_content);

    fflush(stdout);

    

}

int p_write_to_p(int clientfd, const char *msg)

{

    size_t slen = strlen(msg);

    if(slen > MAX_MSG_LENGTH || 0 == slen)

        return -1;

        

    Message msg_send;

    msg_send.msg_head.msg_length = slen;

    strncpy(msg_send.msg_content , msg, slen);

            

    int wn = write(clientfd, &msg_send ,sizeof(msg_send.msg_head)+slen );

    printf(" write finished msg:\t %s  \n",msg );