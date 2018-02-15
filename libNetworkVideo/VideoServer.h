#ifndef __VIDEO_SERVER_H__
#define __VIDEO_SERVER_H__

#define WIN32_LEAN_AND_MEAN

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>

//#include <Windows.h>

#include <string>
#include <queue>
#include <list>
#include <map>

#include "VideoFrame.h"

using boost::asio::ip::tcp;

// Forward declaration
struct ThreadDesc;

// TODO
class CVideoServer
{
    void ThreadListening( ThreadDesc* TD );
 
    void ThreadConnection( ThreadDesc* TD );

public:
    // Constructor
    CVideoServer(void);
    // Destructor
    ~CVideoServer(void);

    // TODO
    bool IsActive();

    // TODO
    int Start( std::string Address, int Port );

    // TODO
    int Stop( void );

    // TODO
    int PushVideoFrame( CVideoFrame* pFrame );

    // TODO
    int PushVideoFrame( CVideoFrame* pFrame, std::string ClientID );


protected:

    // TODO
    CVideoFrame* PopVideoFrame( std::string ClientID );

    // TODO
	int CreateConnectionThread( tcp::socket *Socket );

    // TODO
    void RegisterThread( ThreadDesc* pThread );
    // TODO
    void UnRegisterThread( ThreadDesc* pThread );


    //CRITICAL_SECTION       m_ThreadCrSec;  // TODO
	boost::mutex	m_pThreadMutex;

    std::list<ThreadDesc*> m_Threads;      // TODO

	boost::thread m_pThreadConnection;

    //CRITICAL_SECTION       m_FramesCrSec;  // TODO
	boost::mutex m_pFrameMutex;

    std::map<std::string, std::queue<CVideoFrame*> > m_Frames;

	// сокет отправки данных
	tcp::socket *m_Socket;
	
};

#endif // __VIDEO_SERVER_H__
