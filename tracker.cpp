// ./tracker port_no
#include<bits/stdc++.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <pthread.h>
using namespace std;
std::map<std::string , std::string> seeder;
map<string ,string> user_info;
map<string,set<string> >group_info;
map<string,set<string> >group_file;
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
//fun used for parsing 
vector<string>ArrayOfString(string s,char del)
{
  stringstream ss(s);
  vector<string>a;
  string temp;
  while(getline(ss,temp,del))
  {
    a.push_back(temp);
  }
  return a;
}
// Thread function to receive data from client
// Args: client_socket_ref
void * receiveDataFunc(void * arg)
{
    int clientSocketRef = *((int *)arg);
	//char serverMessage[512] = "message received";
    char buffer[5120] = "";
    int status;
    while(1)
    {
		// recv returns the length of string read
        status = recv(clientSocketRef, &buffer, sizeof(buffer), 0);

		// If length is 0, that means the client has disconnected
        if (status == 0)
        {
            printf("Client %d disconnected!\n",clientSocketRef);
			close(clientSocketRef);
            break;
        }
        printf("Receive Status sis %d\n", status);
        printf("Client %d: %s\n",clientSocketRef, buffer);
        string r=buffer;
        //cout<<r<<endl;
        //printf("%s\n",r );
        vector<string> parse=ArrayOfString(r,';');
		string request=parse[0];
		//cout<<request<<endl;
		bzero(buffer,5120);

		if(request=="create_user")
		{
			string key=parse[1];
			string val=parse[2];
			if(user_info.find(key)!=user_info.end())
			{
				char ans[5120]="0";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
			else
			{
				char ans[5120]="1";
				user_info[key]=val;
				send(clientSocketRef,ans,sizeof(ans),0);
			}
		}
		if(request=="login")
		{
			string key=parse[1];
			string val=parse[2];
			if(user_info.find(key)!=user_info.end() && user_info[key]==val)
			{
				char ans[5120]="1";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
			else
			{
				char ans[5120]="0";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
		}
		if(request=="logout")
		{
			char ans[5120]="1";
			send(clientSocketRef,ans,sizeof(ans),0);
		}
		if(request=="create_group")
		{
			if(group_info.find(parse[1])!=group_info.end())
			{
				char ans [5120]="0";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
			else
			{
				char ans[5120]="1";
				group_info[parse[1]].insert("try");
				send(clientSocketRef,ans,sizeof(ans),0);
			}
		}
		if(request=="join_group")
		{
			string group=parse[1];
			string user=parse[2];
			if(group_info.find(group)!=group_info.end())
			{
				char ans[5120]="1";
				group_info[group].insert(user);
				send(clientSocketRef,ans,sizeof(ans),0);
			}
			else
			{
				char ans[5120]="0";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
		}
		if(request=="leave_group")
		{
			string group=parse[1];
			string user=parse[2];
			if(group_info.find(group)!=group_info.end()  &&  group_info[group].find(user)!=group_info[group].end())
			{
				char ans[5120]="1";
				group_info[group].erase(user);
				send(clientSocketRef,ans,sizeof(ans),0);
			}
			else
			{
				char ans[5120]="0";
				send(clientSocketRef,ans,sizeof(ans),0);
			}
		}
		if(request=="list_group")
		{
			string ans="";
			for(auto &i:group_info)
			{
				ans+=i.first;
				ans+=";";
			}
			send(clientSocketRef,ans.c_str(),ans.size(),0);
		}
		
		//printf("request:%s\n",request);
		if(request=="upload_file")
		{
			string key=parse[1];//file name
			string val=parse[2];//port of sender
			string group=parse[3];//group_id
			if(group_info.find(group)!=group_info.end())
			{
				seeder[key]=val;
			//cout<<seeder[parse[1]];
			//printf("%s\n",seeder[parse[1]]);
			group_file[group].insert(key);
			char serverMessage[5120]="upload done";
			send(clientSocketRef,serverMessage,sizeof(serverMessage),0);
			}
			else
			{
				char serverMessage[5120]="group doesnot exist";
				send(clientSocketRef,serverMessage,sizeof(serverMessage),0);
			} 
		}
		if(request=="list_files")
		{
			string group=parse[1];
			set<string> s=group_file[group];
			set<string>:: iterator it;
			string ans="";
			for(it=s.begin();it!=s.end();it++)
			{
      			 if(it==s.begin())
       			{
       				ans+=(*it);
       			}
       			else
       			{
       				ans+=";";
       				ans+=(*it);
      			 }
			}
			cout<<ans<<endl;
			send(clientSocketRef,ans.c_str(),ans.size(),0);
		}
		if(request=="download_file")
		{
			string group=parse[1];
			string file=parse[2];
			if(group_info.find(group)!=group_info.end())
			{
			string port=seeder[file];
			send(clientSocketRef,port.c_str(),port.size(),0);
			}
			else
			{
				string message="can't download";
				send(clientSocketRef,message.c_str(),message.size(),0);
			}	
		}

        //fgets(serverMessage,512,stdin);
        //printf("%s",serverMessage);
        //std::cout<<serverMessage;
		//send(clientSocketRef, serverMessage, sizeof(serverMessage), 0);
		bzero(buffer,5120);
    }
    return NULL;
}


// Thread function that gets called to listen to connections
// Args: server_socket_ref
void * listenToConnections(void * arg)
{
	int serverSocket = *((int *)arg);
	int client_socket_ref;
	pthread_t * receiveThread;
	printf("In connection listen\n");
	while(1)
	{
		listen(serverSocket, 10);
		// If needed, we can fetch the client address as well
		client_socket_ref = accept(serverSocket, NULL, NULL);
        if (client_socket_ref == -1)
        {
            printf("Error connecting to client!\n");
            continue;
        }
		receiveThread = (pthread_t*)malloc(sizeof(pthread_t));

		// Spwans a new thread that is responsible for receiving incoming data from this newly connected socket
		pthread_create(receiveThread, NULL, receiveDataFunc, &client_socket_ref);
	}
}

int main(int argc,char*argv[])
{
	//user should provide port number 
	if(argc<2)
	{
		fprintf(stderr,"port number not provided ");
		exit(1);
	}

	char input;
	int sockfd,newsockfd,portno,n;
	char buffer[5120];
	struct sockaddr_in serv_addr,cli_addr;
	socklen_t clilen;
	pthread_t listenerThread;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		error("Error opening socket");
	}
	bzero((char *)&serv_addr,sizeof(serv_addr));
	portno=atoi(argv[1]);

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(portno);
	
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		error("Binding failed");

	printf("Over here!\n");
    pthread_create(&listenerThread, NULL, listenToConnections, &sockfd);
    // pressing "q" will lead to exit
	while((input = getchar()) != 'q');


	close(sockfd);

	return 0;

}