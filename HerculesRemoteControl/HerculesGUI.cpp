// tstImgEnhancer.cpp : Defines the entry point for the console application.
// git vvk 
#include "ViewController.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm>

#include "AppGLCanvas.h"
#include "streamReceiver.h"
#include "convertEXR2FLO.h"

#include "IMVideoClient.h"
#include "IMImageFile.h"
#include "IMBMPImageFileFormat.h"

#include "boost/lexical_cast.hpp"
#include "boost/filesystem.hpp" 
#include "boost/date_time.hpp"
#include "boost/atomic.hpp"
#include "boost/thread/mutex.hpp"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>

#include "Aim.h"
#include "MAMatrix.h"
#include "PHOrientation.h"
#include "PHCamera.h"

//#include "Image.h"
//#include "CommonDef.h"
//#include "modChannelAlignment.h"
//#include <IMLibTypes.h>

enum
{
	PAINT_TIMER_ID = wxID_HIGHEST + 1
};

using namespace std;
using namespace cv;


enum imageTypeToShow { HERCULES, TV, IR };

const int windowWIDTH = 512;
const int windowHEIGHT = 756;
const int BORDER = 10;
const int BUTTONS_Y_START = 512 + BORDER;
const int ButtonW = 80;
const int ButtonH = 60;
const int ImageSizeW = 512;
const int ImageSizeH = 512;

class MyFrame : public wxFrame, public IMVideoClientDelegate, public bufferDelegate
{
public:
	MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
private:
	void newImageReady(IMImageFile *image);

	void setBufferTV(cv::Mat img);
	void setBufferIR(cv::Mat img);

	vector<cv::Mat> m_bufTV, m_bufIR;

	void getFloImageTV_IR();


	void setConnectionStatus(bool status);
	void recordStatus(bool status);

	void OnKeyDown(wxKeyEvent& event);
	void setImageToShow(int tp);

	void ForwardButtonClick(wxCommandEvent& event);
	void StopButtonClick(wxCommandEvent& event);
	void BackwardButtonClick(wxCommandEvent& event);
	void LeftButtonClick(wxCommandEvent& event);
	void RightButtonClick(wxCommandEvent& event);
	void CaptureButtonClick(wxCommandEvent& event);
	void TestControlButtonClick(wxCommandEvent& event);
	void OnPaintTimerTimeout(wxTimerEvent& event);
	void LoadTrajectory(wxCommandEvent& event);
	void SetTrajectory(wxCommandEvent& event);

	static const long ForwardButtonID;
	static const long BackwardButtonID;
	static const long LeftButtonID;
	static const long RightButtonID;
	static const long StopButtonID;
	static const long CaptureButtonID;
	static const long TestControlButtonID;
	static const long LoadButtonID;
	static const long choiceTrajID;

	bool m_testControlIsOn;

	wxButton *ForwardButton;
	wxButton *BackwardButton;
	wxButton *LeftButton;
	wxButton *RightButton;
	wxButton *StopButton;
	wxButton *CaptureButton;
	wxButton *TestControlButton;
	wxButton *LoadButton;

	wxChoice *wxChTrajectory;
	std::string trajectory;
	wxArrayString arTrajectories;

	// изображение с камеры
	wxStaticBitmap* video;
	IMImageFile *m_image;

	wxStaticText* infoTextBoxExt;
	wxStaticText* infoFps;
	wxStaticText* infoMarkers;

	AppGLCanvas* m_glCanvasHercules;


	Mat imageTV;
	Mat imageIR;
	streamReceiver* strmIR;

	// смена изображения по нажатию на space
	int typeImg;
	Mat mainScreenImageToShow;

	boost::atomic<bool> m_bCaptureStarted;
	boost::thread captureThr;

	// команды траектории из файла
	std::vector<std::string> trajCommands;
	bool startTrajectory;

	wxPanel *panel;

	ViewController* Hercules;
	boost::thread HerculesInitThr;

	IMVideoClient *client;

	wxStaticBitmap* connectionStatusIcon;
	wxStaticBitmap* recordStatusBMP;

	wxTimer *timer;

	unsigned char* imageHerculesCamera;
	int width;
	int height;
	
	boost::mutex mut;

	std::vector<IMImageFile>  m_vImage;
	std::vector<cv::Mat>  m_vImageFlirTV;
	std::vector<cv::Mat>  m_vImageFlirIR;
	wxDECLARE_EVENT_TABLE();
};


const long MyFrame::ForwardButtonID = ::wxNewId();
const long MyFrame::BackwardButtonID = ::wxNewId();
const long MyFrame::LeftButtonID = ::wxNewId();
const long MyFrame::RightButtonID = ::wxNewId();
const long MyFrame::StopButtonID = ::wxNewId();
const long MyFrame::CaptureButtonID = ::wxNewId();
const long MyFrame::TestControlButtonID = ::wxNewId();
const long MyFrame::LoadButtonID = ::wxNewId();
const long MyFrame::choiceTrajID = ::wxNewId();

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
EVT_BUTTON(ForwardButtonID, MyFrame::ForwardButtonClick)
EVT_BUTTON(StopButtonID, MyFrame::StopButtonClick)
EVT_BUTTON(BackwardButtonID, MyFrame::BackwardButtonClick)
EVT_BUTTON(LeftButtonID, MyFrame::LeftButtonClick)
EVT_BUTTON(RightButtonID, MyFrame::RightButtonClick)
EVT_BUTTON(CaptureButtonID, MyFrame::CaptureButtonClick)
EVT_BUTTON(TestControlButtonID, MyFrame::TestControlButtonClick)
EVT_TIMER(PAINT_TIMER_ID, MyFrame::OnPaintTimerTimeout)
EVT_BUTTON(LoadButtonID, MyFrame::LoadTrajectory)
EVT_CHOICE(choiceTrajID, MyFrame::SetTrajectory)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	// screen resolution
	RECT actualDesktop;
	GetWindowRect(GetDesktopWindow(), &actualDesktop);

	// window position
	int x = actualDesktop.right / 2 - windowWIDTH / 2;
	int y = actualDesktop.bottom / 2 - windowHEIGHT / 2;

	new MyFrame("Fusion", wxPoint(x, y), wxSize(windowWIDTH, windowHEIGHT));
	return true;
}

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
: wxFrame(NULL, wxID_ANY, title, pos, size), m_image(0), m_testControlIsOn(false)
{

	this->Bind(wxEVT_CHAR_HOOK, &MyFrame::OnKeyDown, this);

	panel = new wxPanel(this, -1, wxPoint(0, 0),
		wxSize(windowWIDTH, windowHEIGHT));

	::wxInitAllImageHandlers();

	imageHerculesCamera = 0;
	width = 0;
	height = 0;

	wxBoxSizer *pSizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(pSizer);

	//video = new wxStaticBitmap(panel, wxID_ANY, bmp, wxPoint(0, 0), wxSize(320, 240));
	//pSizer->Add(video, 0, wxGROW | wxALL, 0);

	int stereoAttribList[] = { WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_SAMPLE_BUFFERS, GL_TRUE,
		WX_GL_DEPTH_SIZE, 16,
		0, 0 };

	m_glCanvasHercules = new AppGLCanvas(panel, stereoAttribList);
	pSizer->Add(m_glCanvasHercules, 1, wxGROW | wxALL, 0);
	//m_glCanvasHercules->SetClientSize(640, 480);
	m_glCanvasHercules->SetClientSize(ImageSizeW, ImageSizeH);
	
	ForwardButton = new wxButton(panel, ForwardButtonID, "Forward", 
		wxPoint((windowWIDTH - ButtonW) / 2, BUTTONS_Y_START), 
		wxSize(ButtonW, ButtonH));

	StopButton = new wxButton(panel, StopButtonID, "Stop",
		wxPoint((windowWIDTH - ButtonW) / 2, BUTTONS_Y_START + ButtonH + BORDER),
		wxSize(ButtonW, ButtonH));

	BackwardButton = new wxButton(panel, BackwardButtonID, "Backward",
		wxPoint((windowWIDTH - ButtonW) / 2, BUTTONS_Y_START + 2 * (ButtonH + BORDER)),
		wxSize(ButtonW, ButtonH));

	LeftButton = new wxButton(panel, LeftButtonID, "Left",
		wxPoint((windowWIDTH - ButtonW) / 2 - BORDER - ButtonW, BUTTONS_Y_START + ButtonH + BORDER),
		wxSize(ButtonW, ButtonH));

	RightButton = new wxButton(panel, RightButtonID, "Right",
		wxPoint((windowWIDTH - ButtonW) / 2 + BORDER + ButtonW, BUTTONS_Y_START + ButtonH + BORDER),
		wxSize(ButtonW, ButtonH));

	CaptureButton = new wxButton(panel, CaptureButtonID, "Capture",
		wxPoint(BORDER, BUTTONS_Y_START),
		wxSize(ButtonW, ButtonH));

	TestControlButton = new wxButton(panel, TestControlButtonID, "Test Control",
		wxPoint(BORDER, BUTTONS_Y_START + ButtonH + 30),
		wxSize(ButtonW, ButtonH));

	LoadButton = new wxButton(panel, LoadButtonID, "Load",
		wxPoint(windowWIDTH - ButtonW - BORDER, BUTTONS_Y_START + 45),
		wxSize(ButtonW, ButtonH));

	pSizer->Add(CaptureButton, 0, wxGROW | wxALL, 0);
	pSizer->Add(LeftButton, 0, wxGROW | wxALL, 0);
	pSizer->Add(ForwardButton, 0, wxGROW | wxALL, 0);
	pSizer->Add(RightButton, 0, wxGROW | wxALL, 0);
	pSizer->Add(BackwardButton, 0, wxGROW | wxALL, 0);
	pSizer->Add(StopButton, 0, wxGROW | wxALL, 0);

	// раскрывающийся список формата файла
	arTrajectories.Add("chirp");
	arTrajectories.Add("snake");
	arTrajectories.Add("circle");
	infoTextBoxExt = new wxStaticText(panel, wxID_ANY, "Траектория:",
		wxPoint(windowWIDTH - ButtonW - BORDER, BUTTONS_Y_START));
	wxChTrajectory = new wxChoice(panel, choiceTrajID,
		wxPoint(windowWIDTH - ButtonW - BORDER, BUTTONS_Y_START + 20),
		wxSize(80, 25), arTrajectories);
	pSizer->Add(wxChTrajectory, 0, wxGROW | wxALL, 0);
	wxChTrajectory->SetSelection(0);

	infoFps = new wxStaticText(panel, wxID_ANY, " ",
		wxPoint(windowWIDTH - ButtonW - BORDER, BUTTONS_Y_START + 2 * ButtonH));

	// загрузка и масштабирование bmp
	wxBitmap bmp(_T("connection\\x.png"), wxBITMAP_TYPE_PNG);
	wxImage image = bmp.ConvertToImage();
	image = image.Scale(16, 16);
	bmp = wxBitmap(image);
	// иконки выбора папки
	connectionStatusIcon = new wxStaticBitmap(panel, ::wxNewId(), bmp,
		wxPoint(windowWIDTH - 2 * BORDER + 2, windowHEIGHT - 30),
		wxSize(16, 16));

	wxStaticText* connectionTextBox = new wxStaticText(panel, wxID_ANY, "Соединение:",
		wxPoint(windowWIDTH - ButtonW - BORDER, windowHEIGHT - 30));

	infoMarkers = new wxStaticText(panel, wxID_ANY, "1 ",
		wxPoint(BORDER, windowHEIGHT - 60));

	// загрузка и масштабирование bmp
	wxBitmap bmp1(_T("connection\\x.png"), wxBITMAP_TYPE_PNG);
	wxImage image1 = bmp1.ConvertToImage();
	image1 = image1.Scale(53, 17);
	bmp1 = wxBitmap(image1);
	// иконки выбора папки
	recordStatusBMP = new wxStaticBitmap(panel, ::wxNewId(), bmp1,
		wxPoint(BORDER, BUTTONS_Y_START + ButtonH + BORDER),
		wxSize(53, 17));
	recordStatusBMP->ClearBackground();

	//getFloImageTV_IR();

	// timer
	timer = new wxTimer(this, PAINT_TIMER_ID);
	timer->Start(50);
	
	//imwrite("tets.png", decodedImage);
	Hercules = new ViewController();
	HerculesInitThr = boost::thread(&ViewController::initNetwork, Hercules);

	// загрузка картинки с FLIR и запуск потока

	strmIR = new streamReceiver("192.168.1.2", 8888);
	strmIR->setDelegate(this);
	strmIR->connect();			

	std::string adress = "";
	// настройки клиента
	std::ifstream in;
	in.open("raspberryIP.txt");
	if (in.is_open())
	{
		in >> adress;
	}
	else{
	wxMessageBox("Файл с IP raspberry не найден.");
	in.close();
	return;
	}
	in.close();

	client = new IMVideoClient(adress, 20158);
	client->setDelegate(this);
	client->start();

	startTrajectory = false;
	m_bCaptureStarted = false;
	typeImg = 0;
	mainScreenImageToShow.data = imageHerculesCamera;
	// non-resizable window
	SetMaxClientSize(wxSize(windowWIDTH, windowHEIGHT));
	SetMinClientSize(wxSize(windowWIDTH, windowHEIGHT));
	SetClientSize(windowWIDTH, windowHEIGHT);
	
	Show();
}

int i = 0;
void MyFrame::OnPaintTimerTimeout(wxTimerEvent& event)
{	
	// статус соединения на GUI
	setConnectionStatus(Hercules->m_bConnected);

	// если нажата кнопка записи или произведена загрузка траектории
	m_bCaptureStarted ? recordStatus(true) : recordStatus(false);

	if (strmIR->bConnectedFLIR && strmIR->length > 0 && strmIR->length < 60000)
	{		
		//mut.lock();
		// читаем FLIR
		//imageTV = strmIR->buffTV;
		//imageIR = strmIR->buffIR;
		//mut.unlock();
	}
	else if (!strmIR->bConnectedFLIR)
	{
	//	imageTV.release();
	//	imageIR.release();
	}

	if (width == 0 || typeImg > 0)
	{
		width = 240;
		height = 320;
	}

	// отрисовка данных с камер на GUI	
	mut.lock();	
	setImageToShow(typeImg);

	m_glCanvasHercules->setImage(0, mainScreenImageToShow.data, width, height, mainScreenImageToShow.channels());
	mut.unlock();

	m_glCanvasHercules->Refresh();

	if (trajCommands.size() > 0 && startTrajectory)
	{
		if (i >= trajCommands.size())
		{
			startTrajectory = false;
			Hercules->setCommand("SSS  ");
			trajCommands.clear();
			i = 0;
		}
		else
		{			
			Hercules->setCommand(trajCommands[i].c_str());
			i++;
		}
	}

	// пишем запоминаем кадры в памяти
	// для дальнейшего сохранения
	if (m_bCaptureStarted && strmIR->length > 0)
	{
		if (imageTV.rows > 0 && imageIR.rows > 0)
		{
			m_vImageFlirTV.push_back(imageTV);
			m_vImageFlirIR.push_back(imageIR);
		}
	}

	if (!m_bCaptureStarted && (m_vImage.size() > 0 || m_vImageFlirTV.size() > 0) && trajCommands.size() == 0)
	{
		// формируем папку с именем-датой chirp_12-12-12_10-10-10
		namespace pt = boost::posix_time;
		pt::ptime now = pt::second_clock::local_time();
		boost::posix_time::ptime now1 = boost::posix_time::microsec_clock::local_time();
		std::stringstream ss;
		ss << "_" << now.date().day() << "-" << static_cast<int>(now.date().month())
			<< "-" << now.date().year();
		ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet("_%H-%M-%S")));
		ss << now1;
		std::string filepathToSave = trajectory + ss.str();
		boost::filesystem::path dir(filepathToSave);
		ss.clear();

		// сохраняем только, если папка не создана
		boost::system::error_code ec;
		if (!boost::filesystem::exists(dir))
		{
			boost::filesystem::create_directory(dir, ec);

			// пишем последовательность с родной камеры
			if (m_vImage.size() > 0)
			{
				std::string filepathToSaveHerc = filepathToSave + "\\Hercules";
				boost::filesystem::path dirHerc(filepathToSaveHerc);
				boost::filesystem::create_directory(dirHerc, ec);
				for (int j = 0; j < m_vImage.size(); j++)
				{
					std::string num = boost::lexical_cast<std::string>(j);
					IMBMPImageFileFormat format;
					m_vImage[j].setFileFormat(&format);
					m_vImage[j].save(filepathToSaveHerc + "\\im" + num + ".bmp");
				}
				m_vImage.clear();
			}

			// пишем последовательность FLIR
			if (m_vImageFlirIR.size() > 0)
			{
				std::string filepathToSaveIR = filepathToSave + "\\IR";
				boost::filesystem::path dirIR(filepathToSaveIR);
				boost::filesystem::create_directory(dirIR, ec);

				std::string filepathToSaveTV = filepathToSave + "\\TV";
				boost::filesystem::path dirTV(filepathToSaveTV);
				boost::filesystem::create_directory(dirTV, ec);

				// сохранение в память изображений
				for (int k = 0; k < m_vImageFlirTV.size(); k++)
				{
					std::string num = boost::lexical_cast<std::string>(k);
					//cvtColor(m_vImageFlirTV[k], m_vImageFlirTV[k], CV_BGR2RGB);
					imwrite(filepathToSaveTV + "\\im" + num + ".png", m_vImageFlirTV[k]);
					imwrite(filepathToSaveIR + "\\im" + num + ".png", m_vImageFlirIR[k]);
				}
				m_vImageFlirTV.clear();
				m_vImageFlirIR.clear();
			}


			if (ec)
			{
				string str = ec.message();
			}
			
		}
	}
}

void MyFrame::ForwardButtonClick(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	//Hercules->forward();
	Hercules->move(10, 0);
}

void MyFrame::StopButtonClick(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	Hercules->stop();
}

void MyFrame::BackwardButtonClick(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	// Hercules->backward();
	Hercules->move(-10, 0);
}

void MyFrame::RightButtonClick(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	//Hercules->right();
	Hercules->move(5, -5);
}

void MyFrame::LeftButtonClick(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	//Hercules->left();
	Hercules->move(5, 5);
}

void MyFrame::CaptureButtonClick(wxCommandEvent& event)
{
	m_bCaptureStarted = !m_bCaptureStarted;

	trajectory = "simple_moving";
}

// --------------------------------------------------------
void MyFrame::TestControlButtonClick(wxCommandEvent& event)
{
	//Hercules->move(10, 0);
	m_testControlIsOn = !m_testControlIsOn;
}

void MyFrame::LoadTrajectory(wxCommandEvent& event)
{
	if (!Hercules->m_bConnected)
		return;

	ifstream in;
	std::string fileTraj = trajectory + ".txt";
	in.open(fileTraj);
	if (in.is_open())
	{
		std::string com;
		while (in >> com)
			trajCommands.push_back(com);
	}
	in.close();
	startTrajectory = true;
	m_bCaptureStarted = true;
}

int MyApp::OnExit()
{
	return wxApp::OnExit();
}

// для подсчета fps
int countFrames = 0;
unsigned int start_time = 0.0;
bool first = true;
double search_time;

void MyFrame::newImageReady(IMImageFile *image)
{	
	IMBMPImageFileFormat format;
	image->setFileFormat(&format);
	if (m_image == 0)
	{
		m_image = new IMImageFile;
		*m_image = *image;
		m_image->setImage(0);
		m_image->allocImage();
	}

	memcpy(m_image->image(), image->image(), m_image->pixelsWide() * m_image->pixelsHigh());

	imageHerculesCamera = m_image->image();
	width = m_image->pixelsWide();
	height = m_image->pixelsHigh();

	//Mat newImage(width, height, CV_8U);
	//memcpy(newImage.data, imageHerculesCamera, width*height);
	//double minVal, maxVal;
	//Point minLoc, maxLoc;
	//minMaxLoc(newImage, &minVal, &maxVal, &minLoc, &maxLoc);

	//if (maxVal > 240)
	//{
	//	int a = 0;
	//	//if (maxLoc.x < width / 3)
	//	//	Hercules->left();
	//	//else if (maxLoc.x > 2 * width / 3)
	//	//	Hercules->right();
	//	//else
	//	//	Hercules->stop();
	//}
	//else
	//	Hercules->stop();

	// класс поиска меток
	CAim aim;
	aim.InitPar(image->image(), image->pixelsHigh() - 2, image->pixelsWide() - 2, 7);
	aim.Processing();
	// получаем число найденных меток
	int count = aim.GetCountOfAim();
	int count30_31 = 0;

	MAVector3 X0_, X1_, XX0, XX1, result;
	PHOrientation orientation;
	PHCamera camera;
	double course = 0;
	result.x = 0;
	result.y = 0;
	result.z = 0;
	camera.loadParameters("_impara01.txt");
	orientation.setCamera(camera);

	// если найдены две метки
	if (count >= 2)
	{
		int j;
		for (j = 0; j < count; j++)
		{
			if (aim.m_pCrd[j].num == 30)
			{
				X0_.x = aim.m_pCrd[j].x;
				X0_.y = aim.m_pCrd[j].y;
				X0_.z = 0.0;
				count30_31++;
			}
			else if (aim.m_pCrd[j].num == 31)
			{
				X1_.x = aim.m_pCrd[j].x;
				X1_.y = aim.m_pCrd[j].y;
				X1_.z = 0.0;
				count30_31++;
			}
		}

		std::vector<MAVector3> imagePoints, objectPoints;
		XX0.x = 240;
		XX1.x = -241;
		XX0.y = 300;
		XX1.y = 300;
		objectPoints.push_back(XX0);
		objectPoints.push_back(XX1);

		X0_.x = X0_.x;
		X0_.y = X0_.y;
		X1_.x = X1_.x;
		X1_.y = X1_.y;
		imagePoints.push_back(X0_);
		imagePoints.push_back(X1_);

		orientation.setImagePoints(imagePoints);
		orientation.setObjectPoints(objectPoints);

		// алгоритм определения положения меток
		orientation.makeMLZ(0, 0);

		// положение в системе координат гаража
		result.x = orientation.camera().m_X0.x;
		result.y = orientation.camera().m_X0.y;
		result.z = orientation.camera().m_X0.z;

		// курс в системе координат гаража
		course = orientation.m_alpha;
	}

	stringstream buff;
	string control = "CONTROL: ";
	if (m_testControlIsOn)
	{
		control += "ON";
	}
	else
	{
		control += "OFF";
	}
	buff << setw(3) << course * 180.0 / M_PI;
	string infoAboutMarkers = "camera: x: " + boost::lexical_cast<string>((int)result.x) + "   y: " + boost::lexical_cast<string>((int)result.y) +
		"   z: " + boost::lexical_cast<string>((int)result.z) + "\n  alpha: " + buff.str() + ' ' + control + '\n' +
		"31: x:" + boost::lexical_cast<string>((int)X0_.x) + "   y: " + boost::lexical_cast<string>((int)X0_.y) + '\n' +
		"30: x:" + boost::lexical_cast<string>((int)X1_.x) + "   y: " + boost::lexical_cast<string>((int)X1_.y) + '\n';
	infoMarkers->SetLabelText(infoAboutMarkers);

	double T, tx, K1, K2, dx, td;

	T = 100;
	tx = 150;
	td = 50;

	static int isParked = 0;

	// алгоритм управления
	if(m_testControlIsOn)
	{
		// если видны две метки
		if (count30_31 >= 2)
		{
			//угловая скорость
			//double w = angle_vel;
			//double self_angle;192.168.1
			/*if (abs(result.x) < T)
			{
			Hercules->move(5, 0);
			}*/

			// расстояние в пикселах от заданного положения метки tx по x до текущего положения метки в кадре
			dx = tx - X1_.x;
			// если метка, менее чем в td от заданного положения метки tx, едем вперёд
			if (abs(dx) <= td)
			{
				Hercules->move(5, 0);
			}
			// если метка правее, поворачиваем влево
			else if (dx > td)
			{
				Hercules->move(20, 15);
				Sleep(80);
				Hercules->stop();
			}
			// если метка левее, поворачиваем вправо
			else
			{
				Hercules->move(20, -15);
				Sleep(80);
				Hercules->stop();
			}

			// если подъехали близко (вероятно, стоим хорошо) едем прямо 8 секунд, заезжаем в гараж
			if (result.z < 650)
			{
				Hercules->move(5, 0);
				Sleep(8000);
				Hercules->stop();
				m_testControlIsOn = false;
			}

			double self_angle = atan2(result.x, result.z);
			
		}
		else
		{
			Hercules->move(5, -20);
			Sleep(50);
			Hercules->stop();
		}
	}

	// векторы изображений при записи
	if ( m_bCaptureStarted )
	{
		IMImageFile *copy = new IMImageFile;
		copy = image;
		copy->allocImage();
		memcpy(copy->image(), image->image(), image->pixelsWide()*image->pixelsHigh());
		m_vImage.push_back(*copy);
		if (imageTV.rows > 0 && imageIR.rows > 0)
		{
			m_vImageFlirTV.push_back(imageTV);
			m_vImageFlirIR.push_back(imageIR);
		}
	}

	image->freeImage();

	// высчитываем fps 
	countFrames++;
	search_time = (clock() - start_time) / CLOCKS_PER_SEC; // искомое время
	if (search_time >= 1.0)
	{
		string fps = "Fps: " + boost::lexical_cast<string>(countFrames);
		infoFps->SetLabelText(fps);
		countFrames = 0;
		start_time = clock();
	}

}

void MyFrame::setImageToShow(int type)
{
	switch (type)
	{
	case HERCULES:
		mainScreenImageToShow.data = imageHerculesCamera;
		break;
	case TV:
		mainScreenImageToShow = imageTV;
		break;
	case IR:
		mainScreenImageToShow = imageIR;
		break;
	default:
		break;
	}
}

void MyFrame::setBufferTV(cv::Mat imgTV)
{
	mut.lock();

	imageTV = imgTV.clone();

	if (m_bufTV.size() < 2)
		m_bufTV.push_back(imageTV);
	else
	{
		m_bufTV.pop_back();
		m_bufTV.push_back(imageTV);
	}

	mut.unlock();
}

void MyFrame::setBufferIR(cv::Mat imgIR)
{
	mut.lock();

	imageIR = imgIR.clone();

	if (m_bufIR.size() < 2)
		m_bufIR.push_back(imageIR);
	else
	{
		m_bufIR.pop_back();
		m_bufIR.push_back(imageIR);
	}

	mut.unlock();
}
void MyFrame::OnKeyDown(wxKeyEvent& event)
{
	if (((wxKeyEvent&)event).GetKeyCode() == WXK_SPACE)
	{
		if (typeImg >= 2)
			typeImg = 0;
		else typeImg += 1;
	}
}

void MyFrame::SetTrajectory(wxCommandEvent& event)
{
	int trajId = wxChTrajectory->GetCurrentSelection();
	switch (trajId)
	{
	case 0:
		trajectory = arTrajectories[0];
		break;
	case 1:
		trajectory = arTrajectories[1];
		break;
	case 2:
		trajectory = arTrajectories[2];
		break;
	default:
		break;
	}
}

void MyFrame::setConnectionStatus(bool status)
{
	// загрузка и масштабирование bmp
	std::string cnctStatus = status ? "connection\\y.png" : "connection\\x.png";
	wxBitmap bmp((cnctStatus), wxBITMAP_TYPE_PNG);
	wxImage image = bmp.ConvertToImage();
	image = image.Scale(16, 16);
	bmp = wxBitmap(image);

	connectionStatusIcon->SetBitmap(bmp);
}

void MyFrame::recordStatus(bool status)
{
	if (status)
	{
		// статус записи видеоряда
		wxBitmap bmp1(_T("connection\\rec.png"), wxBITMAP_TYPE_PNG);
		wxImage image1 = bmp1.ConvertToImage();
		image1 = image1.Scale(53, 17);
		bmp1 = wxBitmap(image1);
		recordStatusBMP->SetBitmap(bmp1);
	}
	else recordStatusBMP->ClearBackground();
}

void drawOptFlowMap(const Mat& flow, Mat& cflowmap, int step,
	double scale, const Scalar& color)
{
	double xx = 0, yy = 0;
	int x, y;
	for (y = 0; y < cflowmap.rows; y += step)
	for (x = 0; x < cflowmap.cols; x += step)
	{
		const Point2f& fxy = flow.at<Point2f>(y, x);
		//Ради теста здесь суммируем все вектора
		xx += fxy.x;
		yy += fxy.y;

		line(cflowmap, Point(x, y), Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), color);
		circle(cflowmap, Point(x, y), 2, color, -1);
	}

}

void MyFrame::getFloImageTV_IR()
{
	cv::Mat tt = imread("irprev.png", CV_LOAD_IMAGE_GRAYSCALE);
	m_bufIR.push_back(tt);
	tt = imread("ircur.png", CV_LOAD_IMAGE_GRAYSCALE);
	m_bufIR.push_back(tt);
	tt = imread("tvprev.png", CV_LOAD_IMAGE_GRAYSCALE);
	m_bufTV.push_back(tt);
	tt = imread("tvcur.png", CV_LOAD_IMAGE_GRAYSCALE);
	m_bufTV.push_back(tt);

	// получаем изображения flo
	cv::Mat uflowIR;
	calcOpticalFlowFarneback(m_bufIR[0], m_bufIR[1], uflowIR, 0.5, 3, 15, 3, 5, 1.2, 0);
	cv::Mat tempFloIR;
	cvtColor(m_bufIR[0], tempFloIR, CV_GRAY2BGR);
	drawOptFlowMap(uflowIR, tempFloIR, 16, 1.5, CV_RGB(0, 255, 0));

	CFloatImage flowIRPng(uflowIR.cols, uflowIR.rows, 2);
	CShape sh = flowIRPng.Shape();
	flowIRPng.ReAllocate(sh);
	for (int x = 0; x < uflowIR.cols; x++)
	{
		for (int y = 0; y < uflowIR.rows; y++)
		{
			cv::Point2f val = uflowIR.at<cv::Point2f>(y, x);
			flowIRPng.Pixel(x, y, 0) = val.x;
			flowIRPng.Pixel(x, y, 1) = val.y;
		}
	}

	// получаем изображения flo
	cv::Mat prevgrayTV, curgrayTV, uflowTV;
	calcOpticalFlowFarneback(m_bufTV[0], m_bufTV[1], uflowTV, 0.5, 3, 15, 3, 5, 1.2, 0);
	cv::Mat tempFloTV;
	cvtColor(m_bufTV[0], tempFloTV, CV_GRAY2BGR);
	drawOptFlowMap(uflowTV, tempFloTV, 16, 1.5, CV_RGB(0, 255, 0));

	CFloatImage flowTVPng(uflowTV.cols, uflowTV.rows, 2);
	CShape shtv = flowTVPng.Shape();
	flowTVPng.ReAllocate(shtv);
	for (int x = 0; x < uflowTV.cols; x++)
	{
		for (int y = 0; y < uflowTV.rows; y++)
		{
			cv::Point2f val = uflowTV.at<cv::Point2f>(y, x);
			flowTVPng.Pixel(x, y, 0) = val.x;
			flowTVPng.Pixel(x, y, 1) = val.y;
		}
	}
	
	CFloatImage temp(uflowTV.cols, uflowTV.rows, 2);
	CShape sht = temp.Shape();
	flowTVPng.ReAllocate(sht);

	int imageW = temp.Shape().width;
	int imageH = temp.Shape().height;

	for (int i = 0; i < imageW; i++)
	for (int j = 0; j < imageH; j++)
	{

		temp.Pixel(i, j, 0) = abs(flowTVPng.Pixel(i, j, 0)) > abs(flowIRPng.Pixel(i, j, 0)) ? flowTVPng.Pixel(i, j, 0) : flowIRPng.Pixel(i, j, 0);
		temp.Pixel(i, j, 1) = abs(flowTVPng.Pixel(i, j, 1)) > abs(flowIRPng.Pixel(i, j, 1)) ? flowTVPng.Pixel(i, j, 1) : flowIRPng.Pixel(i, j, 1);
	}

	//CByteImage outim;
	//CShape sho = temp.Shape();
	//sho.nBands = 3;
	//outim.ReAllocate(sho);
	//outim.ClearPixels();
	//MotionToColor(temp, outim, -1);
	cv::Mat test;
	//test.create(imageH, imageW, CV_32FC1, temp);
	//for (int i = 0; i < imageW; i++)
	//for (int j = 0; j < imageH; j++)
	//{
	//	test.at<float>(i,j) = 
	//}
	imshow("test", test);
		waitKey(0);
		int l = 0;
}