#ifndef __NET_H__
#define __NET_H__

//#include <winsock.h>
#include <boost/asio.hpp>

#pragma pack(push)
#pragma pack(1)

using boost::asio::ip::tcp;

struct AuthData
{
    char ClientID[ 260 ];
};

#pragma pack(pop)

    // TODO
	int Recv_Until( tcp::socket& Socket, void* pBuff, int nDataLen, char delim );
    // TODO
	int Recv( tcp::socket& Socket, void* pBuff, int nDataLen );
    // TODO
    int Send( tcp::socket& Socket, const void* pBuff, int nDataLen );

#endif // __NET_H__
