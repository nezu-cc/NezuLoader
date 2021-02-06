#pragma once

template< typename T >
T GetVFunc(void* vTable, int iIndex) {
	return (*(T**)vTable)[iIndex];
}

class Color
{
public:
	unsigned char _color[4];

	Color()
	{
		*((int*)this) = 0;
	}

	Color(int color32)
	{
		*((int*)this) = color32;
	}

	Color(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b, 255);
	}

	Color(int _r, int _g, int _b, int _a)
	{
		SetColor(_r, _g, _b, _a);
	}

	void SetColor(int _r, int _g, int _b, int _a = 255)
	{
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	void SetColor(float _r, float _g, float _b, float _a = 1.f) {
		_color[0] = (unsigned char)(_r * 255);
		_color[1] = (unsigned char)(_g * 255);
		_color[2] = (unsigned char)(_b * 255);
		_color[3] = (unsigned char)(_a * 255);
	}

	void SetColor(float* colorf) {
		_color[0] = (unsigned char)(colorf[0] * 255);
		_color[1] = (unsigned char)(colorf[1] * 255);
		_color[2] = (unsigned char)(colorf[2] * 255);
		_color[3] = (unsigned char)(colorf[3] * 255);
	}

	void GetColor(int& _r, int& _g, int& _b, int& _a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}

	void SetRawColor(int color32)
	{
		*((int*)this) = color32;
	}

	int GetRawColor() const
	{
		return *((int*)this);
	}

	int GetD3DColor() const
	{
		return ((int)((((_color[3]) & 0xff) << 24) | (((_color[0]) & 0xff) << 16) | (((_color[1]) & 0xff) << 8) | ((_color[2]) & 0xff)));
	}

	inline int r() const
	{
		return _color[0];
	}

	inline int g() const
	{
		return _color[1];
	}

	inline int b() const
	{
		return _color[2];
	}

	inline int a() const
	{
		return _color[3];
	}

	inline float rBase() const
	{
		return _color[0] / 255.0f;
	}

	inline float gBase() const
	{
		return _color[1] / 255.0f;
	}

	inline float bBase() const
	{
		return _color[2] / 255.0f;
	}

	inline float aBase() const
	{
		return _color[3] / 255.0f;
	}

	unsigned char& operator[](int index)
	{
		return _color[index];
	}

	const unsigned char& operator[](int index) const
	{
		return _color[index];
	}

	bool operator ==(const Color& rhs) const
	{
		return (*((int*)this) == *((int*)&rhs));
	}

	bool operator !=(const Color& rhs) const
	{
		return !(operator==(rhs));
	}

	Color& operator=(const Color& rhs)
	{
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

#pragma warning(suppress:4172)
	float* Base()
	{
		static float clr[3];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;
#pragma warning(suppress:4172)
		return &clr[0];
	}

#pragma warning(suppress:4172)
	float* BaseAlpha()
	{
		float clr[4];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;
		clr[3] = _color[3] / 255.0f;
#pragma warning(suppress:4172)
		return &clr[0];
	}

	float Hue() const
	{
		if (_color[0] == _color[1] && _color[1] == _color[2])
		{
			return 0.0f;
		}

		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float delta = max - min;
		float hue = 0.0f;

		if (r == max)
		{
			hue = (g - b) / delta;
		}
		else if (g == max)
		{
			hue = 2 + (b - r) / delta;
		}
		else if (b == max)
		{
			hue = 4 + (r - g) / delta;
		}
		hue *= 60;

		if (hue < 0.0f)
		{
			hue += 360.0f;
		}
		return hue;
	}

	float Saturation() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float l, s = 0;

		if (max != min)
		{
			l = (max + min) / 2;
			if (l <= 0.5f)
				s = (max - min) / (max + min);
			else
				s = (max - min) / (2 - max - min);
		}
		return s;
	}

	float Brightness() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		return (max + min) / 2;
	}

	static Color FromHSB(float hue, float saturation, float brightness)
	{
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
			);
		}
		else if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
			);
		}
		else if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
			);
		}
		else
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
			);
		}
	}

	static Color FromHSBA(float hue, float saturation, float brightness, float alpha) {
		Color c = FromHSB(hue, saturation, brightness);
		c[3] = (unsigned char)(alpha * 255);
		return c;
	}

	static Color Red()
	{
		return Color(255, 0, 0);
	}

	static Color Green()
	{
		return Color(0, 255, 0);
	}

	static Color Blue()
	{
		return Color(0, 0, 255);
	}

	static Color LightBlue()
	{
		return Color(100, 100, 255);
	}

	static Color Grey()
	{
		return Color(128, 128, 128);
	}

	static Color DarkGrey()
	{
		return Color(45, 45, 45);
	}

	static Color Black()
	{
		return Color(0, 0, 0);
	}

	static Color White()
	{
		return Color(255, 255, 255);
	}

	static Color Purple()
	{
		return Color(220, 0, 220);
	}

};

struct Vector2D
{
public:
	float x, y;

	Vector2D()
	{}

	Vector2D(float x_, float y_)
	{
		x = x_;
		y = y_;
	}
};

struct FontVertex_t
{
	Vector2D m_Position;
	Vector2D m_TexCoord;

	FontVertex_t()
	{}

	FontVertex_t(const Vector2D& pos, const Vector2D& coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	void Init(const Vector2D& pos, const Vector2D& coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
};

typedef unsigned long HFont;

typedef FontVertex_t Vertex_t;

enum FontDrawType_t
{
	FONT_DRAW_DEFAULT = 0,
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,
	FONT_DRAW_TYPE_COUNT = 2,
};

class ISurface
{
public:
	void DrawSetColor(int r, int g, int b, int a)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 15)(this, r, g, b, a);
	}

	void DrawSetColor(Color col)
	{
		typedef void(__thiscall* OriginalFn)(void*, Color);
		return GetVFunc< OriginalFn >(this, 14)(this, col);
	}

	void DrawFilledRect(int x0, int y0, int x1, int y1)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 16)(this, x0, y0, x1, y1);
	}

	void DrawOutlinedRect(int x0, int y0, int x1, int y1)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 18)(this, x0, y0, x1, y1);
	}

	void DrawLine(int x0, int y0, int x1, int y1)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 19)(this, x0, y0, x1, y1);
	}

	void DrawPolyLine(int* px, int* py, int numPoints)
	{
		typedef void(__thiscall* OriginalFn)(void*, int*, int*, int);
		return GetVFunc< OriginalFn >(this, 20)(this, px, py, numPoints);
	}

	void DrawSetTextFont(HFont font)
	{
		typedef void(__thiscall* OriginalFn)(void*, HFont);
		return GetVFunc< OriginalFn >(this, 23)(this, font);
	}

	void DrawSetTextColor(int r, int g, int b, int a)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 25)(this, r, g, b, a);
	}

	void DrawSetTextColor(Color col)
	{
		typedef void(__thiscall* OriginalFn)(void*, Color);
		return GetVFunc< OriginalFn >(this, 24)(this, col);
	}

	void DrawSetTextPos(int x, int y)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int);
		return GetVFunc< OriginalFn >(this, 26)(this, x, y);
	}

	void DrawPrintText(const wchar_t* text, int textLen, FontDrawType_t drawType)
	{
		typedef void(__thiscall* OriginalFn)(void*, const wchar_t*, int, FontDrawType_t);
		return GetVFunc< OriginalFn >(this, 28)(this, text, textLen, drawType);
	}

	void DrawSetTextureRGBA(int id, const unsigned char* rgba, int wide, int tall)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, const unsigned char*, int, int);
		return GetVFunc< OriginalFn >(this, 37)(this, id, rgba, wide, tall);
	}

	void DrawSetTexture(int id)
	{
		typedef void(__thiscall* OriginalFn)(void*, int);
		return GetVFunc< OriginalFn >(this, 38)(this, id);
	}

	int CreateNewTextureID(bool procedural)
	{
		typedef int(__thiscall* OriginalFn)(void*, bool);
		return GetVFunc< OriginalFn >(this, 43)(this, procedural);
	}

	HFont Create_Font()
	{
		typedef HFont(__thiscall* OriginalFn)(void*);
		return GetVFunc< OriginalFn >(this, 71)(this);
	}

	bool SetFontGlyphSet(HFont font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin, int nRangeMax)
	{
		typedef bool(__thiscall* OriginalFn)(void*, HFont, const char*, int, int, int, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 72)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
	}

	void GetTextSize(HFont font, const wchar_t* text, int& wide, int& tall)
	{
		typedef void(__thiscall* OriginalFn)(void*, HFont, const wchar_t*, int&, int&);
		return GetVFunc< OriginalFn >(this, 79)(this, font, text, wide, tall);
	}

	void DrawOutlinedCircle(int x, int y, int radius, int segments)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, int, int, int);
		return GetVFunc< OriginalFn >(this, 103)(this, x, y, radius, segments);
	}

	void DrawTexturedPolygon(int n, Vertex_t* pVertice, bool bClipVertices)
	{
		typedef void(__thiscall* OriginalFn)(void*, int, Vertex_t*, bool);
		return GetVFunc< OriginalFn >(this, 106)(this, n, pVertice, bClipVertices);
	}
};