//argument passes while running peer : IP(127.0.0.1) portofpeer portoftracker
#include<stdio.h>
#include<bits/stdc++.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<pthread.h>
using namespace std;
bool is_loggedin=false;
string user_id;
string group_id;
void error(const char *msg)
{
	perror(msg);
	exit(1);
}
//fun used for parsing purpose
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
//act as a server for the peer 
void * receiveDataFunc(void * arg)
{
    int clientSocketRef = *((int *)arg);
	char serverMessage[5120] = "Message received by server!\n";
    char buffer[5120] = "";
    int status;
  //   while(1){
		// // recv returns the length of string read
  //       status = recv(clientSocketRef, &buffer, sizeof(buffer), 0);

		// // If length is 0, that means the client has disconnected
  //       if (status == 0){
  //           printf("Client %d disconnected!\n",clientSocketRef);
		// 	close(clientSocketRef);
  //           break;
  //       }
  //       printf("Receive Status sis %d\n", status);
  //       printf("Client %d: %s\n",clientSocketRef, buffer);
		// send(clientSocketRef, serverMessage, sizeof(serverMessage), 0);
		// bzero(buffer,5120);
  //   }
     while(1)
     {
     	status=recv(clientSocketRef,&buffer,sizeof(buffer),0);
     	if (status == 0)
     	{
            printf("Client %d disconnected!\n",clientSocketRef);
			close(clientSocketRef);
            break;
        }
        printf("Client %d: %s\n",clientSocketRef,buffer);
        string filename=buffer;
        bzero(buffer,5120);
	FILE *f;
	int words=1;
	char c;
	f=fopen(filename.c_str(),"r");
	while((c=getc(f))!=EOF)
	{
		fscanf(f,"%s",buffer);
		if(isspace(c)||c=='\t')
		{
			words++;
		}
	}
	printf("%d\n",words);
	int res = write(clientSocketRef,&words,sizeof(int));
    printf("Response is %d\n", res);
	rewind(f);

	char ch;
	while(ch!=EOF)
	{
		fscanf(f,"%s",buffer);
		//int length=strlen(buffer);
		//buffer[length]=' ';
		//read(f,buffer,5120);
		res = write(clientSocketRef,buffer,strlen(buffer));
		printf("%s\n", buffer);
		bzero(buffer,5120);
        printf("Response is %d\n", res);
        recv(clientSocketRef,&buffer,sizeof(buffer),0);
        bzero(buffer,5120);
		ch=fgetc(f);
	}
  }
    return NULL;
}
//fun used for listening to connection
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
	int sockfd,newsockfd,portno,n,sockfd1,sockfd2,portno1,port_no2;
    char input;
	char buffer[5120];
	struct sockaddr_in serv_addr,cli_addr;
	socklen_t clilen;
	pthread_t listenerThread;
	struct hostent *server;
	struct hostent *server1;
	if(argc<3)
	{
		fprintf(stderr,"usage %s hostname port\n " ,argv[0]);
		exit(1);
	}
	//portno=atoi(argv[2]);
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		error("Error opening socket");
	}
	bzero((char *)&serv_addr,sizeof(serv_addr));
	portno=atoi(argv[2]);

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_port=htons(portno);
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
		error("Binding failed");

	pthread_create(&listenerThread, NULL, listenToConnections, &sockfd);
     
	if(argc==4)
	{
		portno1=atoi(argv[3]);
		sockfd1=socket(AF_INET,SOCK_STREAM,0);
		if(sockfd1<0)
		{
		error("Error opening socket");
		}
		server=gethostbyname(argv[1]);
		if (server==NULL)
		{
			fprintf(stderr,"error, no such host");

		}
		bzero((char*)&serv_addr,sizeof(serv_addr));
		serv_addr.sin_family=AF_INET;
		bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
		serv_addr.sin_port=htons(portno1);
		if(connect(sockfd1,(struct sockaddr *)&serv_addr,sizeof( serv_addr))<0)
			error("conn fail");
		std::string request;
		while(1)
		{
			getline(cin,request);
			//cout<<request;
			vector<string>parse=ArrayOfString(request,' ');
			string command=parse[0];
			//cout<<command;

			// create_user user_name password
			if(command=="create_user")
			{
				if(parse.size()!=3)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="create_user";
					token+=";";
					token+=parse[1];//user_name
					token+=";";
			        token+=parse[2];//password
			        send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")cout<<"user created"<<endl;
			        else cout<<"already exist"<<endl;
				}

			}
			// login user_name password
			if(command=="login")
			{
				if(parse.size()!=3)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="login";
					token+=";";
					token+=parse[1];//user_name
					token+=";";
			        token+=parse[2];//password
			        send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")
			        {
			        	is_loggedin=true;
			        	user_id=parse[1];
			         	cout<<"login successful"<<endl;
			     	}
			        else cout<<"login fails"<<endl;
				}
			}
			//logout
			if(command=="logout")
			{
				if(parse.size()!=1)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="logout";
			        send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")
			        {
			        	is_loggedin=false;
			        	cout<<"logout successful"<<endl; 
			        }
				}
			}
			//create_group group_id
			if(command=="create_group")
			{
				if(parse.size()!=2)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="create_group";
					token+=";";
					token+=parse[1];//group_id
					send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")
			        {
			        	cout<<"group created successful"<<endl;
			        }
			        else
			        {
			        	cout<<"group already present"<<endl;
			        	cout<<"enter once again"<<endl;
			        }
				}
			}
			// join_group group_id
			if(command=="join_group")
			{
				if(parse.size()!=2)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="join_group";
					token+=";";
					token+=parse[1];//group_id
					token+=";";
					token+=user_id;//user_name
					send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")
			        {
			        	group_id=parse[1];
			        	cout<<"joined to group"<<endl;
			        }
			        else
			        {
			        	cout<<"group doesnot exist"<<endl;
			        	cout<<"enter valid group and try again"<<endl;
			        }
				}
			}
			// leave_group group_id
			if(command=="leave_group")
			{
				if(parse.size()!=2)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="leave_group";
					token+=";";
					token+=parse[1];//group_id
					token+=";";
					token+=user_id;
					send(sockfd1,token.c_str(),token.size(),0);
			        bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string status=buffer;
			        cout<<endl;
			        if(status=="1")
			        {
			        	group_id="";
			        	cout<<"group exited "<<endl;
			        }
			        else
			        {
			        	cout<<"group doenot exist or user is not in group"<<endl; 
			        }
				}
			}
			// list_group
			if(command=="list_group")
			{
				if(parse.size()!=1)
				{
					cout<<"invalid arg"<<endl;
				}
				else
				{
					string token="list_group";
					send(sockfd1,token.c_str(),token.size(),0);
					bzero(buffer,5120);
			        recv(sockfd1,buffer,sizeof(buffer),0);
			        string buff=buffer;
			        vector<string>list=ArrayOfString(buff,';');
			        for(auto i:list)
			        {
			        	cout<<i<<endl;
			        }
				}
			}
			//upload_file file_name portofpeer group_id
			if(command=="upload_file")
			{
			  if(!is_loggedin)
			  {
			  	cout<<"user is not logged in"<<endl;
			  }
			  else
			  {
			  	if(parse.size()!=4)
			  	{
			  	std::cout<<"invalid arg"<<endl;
			  	}
			  	else
			  	{
			  	string token="upload_file";
			  	token+=";";
			  	token+=parse[1];//file_name
			  	token+=";";
			  	token+=parse[2];//portno of himself so that tracker get to know about it
			  	token+=";";
			  	token+=parse[3];//group_id
			  	send(sockfd1,token.c_str(),token.size(),0);
			  	bzero(buffer,5120);
			  	recv(sockfd1,buffer,sizeof(buffer),0);
			  	string recv=buffer;
			  	cout<<recv<<endl;
			  	//std::cout<<"message received";
				}
			  }
		    }
		// list_files group_id
			if(command=="list_files")
			{
				if(!is_loggedin)
				{
			  	cout<<"user is not logged in"<<endl;
			    }
			   else
			   {
			  	if(parse.size()!=2)
			  	{
			  	std::cout<<"invalid arg"<<endl;
			  	}
			  	else
			  	{
			  		string token="list_files";
			  		token+=";";
			  		token+=parse[1];//group_id
			  		send(sockfd1,token.c_str(),token.size(),0);
			  		bzero(buffer,5120);
			  		recv(sockfd1,buffer,sizeof(buffer),0);
					string ans=buffer;
   					vector<string>arr=ArrayOfString(ans,';');
  					for(int i=0;i<arr.size();i++)
   					{
     					 cout<<arr[i]<<" ";
   					}
  					 cout<<endl;
			  	}
			  }
			}
			// download_file group_id file_name destination_path
			if(command=="download_file")
			{
				if(!is_loggedin)
				{
					cout<<"user is not logged in"<<endl;
				}
				else
				{
				if(parse.size()!=4)
				{
			  	std::cout<<"invalid arg"<<endl;
			    }
			   else
			   {
			   string file=parse[2];
			   string des=parse[3];
			   string token="download_file";
			   token+=";";
			   token+=parse[1];//group_id
			   token+=";";
			   token+=parse[2];//file
			   token+=";";
			   token+=parse[3];//destination
			   send(sockfd1,token.c_str(),token.size(),0);
			   bzero(buffer,5120);
			   recv(sockfd1,buffer,sizeof(buffer),0);
			   string portno2=buffer;
			   // if group_id is not present then can't download 
			   if(portno2!="can't download")
			   {
			   port_no2=atoi(portno2.c_str());
			   sockfd2=socket(AF_INET,SOCK_STREAM,0);
				if(sockfd2<0)
				{
				error("Error opening socket");
				}
				bzero((char*)&serv_addr,sizeof(serv_addr));
				serv_addr.sin_family=AF_INET;
				bcopy((char*)server->h_addr,(char*)&serv_addr.sin_addr.s_addr,server->h_length);
				serv_addr.sin_port=htons(port_no2);
				if(connect(sockfd2,(struct sockaddr *)&serv_addr,sizeof( serv_addr))<0)
				error("conn fail");

				send(sockfd2,file.c_str(),file.size(),0);

				bzero(buffer,5120);
				FILE *fp;
   				printf("file is receiving\n");
				int ch=0;
				fp=fopen(des.c_str(),"a");
				int words;
				read(sockfd2,&words,sizeof(int));
    			printf("Received words\n");
    			char serverMessage[5120] = "Message received by server!\n";
    			//string sendmessage="word received";
    			cout<<words<<endl;
				while(ch!=words)
				{
					recv(sockfd2,buffer,sizeof(buffer),0);
					//printf("%s", buffer);
					cout<<buffer<<endl;
					fprintf(fp , "%s " ,  buffer);
					send(sockfd2,serverMessage,sizeof(serverMessage),0);
					bzero(buffer,5120);
					fflush(fp);
					ch++;
				}
	// *** Flush output stream ***
				fflush(fp);
				fclose(fp);
 				printf("file has been received \n");
			  }
			}
		  }
		}

			// bzero(buffer,512);
			// fgets(buffer,512,stdin);
			// n=write(sockfd1,buffer,strlen(buffer));
			// if(n<0)
			// 	error("error on write");
			// bzero(buffer,512);
			// n=read(sockfd1,buffer,512);
			// if(n<0)
			// 	error("read fail");
			// printf("server: %s",buffer);

			int i=strncmp("bye",buffer,3);
			if(i==0)
				break;
		}
		 close(sockfd1);
	}
		// exit on pressing "q"
	  while((input = getchar()) != 'q');
      close(sockfd);
      return 0;
}