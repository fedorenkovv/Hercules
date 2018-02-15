#include <iostream>
#include "VideoServer.h"
#include "Net.h"
#include <boost/lexical_cast.hpp>
//#include "boost/foreach.hpp"
//#define foreach BOOST_FOREACH

using namespace std;


// http://msdn.microsoft.com/en-us/library/ms740149%28v=VS.85%29.aspx


struct ThreadDesc
{
    ThreadDesc()
	{ 
		pServer = NULL; 
		Address = ""; 
		Port = 0; 
		//Socket; 
		bStop = false; 
	}

    CVideoServer* pServer;

    std::string   Address;
    int           Port;

    HANDLE        Handle;

    std::string   ClientID;

	tcp::socket *Socket;
	boost::asio::io_service io_service;

    volatile bool bStop;
};

void CVideoServer::ThreadListening( ThreadDesc* TD )
{
    //std::auto_ptr<ThreadDesc> pArg( (ThreadDesc*)pParam );

    CVideoServer* pServ = TD->pServer; //pArg->pServer;

    const int BackLog = 5;

    //SOCKET& Socket = pArg->Socket;

    //Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //if( Socket == INVALID_SOCKET)
    //{
    //    std::cerr << "Unable to create a socket" << std::endl;
    //    return SOCKET_ERROR;
    //}

	tcp::resolver resolver(TD->io_service);
	std::string sPort = boost::lexical_cast<std::string>(TD->Port); // int2str

	boost::system::error_code ec;
	//boost::asio::connect(Socket, endpoint_iterator, ec);

	std::cout << "Listening started at " << TD->Address << ":" << TD->Port << std::endl;

	// создаём объект ожидания подключения
	tcp::acceptor acceptor(TD->io_service, tcp::endpoint(tcp::v4(), std::stoi(sPort)));
	
    // ???? 
	 pServ->RegisterThread( TD );

    for( ; TD->bStop == false ; )
    {
		m_Socket = new tcp::socket(TD->io_service);
		acceptor.accept(*m_Socket);

        std::cout << "Connection accepted" << std::endl;

        TD->pServer->CreateConnectionThread( m_Socket );
    }

	// ???
     pServ->UnRegisterThread( TD );

}


void CVideoServer::ThreadConnection( ThreadDesc* TD )
{
    //std::auto_ptr<ThreadDesc> pArg( (ThreadDesc*)pParam );

    CVideoServer* pServ = this;
    //SOCKET Socket       = pArg->Socket;

    CVideoFrame* pVideoFrame = NULL;
	
	tcp::resolver resolver(TD->io_service);
	//tcp::socket Socket(TD->io_service);
	std::string sPort = boost::lexical_cast<std::string>(TD->Port); // int2str
	tcp::resolver::query query(TD->Address.c_str(), sPort);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);


	/*boost::system::error_code ec;
	boost::asio::connect(*m_Socket, endpoint_iterator, ec);
	if(ec)
		std::cout << "Error" << std::endl;*/

	AuthData Auth;
	if(Recv(*TD->Socket, &Auth, sizeof(Auth)) != 0)
	{
		std::cerr << "Authorization error" << std::endl;
	}



    //if( Recv( Socket, &Auth, sizeof( Auth ) ) != 0 )
    //{
    //    cerr << "Connectin lost" << endl;
    //    closesocket( Socket );
    //    return -1;
    //}

    TD->ClientID = Auth.ClientID;

    pServ->RegisterThread( TD );

    for( ; TD->bStop == false && TD->Socket->is_open() ; )
    {
        CVideoFrame* pVideoFrame = pServ->PopVideoFrame( Auth.ClientID );
        if( pVideoFrame == NULL )
        {
            Sleep( 1 );
            continue;
        }

        VideoFrameDesc Desc = *pVideoFrame;

        if( Send( *TD->Socket, &Desc, sizeof( Desc ) ) != 0 )
        {
            break;
        }

        if( Send( *TD->Socket, pVideoFrame->GetDataPtr(), pVideoFrame->GetWidth() * pVideoFrame->GetHeight() ) != 0 )
        {            
            break;
        }

        delete pVideoFrame;
        pVideoFrame = NULL;
    }
	cerr << "Connection lost" << endl;


    delete pVideoFrame;

    pServ->UnRegisterThread( TD );

    //closesocket( Socket );
}


int CVideoServer::CreateConnectionThread( tcp::socket *Socket )
{
    ThreadDesc* pDesc = new ThreadDesc;
    pDesc->pServer	  = this;
	pDesc->Socket = Socket;
    //m_pThreadDesc->Handle   = CreateThread( NULL, 0, CVideoServer::ThreadConnection, pDesc, CREATE_SUSPENDED, NULL );
	m_pThreadConnection = boost::thread(&CVideoServer::ThreadConnection, this, pDesc); 

    //ResumeThread( pDesc->Handle );
	
    return 0;
}


CVideoServer::CVideoServer(void)
{
	
}


CVideoServer::~CVideoServer(void)
{

}


// TODO
bool CVideoServer::IsActive()
{
    bool bActive;

    //EnterCriticalSection( &m_ThreadCrSec );
	m_pThreadMutex.lock();
    {
        bActive = !m_Threads.empty();
    }
    //LeaveCriticalSection( &m_ThreadCrSec );
	m_pThreadMutex.unlock();

    return bActive;
}


int CVideoServer::Start( std::string Address, int Port )
{
    if( IsActive() )
    {
        std::cerr << "VideoServer already started" << endl;
        return -1;
    }

    ThreadDesc* pDesc = new ThreadDesc;

	pDesc->ClientID = "";
    pDesc->pServer  = this;
    pDesc->Address = Address;
    pDesc->Port    = Port;
    //pDesc->Handle  = CreateThread( NULL, 0, CVideoServer::ThreadListening, pDesc, CREATE_SUSPENDED, NULL );
	m_pThreadConnection = boost::thread(&CVideoServer::ThreadListening, this, pDesc); 

    //ResumeThread( pDesc->Handle );

    return 0;
}


int CVideoServer::Stop( void )
{
    if( !IsActive() )
    {
        std::cerr << "VideoServer already stopped" << endl;
        return -1;
    }

	m_pThreadMutex.lock();
    {
        list<ThreadDesc*>::iterator It  = m_Threads.begin();
        list<ThreadDesc*>::iterator End = m_Threads.end();

        for( ; It != End; ++It )
        {
            (*It)->bStop = true;
            //closesocket( (*It)->Socket );
        }
    }
	m_pThreadMutex.unlock();

    while( IsActive() )
    {
        Sleep( 1 );
    }

    return 0;
}


// TODO
int CVideoServer::PushVideoFrame( CVideoFrame* pFrame )
{
	m_pFrameMutex.lock();

    std::map<std::string, std::queue<CVideoFrame*> >::iterator It  = m_Frames.begin();
    std::map<std::string, std::queue<CVideoFrame*> >::iterator End = m_Frames.end();

    for( ; It != End; ++It )
    {
        if( It->first.empty() )
            continue;

        std::queue<CVideoFrame*>& Frames = m_Frames[ It->first ];

        if( Frames.size() > 10 )
        {
            delete Frames.front();
            Frames.pop();
        }

        CVideoFrame* pCopy = new CVideoFrame;

        *pCopy = *pFrame;

        Frames.push( pCopy );
    }

	m_pFrameMutex.unlock();

    delete pFrame;

    return 0;
}


int CVideoServer::PushVideoFrame( CVideoFrame* pFrame, std::string ClientID )
{
	m_pFrameMutex.lock();
	if( m_Frames.find( ClientID ) == m_Frames.end() )
    {
		m_pFrameMutex.unlock();
		return -1;
	}

    std::queue<CVideoFrame*>& Frames = m_Frames[ ClientID ];

    if( Frames.size() > 10 )
    {
        delete Frames.front();
        Frames.pop();
    }

    CVideoFrame* pCopy = new CVideoFrame;

    *pCopy = *pFrame;

    Frames.push( pCopy );

	m_pFrameMutex.unlock();

    return 0;
}


CVideoFrame* CVideoServer::PopVideoFrame( std::string ClientID )
{
    CVideoFrame* pFrame = NULL;

	m_pFrameMutex.lock();

    std::map<std::string, std::queue<CVideoFrame*> >::iterator It  = m_Frames.begin();
    std::map<std::string, std::queue<CVideoFrame*> >::iterator End = m_Frames.end();

    if( m_Frames.find( ClientID ) != m_Frames.end() )
    {
        std::queue<CVideoFrame*>& Frames = m_Frames[ ClientID ];

        if( !Frames.empty() )
        {
            pFrame = Frames.front();
            Frames.pop();
        }
    }

	m_pFrameMutex.unlock();

    return pFrame;
}

// TODO
void CVideoServer::RegisterThread( ThreadDesc* pThread )
{
	m_pThreadMutex.lock();
	m_pFrameMutex.lock();

	cout << pThread->ClientID << endl;

    if( !pThread->ClientID.empty() )
        m_Frames[ pThread->ClientID ];

    m_Threads.push_back( pThread );

	m_pFrameMutex.unlock();
	m_pThreadMutex.unlock();
}


// TODO
void CVideoServer::UnRegisterThread( ThreadDesc* pThread )
{
	m_pThreadMutex.lock();
	m_pFrameMutex.lock();

    std::list<ThreadDesc*>::iterator ThreadIt = std::find( m_Threads.begin(), m_Threads.end(), pThread );
    if( ThreadIt != m_Threads.end() )
        m_Threads.erase( ThreadIt );

    std::map<std::string, std::queue<CVideoFrame*> >::iterator FrameIt = m_Frames.find( pThread->ClientID );
    if( FrameIt != m_Frames.end() )
        m_Frames.erase( FrameIt );

	m_pFrameMutex.unlock();
	m_pThreadMutex.unlock();
}
