#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

int connetion=0;
int client_socket[4];


void broadcasting(char* buf,int res)
{
    for(int i=0; i < connetion; i++)
        send(client_socket[i],buf,res+1,0);
}


void* loop(void* data)
{
    int client_socket=*((int*)data);
    char buf[256];
       while(1)
       {
           ssize_t res=recv(client_socket,buf,255,0);
           if(res ==-1 || res ==0)
           {
               cout<<"receive fail"<<endl;
               exit(0);
               break;
           }

           buf[res]='\0';
           cout<<buf<<endl;
           buf[res]='!';    //add ! at the end of buffer

           if( strstr(buf,"@broadcast") != NULL )
           {
               broadcasting(buf,res);
               continue;
           }

           if(res > 0)
           {
               if( send(client_socket,buf,res+1,0) == -1)
               {
                   cout<<"send error"<<endl;
                   cout<<strerror(errno)<<endl;
                   break;
               }
           }
       }
}

int main(int argc, char *argv[])
{
    int sd;
    sd=socket(AF_INET,SOCK_STREAM,0);
    if (sd == -1)
    {
        cout<<strerror(errno)<<endl;
        exit(0);
    }

    sockaddr_in server;
    memset(&server,0,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(5555);
    server.sin_addr.s_addr=htonl(INADDR_ANY);

    if( bind( sd,(struct sockaddr*)&server ,sizeof(server) ) == -1  )
    {
        cout<<"binding"<<endl;
        cout<<strerror(errno)<<endl;
        exit(0);
    }
    //---//


    int client_addr_size;
    sockaddr_in client_addr[4];
    client_addr_size = sizeof(client_addr[0]);

    pthread_t p_thread[4];
    int thr_id[4];


    while(1)
    {
        if(connetion >3 )
        {
            cout<<"Too much thread!"<<endl;
            exit(1);
        }
        if(listen(sd,5) == -1)
        {
            cout<<"listening"<<endl;
            cout<<strerror(errno)<<endl;
            exit(0);
        }
        client_socket[connetion]=accept(sd,(sockaddr*)&client_addr,(socklen_t*)&client_addr_size);
        if(client_socket[connetion] == -1)
        {
                cout<<"client conneting fail"<<endl;
                cout<<strerror(errno)<<endl;
                exit(0);
        }
        thr_id[connetion] = pthread_create(&p_thread[connetion],NULL,loop,(void*)&client_socket[connetion]);
        if(thr_id[connetion] < 0)
        {
           cout<<"thread error"<<endl;
           cout<<strerror(errno)<<endl;
           exit(1);
        }
        else
            connetion++;
    }

    pthread_detach(p_thread[0]);
    pthread_detach(p_thread[1]);
    pthread_detach(p_thread[2]);
    pthread_detach(p_thread[3]);
    close(sd);
    return 0;
}
