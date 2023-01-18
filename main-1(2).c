#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFLEN 102400
#define ADDRESS "127.0.0.1"
#define PORT 5500

int send_(int s, char *buf, int *len);
int indexOfFromAPoint(char *str, char c, int starting);
char *substring(char *destination, const char *source, int beg, char n);
void cleanup(SOCKET listener);
int readFile(const char *filename, char **output);

int main()
{

    // generate file
    system("node scriptTest2.js");

    printf("Hello, world!\n");

    int res, sendRes;
    int running;
    WSADATA wsaData;
    SOCKET listener, client;
    struct sockaddr_in address, clientAddr;
    char recvbuf[BUFLEN];
    char *inputFileContents;
    int inputFileLength;

    // initialization
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res)
    {
        printf("Startup failed\n");
        return 1;
    }

    // setup server
    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
    {
        printf("Error with construction\n");
        cleanup(0);
        return 1;
    }

    // bind server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ADDRESS);
    address.sin_port = htons(PORT);
    res = bind(listener, (struct sockaddr *)&address, sizeof(address));
    if (res == SOCKET_ERROR)
    {
        printf("Bind failed\n");
        cleanup(listener);
        return 1;
    }

    // set as listener
    res = listen(listener, SOMAXCONN);
    if (res == SOCKET_ERROR)
    {
        printf("Listen failed\n");
        cleanup(listener);
        return 1;
    }

    // load file
    inputFileLength = readFile("index.html", &inputFileContents);
    if (!inputFileLength || !inputFileContents)
    {
        printf("Could not read input form\n");
        cleanup(listener);
        return 1;
    }
    printf("File length: %d\n", inputFileLength);

    // done setting up
    printf("Accepting on %s:%d\n", ADDRESS, PORT);
    running = 1;

    while (running)
    {
        // accept client
        int clientAddrLen;
        client = accept(listener, NULL, NULL);
        if (client == INVALID_SOCKET)
        {
            printf("Could not accept\n");
            cleanup(listener);
            return 1;
        }

        // get client info
        getpeername(client, (struct sockaddr *)&clientAddr, &clientAddrLen);
        printf("Client connected at: %s:%d\n", inet_ntoa(address.sin_addr));

        // receive
        res = recv(client, recvbuf, BUFLEN, 0);
        if (res > 0)
        {
            // print message
            recvbuf[res] = 0;
            printf("%s\n", recvbuf);

            // test if GET command
            if (!memcmp(recvbuf, "GET", 3))
            {
                printf("GET\n");
                sendRes = send(client, inputFileContents, inputFileLength, 0);
                if (sendRes == SOCKET_ERROR)
                {
                    printf("Send failed\n");
                }
            }
            // test if POST command
            else if (!memcmp(recvbuf, "POST", 4))
            {
                printf("POST\n");
                sendRes = send(client, inputFileContents, inputFileLength, 0);
                if (sendRes == SOCKET_ERROR)
                {
                    printf("Send failed\n");
                }

                // parse message
                // find equals sign
                int i = res - 1;
                for (i = 862; i < res - 1; i++)
                {
                    if (recvbuf[i] == '=')
                    {
                        i++;
                        break;
                    }
                }
                // content from cursor onwards contains data
                printf("Received: %s\n", recvbuf + 862);

                // get length
                int len = 0;
                char *Msg = malloc(100000000);
                int k = 0;
                for (int j = 862; j <= res; j++)
                {
                    len++;
                    Msg[k] = recvbuf[j];
                    if (recvbuf[j] == '%')
                    {
                        j += 2;
                    }
                    i++;
                }

                // read characters
                char *msg = malloc(res - 861 + 1);
                for (int cursor = 0, j = 862; cursor < len; cursor++, j++)
                {
                    char c = recvbuf[j];
                    if (c == '%')
                    {
                        // get hex val of the next two characters
                        msg[cursor] = 0;
                        for (int k = 1; k <= 2; k++)
                        {
                            c = recvbuf[j + k];
                            if (c >= 'A')
                            {
                                c = c - 'A' + 10;
                            }
                            else
                            {
                                c -= '0';
                            }
                            msg[cursor] <<= 4;
                            msg[cursor] |= c;
                        }
                        j += 2;
                    }
                    else if (c == '+')
                    {
                        msg[cursor] = ' ';
                    }
                    else
                    {
                        msg[cursor] = c;
                    }
                }
                msg[len] = 0; // terminator

                printf("Parsed (%d): %s\n", len, msg);

                char temp[1024];
                char value[1024];
                i = 0;
                int leng = strlen(msg);
                while (i < leng)
                {
                    // printf("here while\n");
                    substring(temp, msg, i, '=');
                    // printf("here temp\n");
                    i = indexOfFromAPoint(msg, '=', i) + 1;
                    substring(value, msg, i, '&');
                    // printf("here val\n");
                    i = indexOfFromAPoint(msg, '&', i);
                    printf("%s : %s\n", temp, value);
                    // printf("\n%d\n\n", i);
                    if (i == -1)
                        break;
                    i++;
                }
                // test message
                if (!memcmp(msg, "/quit", 5))
                {
                    printf("Received quit message\n");
                    running = 0;
                }
                //================================================================================================

                //=======================================================================================
                free(msg);
                /* char *head = "HTTP/1.1 200 {\"name\":\"Data Send\"}\r\n";

                 char *ctype = malloc(102400);
                 ctype = "content-Type: text/text\r\n";
                 len = strlen(head);

                 send_(client, head, &len);
                 msg = "<html lang=\"en\"><head><title>Document</title></head<<body><script>Alert(\"data received\")</script></body></html>";
                 send(client, msg, strlen(msg), 0);*/
                // sleep(10);
                // send(client, inputFileContents, inputFileLength, 0);

                // shutdown(client, SD_BOTH);
            }
        }
        else if (!res)
        {
            printf("Client disconnected\n");
        }
        else
        {
            printf("Receive failed\n");
        }

        // shutdown client
        shutdown(client, SD_BOTH);
        closesocket(client);
        client = INVALID_SOCKET;
    }

    cleanup(listener);
    printf("Shutting down.\nGood night.\n");

    return 0;
}

void cleanup(SOCKET listener)
{
    if (listener && listener != INVALID_SOCKET)
    {
        closesocket(listener);
    }
    WSACleanup();
}

int send_(int s, char *buf, int *len)
{
    int total;
    int bytesleft;
    int n;
    total = 0;
    bytesleft = *len;
    /// printf("buf = %s\n", buf);
    /// printf("\n\n before the loop len = %d\n",*len);
    while (total < *len)
    {
        n = send(s, buf + total, bytesleft, 0);
        if (n == -1)
        {
            break;
        }
        total += n;
        bytesleft -= n;
        /// printf("n varying %d\n",n);
    }
    /// printf("\n\n after the loop len = %d\n total = %d\n",*len,total);

    *len = total;
    /// printf("n in send_ %d\n",n);
    return n == -1 ? -1 : 0;
}

int indexOfFromAPoint(char *str, char c, int starting)
{
    int i = 0;
    for (i = starting; i < strlen(str); i++)
    {
        if (str[i] == c)
            return i;
    }

    return -1;
}

char *substring(char *destination, const char *source, int beg, char n)
{
    // extracts `n` characters from the source string starting from `beg` index
    // and copy them into the destination string
    int exist = indexOfFromAPoint(source, n, beg);
    if (exist == -1)
        n = '\0';

    while (n != *(source + beg))
    {
        // printf("here sub\n");
        *destination = *(source + beg);

        destination++;
        source++;
    }

    // null terminate destination string
    *destination = '\0';

    // return the destination string
    return destination;
}

int readFile(const char *filename, char **output)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        return 0;
    }

    // get length
    // move cursor to end
    fseek(fp, 0L, SEEK_END);
    // get remaining length
    int len = ftell(fp);
    // return to original position
    fseek(fp, 0, SEEK_SET);

    // read
    *output = malloc(len + 1);
    fread(*output, len, 1, fp);
    (*output)[len] = 0;
    fclose(fp);

    return len;
}