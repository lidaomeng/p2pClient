#include "Torrent.h"
#include "config.h"
#include <unistd.h>
#include "LogManage.h"

#include "soapH.h"
#include "IPeerServiceServiceSoapBinding.nsmap"

#include <boost/asio.hpp>
using namespace boost::asio;

/*******************************************************************************************************/
Handle_t	Handle_Package;
Handle_Status_t Handle_Status;//add by lidm for 暂停只汇报一次

pthread_mutex_t  mutex_ =  PTHREAD_MUTEX_INITIALIZER;

LogManage Log( LOG_INFO, 10*1024*1024, "./../log/", "p2p" );

enum {
	torrents_all,
	torrents_downloading,
	torrents_not_paused,
	torrents_seeding,
	torrents_queued,
	torrents_stopped,
	torrents_checking,
	torrents_feeds,

	torrents_max
};

bool Loop = true;
int num_outstanding_resume_data = 0;
int listen_port = 6881;
int allocation_mode = libtorrent::storage_mode_sparse;
std::string save_path = std::string(".");
int torrent_upload_limit = 888;
int torrent_download_limit = 0;

std::string bind_to_interface = "";

/*do not use temporarily*/
std::string outgoing_interface = "";
int poll_interval = 5;
int max_connections_per_torrent = 50;
bool seed_mode = false;
bool share_mode = false;
bool disable_storage = false;


char WS_WEBSERVICE_HOST[1024];

/*声明一个全局会话变量*/
session ses(fingerprint("LT", LIBTORRENT_VERSION_MAJOR, LIBTORRENT_VERSION_MINOR, 0, 0)
		, session::add_default_plugins
		, alert::all_categories
			& ~(alert::dht_notification
			+ alert::progress_notification
			+ alert::debug_notification
			+ alert::stats_notification));

	
/******************************************************************************************************************/

int BeginSoap( struct soap *spclient, struct soap *spserver );
void EndSoap( struct soap *spclient, struct soap *spserver );



void getTaskProcessReport()
{	
	
	struct soap spclient_;
	
	soap_init(&spclient_);//初始化soap句柄
	soap_set_mode(&spclient_, SOAP_C_UTFSTRING); 
	spclient_.mode |= SOAP_C_UTFSTRING; //关键 	
	
	while( Handle_Package.empty() )
	{
		printf( "Handle_Package.size() = [%d]\n", (int) (Handle_Package.size()) );
		usleep( 10000000 );	
	}

	ns1__getTaskProcessReport taskProcessReport;
	ns1__TaskProcessReport taskarg0;
	taskProcessReport.arg0 = &taskarg0;
	string peerID = "";
	taskarg0.peerID = &peerID;
	vector<ns1__TaskProgress> taskProgressList;
	vector<ns1__TaskInfo> taskInfoList;

	string ftpAdress, baseStorageFtp;
	
	/* Begin 2016-07-29 for mutex */
	pthread_mutex_lock( &mutex_ );
	Handle_t Handle_Package_tmp( Handle_Package );
	pthread_mutex_unlock( &mutex_ );
	/* End 2016-07-29 */
	
	for(Handle_t::iterator itr = Handle_Package_tmp.begin(); itr != Handle_Package_tmp.end(); ++itr)
	{
		torrent_handle temp_handle = itr->second;
		torrent_status const& ts = temp_handle.status();
		
		if ( !temp_handle.is_valid() )
		{
			continue;
		}
		if ( !ts.has_metadata )
		{
			continue;
		}
		/*暂停或删除状态不汇报 2016-10-17*/
		if( (Handle_Status[temp_handle] == 1) || (Handle_Status[temp_handle] == 2)  )
		{
			continue;
		}
		

		ns1__TaskProgress progress_;

		progress_.totalPeerCnt = ts.num_peers;
		progress_.connectedPeerCnt = ts.num_complete;
		progress_.connectingPeerCnt = ts.num_incomplete;
		progress_.downloaded = ts.total_download;
		progress_.downloadRate = ts.download_rate;
		progress_.uploaded = ts.total_upload;
		progress_.uploadRate = ts.upload_rate;
		progress_.percentCmp = ts.progress;
		taskProgressList.push_back(progress_);

		ns1__TaskInfo taskInfo;
		
		if( ts.paused )
		{
			taskInfo.State = 2;
			
			/*1:暂停状态，取消汇报 2016-09-28*/
			Handle_Status_t::iterator pos = Handle_Status.find(temp_handle);
			if(pos != Handle_Status.end())
			{
				Handle_Status[temp_handle] = 1;
			}
		}
		else if( ts.state == 5 )//seeding
		{
			taskInfo.State = 3;//complete
			
			/*2:删除状态，取消汇报 2016-10-17*/
			Handle_Status_t::iterator pos = Handle_Status.find(temp_handle);
			if(pos != Handle_Status.end())
			{
				Handle_Status[temp_handle] = 2;
			}
		}
		else
		{
			taskInfo.State = 0;//downloading
		}
				
		taskInfo.packageName = itr->first;
		taskInfo.torrentUrl = "";
		taskInfo.filedownloadPath = "";
		taskInfo.ftpAdress = &ftpAdress;
		taskInfo.baseStorageFtp = &baseStorageFtp;
		taskInfoList.push_back(taskInfo);

		taskarg0.taskProgressList.push_back(&progress_);
	}
	
	size_t taskSize = taskarg0.taskProgressList.size();//add by lidm 空列表不汇报
	
	for(size_t i=0; i<taskarg0.taskProgressList.size(); i++)
	{
		taskarg0.taskProgressList[i] = &(taskProgressList[i]);
		taskProgressList[i].taskInfo = &(taskInfoList[i]);
	}


	
	ns1__getTaskProcessReportResponse taskProcessReportResponse;
	
	fprintf(stderr,"#######   %s----TaskReportIMPL start   ######\n",__func__);
	//任务进度汇报
	int count = 0;
	while( count < 4 )
	{
		if(0 == taskSize)//add by lidm 空列表不汇报
			break;
			
		if(soap_call___ns1__getTaskProcessReport(&spclient_, WS_WEBSERVICE_HOST, NULL, &taskProcessReport, &taskProcessReportResponse )!= SOAP_OK)
		{
			printf ("function soap_call___ns1__getTaskProcessReport :failed, then try it again.\n");
			
			if (spclient_.error)
			{ 
				soap_print_fault(&spclient_, stderr);
			}
			continue;
			
		}
		else
		{
			printf ("function soap_call___ns1__getTaskProcessReport : ok\n");
			break;
		}
		
	}
	
	fprintf(stderr,"#######   %s----TaskReportIMPL over   ######\n",__func__);
	
	soap_destroy(&spclient_);//销毁句柄
	soap_end(&spclient_);//结束
	soap_done(&spclient_);
	
	
	return ;
}

void* beginReport( void* arg )
{
	char time_[24];
	memset(time_, 0, sizeof(time_) );
	if ( GetProfileString( "./sharestorm.ini", "Sleep", "SleepTime", time_ ) < 0 )
	{
		printf ("read SleepTime from sharestorm.ini failed!\n");
		return NULL;
	}
	
	while(1)
	{
		usleep( atoi(time_) );
		
		getTaskProcessReport();
	}	
	
	return NULL;
}



int main(int argc, char* argv[])
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT,  &signal_handler);

	memset(WS_WEBSERVICE_HOST, 0, 200);
	if ( GetProfileString( "./sharestorm.ini", "Soap", "SoapServer_addr", WS_WEBSERVICE_HOST ) < 0 )
	{
		printf ("read SoapServer_addr from sharestorm.ini failed!\n");
		return -1;
	}
   
	char Active_downloads[10];
	char Active_seeds[10];
	char Active_limit[10];
	
	memset(Active_downloads, 0, 10);
	memset(Active_seeds, 0, 10);
	memset(Active_limit, 0, 10);  
	
	if ( GetProfileString( "./sharestorm.ini", "ActiveTorrent", "Active_downloads", Active_downloads ) < 0 )
	{
		printf ("read Active_downloads from sharestorm.ini failed!\n");
		return -1;
	}
	
	if ( GetProfileString( "./sharestorm.ini", "ActiveTorrent", "Active_seeds", Active_seeds ) < 0 )
	{
		printf ("read Active_seeds from sharestorm.ini failed!\n");
		return -1;
	}
	
	if ( GetProfileString( "./sharestorm.ini", "ActiveTorrent", "Active_limit", Active_limit ) < 0 )
	{
		printf ("read Active_limit from sharestorm.ini failed!\n");
		return -1;
	}	  
	
	struct soap *spserver = NULL;

	/****************************开始设置会话属性**********************************/
	session_settings settings;

	proxy_settings ps;

	bool start_dht = true;
	bool start_upnp = true;
	bool start_lsd = true;

	int counters[torrents_max];
	memset(counters, 0, sizeof(counters));


	std::vector<char> in;
	error_code ec;
	if (load_file(".ses_state", in, ec) == 0)
	{
		lazy_entry e;
		if (lazy_bdecode(&in[0], &in[0] + in.size(), e, ec) == 0)
			ses.load_state(e);
	}

	/*create directory for resume files*/
	string dir_path = path_append(save_path, ".resume");
	
	if( opendir( dir_path.c_str() ) == NULL )
	{
		if( mkdir(dir_path.c_str(), 0777) < 0 )
			printf("mkdir failed!\n");	
	}


	if (start_lsd)
		ses.start_lsd();

	if (start_upnp)
	{
		ses.start_upnp();
		ses.start_natpmp();
	}
	

	ses.set_proxy(ps);

	ses.listen_on(std::make_pair(listen_port, listen_port)
		, ec, bind_to_interface.c_str());
	if (ec)
	{
		fprintf(stderr, "failed to listen%s%s on ports %d-%d: %s\n"
			, bind_to_interface.empty() ? "" : " on ", bind_to_interface.c_str()
			, listen_port, listen_port+1, ec.message().c_str());
	}

#ifndef TORRENT_DISABLE_DHT

	dht_settings dht;
	dht.privacy_lookups = true;
	ses.set_dht_settings(dht);

	if (start_dht)
	{
		settings.use_dht_as_fallback = false;
		#if 0
		ses.add_dht_router(std::make_pair(
			std::string("router.bittorrent.com"), 6881));
		
		ses.add_dht_router(std::make_pair(
			std::string("router.utorrent.com"), 6881));
		ses.add_dht_router(std::make_pair(
			std::string("router.bitcomet.com"), 6881));
		#endif
		
		ses.start_dht();
	}
	
#endif

	settings.user_agent = "client_test/" LIBTORRENT_VERSION;
	settings.choking_algorithm = session_settings::auto_expand_choker;
	settings.disk_cache_algorithm = session_settings::avoid_readback;
	settings.volatile_read_cache = false;
	
	settings.active_downloads = atoi(Active_downloads);
	settings.active_seeds     = atoi(Active_seeds);
	settings.active_limit     = atoi(Active_limit);
	

	ses.set_settings(settings);
	/****************************结束设置会话属性**********************************/

	printf ("The program is running, use Kill -9 [Pid] to exit!\n");
	Log.WriteLog(LOG_INFO, "The program is running, use Kill -9 [Pid] to exit!");
		
	BeginSoap( NULL, spserver );
	
	EndSoap( NULL, spserver );

	
	/*keep track of the number of resume data alerts to wait for*/
	int num_paused = 0;
	int num_failed = 0;

	ses.pause();
	printf("saving resume data\n");
	std::vector<torrent_status> temp;
 	ses.get_torrent_status(&temp, &yes, 0);
	for (std::vector<torrent_status>::iterator i = temp.begin();
		i != temp.end(); ++i)
	{
		torrent_status& st = *i;
		if (!st.handle.is_valid())
		{
			printf("  skipping, invalid handle\n");
			continue;
		}
		if (!st.has_metadata)
		{
			printf("  skipping %s, no metadata\n", st.name.c_str());
			continue;
		}
		if (!st.need_save_resume)
		{
			printf("  skipping %s, resume file up-to-date\n", st.name.c_str());
			continue;
		}

		// save_resume_data will generate an alert when it's done
		st.handle.save_resume_data();
		++num_outstanding_resume_data;
		printf("\r%d  ", num_outstanding_resume_data);
	}
	printf("\nwaiting for resume data [%d]\n", num_outstanding_resume_data);

	while (num_outstanding_resume_data > 0)
	{
		alert const* a = ses.wait_for_alert(seconds(10));
		if (a == 0) continue;

		std::deque<alert*> alerts;
		ses.pop_alerts(&alerts);
		std::string now = timestamp();
		for (std::deque<alert*>::iterator i = alerts.begin()
			, end(alerts.end()); i != end; ++i)
		{
			// make sure to delete each alert
			std::auto_ptr<alert> a(*i);

			torrent_paused_alert const* tp = alert_cast<torrent_paused_alert>(*i);
			if (tp)
			{
				++num_paused;
				printf("\rleft: %d failed: %d pause: %d "
					, num_outstanding_resume_data, num_failed, num_paused);
				continue;
			}

			if (alert_cast<save_resume_data_failed_alert>(*i))
			{
				++num_failed;
				--num_outstanding_resume_data;
				printf("\rleft: %d failed: %d pause: %d "
					, num_outstanding_resume_data, num_failed, num_paused);
				continue;
			}

			save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>(*i);
			if (!rd) continue;
			--num_outstanding_resume_data;
			printf("\rleft: %d failed: %d pause: %d\n"
				, num_outstanding_resume_data, num_failed, num_paused);

			if (!rd->resume_data) continue;

			torrent_handle h = rd->handle;
			torrent_status st = h.status(torrent_handle::query_save_path);
			std::vector<char> out;
			bencode(std::back_inserter(out), *rd->resume_data);
			save_file(path_append(st.save_path, path_append(".resume", to_hex(st.info_hash.to_string()) + ".resume")), out);
		}
	}

	printf("saving session state\n");
	{
		entry session_state;
		ses.save_state(session_state);

		std::vector<char> out;
		bencode(std::back_inserter(out), session_state);
		save_file(".ses_state", out);
	}

	printf("closing session\n");

	return 0;
}

int sendTaskProcess( int type, ns1__TaskInfo* pTaskInfo )
{
	
	ns1__TaskInfo &TaskInfo = *pTaskInfo;
	
	switch ( type )
	{
		case 0://Add 
		{	
			add_torrent(ses, TaskInfo.packageName,
				allocation_mode,  TaskInfo.filedownloadPath , false,
				torrent_upload_limit, torrent_download_limit);
			
			break;
		}
		case 1://Pause
		{
			pthread_mutex_lock( &mutex_ );
			torrent_handle handle = Handle_Package[TaskInfo.packageName];
			pthread_mutex_unlock( &mutex_ );
			
			if ( handle.is_valid() )
			{
				printf ("Pause [%s]\n", TaskInfo.packageName.c_str() );
				
				string Pause = "Pause " + TaskInfo.packageName;
				Log.WriteLog(LOG_INFO, Pause.c_str());
				
				torrent_status const& ts = handle.status();
				if (!ts.auto_managed && ts.paused)
				{
					ts.handle.auto_managed(true);
				}
				else
				{
					ts.handle.auto_managed(false);
					ts.handle.pause(torrent_handle::graceful_pause);
				}
				
			}
			else
			{
					delInvalidHandle ( TaskInfo.packageName );
			}
			
			break;			
		}
		case 2://Stop
		{
			pthread_mutex_lock( &mutex_ );
			torrent_handle handle = Handle_Package[TaskInfo.packageName];
			pthread_mutex_unlock( &mutex_ );
			
			if ( handle.is_valid() || handle.status().paused )
			{
				printf ("Delete [%s]\n", TaskInfo.packageName.c_str() );
				
				string Del = "Delete " + TaskInfo.packageName;
				Log.WriteLog(LOG_INFO, Del.c_str() );
				
				/* ses.remove_torrent(handle, session::delete_files); */
				ses.remove_torrent( handle );
			}
			/*删除无效的句柄*/
			delInvalidHandle( TaskInfo.packageName );			
			
			break;
		}
		default:
			break;
		
	}
	
	return 0;
}

/*********************对外提供的SOAP接口************************************************************************/

SOAP_FMAC5 int SOAP_FMAC6 __ns1__getTaskProcessReport(struct soap*, ns1__getTaskProcessReport *ns1__getTaskProcessReport_, ns1__getTaskProcessReportResponse *ns1__getTaskProcessReportResponse_)
{
	return 0;
}


SOAP_FMAC5 int SOAP_FMAC6 __ns1__getPackList(struct soap*, ns1__getPackList *ns1__getPackList_, ns1__getPackListResponse *ns1__getPackListResponse_)
{
	return 0;
}



SOAP_FMAC5 int SOAP_FMAC6 __ns1__getExceptionReport(struct soap*, ns1__getExceptionReport *ns1__getExceptionReport_, ns1__getExceptionReportResponse *ns1__getExceptionReportResponse_)
{

	return 0;
}

int __ns1__getDcp(struct soap*, ns1__getDcp *ns1__getDcp_, ns1__getDcpResponse *ns1__getDcpResponse_)
{
	return 0;
}

SOAP_FMAC6 SOAP_FMAC5 int SOAP_FMAC6 __ns1__sendTaskProcess(struct soap*, ns1__sendTaskProcess *ns1__sendTaskProcess_, ns1__sendTaskProcessResponse *ns1__sendTaskProcessResponse_)
{
	ns1__TaskOper* pns1_TaskOper=ns1__sendTaskProcess_->arg0;
	int nret=-1;
	if(pns1_TaskOper)
	{
		string* operType=pns1_TaskOper->operType;
		for(size_t i=0;i<pns1_TaskOper->taskInfoList.size();i++)
		{
			nret = -1;
			ns1__TaskInfo* pTaskinfo=pns1_TaskOper->taskInfoList[i];
			int aa=atoi(operType->c_str());
			
			if((aa>=0)&&(aa<=3))
				nret = sendTaskProcess( aa, pTaskinfo );
				
			pTaskinfo->State = nret;
		}
		ns1__sendTaskProcessResponse_->return_=pns1_TaskOper;
	}		
	return 0;
}

/*********************对外提供的SOAP接口************************************************************************/



int BeginSoap( struct soap *spclient, struct soap *spserver )
{
		spserver=soap_new();	
		soap_init(spserver);//初始化soap句柄
		soap_set_mode(spserver, SOAP_C_UTFSTRING); 
		spserver->mode |= SOAP_C_UTFSTRING; //关键 
		
		/*获取客户端地址*/
		char	clientAddr[1024] = "";
		if ( GetProfileString( "./sharestorm.ini", "Soap", "SoapClient_addr", clientAddr ) < 0 )
			printf ("read from sharestorm.ini failed!!\n");
		
		string m_clientaddr = clientAddr;
		int n = m_clientaddr.rfind(":");	
		if(n<0)
		{
			printf ("an invalid soap address:%s\n", m_clientaddr.c_str() );
			return -1;
		}
		
		int len = m_clientaddr.length();
		string szport = m_clientaddr.substr(n+1,len-n-1);
		if(szport.empty())
		{
			printf ("an invalid soap address:%s\n", m_clientaddr.c_str() );
			return -1;
		}
		


		int m = soap_bind(spserver, NULL,atoi(szport.c_str()), 100);//绑定服务端口
		if(m<0)
		{
			printf ("GSoapServiceThreadFunc——端口绑定失败\n");
			exit(0);
		}
		
		
		pthread_t tid;
		if( pthread_create(&tid, NULL, beginReport, NULL) != 0 )
		{
			printf("pthread_create  getTaskProcessReport failed~~!\n");	
			exit(0);
		}
		
		

		while(Loop)
		{
			int s = soap_accept(spserver);//接受客户调用
			
			#if 0
			printf("Connection %d accepted from IP = %d.%d.%d.%d, slave socket = %d\n", 1,
				(spserver.ip >> 24) & 0xff, (spserver.ip >> 16) & 0xff, (spserver.ip >> 8) & 0xff, spserver.ip & 0xff, s);
			#endif
				
			if( !soap_valid_socket(s) )
			{
				if( spserver->errnum )
				{
					soap_print_fault(spserver, stderr);
					soap_done(spserver); 
				}
				printf ("GSoapServiceThreadFunc——等待客户端连接超时!\n");
				continue;
			}
			else
			{
				soap_print_fault(spserver, stderr);
			}
			soap_serve(spserver);
		}

		return 0;
}

void EndSoap( struct soap *spclient, struct soap *spserver )
{
		
	soap_destroy(spserver);//销毁句柄
	soap_end(spserver);//结束
	soap_done(spserver);	
	
}

