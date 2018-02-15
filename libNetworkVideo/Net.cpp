//#include <Winsock2.h>
#include "Net.h"
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace asio = boost::asio;

int Recv_Until( tcp::socket& Socket, void* pBuff, int nDataLen, char delim )
{
	char* pchBuff = static_cast<char*>( pBuff );
    int nDataCnt = 0;

	boost::system::error_code error;

	asio::streambuf buf;

	int nRecv = asio::read_until(Socket, buf, delim);

	memcpy(pBuff, asio::buffer_cast<const char*>(buf.data()), nRecv);

	if(error = boost::asio::error::eof)
		return -1;


    if( nDataCnt == nDataLen )
        return 0; // Success

    return -1; // Never come here
}

int Recv( tcp::socket& Socket, void* pBuff, int nDataLen )
{
    char* pchBuff = static_cast<char*>( pBuff );
    int nDataCnt = 0;

	boost::system::error_code error;

	//asio::streambuf receive_buffer;
 //   int nRecv = asio::read( Socket, receive_buffer, asio::transfer_all(), error );

	//memcpy(pBuff, asio::buffer_cast<const char*>(receive_buffer.data()), nRecv);

	//return 0;


    for( ;; )
    {
		int nRecv = 0;

		try
		{
			//int nRecv = recv( Socket, &pchBuff[ nDataCnt ], nDataLen - nDataCnt, 0 );
			nRecv = Socket.read_some(boost::asio::buffer(pchBuff + nDataCnt, nDataLen - nDataCnt));
		}
		catch(std::exception e)
		{
			 std::cout << "Read exception: " << e.what() << std::endl;
			 break;
		}

        //if( nRecv == 0 || nRecv == SOCKET_ERROR ) // If 'nRecv' is '0' then the connection has been gracefully closed but it doesn't matter
        //    return SOCKET_ERROR;

		if(error == boost::asio::error::eof)
			break;

        nDataCnt += nRecv;

        if( nDataCnt == nDataLen )
            return 0; // Success
    }

    return -1; // Never come here
}

int Send( tcp::socket& Socket, const void* pBuff, int nDataLen )
{
    const char* pchBuff = static_cast<const char*>( pBuff );
    int nDataCnt = 0;

	for( ;Socket.is_open(); )
    {
        //int nSend = send( Socket, &pchBuff[ nDataCnt ], min(nDataLen - nDataCnt, 1400), 0 );
		boost::system::error_code error;
		int nSend = boost::asio::write(Socket, boost::asio::buffer(pchBuff, nDataLen), error);

        //if( nSend == 0 || nSend == SOCKET_ERROR ) // If 'nSend' is '0' then the connection has been gracefully closed but it doesn't matter
        //    return SOCKET_ERROR;
		if(nSend == 0 || error == boost::asio::error::eof)
			break;

        nDataCnt += nSend;

        if( nDataCnt == nDataLen )
            return 0; // Success
    }

    return -1; // Never come here
}
