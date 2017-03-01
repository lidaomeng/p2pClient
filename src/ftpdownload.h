#ifndef FTPDOWNLOAD_H_
#define FTPDOWNLOAD_H_
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


typedef unsigned int SOCKET;

//using namespace std;

class CFtpDownload
{
public:
	bool begin(std::string szurl,std::string szsaveas,std::string &sztorrentname,std::string szuser,std::string szpwd,int ftp_mode);
	bool AnalyseReceive(std::string strRead);
	int get_free_port();
	char *getlocalip();
	bool AnalyseURL(std::string szurl);
	void listenrandport(int port);
	CFtpDownload();
	~CFtpDownload();
	int xconnect(struct sockaddr_in *saddr ,int type);
	int fill_host_addr(char * host_ip_addr, struct sockaddr_in * host, int port);
	void ftp_get(const char*src_file,const char*dst_file);
	int ftp_login(char* user,char* pwd);
	int ftp_get_reply(int sock_fd);
	int ftp_get_reply2(int sock_fd,int flags);
	int ftp_send_cmd(const char *s1, const char *s2, int sock_fd);
	int rand_local_port();
	int xconnect_ftpdata();
	int get_port();
	int Sends(SOCKET s,char  * buf,int len,int flags);
	int Recvs(SOCKET s,char  * buf,int len,int flags);
	SOCKET Accepts(SOCKET s,struct sockaddr  * addr,int  * addrlen);
	int Listens(SOCKET s,int backlog);
#ifndef WIN32
	int CreateDir(const char *sPathName);
#endif
protected:
	bool m_bResume;
	int m_Port;
	std::string m_strSavePath;
	std::string m_strUser;	
	std::string m_strPwd;
	std::string m_strObject;
	std::string m_strServer;
	int filesize;//文件大小
	SOCKET sock_control;
	int m_ftp_mode;
};

const char* Ftpdownlaodfile(const char* szurl,const char* szsaveas, const char* szuser, const char* szpwd,int ftp_mode);
#endif
