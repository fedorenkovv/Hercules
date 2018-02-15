#include "streamReceiver.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

namespace asio = boost::asio;

streamReceiver::streamReceiver(std::string ip, int port) : buffDlgt(0)
{
	m_adress = ip;
	m_port = port;
	expectedData = 4;
	bConnectedFLIR = false;
}


streamReceiver::~streamReceiver()
{
	bConnectedFLIR = false;
	delete m_socket;
	cnctThr->interrupt();
	delete cnctThr;
}

void streamReceiver::connect()
{
	cnctThr = new boost::thread(&streamReceiver::accept, this);
	//cnctThr->detach();
}

void streamReceiver::accept()
{
	tcp::resolver resolver(m_io_service);
	m_socket = new tcp::socket(m_io_service);
	std::string sPort = boost::lexical_cast<std::string>(m_port); // int2str
	acceptor = new tcp::acceptor(m_io_service, tcp::endpoint(tcp::v4(), std::stoi(sPort)));
	acceptor->accept(*m_socket);

	bConnectedFLIR = true;
	recv();
}

void streamReceiver::recv()
{
	int nDataCnt = 0;
	
	boost::system::error_code error;   
	for (;;)
	{
		int nRecv = 0;
		try
		{
			boost::asio::streambuf read_buffer;
			// читаем длину картинки
			unsigned char buffer[4];
			size_t bytesRead = asio::read(*m_socket, asio::buffer(buffer, 4));
			int N = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
			
			if (N > 0){
				length = N;
				int toRecieve = N;
				int pos = 0;
				// читаем картинку
				unsigned char* buffImage = new unsigned char[N];
				size_t img;
				do
				{
					img = asio::read(*m_socket, asio::buffer(buffImage+pos, toRecieve));
					toRecieve -= img;
					pos += img;
				} while (toRecieve > 0);
				std::vector<unsigned char> vec(buffImage, buffImage + length);
				cv::Mat tempBuffer = cv::imdecode(cv::Mat(vec), CV_LOAD_IMAGE_COLOR);

				// освобождаем пам€ть (иначе капает)
				delete[] buffImage;		

				// приводим к размерам 240x320 и разбор по буферам 
				// исход€ из размеров (“¬ больше)
				int width = tempBuffer.cols;
				if (buffDlgt)
				{
					if (width == 240)
					{					
						cv::cvtColor(tempBuffer, tempBuffer, cv::COLOR_RGB2GRAY);
						buffDlgt->setBufferIR(tempBuffer);					
						//buffIR = tempBuffer;
					}
					else
					{		
						//cv::cvtColor(tempBuffer, tempBuffer, cv::COLOR_RGB2BGR);
						cv::resize(tempBuffer, tempBuffer, cv::Size(240, 320));
						buffDlgt->setBufferTV(tempBuffer);	
						//buffTV = tempBuffer;
					}
				}
			} // if N > 0
		} // end of TRY-block

		catch (std::exception e)
		{
			bConnectedFLIR = false;
			std::string excp = /*"Read exception: " + */e.what();// << std::endl;
			break;
		}
		if (error == boost::asio::error::eof)
			break;

		length = -1;

	}
	if (!bConnectedFLIR)
	{
		length = -1;
		bConnectedFLIR = false;
		acceptor->close();
		delete m_socket;
		delete acceptor;
		accept();
	}
}
