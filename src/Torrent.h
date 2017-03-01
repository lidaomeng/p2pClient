#ifndef TORRENT_H_
#define TORRENT_H_

/**********************************************************************/
#include "libtorrent/config.hpp"

#ifdef TORRENT_WINDOWS
#include <direct.h> // for _mkdir
#include <sys/types.h> // for _stat
#include <sys/stat.h>
#endif

#ifdef _MSC_VER
#pragma warning(push, 1)
#endif

#include <boost/bind.hpp>
#include <boost/unordered_set.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "libtorrent/extensions/metadata_transfer.hpp"
#include "libtorrent/extensions/ut_metadata.hpp"
#include "libtorrent/extensions/ut_pex.hpp"
#include "libtorrent/extensions/smart_ban.hpp"

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/identify_client.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/ip_filter.hpp"
#include "libtorrent/magnet_uri.hpp"
#include "libtorrent/bitfield.hpp"
#include "libtorrent/peer_info.hpp"
#include "libtorrent/time.hpp"
#include "libtorrent/create_torrent.hpp"
#include "libtorrent/peer_info.hpp"

using boost::bind;
using namespace libtorrent;
using libtorrent::torrent_status;
using namespace std;

#include <iterator>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <map>
#include <string>
#include <fstream>
#include <vector>

#include "soapH.h"
typedef map<string, torrent_handle> Handle_t;
typedef map<torrent_handle, int> Handle_Status_t;

//pthread_mutex_t  mutex_ =  PTHREAD_MUTEX_INITIALIZER;

/************************************************************/

extern bool Loop;
extern bool share_mode;
extern bool disable_storage;
extern bool seed_mode;
extern std::string save_path;
extern Handle_t	Handle_Package;
extern Handle_Status_t Handle_Status;
extern char WS_WEBSERVICE_HOST[1024];

/************************************************************/

int ForBrowserXXXX(soap *_soap);

int load_file(std::string const& filename, std::vector<char>& v, libtorrent::error_code& ec, int limit = 8000000);
	
std::string path_append(std::string const& lhs, std::string const& rhs);
	
bool is_hex(char const *in, int len);

bool yes(libtorrent::torrent_status const&);	

char const* timestamp();

int save_file(std::string const& filename, std::vector<char>& v);


void add_torrent(libtorrent::session& ses
	, std::string const& packageName
	, int allocation_mode
	, std::string const& savePath
	, bool monitored_dir
	, int torrent_upload_limit
	, int torrent_download_limit);
	
	
void delInvalidHandle(const std::string& Package );
	
void signal_handler(int signo);	

#endif
