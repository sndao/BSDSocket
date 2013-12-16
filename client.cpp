//
//  main.cpp
//  stevendao-pa3-client
//
//  Created by Steven Dao on 12/6/13.
//  Copyright (c) 2013 Steven Dao. All rights reserved.

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

struct agent
{
    char rt, tt, ct, cpt, rk, tk, ck, cpk; //t = time, k = # seats
    //char r[100], t[100], c[100], cp[100];
    char transactions[5][250];  //0=times, 1=reserve, 2=ticket, 3=cancel, 4=chk_passenger
};

int main(int argc, char *argv[])
{
    FILE *fr;
    char fcountchar[100];
    int fcount, agents;
    char agentSend[5][250];
    char line[100];
    
    printf("\n\n");
    
    fr = fopen ("data.txt", "rt"); 
    
    fgets(fcountchar, 100, fr);
    sscanf(fcountchar, "%d", &fcount);
    int i=0;
    while (i<fcount)
    {
        //skip flight info part of file
        fgets(line, 100, fr);
        i++;
    }
    fgets(line, 100, fr);
    sscanf(line, "%d", &agents);    //set # agents
    
    struct agent agent[agents];
    
    int j=0;
    while (j<agents)
    {
        fgets(line, 100, fr);   //agent_
        snprintf(agent[j].transactions[0], 1024, "X ");
        snprintf(agent[j].transactions[0], 1024, "%s%d", agent[j].transactions[0], j);
        
        fgets(line, 100, fr);   //rt
        //strncat(agent[j].transactions[0], &line[8], 2);
        //agent[j].transactions[0][1] = line[8];
        snprintf(agent[j].transactions[0], 1024, "%s%c", agent[j].transactions[0], line[8]);
        
        fgets(line, 100, fr);   //tt
        snprintf(agent[j].transactions[0], 1024, "%s%c", agent[j].transactions[0], line[7]);
        
        fgets(line, 100, fr);   //ct
        snprintf(agent[j].transactions[0], 1024, "%s%c", agent[j].transactions[0], line[7]);
        
        fgets(line, 100, fr);   //cpt
        snprintf(agent[j].transactions[0], 1024, "%s%c", agent[j].transactions[0], line[16]);
        
        fgets(line, 100, fr);   //r
        for (int k=0; k<7; k++) {   line[k]=' ';}
        
        char * pch;
        pch = strtok (line," ");
        snprintf(agent[j].transactions[1], 1024, "R");
        snprintf(agent[j].transactions[1], 1024, "%s %d", agent[j].transactions[1], j);
        
        while (pch != NULL)
        {
            snprintf(agent[j].transactions[1], 1024, "%s %s",agent[j].transactions[1],pch);
            pch = strtok (NULL, " ");
        }
        //printf ("%s",agent[j].transactions[1]);
        
        fgets(line, 100, fr);   //t
        for (int k=0; k<6; k++) {   line[k]=' ';}
        
        
        for (int k=(int)strlen(line); k>0; k--)
        {
            if (!isalnum(line[k]))
                line[k] = '\0';
            else
                break;
        }
       
        
        pch = strtok (line," ");
        snprintf(agent[j].transactions[2], 1024, "T");
        snprintf(agent[j].transactions[2], 1024, "%s %d", agent[j].transactions[2], j);

        while (pch != NULL)
        {
            snprintf(agent[j].transactions[2], 1024, "%s %s",agent[j].transactions[2],pch);
            pch = strtok (NULL, " ");
        }
        //printf ("%s",agent[j].transactions[2]);
        
        fgets(line, 100, fr);   //c
        for (int k=0; k<6; k++) {   line[k]=' ';}
        
        pch = strtok (line," ");
        snprintf(agent[j].transactions[3], 1024, "C");
        snprintf(agent[j].transactions[3], 1024, "%s %d", agent[j].transactions[3], j);

        while (pch != NULL)
        {
            snprintf(agent[j].transactions[3], 1024, "%s %s",agent[j].transactions[3],pch);
            pch = strtok (NULL, " ");
        }
        //printf ("%s",agent[j].transactions[3]);
        
        
        fgets(line, 100, fr);   //cp --> P
        for (int k=0; k<15; k++) {   line[k]=' ';}
        pch = strtok (line," ");
        snprintf(agent[j].transactions[4], 1024, "P");
        snprintf(agent[j].transactions[4], 1024, "%s %d", agent[j].transactions[4], j);
        while (pch != NULL)
        {
            snprintf(agent[j].transactions[4], 1024, "%s %s",agent[j].transactions[4],pch);
            pch = strtok (NULL, " ");
        }
        //printf ("%s",agent[j].transactions[4]);
        
        fgets(line, 100, fr);   //end.
        
        j++;
    }
    fclose(fr);
    
    
    for (i = 0; i < agents; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            printf ("Logged into client: agent_%d\n", i+1);
            
            int sockfd = 0, n = 0;
            char recvBuff[1024];
            struct sockaddr_in serv_addr;
            
            int j=0;
            while (j<5)
            {
                if(argc != 2)
                {
                    printf("\n Usage: %s <ip of server> ex. 127.0.0.1 \n",argv[0]);
                    return 1;
                }
                memset(recvBuff, '0',sizeof(recvBuff));
                if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                {
                    printf("\n Error: Could not create socket \n");
                    return 1;
                }
                
                memset(&serv_addr, '0', sizeof(serv_addr));
                
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(5000);
                
                if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
                {
                    printf("\ninet_pton error occurred\n");
                    return 1;
                }
                if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                {
                    printf("\n Error: Connect Failed \n");
                    return 1;
                }
                
                write(sockfd, agent[i].transactions[j], 1024);
                while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
                {
                    recvBuff[n] = 0;
                    int count=0;
                    
                    if(fputs(recvBuff, stdout) == EOF)
                    {
                        printf("\n Error: Fputs error\n");
                    }
                    count++;
                }
                j++;
                if (j>4)
                {
                    printf("agent_%d finished.\n\n", i);
                    break;
                }
            }
            
            if(n < 0)
            {
                printf("\n Read error \n");
            }
            
            break;
        }
    }
    return 0;
}
