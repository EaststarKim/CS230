#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void *thread(void *vargp){
		int connfd=(int)vargp;
		pthread_detach(pthread_self());
		char buf[MAXLINE];
		rio_t rio;

		Rio_readinitb(&rio,connfd);
		while(Rio_readlineb(&rio,buf,MAXLINE)!=0){
			int clientfd;
			char host[MAXLINE],port[MAXLINE]="80",request[MAXLINE]="GET ";

			if(strlen(buf)<3){
					break;
			}
			if(buf[0]!='G')continue;

			int len=0;
			char *p=strstr(buf,":")+3,*q,*r,*s;
			for(q=p;*q!='/'&&*q!=':';++q,++len);
			snprintf(host,len+1,p);

			if(*q==':'){
					for(r=++q,len=0;*r!='/'&&*r!=' ';++r,++len);
					snprintf(port,len+1,q);
			}
			else if(*q=='/')r=q;
			else return NULL;

			for(s=r,len=0;*s!=' '&&*s!='\n';++s,++len);
			strncat(request,r,len);
			strcat(request," HTTP/1.0\r\n");
			strcat(request,"Host: ");
			strcat(request,host);
			strcat(request,"\r\n");
			strcat(request,user_agent_hdr);
			strcat(request,"Connection: close\r\n");
			strcat(request,"Proxy-Connection: close\r\n\r\n");
			
			puts(request);

			size_t n;
			rio_t rios;
			clientfd=open_clientfd(host,port);
			Rio_readinitb(&rios,clientfd);
			Rio_writen(clientfd,request,strlen(request));
			while((n=Rio_readlineb(&rios,buf,MAXLINE))!=0)
					Rio_writen(connfd,buf,n);
			close(clientfd);
		}
		close(connfd);
		return NULL;
}

int main(int argc, char **argv)
{
    printf("%s", user_agent_hdr);

		char *port=argv[1];
		struct sockaddr_in clientaddr;
		int clientlen=sizeof(struct sockaddr_storage);
		pthread_t tid;
		
		int listenfd=open_listenfd(port);
		while(1){
				int connfdp=accept(listenfd,(SA *)&clientaddr,&clientlen);
				pthread_create(&tid,NULL,thread,(void *)connfdp);
		}
		close(listenfd);
    return 0;
}
