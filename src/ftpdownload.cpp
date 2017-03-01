#include "ftpdownload.h"
//#include <tchar.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;

#define MSGSIZE        510
#define MSGSIZE2        512

struct hostent * server_hostent;
//#ifdef WIN32
struct sockaddr_in ftp_server;
struct sockaddr_in local_host;
//#else
//struct sockaddr_in ftp_server;
//struct sockaddr_in local_host;
//#endif

CFtpDownload::CFtpDownload()
{
#ifdef WIN32
	WSADATA     wsaData;
	WSAStartup(0x0202, &wsaData);
#endif
}

CFtpDownload::~CFtpDownload()
{
#ifdef WIN32
	WSACleanup();
#endif
}

bool CFtpDownload::AnalyseURL(string szurl)
{
	m_strServer="";
	m_strObject="";
	m_Port=0;
	int n =szurl.find("://");
	if(n<0)
		return false;
	string sztemp=szurl.substr(n+3);	
	//解析ftp服务器/端口/用户名/密码
	n =sztemp.rfind('@');
	//@以后的部分。
	string ss =sztemp.substr(n+1);
	int m = ss.find('/');
	m_strObject = ss.substr(m+1);
	ss = ss.substr(0,m);
	m = ss.rfind(':');
	if(m>=0)
	{
		m_strServer = ss.substr(0,m);
#ifdef WIN32
		m_Port = (USHORT)_ttoi(ss.substr(m+1).c_str());
#else
		m_Port = atoi(ss.substr(m+1).c_str());
#endif
	}
	else
	{
		m_strServer =ss;
		m_Port =21;
	}
	//@和http://之间的符号
	sztemp = sztemp.substr(0,n);
	m_strUser = sztemp.substr(0,sztemp.find(':'));
	m_strPwd = sztemp.substr(sztemp.find(':')+1);

	return true;
}

bool CFtpDownload::AnalyseReceive(string strRead)
{
	if(strRead.empty()) return false;
	int n = strRead.find(" ");
	string sztemp = strRead.substr(0,n);
#ifdef WIN32
	DWORD code = (DWORD)_ttoi(sztemp.c_str());
#else
	int code = atoi(sztemp.c_str());
#endif
	if(code < 400)return true;
	return false;
}

bool CFtpDownload::begin(string szurl, string szsaveas, string& sztorrentname,string szuser, string szpwd,int ftp_mode)
{
#ifdef WIN32
	sock_control = NULL;
#else
	sock_control = 0;
#endif	
	m_ftp_mode=ftp_mode;
	try
	{
		m_strSavePath = szsaveas;
		//if (GetFileAttributes(m_strSavePath.c_str()) == FILE_ATTRIBUTE_DIRECTORY)	
#ifdef WIN32		
		CreateDirectory(m_strSavePath.c_str(),NULL);
#else
		CreateDir(m_strSavePath.c_str());
#endif
		//解析url，得到ftp之server,port,user和pwd信息。
		m_strUser = szuser;
		m_strPwd = szpwd;
		printf("解析ftp url是否合法？\r\n");
		if(!AnalyseURL(szurl))
		{
			throw( "URL错误，请确认地址正确\r\n");
		}//调用分析地址函数
		printf("解析ftp url合法！\r\n");
		int err;
		printf("m_strServer:%s\nm_Port:%d\n",m_strServer.c_str(),m_Port);
		err = fill_host_addr((char*)m_strServer.c_str(), &ftp_server, m_Port);
		if(err == 254)
			throw("Invalid port!\r\n");
		if(err == 253)
			throw("Invalid server address!\r\n");
		sock_control = xconnect(&ftp_server,0);
		if((err =  ftp_get_reply(sock_control)) != 220)
			throw("Connect error!\n");
		printf("登录ftp server\n");
		//登录ftp server。根据user、pwd信息。
		//do{
		printf("m_strUser:%s\nm_strPwd:%s\n",m_strUser.c_str(),m_strPwd.c_str());
		err=ftp_login((char*)m_strUser.c_str(),(char*)m_strPwd.c_str());
		//}while(err!=1);
		if(err!=1)throw("登录ftp server失败！\r\n");
		printf("登录ftp server成功！\n");
#ifdef WIN32		
		string sztemptorrentpath=m_strSavePath+"\\"+""+m_strObject;
		::DeleteFileA(sztemptorrentpath.c_str());
		//开始下载数据
		printf("开始下载数据\n");
		Sleep(10);
		ftp_get((char*)m_strObject.c_str(),(char*)m_strSavePath.c_str());
		printf("%s数据下载完毕！\n",m_strObject.c_str());
		Sleep(100);
		sztorrentname = m_strObject.substr(m_strObject.rfind('/')+1,m_strObject.length());
		if(closesocket(sock_control)==SOCKET_ERROR)printf("closesocket(sock_control) faild\n");
#else
		string sztemptorrentpath=m_strSavePath+"/"+""+m_strObject;
		remove(sztemptorrentpath.c_str());
		//开始下载数据
		printf("开始下载数据\n");
		sleep(5);//10
		ftp_get((char*)m_strObject.c_str(),(char*)m_strSavePath.c_str());
		printf("%s数据下载完毕！\n",m_strObject.c_str());
		sleep(10);//100
		sztorrentname = m_strObject.substr(m_strObject.rfind('/')+1,m_strObject.length());
		if(close(sock_control)==-1)printf("close(sock_control) faild\n");
#endif	
		return true;
	}
	catch(const char*e)
	{
#ifdef WIN32
		closesocket(sock_control);
#else
		close(sock_control);
#endif
		printf("ftp访问错误：错误信息：%s\r\n",e);
		if(atoi(e)==100)
			throw(e);
		return false;	
	}
	catch(...){
#ifdef WIN32
		int iret=WSAGetLastError();
		closesocket(sock_control);
		if(iret!=0)
			printf("ftp网络连接失败,code:%d\r\n",iret);
#else
		close(sock_control);
#endif
		return false;	
	}

}

const char* Ftpdownlaodfile(const char* szurl,const char* szsaveas, const char* szuser, const char* szpwd,int ftp_mode)
{
	CFtpDownload ftd;
	string sztorrentname;
	if(ftd.begin(szurl,szsaveas,sztorrentname,szuser,szpwd,ftp_mode))
	{
		char* pname= new char[sztorrentname.length()+1];
		strcpy(pname,sztorrentname.c_str());
		return pname;
	}	
	return "";
}

int CFtpDownload::ftp_send_cmd(const char *s1, const char *s2, int sock_fd)
{
	char send_buf[256];
	int send_err, len;
	if(s1) 
	{

		strcpy(send_buf,s1);
		if(s2)
		{	
			strcat(send_buf, s2);
			strcat(send_buf,"\r\n");
			len = strlen(send_buf);
			printf("%s\n",send_buf);
			send_err = Sends(sock_fd, send_buf, len, 0);
		}
		else 
		{
			strcat(send_buf,"\r\n");
			len = strlen(send_buf);
			printf("%s\n",send_buf);
			send_err = Sends(sock_fd, send_buf, len, 0);
		}
	}

	if(send_err < 0)
	{
#ifdef WIN32
		int iret=WSAGetLastError();
		if(iret!=0)
			printf("send() error!\n,code:%d\r\n",iret);
#endif
	}
	return send_err;
}

int CFtpDownload::ftp_get_reply2(int sock_fd,int flags)
{
	//	int nret = ftp_get_reply(sock_fd);
	/*	if(flags==nret)
	return flags;
	return 0;*/
	int i=0;
	while(1)
	{
		int nret = ftp_get_reply(sock_fd);
		if(flags==nret)
			return flags;
		else if(nret>=400)
			return 0;
		else if(++i>5)
			return 0;
	}
}

int CFtpDownload::ftp_get_reply(int sock_fd)
{
	printf("accept socket fd :%d\n",sock_fd);
	static int count=0;
	char rcv_buf[MSGSIZE2];
	count=Recvs(sock_fd, rcv_buf, MSGSIZE,0);
	printf("%s\n",rcv_buf);
	if(count > 0)
		return atoi(rcv_buf);
	return 0;

}

int CFtpDownload::Sends(SOCKET s,char * buf,int len,int flags)
{
	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(s, &writefds);
	timeval tv={2,0};
	printf("SEND:\n socket fd:%d\n buf:%s",s,buf);
	int ret = select(s+1,0,&writefds,0,&tv);
	if(ret<=0)
	{
#ifdef WIN32
		int iret=WSAGetLastError();
		if(iret!=0)
			printf("connect,ftp网络连接失败,code:%d\r\n",iret);
#endif
		return ret;
	}
	return send(s,buf,len,flags);
}

int CFtpDownload::Recvs(SOCKET s,char * buf,int len,int flags)
{

	fd_set readfds;
	int i=0;
	int ret;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);
		timeval tv={10,0};
		ret = select(s+1,&readfds,0,0,&tv);
		if(FD_ISSET(s,&readfds))
		{
			return recv(s,buf,len,flags);
		}
		i++;
		if(i>3)return ret;
	}
	return ret;
}

SOCKET CFtpDownload::Accepts(SOCKET s,struct sockaddr * addr,int * addrlen)
{
	fd_set readfds,exceptfds;
	timeval tv={2,0};
	int i=0;
	while(1)
	{
		FD_ZERO(&readfds);
		FD_SET(s, &readfds);

		FD_ZERO(&exceptfds);
		FD_SET(s, &exceptfds);
		select(s+1,&readfds,0,&exceptfds,&tv);

		if(FD_ISSET(s,&readfds))
		{
			printf("Accepts call select \n");
			break;
		}
		if(FD_ISSET(s,&exceptfds))
		{
			printf("Accepts faild,FD_ISSET(s,&exceptfds)\n");
			throw("Accepts faild,FD_ISSET(s,&exceptfds)\n");
		}
		i++;
		if(i>5)throw("Accepts faild\n");
	}	
	return accept(s,addr,(socklen_t*)addrlen);
}

int  CFtpDownload::Listens(SOCKET s,int backlog)
{
	return 0;
}

int CFtpDownload::xconnect(struct sockaddr_in *saddr ,int type)
{
	struct timeval outtime;
	int set;
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s < 0)
		throw("creat socket error!\r\n");
#ifdef	 WIN32		
	struct timeval tmout={0,200};
	//DWORD opt=1;
	set = setsockopt(s, SOL_SOCKET,SO_RCVTIMEO, (char*)&tmout,sizeof(tmout));
	if(set !=0)
	{
		char cerr[50];
		_snprintf(cerr,sizeof(cerr),"recv,set socket %s errno:%d\n",strerror(errno),errno);
		throw(cerr);
	}
	set = setsockopt(s, SOL_SOCKET,SO_SNDTIMEO, (char*)&tmout,sizeof(tmout));
	if(set !=0)
	{
		char cerr[50];
		_snprintf(cerr,sizeof(cerr),"send,set socket %s errno:%d\n",strerror(errno),errno);
		throw(cerr);
	}

#else
	set = setsockopt(s, SOL_SOCKET,SO_RCVTIMEO, (char*)&outtime,sizeof(outtime));
	if(set !=0)
	{
		char cerr[50];
		snprintf(cerr,sizeof(cerr),"recv,set socket %s errno:%d\n",strerror(errno),errno);
		throw(cerr);
	}
	set = setsockopt(s, SOL_SOCKET,SO_SNDTIMEO, (char*)&outtime,sizeof(outtime));
	if(set !=0)
	{
		char cerr[50];
		snprintf(cerr,sizeof(cerr),"send,set socket %s errno:%d\n",strerror(errno),errno);
		throw(cerr);
	}
#endif
	//connect to the server
	connect(s,(struct sockaddr *)saddr,sizeof(struct sockaddr_in));
	printf("connect end \n");
#ifdef WIN32
	unsigned long p = 1;
	ioctlsocket(s,FIONBIO,&p);
#else
	int flags = 0; 
	flags = fcntl(s, F_GETFL, 0);
	fcntl(s, F_SETFL, flags | O_NONBLOCK);   //设置为非阻塞模式
#endif
	if(type == 1)
	{
		outtime.tv_sec = 1;
		outtime.tv_usec = 0;
	}
	else
	{
		outtime.tv_sec = 1;
		outtime.tv_usec = 0;
	}
	/*	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(s, &readfds);
	int ret = select(0,&readfds,0,0,&outtime);
	if(FD_ISSET(s,readfds))*/
	/*if(ret<=0)
	{
	int iret=WSAGetLastError();
	if(iret!=0)
	printf("connect,ftp网络连接失败,code:%d\r\n",iret);
	char cerr[50];
	_snprintf(cerr,sizeof(cerr),"Can't connect to server %s, port %d\n",\
	inet_ntoa(saddr->sin_addr),ntohs(ftp_server.sin_port));
	throw(cerr);
	}*/
	return s;
}

int CFtpDownload::fill_host_addr(char * host_ip_addr, struct sockaddr_in * host, int port)
{
	if(port <= 0 || port > 65535) 
		return 254;
#ifdef WIN32
	ZeroMemory(host,sizeof(struct sockaddr_in));
#else
	memset(host,0,sizeof(struct sockaddr_in));
#endif
	host->sin_family = AF_INET;
	if((int)inet_addr(host_ip_addr) != -1)
	{
		host->sin_addr.s_addr = inet_addr(host_ip_addr);
	}
	else 
	{
		if((server_hostent = gethostbyname(host_ip_addr)) != 0)
		{
			memcpy(&host->sin_addr, server_hostent->h_addr,\
				sizeof(host->sin_addr));
		}	       
		else return 253;
	}
	host->sin_port = htons(port);
	return 1;
}

//登录ftp server
int CFtpDownload::ftp_login(char* user,char* pwd)
{
	try{
		int err;
		//char usr_cmd[1024];
		if(ftp_send_cmd("USER ", user, sock_control) < 0)
		{

			throw("Can not send message\r\n");
		}

		err = ftp_get_reply2(sock_control,331);

		if(err <=0 )
		{
			throw("User error!\n");
		}
		else
		{
			if(ftp_send_cmd("PASS ", pwd, sock_control) <= 0)throw("Can not send message \r\n");
			err = ftp_get_reply2(sock_control,230);
			if(err != 230)throw("Password error!\n");
		}

		return 1;
	}
	catch(const char*e)
	{
		printf(e);
		return 0;	
	}
}

void CFtpDownload::ftp_get(const char*src_file,const char*dst_file)
{
	FILE* local_file=NULL;
	int get_sock=-1;
	int new_sock=-1;
	try{
		int set;
		char rcv_buf[MSGSIZE2];
		char cfile[MSGSIZE2];
		get_sock = xconnect_ftpdata();
		printf("set ftp server command!\n");
		if(get_sock < 0)
			throw("socket error!\n");
		set = sizeof(local_host);
		strcpy(cfile,dst_file);
		ftp_send_cmd("SIZE ", src_file, sock_control);
		if(ftp_get_reply2(sock_control,213) != 213)
			throw("SIZE error!\n");
		printf("SIZE指令成功！\r\n");
		strcat(cfile,src_file);
		ftp_send_cmd("TYPE I", NULL, sock_control);
		if(ftp_get_reply2(sock_control,200)!=200)
			throw("TYPE I指令失败！\r\n");
		printf("TYPE I指令成功！\r\n");
		ftp_send_cmd("REST 0\r\n", NULL, sock_control);
		if(ftp_get_reply2(sock_control,350)!=350)
			throw("REST指令失败！\n");
		printf("REST指令成功！\n");
		ftp_send_cmd("RETR ", src_file, sock_control);	
		if(ftp_get_reply(sock_control)<=0)
			throw("RETR指令失败！\n");
		printf("RETR指令成功！\n");
		int count = 0;	
		if(m_ftp_mode>0)
		{
			printf("accept before\n");
			new_sock = Accepts(get_sock, (struct sockaddr *)&local_host,(int *)&set);
			printf("accept end\n");
#ifdef WIN32
			if(new_sock == INVALID_SOCKET||new_sock == SOCKET_ERROR)
			{
				printf("accept return:%s errno: %d\n", strerror(errno),errno);
				printf("Accepts faild!\n");
				throw("Accepts faild!\n");
			}
			if(new_sock ==INVALID_SOCKET)
				throw("Sorry, you can't use PORT mode. There is something wrong when the server connect to you.\n");
			printf("收到来自ftp 服务器的连接!\n");
			printf("cfile:%s\n",cfile);

			local_file = fopen(cfile, "wb");
			if(local_file < 0)
			{
				char cerr[50];
				_snprintf(cerr, sizeof(cerr), "creat local file %s error!\n",dst_file);
				throw(cerr);
			}
#else
			if(new_sock < 0)//wsr 2014.12.22
			{
				printf("Accepts faild!\n");
				throw("Accepts faild!\n");
			}
			printf("收到来自ftp 服务器的连接!\n");
			printf("cfile:%s\n",cfile);
			string filePath = cfile;
			int endPathIndex = filePath.rfind("/");
			string pathName;
			pathName.assign(filePath,0,endPathIndex);
			CreateDir(pathName.c_str()); 
			local_file = fopen(cfile, "wb");
			if(local_file < 0)
			{
				char cerr[50];
				snprintf(cerr, sizeof(cerr), "creat local file %s error!\n",dst_file);
				throw(cerr);
			}
#endif						
			fseek(local_file,0,SEEK_END);
			while(1)
			{	
				memset(rcv_buf,0,sizeof(rcv_buf));
				count = recv(new_sock, rcv_buf, sizeof(rcv_buf),0);
				if(count < 0)	
				{
					printf("未收到有效数据数据\n");
					throw("'100'\n");
					break;
				}
				else if(count==0)
				{
					printf("数据数据接收完毕\n");
					break;
				}
				else
				{
					int nw = fwrite(rcv_buf,sizeof(char),count,local_file);
					printf("write data to file：%d nw=,count=%d\n",nw,count);
					if(nw<count)
					{
						printf("fwrite<count,throw('100')\n");				
						throw("recv data write to file failed!");
					}
					printf("%s\n",rcv_buf);
				}
			}
			int nret =fflush(local_file);
			//char* pss=strerror(errno);
			if(nret!=0)
			{
				throw("100");
			}
#ifdef WIN32
			if(closesocket(get_sock)==SOCKET_ERROR)printf("closesocket(get_sock) faild!\n");
			if(closesocket(new_sock)==SOCKET_ERROR)printf("closesocket(new_sock) faild!\n");
#else
			if(close(get_sock)==-1)printf("closesocket(get_sock) faild!\n");
			if(close(new_sock)==-1)printf("closesocket(new_sock) faild!\n");
#endif
			if(fclose(local_file)!=0)printf("fclose faild!\n");
			//			ftp_get_reply(sock_control); 
		}
		else
		{
#ifdef WIN32			
			local_file = fopen(cfile, "wb");
			if(local_file < 0)
			{
				char cerr[50];

				_snprintf(cerr, sizeof(cerr), "creat local file %s error!\n",dst_file);
#else
			string filePath = cfile;
			int endPathIndex = filePath.rfind("/");
			string pathName;
			pathName.assign(filePath,0,endPathIndex);
			CreateDir(pathName.c_str());
			local_file = fopen(cfile, "wb");
			if(local_file < 0)
			{
				char cerr[50];
				snprintf(cerr, sizeof(cerr), "creat local file %s error!\n",dst_file);
#endif
				throw(cerr);
			}
			fseek(local_file,0,SEEK_END);
			//			ftp_get_reply(sock_control);
			while(1)
			{	
				memset(rcv_buf,0,sizeof(rcv_buf));
				count = recv(new_sock, rcv_buf, sizeof(rcv_buf),0);
				if(count < 0)	
				{
					printf("未收到有效数据数据\n");
					throw("'100'\n");
					break;
				}
				else if(count==0)
				{
					printf("数据数据接收完毕\n");
					break;
				}
				else
				{
					int nw = fwrite(rcv_buf,sizeof(char),count,local_file);
					printf("write data to file：%d nw=,count=%d\n",nw,count);
					if(nw<count)throw("100");
				}
				printf("%s\n",rcv_buf);

			}
			int nret =fflush(local_file);
			if(nret!=0)
			{
				throw("100");
			}
			//char* pss=strerror(errno);
			printf("data recv completed!\n");
			fclose(local_file);		
			//			ftp_get_reply(sock_control); 	
		}
	}
	catch(const char*e)
	{
		if(local_file!=NULL)
			fclose(local_file);
#ifdef WIN32			
		if(get_sock!=-1)
			closesocket(get_sock);
		if(new_sock!=-1)
			closesocket(new_sock);
#else
		if(get_sock!=-1)
			close(get_sock);
		if(new_sock!=-1)
			close(new_sock);
#endif			
		//		printf("ftp访问错误：错误信息：%s\r\n",e);
		throw(e);
	}
}

int CFtpDownload::rand_local_port()
{
	int local_port;
	srand((unsigned)time(NULL));
	local_port = rand() % 40000 + 1025;
	return local_port;
}

int CFtpDownload::xconnect_ftpdata()
{

	//以下获取本机的ip地址

	if(m_ftp_mode>0)
	{
		int client_port, get_sock, set;
		char cmd_buf[32];
		struct timeval outtime;
		struct sockaddr_in local;
		char local_ip[24];
		char *ip_1, *ip_2, *ip_3, *ip_4;
		int addr_len =  sizeof(struct sockaddr);
		client_port = rand_local_port();
		get_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(get_sock < 0)throw("socket() \r\n");
		//set outtime for the data socket
#ifdef	 WIN32

		opt=1;
		set = setsockopt(get_sock, SOL_SOCKET,SO_RCVTIMEO, (char*)&opt,sizeof(DWORD));
		if(set !=0)
		{
			char cerr[50];
			_snprintf(cerr, sizeof(cerr), "set socket %s errno:%d\n",strerror(errno),errno);
			throw(cerr);
		}
		set = setsockopt(get_sock, SOL_SOCKET,SO_REUSEADDR, \
			(char*)&opt,sizeof(DWORD));
		if(set !=0)
		{					
			char cerr[50];
			_snprintf(cerr, sizeof(cerr), "set socket %s errno:%d\n",strerror(errno),errno);
			throw(cerr);
		}

#else
		outtime.tv_sec = 7;
		outtime.tv_usec = 0;
		set = setsockopt(get_sock, SOL_SOCKET,SO_RCVTIMEO, (char*)&outtime,sizeof(outtime));
		if(set !=0)
		{
			char cerr[50];
			snprintf(cerr, sizeof(cerr), "set socket %s errno:%d\n",strerror(errno),errno);
			throw(cerr);
		}	
		set = setsockopt(get_sock, SOL_SOCKET,SO_REUSEADDR, \
			(char*)&outtime,sizeof(outtime));
		if(set !=0)
		{

			char cerr[50];
			snprintf(cerr, sizeof(cerr), "set socket %s errno:%d\n",strerror(errno),errno);
			throw(cerr);
		}
#endif

#ifdef WIN32
		ZeroMemory(&local_host,sizeof(local_host));
#else
		memset(&local_host,0,sizeof(local_host));
#endif
		local_host.sin_family = AF_INET;
		local_host.sin_port = htons(client_port);
		local_host.sin_addr.s_addr = htonl(INADDR_ANY);
#ifdef WIN32
		ZeroMemory(&local, sizeof(struct sockaddr));
#else
		memset(&local,0, sizeof(struct sockaddr));
#endif
		while(1)
		{
			set = bind(get_sock, (struct sockaddr *)&local_host, \
				sizeof(local_host));
			if(set != 0 && errno == 11)
			{
				client_port = rand_local_port();
				continue;
			}
			set = listen(get_sock, 1);
			if(set != 0 && errno == 11)throw("listen failed!\n");
			//get local host's ip
			if(getsockname(sock_control,(struct sockaddr*)&local,\
				(socklen_t*)&addr_len) < 0)throw("get local host's ip failed!\r\n");
#ifdef WIN32
			_snprintf(local_ip, sizeof(local_ip), inet_ntoa(local.sin_addr));
#else
			snprintf(local_ip, sizeof(local_ip), inet_ntoa(local.sin_addr));
#endif
			//change the format to the PORT command needs.
			local_ip[strlen(local_ip)]='\0';
			ip_1 = local_ip;
			ip_2 = strchr(local_ip, '.');
			*ip_2 = '\0';
			ip_2++;
			ip_3 = strchr(ip_2, '.');
			*ip_3 = '\0';
			ip_3++;
			ip_4 = strchr(ip_3, '.');
			*ip_4 = '\0';
			ip_4++;
#ifdef WIN32
			_snprintf(cmd_buf, sizeof(cmd_buf), "PORT %s,%s,%s,%s,%d,%d", \
				ip_1, ip_2, ip_3, ip_4,	client_port >> 8, client_port&0xff);
#else
			snprintf(cmd_buf, sizeof(cmd_buf), "PORT %s,%s,%s,%s,%d,%d", \
				ip_1, ip_2, ip_3, ip_4,	client_port >> 8, client_port&0xff);
#endif
			ftp_send_cmd(cmd_buf, NULL, sock_control);
			int nresult = ftp_get_reply(sock_control);
			printf("ftp get reply result:%d\n",nresult);
			if(nresult!= 200)throw("Can not use PORT mode!Please use \"mode\" change to PASV mode.\n");
			else
				return get_sock;
		}
	}
	else
	{		
		int data_port = get_port();
		printf("被动模式获取端口：%d",data_port);
		if(data_port<=0)throw("get port failed from ftp server!\n");
		ftp_server.sin_port=htons(data_port);
		return(xconnect(&ftp_server, 0));
	}
}

int CFtpDownload::get_port()
{
	char port_respond[MSGSIZE2];
	char *buf_ptr;
	int count,port_num;
	memset(port_respond,0,MSGSIZE2);
	ftp_send_cmd("PASV",NULL,sock_control);
	count = Recvs(sock_control, port_respond, MSGSIZE,0);
	if(count <= 0)
		return 0;
	port_respond[count]='\0';
	if(atoi(port_respond) == 227)
	{
		//get low byte of the port
		buf_ptr = strrchr(port_respond, ',');
		port_num = atoi(buf_ptr + 1);
		*buf_ptr = '\0';
		//get high byte of the port
		buf_ptr = strrchr(port_respond, ',');
		port_num += atoi(buf_ptr + 1) * 256;
		return port_num;
	}
	return 0;
}


#ifndef WIN32

int CFtpDownload::CreateDir(const char *sPathName)  
 {  
  char DirName[256];  
  strcpy(DirName, sPathName);  
  int i,len = strlen(DirName);  
  if(DirName[len-1]!='/')  
	strcat(DirName,   "/");  
  len = strlen(DirName);  
  for(i=1; i<len; i++)  
  {  
	if(DirName[i]=='/')  
	{  
		DirName[i] = 0;  
		if( access(DirName, R_OK)!=0)  
		{  
			if(mkdir(DirName, 0755)==-1)  
			{   
				perror("mkdir error");   
                return   -1;   
			}  
		}  
	DirName[i] = '/';  
	}  
  }  
   
  return 0;  
}

#endif
