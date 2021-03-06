/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// #include <iostream>
// #include <fstream>



#include </home/scorpion/mtechProj/hmac.cpp>     // include crypto functions 

#define CREATE_SERVER_SOCKET(port) \
    int sockfd, newsockfd, portno;\
    socklen_t clilen;\
    struct sockaddr_in serv_addr, cli_addr;\
    /*create a socket file descriptor */ \
    sockfd = socket(AF_INET, SOCK_STREAM, 0); \
    if (sockfd < 0) \
       error("ERROR opening socket"); \
    bzero((char *) &serv_addr, sizeof(serv_addr));    /* set server address bits to zero*/ \
    portno = port; \
    serv_addr.sin_family = AF_INET;  \
    serv_addr.sin_addr.s_addr = INADDR_ANY;  /* INADDR_ANY is server ip address*/ \
    serv_addr.sin_port = htons(portno);   /* htons converts port number in host byte order to a port number in network byte order.*/ \
    /* bind the server to port and host ... this steps makes it a server socket */ \
    if (bind(sockfd, (struct sockaddr *) &serv_addr, \
              sizeof(serv_addr)) < 0)  \
              error("ERROR on binding"); 


#define CREATE_CLIENT_SOCKET(port,ser)  \
    int32_t sockfd, portno, n;\
    struct sockaddr_in serv_addr;\
    struct hostent *server;\
    sockfd = socket(AF_INET, SOCK_STREAM, 0); \
    if (sockfd < 0) \
        error("ERROR opening socket");\
    /*Get the port number and server */ \
    portno = port; \
    server = ser; \
    if (server == NULL) {\
        fprintf(stderr,"ERROR, no such host\n"); \
        exit(0); \
    } \
    /*Initialie socket port, address and address family */ \
    bzero((char *) &serv_addr, sizeof(serv_addr)); \
    serv_addr.sin_family = AF_INET; \
    bcopy((char *)server->h_addr, \
         (char *)&serv_addr.sin_addr.s_addr, \
         server->h_length); \
    serv_addr.sin_port = htons(portno); \
    /*connect to server */ \
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  \
        error("ERROR connecting");


typedef unsigned char byte;

struct bundle{
    int a,b;
    byte eflag;
    byte nonce[16];
    byte content_ab[32];  // fixing the content size to 32 bytes for now
    byte hmac[32];
    byte inputs;
    int outputs;
};

typedef struct bundle packet;

using namespace std;

int block_recv(const int fd, char* data, unsigned int len)
{
    int i, j = 0;

    while (len > 0) {
        i = recv(fd, data, len, 0);
        if (i <= 0) {
            return -1;
        }
        data += i;
        len -= i;
        j += i;
    }

    return j;
}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void fetch_content(packet* p, int a, int b)
{
    byte mem[1000];
    for(int i = 0; i < 1000;i++)
    {
        mem[i] = i;
    }
    int count = 0;
    for(int i = a;i<=b;i++)
        p->content_ab[count++] = mem[i];
}

int verify_sign(byte* msg, int len, byte* sign)
{
        EVP_PKEY *vkey = NULL;
        int rc = make_skey(&vkey); /* generate vkey */
        assert(rc == 0);
        if(rc != 0)
            exit(1);
        
        assert(vkey != NULL);
        if(vkey == NULL)
            exit(1); 
        printf("%d\n", sizeof(sign));
        rc = verify_it(msg, len, sign, 32, vkey);
        return rc;
}

void fwdReqToDom0(packet p)
{
   
    CREATE_CLIENT_SOCKET(7891,gethostbyname("localhost"))

    // send packet
    int left = sizeof(p);
    int wc;
    while (left) {
        wc = write(sockfd, (byte*)(&p) + sizeof(p) - left, left);
        if (wc < 0) 
            error("ERROR writing to socket");
        left -= wc;
    }

    byte sendBuffer[256];
    bzero(sendBuffer,sizeof(sendBuffer));
    n = read(sockfd,sendBuffer,sizeof(sendBuffer) -1 );
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",sendBuffer);
    close(sockfd);
}

int main(int argc, char *argv[])
{
    //Declaration section 
    

    packet p;
    byte buffer[256];

    int x[10] = {0,32,64,96,128,170,192,224,256,288};
    int y[10] = {31,63,95,127,169,191,223,255,287,319};

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    CREATE_SERVER_SOCKET(atoi(argv[1])) // creates and binds the server socket

    // while(1){     /* listen continuously */
        listen(sockfd,5);

        // accept a connection 
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, 
                    (struct sockaddr *) &cli_addr, 
                    &clilen);
        if (newsockfd < 0) 
             error("ERROR on accept");

        if(block_recv(newsockfd, (char*)(&p), sizeof(p)) != sizeof(p)) 
        {
            error("ERROR while reading from socket");
        }
        p.a = ntohl(p.a);
        printf("\na: %d\n",p.a);
        p.b = ntohl(p.b);
        printf("b: %d\n\n",p.b);
        print_it("nonce",p.nonce,sizeof(p.nonce));
        print_it("signature",p.hmac,sizeof(p.hmac));
        printf("eflag: %d\n", p.eflag);


        // verify the signature to check integrity of request ..code commented due to some problems
        // int rc = verify_sign((byte*)&p,(sizeof(int)*2)+17,p.hmac);
        // if(rc == 0) {
        //     printf("Verified signature\n");
        // } else {
        //     printf("Failed to verify signature, return code %d\n", rc);
        //     close(newsockfd);
        //     continue; /* close socket start listening again */
        // }


        bzero(p.hmac,sizeof(p.hmac)); /*remove signature after verification*/
        fetch_content(&p,p.a,p.b); /*Fetch contents of memory between locations a & b*/
        print_it("Content[a,b]",p.content_ab,sizeof(p.content_ab));
        p.inputs = 5; /* give a random input */

        int n = write(newsockfd,"app dom received request",24);
        if (n < 0) error("ERROR writing to socket");
        close(newsockfd);
        close(sockfd);

        //send data to control domain
        // we can later use a seperate thread to do this instead of a function call 
        fwdReqToDom0(p);
        
     // }
     return 0; 
}

