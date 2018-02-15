#pragma once

#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <boost/thread.hpp>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using boost::asio::ip::tcp;

class bufferDelegate
{
public:
	virtual void setBufferTV(cv::Mat image) = 0;
	virtual void setBufferIR(cv::Mat image) = 0;
};

class streamReceiver
{
private:
	int expectedData;
	bufferDelegate* buffDlgt;
	boost::thread* cnctThr;
	tcp::acceptor* acceptor;

	void recv();
	void accept();

public:
	streamReceiver(std::string ip, int port);
	~streamReceiver();

	void connect();

	int m_port;
	std::string m_adress;

	boost::asio::io_service m_io_service;
	tcp::socket* m_socket;

	unsigned char* buffData;
	int length;
	bool bConnectedFLIR;

	cv::Mat buffIR;
	cv::Mat buffTV;


	void setDelegate(bufferDelegate *delegate)
	{
		buffDlgt = delegate;
	}


};

