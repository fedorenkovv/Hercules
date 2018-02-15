#ifndef __TCP_VIDEO_CLIENT_H__
#define __TCP_VIDEO_CLIENT_H__

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>

//#include <Windows.h>
#include <string>
#include <queue>


#include "Net.h"

#include "VideoFrame.h"

using boost::asio::ip::tcp;

class CTCPVideoClient;

// Forward declaration
struct ThreadDesc
{
	ThreadDesc()
	{
		pClient = NULL;
		Port = 0;
		//Socket = NULL;
		Handle = NULL;
		bStop = false;
	}

	CTCPVideoClient* pClient;

	std::string      ClientID;
	std::string      Address;
	int              Port;

	//SOCKET           Socket;
	HANDLE           Handle;

	boost::asio::io_service io_service;

	volatile bool    bStop;
};

// TODO
class CTCPVideoClient
{
    // TODO
    int ThreadConnection( ThreadDesc* TD );
	void cleanUp();
public:
    // Constructor
    CTCPVideoClient(void);
    // Destructor
    ~CTCPVideoClient(void);

    // TODO
    int Start( std::string Address, int Port, std::string ClientID );
    // TODO
    int Stop();

    // TODO
    int PushVideoFrame( CVideoFrame* pFrame );
    // TODO
    CVideoFrame* PopVideoFrame();
	tcp::socket*		 m_Socket;

protected:

    ThreadDesc*              m_pThreadDesc;  // TODO

    //mutable CRITICAL_SECTION         m_FramesCrSec;  // TODO
	boost::mutex m_pMutex;
	boost::thread m_pThreadConnection;

    std::queue<CVideoFrame*> m_VideoFrames;  // TODO
};

#endif // __TCP_VIDEO_CLIENT_H__
