// ArgusDialog.h

#pragma once

#ifdef ARGUSDIALOG_EXPORTS
#define ARGUSDIALOG_API __declspec(dllexport) 
#else
#define ARGUSDIALOG_API __declspec(dllimport) 
#endif

#include <string>
#include <vector>

struct ArgusButton
{
	ArgusButton()
	{
		name = "";
		type = -1;
		state = -1;
		id = -1;
	}

	ArgusButton(std::string _name, int _type, int _state, int _id)
	{
		name = _name;
		type = _type;
		state = _state;
		id = _id;
	}

	// имя кнопки
	std::string name;

	// тип кнопки 0 - флажок, 1 - обычная кнопка, 2 - кнопка выбора параметра
	int type;

	// состояние кнопки 0 - выключена, 1 - нажата
	int state;

	// идентификатор кнопки
	int id;

	static const int TOGGLE_BUTTON = 0;
	static const int ACTION_BUTTON = 1;
	static const int PARAM_BUTTON = 2;
};

struct ArgusSlider
{
	ArgusSlider()
	{
		value = 0;
		min = 0;
		max = 100;
	}

	int value;
	int min;
	int max;
};

struct ArgusMousePosition
{
	// текущая экранная координата x курсора
	int x;
	// текущая экранная координата y курсора
	int y;

	// номер активной подобласти
	int activeView;

	// номер кнопки
	int nButton;

	// номер класса объекта
	int objectClass;

	// текущая координата x курсора в подобласти окна
	double viewX;
	// текущая координата y курсора в подобласти окна
	double viewY;

	ArgusMousePosition() :
		x(-1),
		y(-1),
		nButton(0),
		objectClass(-1),
		activeView(-1),
		viewX(-1),
		viewY(-1)
	{}
};

struct ArgusMouseState
{
	ArgusMousePosition active;
	ArgusMousePosition click;
};

namespace Argus
{
	// This class is exported from the ArgusDialog.dll
	class ArgusDialog
	{
	public:
		bool m_isOpen;

		// создаёт и отображает диалога
		ARGUSDIALOG_API void Create();

		// закрывает диалог и освобождает ресурсы
		ARGUSDIALOG_API void Delete();

		// проверяет открыт ли диалог
		ARGUSDIALOG_API bool IsOpen();

		// установка изображений
		ARGUSDIALOG_API void SetImages(unsigned char* image0, unsigned char* image1, unsigned char* image2, unsigned char* image3, unsigned char* image4, int width, int height);

		// установка номеров отображаемых подобластей
		ARGUSDIALOG_API void SetSubviews(std::vector<int> indices);
		
		// установка кнопок
		ARGUSDIALOG_API void SetButtons(std::vector<ArgusButton>& buttons0, std::vector<ArgusButton>& buttons1, std::vector<ArgusButton>& buttons2, std::vector<ArgusButton>& buttons3, std::vector<ArgusButton>& buttons4);

		// получение значений кнопок
		ARGUSDIALOG_API void GetButtons(std::vector<ArgusButton>& buttons0, std::vector<ArgusButton>& buttons1, std::vector<ArgusButton>& buttons2, std::vector<ArgusButton>& buttons3, std::vector<ArgusButton>& buttons4);

		// установка ползунка
		ARGUSDIALOG_API void SetSlider(ArgusSlider& slider);

		// получение значений ползунка
		ARGUSDIALOG_API void GetSlider(ArgusSlider& slider);

		// получение текущих координат курсора
		ARGUSDIALOG_API void GetMouseState(ArgusMouseState& mouseState);
	};
}