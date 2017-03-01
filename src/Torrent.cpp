#include "Torrent.h"
#include "config.h"
#include "ftpdownload.h"
#include "LogManage.h"

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

extern pthread_mutex_t  mutex_;
extern LogManage Log;

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;  
    FILE *fp;  
    fp = fopen(path, "r");  
    if(fp == NULL)  
        return filesize;  
    fseek(fp, 0L, SEEK_END);  
    filesize = ftell(fp);  
    fclose(fp);  
    return filesize;  
}  

int load_file(std::string const& filename, std::vector<char>& v, libtorrent::error_code& ec, int limit )
{
	ec.clear();
	FILE* f = fopen(filename.c_str(), "rb");
	if (f == NULL)
	{
		ec.assign(errno, boost::system::get_generic_category());
		return -1;
	}

	int r = fseek(f, 0, SEEK_END);
	if (r != 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}
	long s = ftell(f);
	if (s < 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	if (s > limit)
	{
		fclose(f);
		return -2;
	}

	r = fseek(f, 0, SEEK_SET);
	if (r != 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	v.resize(s);
	if (s == 0)
	{
		fclose(f);
		return 0;
	}

	r = fread(&v[0], 1, v.size(), f);
	if (r < 0)
	{
		ec.assign(errno, boost::system::get_generic_category());
		fclose(f);
		return -1;
	}

	fclose(f);

	if (r != s) return -3;

	return 0;
}


std::string path_append(std::string const& lhs, std::string const& rhs)
{
	if (lhs.empty() || lhs == ".") return rhs;
	if (rhs.empty() || rhs == ".") return lhs;

#if defined(TORRENT_WINDOWS) || defined(TORRENT_OS2)
#define TORRENT_SEPARATOR "\\"
	bool need_sep = lhs[lhs.size()-1] != '\\' && lhs[lhs.size()-1] != '/';
#else
#define TORRENT_SEPARATOR "/"
	bool need_sep = lhs[lhs.size()-1] != '/';
#endif
	return lhs + (need_sep?TORRENT_SEPARATOR:"") + rhs;
}


bool is_hex(char const *in, int len)
{
	for (char const* end = in + len; in < end; ++in)
	{
		if (*in >= '0' && *in <= '9') continue;
		if (*in >= 'A' && *in <= 'F') continue;
		if (*in >= 'a' && *in <= 'f') continue;
		return false;
	}
	return true;
}

bool yes(libtorrent::torrent_status const&)
{
	return true;
}


void signal_handler(int signo)
{
	printf("Catch a signal %d\n", signo);
	Loop = false;
}



int ftpDownlaodFile( const string& torrentName )
{
	string user;
	string passwd;
	string Ip;
	string TorrentPath;


	char    Temp[200];

	/*开始获取配置文件*/
	memset(Temp, 0, 200);
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Ftp_usr", Temp ) < 0 )
        printf ("read Path_storePath from sharestorm.ini failed!\n");
	user = Temp;

	memset(Temp, 0, 200);
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Ftp_passwd", Temp ) < 0 )
        printf ("read Path_storePath from sharestorm.ini failed!\n");
	passwd = Temp;

	memset(Temp, 0, 200);
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Ftp_IP", Temp ) < 0 )
        printf ("read Path_storePath from sharestorm.ini failed!\n");
	Ip = Temp;

	memset(Temp, 0, 200);
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Path_torrent", Temp ) < 0 )
        printf ("read Path_storePath from sharestorm.ini failed!\n");
	TorrentPath = Temp;
	/*结束获取配置文件*/


	try
	{
		string szfile = "./";
		string url = "ftp://" + user + ":" + passwd + "@" + Ip + "/" + torrentName;

		string szTorrnetName = (char*)(Ftpdownlaodfile)( url.c_str(), TorrentPath.c_str(), user.c_str(), passwd.c_str(), 1 );

		if( szTorrnetName.empty() )
		{
			printf("download torrent [%s] failed!!\n", torrentName.c_str() );
			return -1;
		}
		else
		{
			printf("download torrent [%s] success!!\n", torrentName.c_str() );
		}


	}
	catch(const char*e)
	{
		printf(e);
	}
	return 0;
}


void add_torrent(libtorrent::session& ses
	, std::string const& packageName
	, int allocation_mode
	, std::string const& savePath
	, bool monitored_dir
	, int torrent_upload_limit
	, int torrent_download_limit)
{
	
	pthread_mutex_lock( &mutex_ );
	Handle_t::iterator itr = Handle_Package.find( packageName );
	if( itr != Handle_Package.end() )
	{
		torrent_handle handle_ = Handle_Package[ packageName ];
		pthread_mutex_unlock( &mutex_ );
	
		Log.WriteLog(LOG_INFO, packageName.c_str() );
		Log.WriteLog(LOG_INFO, "If pause then restart, not pause then Direct Return!\n");
	
		/*暂停恢复*/
		if ( !handle_.status().auto_managed && handle_.status().paused )
		{
			string Resum = "[Restart] " + packageName;
			Log.WriteLog(LOG_INFO, Resum.c_str() );
	
			handle_.auto_managed( true );
			handle_.resume();
			
			/*暂停恢复，重新开始汇报 2016-09-28*/
			Handle_Status_t::iterator pos = Handle_Status.find(handle_);
			/*只重置暂停状态，删除状态不动！2016-10-17*/
			if( (pos != Handle_Status.end()) && (Handle_Status[handle_] == 1) )
			{
				Handle_Status[handle_] = 0;
			}
		}
	
		return ;
	}
	pthread_mutex_unlock( &mutex_ );
	
	
	boost::intrusive_ptr<torrent_info> t;
	error_code ec;
	
	
	string torrent = packageName + string(".torrent");
	
	/*此处判断是否进行ftp下载*/
	char    Temp[200];
	char		download[24];
	char		upload[24];
	char		dcp_download_path[1024];
	
	memset(Temp, 0, 200);
	memset(download, 0, 24);
	memset(upload, 0, 24);
	memset(dcp_download_path, 0, 1024);
	
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Path_torrent", Temp ) < 0 )
	      printf ("read Path_torrent from sharestorm.ini failed!\n");
	if ( GetProfileString( "./sharestorm.ini", "SpeedLimit", "download_limit", download ) < 0 )
	      printf ("read download_limit from sharestorm.ini failed!\n");
	if ( GetProfileString( "./sharestorm.ini", "SpeedLimit", "upload_limit", upload ) < 0 )
	      printf ("read upload_limit from sharestorm.ini failed!\n");
	if ( GetProfileString( "./sharestorm.ini", "FilePath", "Path_downlaod", dcp_download_path ) < 0 )
				printf ("read Path_downlaod from sharestorm.ini failed!!\n");
	
	string torrentPath = string(Temp) + torrent;
	
	if(access(torrentPath.c_str(), F_OK) != 0)
	{
			if( ftpDownlaodFile( torrent ) < 0 )
				return ;
	}
	
	/*Begin check Torrent file size*/
	//struct   stat   buf;
	//::stat(torrentPath.c_str(),&buf);
		
	unsigned long  torrent_File_Size = get_file_size( torrentPath.c_str() );
	
	if(0 == torrent_File_Size)
	{
		string torrentSize = torrentPath + " FileSize is 0! Warning!!!";
		Log.WriteLog(LOG_ERROR, torrentSize.c_str() );/*报错：种子文件为空！*/
		
		/*删除空种子文件，并重新下载！*/
		if( remove( torrentPath.c_str() ) < 0)
		{
		   Log.WriteLog(LOG_ERROR, "delete torrent file failed!" );
		   return ;
		}
		
		if( ftpDownlaodFile( torrent ) < 0 )
		{
		    Log.WriteLog(LOG_ERROR, "ftpDownlaod torrent file failed!" );
		    return ;
		}
	}
	/*End check Torrent file size*/
	
	//记录一下
	Log.WriteLog(LOG_INFO, torrentPath.c_str() );
	
	/*异常判断*/
	try
	{
		t = new torrent_info( torrentPath.c_str(), ec );
		if (ec)
		{
			Log.WriteLog(LOG_ERROR, ec.message().c_str() );/*Note*/
		
			//::stat(torrentPath.c_str(),&buf);
			torrent_File_Size = get_file_size( torrentPath.c_str() );
		
			if(0 == torrent_File_Size)
			{
				string temp = torrentPath + " is 0 byte!AAAAA";
				Log.WriteLog(LOG_ERROR, temp.c_str() ); /*Note*/
			}
		
			return ;
		}
		
		add_torrent_params p;
		if (seed_mode) p.flags |= add_torrent_params::flag_seed_mode;
		if (disable_storage) p.storage = disabled_storage_constructor;
		if (share_mode) p.flags |= add_torrent_params::flag_share_mode;
		lazy_entry resume_data;
		
		std::string filename = path_append(save_path, path_append(".resume", to_hex(t->info_hash().to_string()) + ".resume"));
		
		load_file(filename.c_str(), p.resume_data, ec);
		
		
		p.ti = t;
		p.save_path = dcp_download_path;
		p.storage_mode = (storage_mode_t)allocation_mode;
		p.flags |= add_torrent_params::flag_paused;
		p.flags &= ~add_torrent_params::flag_duplicate_is_error;/*If add an exist Task, then return the exist Handle.*/
		p.flags |= add_torrent_params::flag_auto_managed;
		p.userdata = (void*)strdup(torrent.c_str());
		
		torrent_handle handle = ses.add_torrent(p, ec);/*如果任务已经存在，则返回已存在的任务句柄*/
		if (ec)
		{
			Log.WriteLog(LOG_ERROR, ec.message().c_str() );/*Note*/
		
			//::stat(torrentPath.c_str(),&buf);
		
			torrent_File_Size = get_file_size( torrentPath.c_str() );
		
			if(0 == torrent_File_Size)
			{
				string temp = torrentPath + " is 0 byte!BBBBB";
				Log.WriteLog(LOG_ERROR, temp.c_str() ); /*Note*/
			}
		
			return ;
		}
		
		
		handle.set_download_limit( atoi(download) );
		handle.set_upload_limit( atoi(upload) );
		
		/* 此处有待研究*/
		pthread_mutex_lock( &mutex_ );
		Handle_Package[packageName] = handle;
		Handle_Status[handle] = 0;/*增加句柄：暂停只汇报一次 2016-09-28*/
		pthread_mutex_unlock( &mutex_ );
		/***********************/
	}
	catch(...)
	{
		Log.WriteLog(LOG_ERROR, "种子文件大小为0！" );
		return ;
	}
	
	return ;
}

char const* timestamp()
{
	time_t t = std::time(0);
	tm* timeinfo = std::localtime(&t);
	static char str[200];
	std::strftime(str, 200, "%b %d %X", timeinfo);
	return str;
}

int save_file(std::string const& filename, std::vector<char>& v)
{
	FILE* f = fopen(filename.c_str(), "wb");
	if (f == NULL)
		return -1;

	int w = fwrite(&v[0], 1, v.size(), f);
	if (w < 0)
	{
		fclose(f);
		return -1;
	}

	if (w != int(v.size())) return -3;
	fclose(f);
	return 0;
}


int ForBrowserXXXX(soap *_soap)
{
	ifstream fin("p2p.wsdl",ios::binary);
	if(!fin.is_open())
	{
		return -1;
	}
	_soap -> http_content = "text/xml";
	soap_response(_soap,SOAP_FILE);

	while(!fin.eof())
	{
		fin.read(_soap -> tmpbuf,sizeof(_soap -> tmpbuf));
		if(soap_send_raw(_soap,_soap->tmpbuf,fin.gcount()));
		continue;
	}
	fin.close();
	soap_end_send(_soap);

	return 0;
}

void delInvalidHandle(const std::string& Package )
{
	//当句柄无效或是删除种子时，去掉全局变量Handle_Package对应的元素
	std::string delPackage = Package;

	pthread_mutex_lock( &mutex_ );
	Handle_t::iterator itr = Handle_Package.find( delPackage );
	if ( itr != Handle_Package.end() )
	{
		/*Begin delete Handle_Status item!*/
		torrent_handle handle = itr->second;
		
		Handle_Status_t::iterator pos = Handle_Status.find(handle);
		if(pos != Handle_Status.end())
		{
			Handle_Status.erase( pos );
		}
		/*End delete Handle_Status item!*/

		Handle_Package.erase( itr );
	}
	pthread_mutex_unlock( &mutex_ );

	return ;
}

