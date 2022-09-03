/* Creates a datagram server. The port number is passed as an argument. This server runs forever */
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

struct node // structure to store information about rate,quantity and trader_id of particular trader for either buy request or sell request
{
    int rate, qty, t_id;
    struct node *right;
};

struct trade_data // structure to store information about buy request or sell request of a specific trader
{
    struct node *sell_request;
    struct node *buy_request;
    int item_no;
};

char *trade_info[] = {"trader1.txt", "trader2.txt", "trader3.txt", "trader4.txt", "trader5.txt"}; // Traders Buy and Sell information will be stored in text file

struct trade_data trade_array[10]; // Array of 10 traders which stores information about buy and sell request of a particular trader

char *info_file_names[] = {"item1.txt", "item2.txt", "item3.txt", "item4.txt", "item5.txt", "item6.txt",
                           "item7.txt", "item8.txt", "item9.txt", "item10.txt"}; // Items Buy and Sell information will be stored in text file

int trader_id_sd[5] = {-1, -1, -1, -1, -1};

struct node *get_Node(int quantity, int price, int trader_id) // structure to store information about rate,quantity and trader_id of particular trader for either buy request or sell request
{
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    new_node->t_id = trader_id;
    new_node->rate = price;
    new_node->qty = quantity;
    new_node->right = NULL;

    return new_node;
}

void print_list(struct node *counter) // Function to print quantity and rate of available items
{
    while (counter != NULL)
    {
        printf("%d , %d ", counter->qty, counter->rate);
        counter = counter->right;
    }
    printf("\n");
}

void get_trade_status(int sd, int trader_id) // The trader can view his matched trades. This will provide the trader with the details of
                                             //  what orders were matched, their quantities, prices and counterparty code.
{
    FILE *fp;
    char *socket_buffer;
    char *order_str;

    fp == fopen(trade_info[trader_id], "r");
    socket_buffer = (char *)malloc(40 * sizeof(char));
    order_str = (char *)malloc(1024 * sizeof(char));
    int n;

    strcpy(order_str, "\ntrade-type(item_no) qty ,price - trader_id ,qty ,price\n");

    while (fgets(socket_buffer, 39, fp) != NULL)
    {
        strcat(order_str, socket_buffer);
    }
    n = write(sd, order_str, strlen(order_str));
    if (!(n >= 0))
        perror("Error in Writing to Socket");
    fclose(fp);
}

void view_order_status(int sd) // The Trader can view the position of buy and sell orders in the system. This will
                               // display the current best sell (least price) and the best buy (max price) for each item and their quantities.
{
    struct node *head;
    int i = 0, n;
    char *socket_buffer = (char *)malloc(30 * sizeof(char));
    char *order_str = (char *)malloc(1024 * sizeof(char));
    strcpy(order_str, "");
    int quantity, price;
    for (i = 0; i < 10; i++)
    {
        sprintf(socket_buffer, "\n%d:\n", i + 1);
        strcat(order_str, socket_buffer);
        head = trade_array[i].buy_request;
        if (head != NULL)
        {
            quantity = head->qty;
            price = head->rate;
            while (head->right && head->right->rate == price)
            {
                head = head->right;
                quantity += head->qty;
            }
            sprintf(socket_buffer, "buy: %d @ %d\n", quantity, price);
            strcat(order_str, socket_buffer);
        }
        else
        {
            sprintf(socket_buffer, "buy: \n");
            strcat(order_str, socket_buffer);
        }

        head = trade_array[i].sell_request;
        if (head != NULL)
        {
            quantity = head->qty;
            price = head->rate;
            while (head->right && head->right->rate == price)
            {
                head = head->right;
                quantity += head->qty;
            }
            sprintf(socket_buffer, "sell: %d @ %d\n", quantity, price);
            strcat(order_str, socket_buffer);
        }
        else
        {
            sprintf(socket_buffer, "sell: \n");
            strcat(order_str, socket_buffer);
        }
    }
    n = write(sd, order_str, strlen(order_str));
    if (n < 0)
        perror("Error writing to socket");
}

void insert_into_buy_queue(int item_no, int quantity, int price, int trader_id) // function to insert any buy request in the queue along with item no,quantity,
                                                                                // price and trader_id
    {
        struct node *newnode = get_Node(quantity, price, trader_id);
    struct node *temp = trade_array[item_no - 1].buy_request;
    (temp == NULL) ? ({
        trade_array[item_no - 1].buy_request = newnode;
    })
                   : ({
                         if (temp->rate < price)
                         {
                             newnode->right = temp;
                             trade_array[item_no - 1].buy_request = newnode;
                             return;
                         }
                         for (; temp->right != NULL;)
                         {

                             if (temp->right->rate < price)
                             {
                                 newnode->right = temp->right;
                                 temp->right = newnode;
                                 break;
                             }

                             temp = temp->right;
                         }
                         if (temp->right == NULL)
                         {
                             temp->right = newnode;
                         }
                     });
}

void insert_into_sell_queue(int item_no, int quantity, int price, int trader_id) // function to insert any sellrequest in the queue along with item no,quantity,
                                                                                 // price and trader_id
{
    struct node *newnode = get_Node(quantity, price, trader_id);
    struct node *temp = trade_array[item_no - 1].sell_request;
    (temp == NULL) ? ({
        trade_array[item_no - 1].sell_request = newnode;
    })
                   : ({
                         if (temp->rate > price)
                         {
                             newnode->right = temp;
                             trade_array[item_no - 1].sell_request = newnode;
                             return;
                         }
                         for (; temp->right != NULL;)
                         {

                             if (temp->right->rate > price)
                             {
                                 newnode->right = temp->right;
                                 temp->right = newnode;
                                 break;
                             }

                             temp = temp->right;
                         }
                         if (temp->right == NULL)
                         {
                             temp->right = newnode;
                         }
                     });
}

void update_file(int fileindex) // function to update the text file for specific trader after performing either buy  request or sell request
{
    FILE *fp = fopen(info_file_names[fileindex], "w");
    struct node *buy_head = trade_array[fileindex].buy_request;
    struct node *sell_head = trade_array[fileindex].sell_request;

    (fp) ? ({
        fprintf(fp, "buy");
        for (; buy_head != NULL;)
        {
            fprintf(fp, " %d,%d,%d", buy_head->t_id, buy_head->qty, buy_head->rate);
            buy_head = buy_head->right;
        }
        fprintf(fp, "\nsell");
        for (; sell_head != NULL;)
        {
            fprintf(fp, " %d,%d,%d", sell_head->t_id, sell_head->qty, sell_head->rate);
            sell_head = sell_head->right;
        }
        fclose(fp);
    })
         : (printf("File %s cannot be opened\n", info_file_names[fileindex]));
}

void initialise() // function to initialise all item_number files and insert any buy request or sell request into that item_number file
{
    int i = 0;
    FILE *fp;
    char *type = (char *)malloc(10 * sizeof(char));
    char c;
    int quantity, price, trader_id;

    i = 0;
    while (i < 10)
    {
        fp = fopen(info_file_names[i], "r");
        (fp) ? ({
            if (fscanf(fp, "%s", type) != EOF)
            {
                if (strcmp(type, "buy") == 0)
                {
                    c = fgetc(fp);
                    for (; c != '\n' && fscanf(fp, "%d,%d,%d", &trader_id, &quantity, &price) != EOF;)
                    {
                        insert_into_buy_queue(i + 1, quantity, price, trader_id);// function to insert any buy request in the queue along with item no,quantity,
                                                                                // price and trader_id
                         
                        c = fgetc(fp);
                    }
                }
                fscanf(fp, "%s", type);
                if (strcmp(type, "sell") == 0)
                {
                    c = fgetc(fp);
                    for (; c != '\n' && fscanf(fp, "%d,%d,%d", &trader_id, &quantity, &price) != EOF;)
                    {
                        insert_into_sell_queue(i + 1, quantity, price, trader_id);// function to insert any sell request in the queue along with item no,quantity,
                                                                                 // price and trader_id
                        c = fgetc(fp);
                    }
                }
            }
            fclose(fp);
        })
             : (printf("File %s does not exist\n", info_file_names[i]));

        i++;
    }
}

void handle_buy_request(int item_no, int quantity, int price, int trader_id) // function to handle on a buy Request at price P and quantity Q of an item I, 
                                                                             //the server will check if there is any pending sell
                                                                             //order for the same item at old price P’ ≤ P. Among all such pending sell orders,
                                                                             //the match will be made with the one having the least selling price.
{
    FILE *gp;
    struct node *temp = trade_array[item_no - 1].sell_request;
    FILE *fp = fopen(trade_info[trader_id], "a");
    for (; quantity > 0;)
    {
        temp = trade_array[item_no - 1].sell_request;
        (temp != NULL) ? ({
            (temp->rate <= price) ? ({        //on a buy Request at new price P and quantity Q of an item I, the server will check if there is any pending sell
                                              //order for the same item at old price P’ ≤ P
                
                gp = fopen(trade_info[temp->t_id], "a");
                (temp->qty <= quantity) ? ({   //if old quantity Q' and new quantity Q,Q’ < Q then the sell order will be fully traded and the remaining buy order will be
                                               //tested for more matches.
                    
                    
                    fprintf(fp, "buy%d: %d,%d %d,%d,%d\n", item_no, quantity, price, temp->t_id + 1, temp->qty, temp->rate);  
                                        //writing buy request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file                       
                    fprintf(gp, "sell%d: %d,%d %d,%d,%d\n", item_no, temp->qty, temp->rate, trader_id + 1, quantity, price);
                                        //writing sell request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file
                    quantity = quantity - temp->qty;
                    trade_array[item_no - 1].sell_request = temp->right;
                    free(temp);
                })
                                        : ({  //If old quantity Q' and new quantity Q,Q’ > Q then the buy request will be fully traded and the remaining part, 
                                              //i.e., Q’ – Q of the sell order will remain in the sell queue at the same price P’.
                                              
                                              fprintf(fp, "buy%d: %d,%d %d,%d,%d\n", item_no, quantity, price, temp->t_id + 1, temp->qty, temp->rate);
                                                                   ///writing buy request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file 
                                              
                                              fprintf(gp, "sell%d: %d,%d %d,%d,%d\n", item_no, temp->qty, temp->rate, trader_id + 1, quantity, price);
                                                                   //writing sell request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file
                                            
                                              temp->qty = temp->qty - quantity;
                                              quantity = 0;
                                          });
                fclose(gp);
                continue;
            })
                                  : ({
                                        insert_into_buy_queue(item_no, quantity, price, trader_id);// function to insert any buy request in the queue along with item no,quantity,
                                                                                // price and trader_id
                                        break;
                                    });
        })
                       : ({
                             insert_into_buy_queue(item_no, quantity, price, trader_id);// function to insert any buy request in the queue along with item no,quantity,
                                                                                // price and trader_id
                             break;
                         });
    }
    fclose(fp);
    update_file(item_no - 1);
}

void handle_sell_request(int item_no, int quantity, int price, int trader_id) // function to handle on a sell Request at price P and quantity Q of an item I, 
                                                                             //the server will check if there is any pending sell
                                                                             //order for the same item at old price P’ ≤ P. Among all such pending sell orders,
                                                                             //the match will be made with the one having the least selling price.
{
    FILE *gp;
    struct node *temp = trade_array[item_no - 1].buy_request;
    FILE *fp = fopen(trade_info[trader_id], "a");
    for (; quantity > 0;)
    {
        temp = trade_array[item_no - 1].buy_request;
        (temp != NULL) ? ({
            (temp->rate >= price) ? ({
                gp = fopen(trade_info[temp->t_id], "a");

                (temp->qty <= quantity) ? ({
                    
                    fprintf(fp, "sell%d: %d,%d %d,%d,%d\n", item_no, quantity, price, temp->t_id + 1, temp->qty, temp->rate);
                                 //writing sell request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file

                    fprintf(gp, "buy%d: %d,%d %d,%d,%d\n", item_no, temp->qty, temp->rate, trader_id + 1, quantity, price);
                                  //writing buy request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file   
            
                    quantity = quantity - temp->qty;
                    trade_array[item_no - 1].buy_request = temp->right;
                    free(temp);
                })
                                        : ({
                                              
                                              fprintf(fp, "sell%d: %d,%d %d,%d,%d\n", item_no, quantity, price, temp->t_id + 1, temp->qty, temp->rate);
                                                    //writing sell request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file

                                              fprintf(gp, "buy%d: %d,%d %d,%d,%d\n", item_no, temp->qty, temp->rate, trader_id + 1, quantity, price);
                                                    //writing buy request in a text file along with item_no, quantity, price, trader_id,quantity and rate in a specific trader text file   
                                              
                                              temp->qty = temp->qty - quantity;
                                              quantity = 0;
                                          });
                fclose(gp);
                continue;
            })
                                  : ({
                                        insert_into_sell_queue(item_no, quantity, price, trader_id); // function to insert any sellrequest in the queue along with item no,quantity,
                                                                                 // price and trader_id
                                        break;
                                    });
        })
                       : ({
                             insert_into_sell_queue(item_no, quantity, price, trader_id);// function to insert any sellrequest in the queue along with item no,quantity,
                                                                                 // price and trader_id
                             break;
                         });
    }
    fclose(fp);
    update_file(item_no - 1);
}

void error(char *msg)       //function to print error message
{
    perror(msg);
    exit(1);
}

int verifyLogin(int newsockfd, char *LoginDetails[10])   //function to verify login details 
{
    int n, i;
    char buffer[256];
    bzero(buffer, 256);
    char *id = (char *)malloc(64 * sizeof(char));
    char *password = (char *)malloc(64 * sizeof(char));
    int flag = 0;

    n = read(newsockfd, buffer, 255);
    if (n < 0)
    {
        error("ERROR reading from socket");
    }

    strcpy(id, buffer);

    i = 0;
    while (i < 10)
    {
        if (strcmp(id, LoginDetails[i]) == 0)
        {
            (trader_id_sd[i / 2] != -1) ? ({
                n = write(newsockfd, "3", 1);
                if (n < 0)
                    perror("error writing to socket");
                return 0;
            })
                                        : ({
                                              flag = 1;
                                              strcpy(password, LoginDetails[i + 1]);
                                              break;
                                          });
        }

        i += 2;
    }

    (flag == 0) ? ({
        n = write(newsockfd, "0", 1);

        if (n < 0)
        {
            error("ERROR writing to socket");
        }
        return 0;
    })
                : ({
                      n = write(newsockfd, "1", 1);

                      if (n < 0)
                      {
                          error("ERROR writing to socket");
                      }
                      bzero(buffer, 256);
                      n = read(newsockfd, buffer, 255);

                      if (n < 0)
                      {
                          error("ERROR reading from socket");
                      }

                      (strcmp(buffer, password) != 0) ? ({
                          n = write(newsockfd, "0", 1);

                          if (n < 0)
                          {
                              error("ERROR writing to socket");
                          }
                          return 0;
                      })
                                                      : ({
                                                            n = write(newsockfd, "1", 1);

                                                            if (n < 0)
                                                            {
                                                                error("ERROR writing to socket");
                                                            }
                                                            trader_id_sd[i / 2] = newsockfd;
                                                            return 1;
                                                        });
                  });
}

void getLoginDetails(char *LoginDetails[10])   //function to get login details from a text file called pass.txt
{
    FILE *fp;
    int i = 0;
    fp = fopen("pass.txt", "r");
    char *id, *password;
    id = (char *)malloc(64 * sizeof(char));
    password = (char *)malloc(64 * sizeof(char));

    if (!fp)
    {
        printf("password file does not exist\n");
        return;
    }

    while (fscanf(fp, "%[^,],%s\n", id, password) != EOF)
    {
        strcpy(LoginDetails[i++], id);
        strcpy(LoginDetails[i++], password);
    }
    fclose(fp);
}

int main(int argc, char *argv[])           //main function starts from here
{
    initialise();
    int opt = 1;
    int portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int master_socket, serv_len;
    int client_socket[5], max_clients = 5, activity, sd, new_socket;
    int max_sd;

    int login_done = 0;
    char buffer[256];

    int n, i, j;

    fd_set readfds;

    for (i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    //	if (argc < 2)
    //	{
    //		fprintf(stderr,"ERROR, no port provided\n");
    //		exit(1);
    //	}

    master_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (master_socket < 0)
    {
        error("ERROR opening socket");
    }

    // set master socket to allow multiple connections ,
    // this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        error("setsockopt");
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));

    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(master_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("ERROR on binding");
    }

    printf("Listener on port %d \n", portno);

    char *LoginDetails[10];

    for (i = 0; i < 10; ++i)
    {
        LoginDetails[i] = (char *)malloc(64 * sizeof(char));
    }

    getLoginDetails(LoginDetails);

    if (listen(master_socket, 3) < 0)
    {
        error("listen");
    }

    // accept the incoming connection
    serv_len = sizeof(serv_addr);
    puts("Waiting for connections ...");

    while (1)
    {
        // clear the socket set
        FD_ZERO(&readfds);

        // add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            // socket descriptor
            sd = client_socket[i];

            // if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);

            // highest file descriptor number, need it for the select function
            if (sd > max_sd)
                max_sd = sd;
        }

        // wait for an activity on one of the sockets , timeout is NULL ,
        // so wait indefinitely
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket ,
        // then its an incoming connection

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen)) < 0)
            {
                error("accept");
            }
            // inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

            // add new socket to array of sockets
            for (i = 0; i < max_clients; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    n = write(new_socket, "1", 1);
                    if (n < 0)
                        perror("error in writing\n");

                    login_done = verifyLogin(new_socket, LoginDetails);
                    if (!login_done)
                    {
                        n = close(new_socket);
                        if (n < 0)
                            perror("socket cannot be closed");
                    }
                    else
                    {
                        printf("Login Done\n");
                        client_socket[i] = new_socket;
                        printf("Adding to list of sockets as %d\n", i);
                    }
                    break;
                }
            }
            if (i == 5)
            {
                n = write(new_socket, "2", 1);
                if (n < 0)
                    perror("Error writing\n");
                n = close(new_socket);
                if (n < 0)
                    perror("socket cannot be closed");
            }
        }

        // else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing , and also read the
                // incoming message
                bzero(buffer, 256);
                if ((n = read(sd, buffer, 256)) == 0)
                {
                    // Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_addr);

                    printf("Host disconnected , ip %s , port %d \n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    for (j = 0; j < 5; j++)
                    {
                        if (trader_id_sd[j] == sd)
                            trader_id_sd[j] = -1;
                    }
                    client_socket[i] = 0;
                }

                // Echo back the message that came in
                else
                {
                    // set the string terminating NULL byte on the end
                    // of the data read
                    int trader_id;
                    for (j = 0; j < 5; j++)
                    {
                        if (trader_id_sd[j] == sd)
                        {
                            trader_id = j;
                            break;
                        }
                    }
                    int option, item_no, quantity, price;
                    buffer[n] = '\0';
                    switch (buffer[0])
                    {
                    case '1':
                        sscanf(buffer, "%d %d %d %d", &option, &item_no, &quantity, &price);
                        handle_buy_request(item_no, quantity, price, trader_id);
                        break;
                    case '2':
                        sscanf(buffer, "%d %d %d %d", &option, &item_no, &quantity, &price);
                        handle_sell_request(item_no, quantity, price, trader_id);
                        break;
                    case '3':
                        view_order_status(sd);
                        break;
                    case '4':
                        get_trade_status(sd, trader_id);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
