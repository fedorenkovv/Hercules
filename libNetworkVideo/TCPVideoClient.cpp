#include <iostream>

#include "TCPVideoClient.h"

#include "JPG_decoder.h"
#include <boost/lexical_cast.hpp>

using namespace std;

CTCPVideoClient::CTCPVideoClient(void)
{
    m_pThreadDesc = NULL;

    //InitializeCriticalSection( &m_FramesCrSec );
}


CTCPVideoClient::~CTCPVideoClient(void)
{
	m_pThreadConnection.detach();
	cleanUp();
    //DeleteCriticalSection( &m_FramesCrSec );
}

// подключение и чтение пактов по сети
int CTCPVideoClient::ThreadConnection( ThreadDesc* TD )
{
    //std::auto_ptr<ThreadDesc> pArg( (ThreadDesc*)pParam );

    CTCPVideoClient* pClient = this;
    //SOCKET&       Socket  = pArg->Socket;

    //Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    //if( Socket == INVALID_SOCKET)
    //    return -1;
	
	tcp::resolver resolver(m_pThreadDesc->io_service);
	m_Socket = new tcp::socket(m_pThreadDesc->io_service);
	std::string sPort = boost::lexical_cast<std::string>(m_pThreadDesc->Port); // int2str
	tcp::resolver::query query(m_pThreadDesc->Address.c_str(), sPort);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	boost::system::error_code ec;
	
	boost::asio::connect(*m_Socket, endpoint_iterator, ec);

	int count = 5;
	while(ec){
		boost::asio::connect(*m_Socket, endpoint_iterator, ec);
		std::cout << "Can`t connect to server..." << std::endl;
		count--;
		if(count < 0){
			Sleep(2500);
		}
	}

	AuthData Auth;
	strcpy(Auth.ClientID, m_pThreadDesc->ClientID.c_str());
	if(Send(*m_Socket, &Auth, sizeof(Auth)) != 0)
	{
		std::cerr << "Authorization error" << std::endl;
	}


	CVideoFrame* pFrame = 0;

	

	for( ; m_pThreadDesc->bStop == false; )
    {
		VideoFrameDesc Desc;
		memset(&Desc, 0, sizeof(Desc));

		try
		{
			if( Recv( *m_Socket, &Desc, sizeof( Desc ) ) != 0 )
			{
				cerr << "Connection lost" << endl;
				break;
			}
		}
		catch(boost::exception& e)
		{
			cerr << "Server switched off" << endl;
			m_pThreadDesc->bStop = true;
			continue;
		}

        //CVideoFrame* pFrame = new CVideoFrame;
		//vvf		
		pFrame = new CVideoFrame;

		if (Desc.m_iJpegFrameSize != 0)
		{
			pFrame->CreateRGBY(Desc.m_iWidth, Desc.m_iHeight);
			unsigned char* pJPGData = new(nothrow) unsigned char[Desc.m_iJpegFrameSize];

			if (Recv(*m_Socket, pFrame->GetDataPtr_Y(), Desc.m_iWidth * Desc.m_iHeight) != 0)
			{
				cerr << "Connection lost" << endl;
				break;
			}
			if (Recv(*m_Socket, pJPGData, Desc.m_iJpegFrameSize) != 0)
			{
				cerr << "Connection lost" << endl;
				break;
			}
			
			//if (jpg_decode.DecodeJPG_preallocation(pJPGData, Desc.m_iJpegFrameSize, pFrame->GetDataPtr(), pFrame->GetWidth(), pFrame->GetHeight(), pFrame->GetBpp() / 8, pFrame->GetSize()) != 0)
			//{
			//	cerr << "Error JPEG decode" << endl;
			//	break;
			//}
			//delete[] pJPGData;
		}
		else
		{
			pFrame->Create(Desc.m_iWidth, Desc.m_iHeight);
			
			if (Recv(*m_Socket, pFrame->GetDataPtr(), Desc.m_iStrideWidth * Desc.m_iHeight) != 0)
			{
					cerr << "Connection lost" << endl;
					break;
			}
		}        

        pFrame->nID        = Desc.nID;
        pFrame->VegaInfo   = Desc.VegaInfo;
        pFrame->NavData    = Desc.NavData;
        pFrame->FlightData = Desc.FlightData;
     
        pClient->PushVideoFrame( pFrame );
    }

	// vvf
	// УТЕКАЕТ ПАМЯТЬ ТУТ?!
	/*if(pFrame)
		delete pFrame;*/

    //closesocket( Socket );
	//cleanUp();

    return 0;
}


int CTCPVideoClient::Start( std::string Address, int Port, std::string ClientID )
{
    if( m_pThreadDesc != NULL )
        return -1;

    m_pThreadDesc = new ThreadDesc;

    m_pThreadDesc->pClient  = this;
    m_pThreadDesc->Address  = Address;
    m_pThreadDesc->Port     = Port;
    m_pThreadDesc->ClientID = ClientID;
	    m_pThreadDesc->bStop = false;
    //m_pThreadDesc->Handle   = CreateThread( NULL, 0, CTCPVideoClient::ThreadConnection, m_pThreadDesc, CREATE_SUSPENDED, NULL );
	m_pThreadConnection = boost::thread(&CTCPVideoClient::ThreadConnection, this, m_pThreadDesc);
	//vvf
	m_pThreadConnection.detach();
    //if( m_pThreadDesc->Handle == NULL )
    //{
    //    delete m_pThreadDesc;
    //    return -1;
    //}

    //ResumeThread( m_pThreadDesc->Handle );

    return 0;
}


int CTCPVideoClient::Stop( void )
{
    if( m_pThreadDesc == NULL )
        return -1;

    m_pThreadDesc->bStop = true;

    // Объект m_pThreadDesc освободится при завершении потока
	if(!m_pThreadConnection.joinable())
		return -1;
    //WaitForSingleObject( m_pThreadDesc->Handle, 1000 );

    m_pThreadDesc = NULL;
   
    return 0;
}


int CTCPVideoClient::PushVideoFrame( CVideoFrame* pFrame )
{
    //EnterCriticalSection( &m_FramesCrSec );
	m_pMutex.lock();

    if( m_VideoFrames.size() > 10 )
    {
        delete m_VideoFrames.front();
        m_VideoFrames.pop();
    }

    m_VideoFrames.push( pFrame );

    //LeaveCriticalSection( &m_FramesCrSec );
	m_pMutex.unlock();

    return 0;
}


CVideoFrame* CTCPVideoClient::PopVideoFrame()
{
    CVideoFrame* pFrame = NULL;

    //EnterCriticalSection( &m_FramesCrSec );
	m_pMutex.lock();

    if( !m_VideoFrames.empty() )
    {
        pFrame = m_VideoFrames.front();
        m_VideoFrames.pop();
    }

    //LeaveCriticalSection( &m_FramesCrSec );
	m_pMutex.unlock();

    return pFrame;
}

void CTCPVideoClient::cleanUp()
{
	boost::system::error_code errorcode;
	m_Socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorcode);

	if (errorcode)
	{
		cerr << "socket.shutdown error: " << errorcode.message() << endl;
	}

	m_Socket->close(errorcode);
	if (errorcode)
	{
		cerr << "socket.close error: " << errorcode.message() << endl;
	} 
}