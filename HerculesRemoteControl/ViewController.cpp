//
//  ViewController.m
//  HerculesRemoteControl
//
//  Created by Vladimir Knyaz on 14.02.16.
//  Copyright © 2016 Vladimir Knyaz. All rights reserved.
//
#ifndef __VIEW_CONTROLLER_H__
#define __VIEW_CONTROLLER_H__

#define WIN32_LEAN_AND_MEAN
#include <boost/lexical_cast.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <fstream>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <netdb.h>

#include "ViewController.h"
#include "HerculesView.h"

#include <iostream>
#include <sstream>
//#include <thr/threads.h>
//#include <chrono>
#include <ctime>
#include <iomanip>
#include <math.h>

#include "HerculesView.h"

//#include "IMImageFile.h"
//#include "IMTIFFImageFileFormat.h"
//#include "IMVideoClient.h"

int sockfd;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//HerculesView *gHerculesView;
//NSImageView *gImageView;

bool needsToSaveImage = 0;

using namespace std;

ViewController::ViewController()
{	
	// настройки клиента
	m_adress = "192.168.1.2";
	m_port = 12345;
	m_bConnected = false;

	std::ifstream in;
	in.open("raspberryIP.txt");
	if (in.is_open())
	{
		in >> m_adress;
	}
	else{
		in.close();
		return;
	}
	in.close();
}

ViewController::~ViewController()
{
	m_socket->close();
	delete m_socket;
}
class ImageProcessor/*:public IMVideoClientDelegate*/
{
    //virtual void newImageReady(IMImageFile* image)
    //{
    //    static int counter = 0;
    //    stringstream str;
    //    auto t = std::time(nullptr);
    //    auto tm = *std::localtime(&t);
    //    str << "/Users/vladimirknyaz/Documents/WORK/Herc/" << std::put_time(&tm, "%d-%m-%Y_%H-%M-%S") << "_" << counter << ".tiff";
    //    image->setFileFormat(new IMTIFFImageFileFormat);
    //    if(needsToSaveImage)
    //    {
    //        image->save(str.str().c_str());
    //        counter++;
    //        needsToSaveImage = false;
    //    }
    //    
    //    unsigned char* pImage = image->image();
    //    
    //    NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:&pImage
    //                                                                    pixelsWide:image->pixelsWide()
    //                                                                    pixelsHigh:image->pixelsHigh()
    //                                                                 bitsPerSample:image->bitsPerSample()
    //                                                               samplesPerPixel:image->samplesPerPixel()
    //                                                                      hasAlpha:image->hasAlpha()
    //                                                                      isPlanar:image->isPlanar()
    //                                                                colorSpaceName:NSDeviceWhiteColorSpace
    //                                                                  bitmapFormat:0
    //                                                                   bytesPerRow:image->bytesPerRow()
    //                                                                  bitsPerPixel:image->bitsPerSample()];
    //    NSImage *imagens = [[NSImage alloc] initWithSize:NSMakeSize(640, 480)];
    //    [imagens addRepresentation:rep];
    //    [gImageView setImage:imagens];
    //    
    //    //[gHerculesView setImage:pImage];
    //    //[gHerculesView setNeedsDisplay:YES];
    //}
};

// init network
void ViewController::initNetwork()
{
    //int portno, n;
    //struct sockaddr_in serv_addr;
    //struct hostent *server;
    

    //char buffer[256];

    //portno = 12345;
    //sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //if (sockfd < 0)
    //    error("ERROR opening socket");
    //server = gethostbyname("192.168.1.4");
    //if (server == NULL) {
    //    fprintf(stderr,"ERROR, no such host\n");
    //    exit(0);
    //}

	if (!m_bConnected)
	{
		tcp::resolver resolver(m_io_service);
		m_socket = new tcp::socket(m_io_service);
		std::string sPort = boost::lexical_cast<std::string>(m_port); // int2str
		tcp::resolver::query query(m_adress, sPort);
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		boost::system::error_code ec;

		boost::asio::connect(*m_socket, endpoint_iterator, ec);
		int count = 5;
		while (ec){
			boost::asio::connect(*m_socket, endpoint_iterator, ec);
			std::cout << "Can`t connect to server..." << std::endl;
			count--;
			if (count < 0){
				Sleep(2000);
			}
		}
		m_bConnected = true;
		return;
	}
	else
	{
		m_bConnected = false;
		m_socket->close();
	}
		
}

int ViewController::Send(tcp::socket& Socket, const void* pBuff, int nDataLen)
{
	const char* pchBuff = static_cast<const char*>(pBuff);
	int nDataCnt = 0;

	for (; Socket.is_open();)
	{
		//int nSend = send( Socket, &pchBuff[ nDataCnt ], min(nDataLen - nDataCnt, 1400), 0 );
		boost::system::error_code error;
		int nSend = Socket.write_some(boost::asio::buffer(pchBuff, nDataLen));
		//int nSend = boost::asio::write(Socket, boost::asio::buffer(pchBuff, nDataLen), error);

		//if( nSend == 0 || nSend == SOCKET_ERROR ) // If 'nSend' is '0' then the connection has been gracefully closed but it doesn't matter
		//    return SOCKET_ERROR;
		if (nSend == 0 || error == boost::asio::error::eof)
			break;

		nDataCnt += nSend;

		if (nDataCnt == nDataLen)
			return 0; // Success
	}

	return -1; // Never come here
}


void ViewController::capture()
{
    needsToSaveImage = true;
	m_adress = "192.168.1.2";
	m_port = 28800;
}

void ViewController::forward()
{
	char buffer[6] = "RBBBB";
	Send(*m_socket, buffer, strlen(buffer));
}

void ViewController::stop()
{
	char buffer[6] = "SSS ";
	Send(*m_socket, buffer, strlen(buffer));
}

void ViewController::backward()
{
	char buffer[6] = "RFBFB";
	Send(*m_socket, buffer, strlen(buffer));
}

void ViewController::left()
{
	char buffer[6] = "RBDB9";
	Send(*m_socket, buffer, strlen(buffer));
}

void ViewController::right()
{
	char buffer[6] = "RB9BD";
	Send(*m_socket, buffer, strlen(buffer));
}

void ViewController::setCommand(const char* command)
{	
	char buffer[6];
	for (int i = 0; i < 6; i++)
		buffer[i] = command[i];
	buffer[5] = '\0';
	Send(*m_socket, buffer, strlen(buffer));
}

// посылает команду управления роботу на перемещение со скоростью
// speed, и курсом course
void ViewController::move(double speed, double course)
{
	double R, L;
	R = speed + course;
	L = speed - course;
	char buffer[6] = { 0, 0, 0, 0, 0, 0 };
	buffer[0] = 'R';

	if (R > 0){
		buffer[1] = 'B';
	}
	else
	{
		buffer[1] = 'F';
	}
	int r = abs((int)floor(R));
	buffer[2] = (char)(r) + 48;

	if (L > 0){
		buffer[3] = 'B';
	}
	else
	{
		buffer[3] = 'F';
	}

	int l = abs((int)floor(L));
	buffer[4] = (char)(l) + 48;

	// заполнить буфер
	Send(*m_socket, buffer, strlen(buffer));
}

#endif