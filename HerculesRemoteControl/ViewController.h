//
//  ViewController.h
//  HerculesRemoteControl
//
//  Created by Vladimir Knyaz on 14.02.16.
//  Copyright © 2016 Vladimir Knyaz. All rights reserved.
//


//#include <Cocoa/Cocoa.h>

#define WIN32_LEAN_AND_MEAN

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/array.hpp>

#include "HerculesView.h"

using boost::asio::ip::tcp;

class ViewController /*: NSViewController*/
{
public:

	std::string m_adress;
	int m_port;
	boost::asio::io_service m_io_service;
	tcp::socket* m_socket;

	bool m_bConnected;

	ViewController();
	~ViewController();

	void initNetwork();

	int Send(tcp::socket& Socket, const void* pBuff, int nDataLen);

	void setCommand(const char* command);

	// посылает команду управления роботу на перемещение со скоростью
	// speed, и курсом course
	void move(double speed, double course);

	void forward();
	void stop();
	void backward();
	void left();
	void right();
	void capture();

	//-(IBAction)capture:(id)sender;

	//property IBOutlet HerculesView * herculesView;
	//property IBOutlet NSImageView * imageView;

};


