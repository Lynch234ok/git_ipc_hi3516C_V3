
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "upgrade_cgi.h"

//static void *upgradeCgi_acceptRequest(void *thread_arg);
static void upgradeCgi_acceptRequest(int arg);
static int upgradeCgi_getLine(int, char *, int);
static void upgradeCgi_ackResponse(int);
static int upgradeCgi_buildlink(int *);


#define ISspace(x) isspace((int)(x))

static int gs_currentUpgradeRate;

static void upgradeCgi_acceptRequest(int arg)
{
    // 子线程处理http请求
    char buf[1024] = {0};
    int numchars = 0;
    char method[255] = {0};
    char url[255] = {0};

    size_t i = 0, j = 0;

    int client = 0;
    client = (int) arg;

    //pthread_detach(pthread_self());
	//获取每行数据
    numchars = upgradeCgi_getLine(client, buf, sizeof (buf));
    i = 0;
    j = 0;
    //不是空格，有method参数
    while (!ISspace(buf[j]) && (i < sizeof (method) - 1))
    {
        method[i] = buf[j];
        i++;
        j++;
    }
    method[i] = '\0';

    i = 0;
    while (ISspace(buf[j]) && (j < sizeof (buf))){
        j++;
    }
    while (!ISspace(buf[j]) && (i < sizeof (url) - 1) && (j < sizeof (buf))) {
        url[i] = buf[j];
        i++;
        j++;
    }
    url[i] = '\0';

    while ((numchars > 0) && strcmp("\n", buf)) /* read & discard headers */
   	{
            numchars = upgradeCgi_getLine(client, buf, sizeof (buf));
   	}

	if (0 >= strcasecmp("/cgi-bin/upgrade_rate.cgi?cmd=upgrade_rate", url) &&
		0 == strcasecmp(method, "GET"))
	{
        upgradeCgi_ackResponse(client);

    }else{

    }

	if(client > 0){
    	close(client);
	}

	//pthread_exit(NULL);

}

static int upgradeCgi_getLine(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n')) {
        n = recv(sock, &c, 1, 0);
        /* DEBUG printf("%02X\n", c); */
        if (n > 0) {
            if (c == '\r') {
                n = recv(sock, &c, 1, MSG_PEEK);
                /* DEBUG printf("%02X\n", c); */
                if ((n > 0) && (c == '\n'))
                    recv(sock, &c, 1, 0);
                else
                    c = '\n';
            }
            buf[i] = c;
            i++;
        } else
            c = '\n';
    }
    buf[i] = '\0';

    return (i);
}

static void upgradeCgi_ackResponse(int client)
{
    //返回http 404协议
    char buf[1024] = {0};
	static int upgradeRate;
    //先发送http协议头
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    //再发送serverName
    sprintf(buf, "Server: nginx");
    send(client, buf, strlen(buf), 0);
    //再发送Content-Type
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    //发送换行符
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    //发送html主体内容

    sprintf(buf, "%d", gs_currentUpgradeRate);
    send(client, buf, strlen(buf), 0);

}

static int upgradeCgi_buildlink(int *port)
{
	int httpd = 0;
    struct sockaddr_in name;

    httpd = socket(PF_INET, SOCK_STREAM, 0);

	if (httpd == -1){
    	    printf("socket failed");
			return -1;
    }

    memset(&name, 0, sizeof (name));
    name.sin_family = AF_INET;
    name.sin_port = htons(*port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
	setsockopt( httpd, SOL_SOCKET, SO_REUSEADDR, &name, sizeof(name));

    if (bind(httpd, (struct sockaddr *) &name, sizeof (name)) < 0){
    	    printf("bind error");
			return -1;
    }

    if (*port == 0) /* if dynamically allocating a port */ {

		socklen_t namelen = sizeof (name);

		if (getsockname(httpd, (struct sockaddr *) &name, &namelen) == -1){
        	    printf("getsockname error");
				return -1;
        }

        *port = ntohs(name.sin_port);
    }

    if (listen(httpd, 5) < 0){
    	    printf("listen error");
			return -1;
    }

    return (httpd);
}


static void *upgradeCgi_GetUpgradeRate(void *arg)
{
	int  port = 80; // 绑定HTTP的80端口
	int server_sock = -1;
 	int client_sock = -1;

    struct sockaddr_in client_name;
    socklen_t client_name_len = sizeof (client_name);
    pthread_t newthread;

    pthread_detach(pthread_self());

	//while (!Thread->terminate(Thread))
	{
        //建立socket，监听端口
        server_sock = upgradeCgi_buildlink(&port);

        printf("httpd running on port %d\n", port);
    	if(server_sock < 0)
    	{
    		goto Error;
    	}

       //while (!Thread->terminate(Thread))
        while(1)
       	{
            client_sock = accept(server_sock,(struct sockaddr *) &client_name, &client_name_len);

    		if (client_sock == -1){
                printf("accept");
    			goto Error;
    		}

            upgradeCgi_acceptRequest(client_sock);
            //子线程处理http 请求
            /*if (pthread_create(&newthread, NULL, upgradeCgi_acceptRequest, (void*) client_sock) != 0)
        	{
            	perror("pthread_create");
				goto Error;
        	}*/
        }
    }
Error:
	if(server_sock > 0)
	{
        close(server_sock);
    }
}

int UPGRADE_CGI_GetUpgradeRate()
{
    pthread_t tid = (pthread_t)NULL;

    if(pthread_create(&tid, NULL, upgradeCgi_GetUpgradeRate, NULL) != 0)
    {
        perror("pthread_create");
        return -1;
    }

}

void UPGRADE_CGI_updateRate(const int upgradeRate)
{
    gs_currentUpgradeRate = upgradeRate;

}

//#define UPGRADE_CGI_TEST
#ifdef UPGRADE_CGI_TEST
int main()
{
    UPGRADE_CGI_GetUpgradeRate();
    while(1);

}

#endif /* endif UPGRADE_CGI_TEST */

