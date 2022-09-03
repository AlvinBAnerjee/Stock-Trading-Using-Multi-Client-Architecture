#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<arpa/inet.h>



void RequestSender(int x, int sockfd) //function to request the sender to enter the item code , total quantity to buy, price per item
{
    int temp1=0;
    int n;
    int ICode, quantity, rate;
    printf("enter the item code (x) , total quantity to buy(y), price per item(z)  (separated by space):\n example: x y z \n");
    scanf("%d %d %d", &ICode, &quantity, &rate);
    if(temp1 || ICode > 10 || ICode < 1  ){
        printf("invalid item code\n");
        return;
    }
    if(rate < 0 || quantity <= 0 ){
        printf("invalid qty and price\n");
        return;
    }

    char sender_msg[20];
    sprintf(sender_msg, "%d %d %d %d", x, ICode, quantity, rate);
    n = write(sockfd, sender_msg, strlen(sender_msg));
    (n < 0)?({
        perror("Writing Error to Socket");
        exit(0);
    }):({ temp1=-1;});
    return;

}

void StatusViewer(int x, int sockfd)//function to check the status of buy and sell orders
{
    int y=3;
    int temp=y&1;
    while(temp!=0){
        temp--;
        int n;

        if(x == 3)
            n = write(sockfd,"3",1);
        else
            n = write(sockfd,"4",1);
        if(n < 0)
        {
            perror("Writing Error to Socket");
            exit(0);
        }
        char character_buffer[1024];

        //bzero(buffer,1024);
        for(int i=0;i<1024;i++)
        {
            character_buffer[i]=0;
        }
        n = read(sockfd, character_buffer, 1023);
        if(n < 0)
        {
            perror("Reading Error to Socket");
            exit(0);
        }
        printf("%s", character_buffer);
    }
}

void LoginUtil(int sockfd)//function for checking whether login credentials is correct or not from pass.txt file
{
    int x=0;
    int N;
    char socket_buffer[256];
    N = read(sockfd, socket_buffer, 255);
    (!(N >= 0)) ? (

            perror("Reading Error"),
            exit(0)

            )
            :
            (
                    printf("\n")
                    );

    (socket_buffer[0] == 50) ? (//buffer[0]=='2'
    {
        perror("Max limit for client exceeded\n");
        exit(0);
    }) : (x=-1);
    //bzero(buffer,256);
    for(int i=0;i<256;i++)
        socket_buffer[0]=0;

    printf("Please Enter Login id: ");

    fgets(socket_buffer, 255, stdin);
    int sl=strlen(socket_buffer);
    socket_buffer[sl - 1] = '\0';

    N = write(sockfd, socket_buffer, sl);

    (!(N >= 0)) ?
    ({
        perror("Writing error\n");
        exit(0);
    }) :(x=-1);
    //bzero(buffer,256);
    for(int j=0;j<256;j++)
        socket_buffer[0]=0;

    N = read(sockfd, socket_buffer, 255);

    (!(N >= 0)) ?
    ({
        perror("Reading Error");
        exit(0);
    }) :(x=-1);

    if(socket_buffer[0] == '3')
    {
        perror("Invalid Login ID\n");
        exit(0);
    }
    else if(socket_buffer[0] == '0')
    {
        perror("Invalid login Id\n");
        exit(0);
    }
    else
    {
        printf("Please enter the Password: ");
        bzero(socket_buffer, 256);
        fgets(socket_buffer, 255, stdin);

        socket_buffer[strlen(socket_buffer) - 1] = '\0';
        N = write(sockfd, socket_buffer, strlen(socket_buffer));

        (!(N >= 0)) ?
        ({
            perror("Writing Error to Socket");
            exit(0);
        }) :(x=-1);

        //bzero(buffer,256);
        for(int i=0;i<256;i++)
            socket_buffer[i]=0;
        N = read(sockfd, socket_buffer, 255);

        (N < 0) ?
        ({
            perror("Reading Error From Socket");
            exit(0);
        }) :(x=-1);

        if(socket_buffer[0] == '0')
        {
            perror("invalid password\n");
            exit(0);
        }

        // if(x==-1)
        //     printf("Login Unsuccessful");
    }
}

int main(int argc, char *argv[])
{
    struct hostent *server;
    int sockfd;
    char socket_buffer[256];
    int portno;
    int n;
    int count=0;
    struct sockaddr_in serv_addr;
    portno = 5555;//atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(count)
    {
        ;
    }
    else if(sockfd < 0)
    {
        perror("Socket Opening Error");
        exit(0);
    }

    server = gethostbyname("localhost");

    if(server != NULL)
    {
        ;
    }
    else
    {
        fprintf(stderr, "ERROR, no such host\n");
        ;
    }
    
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    serv_addr.sin_addr.s_addr =inet_addr(argv[1]);

   // bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);

    (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)?({perror("ERROR connecting");exit(0);}):({count=1;});


    LoginUtil(sockfd);

    printf("Login Done\n");
    printf("Press 1 To send a buy request \n");
    printf("Press 2 To send a sell request \n");
    printf("Press 3 To view order status \n");
    printf("Press 4 To view your trade status \n");
    printf("Enter 5 to Exit\n");

    char c;
    while(1)
    {
        printf("enter your option: ");
        c = getchar();
        getchar();
        if(c=='1'){
            RequestSender(1, sockfd);
            getchar();
        }
        else if (c=='2'){
            RequestSender(2, sockfd);
            getchar();
        }
        else if(c=='3'){
            StatusViewer(3, sockfd);
        }
        else if(c=='4'){
            StatusViewer(4, sockfd);
        }
        else if(c=='5')
        {
            exit(0);
        }
        else{
            printf("Invalid Input\n");
        }
    }

    return 0;
}