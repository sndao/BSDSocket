//
//  main.cpp
//  stevendao-pa3-server
//
//  Created by Steven Dao on 12/6/13.
//  Copyright (c) 2013 Steven Dao. All rights reserved.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <iostream>

//struct seat
//{
//    int status; //0=free/cancelled, 1=reserved, 2=ticketed
//    char name[5];
//    char number[5];
//};
struct flight
{
    int number, row;
    char seatsReserved[1024];
    char seatsTicketed[1024];
    char seatsCancelled[1024];
    int seatsTaken;
};
struct agent
{
    int rTime, tTime, cTime, pTime;
    int rCount, tCount, cCount, pCount;
};

int main(int argc, char *argv[])
{
    char * pch;
    
    FILE *fr;
    char fcountchar[100];
    int fcount;
    char line[100];
    
    /*extract data from input file*/
    fr = fopen ("data.txt", "rt");
    fgets(fcountchar, 100, fr);
    sscanf(fcountchar, "%d", &fcount);  //set # flights
    struct flight flight[fcount];
    struct agent agent[10];
    
    printf("\n\n");
    
    int i=1;
    while (i<=fcount)
    {
        fgets(line, 100, fr);
        int j=0, num[3];
        char * pch;
        pch = strtok (line," ");
        
        while (j<3 && pch != NULL)
        {
            sscanf(pch, "%d", &num[j]);
            pch = strtok (NULL, " ");
            j=j+1;
        }
        if (i>0)
        {
            flight[i].number = num[0];
            flight[i].row = num[1];
        }
        if (i==fcount)
        {
            flight[0].number = num[0];
            flight[0].row = num[1];
        }
        i++;
    }
    
    for (int i=0; i<10; i++)
    {
        flight[i].seatsTaken=0;
        flight[i].seatsReserved[0] = ' ';
        flight[i].seatsTicketed[0] = ' ';
    }
    fclose(fr);
    
    /* connecting with sockets */
    int sockfd = 0, n = 0;
    int listenfd = 0, connfd = 0;
    int end=15;
    struct sockaddr_in serv_addr;
    char sendBuff[1024];
    char recvBuff[1024];
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(5000);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    listen(listenfd, 10);
    int j=1;
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);    //accept connection
        
        while ((n = read(connfd, recvBuff, sizeof(recvBuff)-1)) > 0)
        {
            if (recvBuff[0]=='X')   //process time (format ex: X 11111), agent, rTime, tTime, cTime, pTime
            {
                int currentAgent = recvBuff[2] - '0';
                printf("Processing agent execution timings for agent_%d:\n%s\n",currentAgent, recvBuff);
                agent[currentAgent].rTime = (recvBuff[3] - '0')*1000;   //for 1000 microseconds = 1 ms
                agent[currentAgent].tTime = (recvBuff[4] - '0')*1000;
                agent[currentAgent].cTime = (recvBuff[5] - '0')*1000;
                agent[currentAgent].pTime = (recvBuff[6] - '0')*1000;
                snprintf(sendBuff, sizeof(sendBuff), "TIMINGS RECEIVED FOR agent_%d. [%d ms]\n",currentAgent, agent[currentAgent].pTime/1000);
            }
            
            if (recvBuff[0]=='R')   //process reservations (format ex: R 1 1111 4 1A A 1B B 5C C 19D D)
            {
                int currentAgent = recvBuff[2] - '0';
                int currentFlight = recvBuff[7] - '0';
                //int count =recvBuff[9] - '0';
                //agent[currentAgent].rCount = (recvBuff[9] - '0');
                printf("Processing reservations for agent_%d:\n%s\n",currentAgent, recvBuff);
                char subbuff[50];
                char seat[10];
                char name[10];
                char seatname[10];
                
                memcpy(subbuff, &recvBuff[11], 50);
                snprintf(sendBuff, 1024, "\n");
                
                pch = strtok (subbuff," ");
                while (pch != NULL)
                {
                    strcpy(seat, pch);
                    pch = strtok (NULL, " ");
                    strcpy(name, pch);
                    
                    snprintf(seatname, 10, "%s %s", seat, name);
                    
                    if (strstr(flight[currentFlight].seatsReserved, seat) == NULL || strstr(flight[currentFlight].seatsReserved, seatname) != NULL)
                    {
                        snprintf(flight[currentFlight].seatsReserved, 1024, "%s %s",flight[currentFlight].seatsReserved, seat);
                        snprintf(sendBuff, 1024, "%s\nAGENT_%d R CONFIRM: %s", sendBuff, currentAgent, seat);
                        
                        snprintf(flight[currentFlight].seatsReserved, 1024, "%s %s",flight[currentFlight].seatsReserved, name);
                        snprintf(sendBuff, 1024, "%s %s", sendBuff, name);
                    }
                    else
                    {
                        snprintf(sendBuff, 1024, "%s\nAGENT_%d R DENY: %s", sendBuff, currentAgent, seatname);
                    }
                    pch = strtok (NULL, " ");
                }
                
                //printf("%s", flight[currentFlight].seatsReserved);
                usleep(agent[currentAgent].rTime);
            }
            
            
            if (recvBuff[0]=='T')   //process tickets (format ex: T 1 1111 4 1A A 1B B 5C C 19D D)
            {
                int currentAgent = recvBuff[2] - '0';
                int currentFlight =recvBuff[7] - '0';
                printf("\n\nTICKETING FOR agent_%d:\n%s\n",currentAgent, recvBuff);
                
                char subbuff[100];
                char seat[100];
                char name[100];
                char seatname[100];
                
                memcpy(subbuff, &recvBuff[11], 100);
                snprintf(sendBuff, 1024, "\n");
                
                pch = strtok (subbuff," ");
                
                while (pch != NULL)
                {
                    strcpy(seat, pch);
                    pch = strtok (NULL, " \0");
                    strcpy(name, pch);
                    
                    snprintf(seatname, 10, "%s %s", seat, name);
                    if ((strstr(flight[currentFlight].seatsReserved, seat) == NULL || strstr(flight[currentFlight].seatsReserved, seatname) != NULL) && (strstr(flight[currentFlight].seatsTicketed, seat) == NULL || strstr(flight[currentFlight].seatsTicketed, seatname) != NULL))
                    {
                        snprintf(flight[currentFlight].seatsTicketed, 1024, "%s %s",flight[currentFlight].seatsTicketed, seatname);
                        snprintf(sendBuff, 1024, "%s\nAGENT_%d T CONFIRM: %s", sendBuff, currentAgent, seatname);
                    }
                    else
                    {
                        snprintf(sendBuff, 1024, "%s\nAGENT_%d T DENY: %s", sendBuff, currentAgent, seatname);
                    }
                    pch = strtok (NULL, " ");
                }
                
                printf("Ticketed: %s", flight[currentFlight].seatsTicketed);
                usleep(agent[currentAgent].tTime);
                
            }
            
            if (recvBuff[0]=='C')   //process cancellations (format ex: C 1 1111 4 1A A 1B B 5C C 19D D)
            {
                int currentAgent = recvBuff[2] - '0';
                int currentFlight =recvBuff[7] - '0';
                recvBuff[12] = toupper(recvBuff[12]);
                printf("\n\nCANCELLATIONS FOR agent_%d:\n%s\n",currentAgent, recvBuff);
                
                char subbuff[100];
                char seat[100];
                char name[100];
                char seatname[100];
                memcpy(subbuff, &recvBuff[11], 100);
                
                snprintf(sendBuff, 1024, "\n");
                pch = strtok (subbuff," ");
                
                strcpy(seat, pch);
                pch = strtok (NULL, " ");
                strcpy(name, pch);
                snprintf(seatname, 10, "%s %s", seat, name);
                std::string str(flight[currentFlight].seatsTicketed);
                std::string str2(seatname);
                
                printf("Cancelled: %s", flight[currentFlight].seatsTicketed);
                usleep(agent[currentAgent].cTime);
            }
            
            
            if (recvBuff[0]=='P')   //process tickets (format ex: P 1 E2)
            {
                int currentAgent = recvBuff[2] - '0';
                char subbuff[100];
                char name[100];
                memcpy(subbuff, &recvBuff[11], 100);
                snprintf(sendBuff, 1024, "\n");
                
                pch = strtok (subbuff," ");
                while (pch != NULL)
                {
                    strcpy(name, pch);
                    pch = strtok (NULL, " \0");
                    snprintf(sendBuff, 1024, "\nAGENT_%d T CHECK PASSENGER: %s", currentAgent, name);
                }
                printf("Checking Passenger: %s", name);
                usleep(agent[currentAgent].tTime);
                
            }
            j++;
            
            if (j==end)
            {
                printf("\n\n\nSummary Report:\n");
                
                for (int i=0; i<fcount; i++)
                {
                    printf("Flight: %d\n", flight[i].number);
                    printf("Seats Reserved: %s\n", flight[i].seatsReserved);
                    printf("Seats Ticketed: %s\n\n", flight[i].seatsTicketed);
                }
            }
            recvBuff[n] = 0;
            break;
        }
        write(connfd, sendBuff, strlen(sendBuff));
        close(connfd);
        sleep(1);
    }
}


