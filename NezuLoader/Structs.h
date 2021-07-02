#pragma once

#define EVENT_DEBUG_ID_INIT 42
#define EVENT_DEBUG_ID_SHUTDOWN 13

template< typename T >
T GetVFunc(void* vTable, int iIndex) {
	return (*(T**)vTable)[iIndex];
}

struct angle;

struct vec {
	float x, y, z;

	vec() {
		x = y = z = 0;
	}

	vec(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vec operator+ (vec v) const {
		return { x + v.x, y + v.y, z + v.z };
	}

	vec operator- (vec v) const {
		return { x - v.x, y - v.y, z - v.z };
	}

	vec operator- () const {
		return { -x, -y, -z };
	}

	vec operator+= (vec v) {
		*this = *this + v;
		return *this;
	}

	vec operator-= (vec v) {
		*this = *this - v;
		return *this;
	}

	vec operator* (vec v) const {
		return { x * v.x, y * v.y, z * v.z };
	}

	vec operator* (float v) const {
		return { x * v, y * v, z * v };
	}

	vec operator*= (float v) {
		*this = *this * v;
		return *this;
	}

	vec operator/ (float v) const {
		return { x / v, y / v, z / v };
	}

	vec& operator/=(float fl) {
		x /= fl;
		y /= fl;
		z /= fl;
		return *this;
	}

	bool operator> (vec v) const {
		return sqrt((x * x) + (z * z) + (y * y)) > sqrt((v.x * v.x) + (v.z * v.z) + (v.y * v.y));
	}

	bool operator< (vec v) const {
		return sqrt((x * x) + (z * z) + (y * y)) < sqrt((v.x * v.x) + (v.z * v.z) + (v.y * v.y));
	}

	bool operator> (float v) const {
		return sqrt((x * x) + (z * z) + (y * y)) > v;
	}

	bool operator< (float v) const {
		return sqrt((x * x) + (z * z) + (y * y)) < v;
	}

	bool operator== (vec v) const {
		return x == v.x && z == v.z && y == v.y;
	}
	bool operator!= (vec v) const {
		return !(*this == v);
	}

	inline float& operator[](int i) {
		return ((float*)this)[i];
	}

	float operator[](int i) const {
		return ((float*)this)[i];
	}

	float len() const {
		return sqrtf((float)(x * x) + (z * z) + (y * y));
	}

	float len2d() const {
		return sqrtf((float)(x * x) + (y * y));
	}

	float len2() const {
		return (x * x) + (z * z) + (y * y);
	}

	inline float Dot(vec v) const {
		return (this->x * v.x + this->z * v.z + this->y * v.y);
	}

	inline float diff(vec v) {
		return acosf(this->Dot(v) / sqrtf((float)(((x * x) + (z * z) + (y * y)) * ((v.x * v.x) + (v.z * v.z) + (v.y * v.y)))));
	}

	inline vec Normalized() {
		float l = len();
		float m = l = 0 ? 0 : 1.f / l;

		return *this * m;
	}

	void normalize() {
		*this /= len();
	}

	inline vec abs() {
		return vec{ fabsf(this->x), fabsf(this->y), fabsf(this->z) };
	}

	inline vec cross(vec b) {
		return vec{
			this->y * b.z - this->z * b.y,   // x
			this->z * b.x - this->x * b.z,   // y
			this->x * b.y - this->y * b.x    // z
		};
	}

	operator angle();
};

struct line {
	vec start;
	vec end;

	inline float len() {
		return (end - start).len();
	}
};

struct vec2 {
	int x, y;

	bool operator> (vec2 v) {
		return sqrt((x * x) + (y * y)) > sqrt((v.x * v.x) + (v.y * v.y));
	}

	bool operator< (vec2 v) {
		return sqrt((x * x) + (y * y)) < sqrt((v.x * v.x) + (v.y * v.y));
	}

	vec2 operator- (vec2 v) {
		return { x - v.x, y - v.y };
	}

	float getLen() { return sqrtf((float)(x * x) + (y * y)); }
};

struct angle
{
	float x, y, z;

	angle operator+ (angle v) {
		return { x + v.x, y + v.y, z + v.z };
	}

	angle operator+= (angle v) {
		*this = *this + v;
		return *this;
	}

	angle operator- (angle v) {
		return { x - v.x, y - v.y, z - v.z };
	}

	angle operator-= (angle v) {
		*this = *this - v;
		return *this;
	}

	angle operator* (angle v) {
		return { x * v.x, y * v.y, z * v.z };
	}

	angle operator* (float v) {
		return { x * v, y * v, z * v };
	}

	angle operator/ (float v) {
		return { x / v, y / v, z / v };
	}

	bool operator== (angle v) const {
		return x == v.x && z == v.z && y == v.y;
	}

	bool operator!= (angle v) const {
		return !(*this == v);
	}

	operator vec() { return vec{ x, z, y }; }

	float len() { return sqrtf((x * x) + (y * y) + (z * z)); }

	inline void normalize() {
		if (x < -89.0f) x = -89.0f;
		if (x > 89.0f) x = 89.0f;
		while (y < -180.0f) y += 360.0f;
		while (y > 180.0f) y -= 360.0f;
	}

	inline float& operator[](int i) {
		return ((float*)this)[i];
	}

	float operator[](int i) const {
		return ((float*)this)[i];
	}
};

inline vec::operator angle() { return angle{ x, y, z }; }

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

struct ClientClass;
enum class Team;

class Entity {
	char __pad[0x64];
public:
	int index;

	inline ClientClass* GetClientClass() {
		PVOID pNetworkable = (PVOID)((DWORD)(this) + 0x8);
		if (!pNetworkable) 
			return (ClientClass*)0;
		typedef ClientClass* (__thiscall* OriginalFn)(PVOID);
		return GetVFunc<OriginalFn>(pNetworkable, 2)(pNetworkable);
	}

	inline bool IsDormant() {
		PVOID pNetworkable = (PVOID)((DWORD)(this) + 0x8);
		if (!pNetworkable) 
			return true;
		typedef bool (__thiscall* OriginalFn)(PVOID);
		return GetVFunc<OriginalFn>(pNetworkable, 9)(pNetworkable);
	}

	inline const vec& GetAbsOrigin() {
		typedef const vec&(__thiscall* OriginalFn)(PVOID);
		return GetVFunc<OriginalFn>(this, 10)(this);
	}

	inline const Team GetTeam() {
		typedef const Team(__thiscall* OriginalFn)(PVOID);
		return GetVFunc<OriginalFn>(this, 87)(this);
	}

	inline bool IsEnemy() {//TODO: add dangerzone
		assert(G::LocalPlayer);
		return G::LocalPlayer->GetTeam() != GetTeam();
	}

	NETVAR(ownerEntity, "CBaseEntity", "m_hOwnerEntity", int)

	NETVAR(fireXDelta, "CInferno", "m_fireXDelta", int[100])
	NETVAR(fireYDelta, "CInferno", "m_fireYDelta", int[100])
	NETVAR(fireZDelta, "CInferno", "m_fireZDelta", int[100])
	NETVAR(fireIsBurning, "CInferno", "m_bFireIsBurning", bool[100])
	NETVAR(fireCount, "CInferno", "m_fireCount", int)

};

enum class Team {
	None = 0,
	Spectators,
	TT,
	CT
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

class Color;
class IMaterialVar;

enum PreviewImageRetVal_t
{
	MATERIAL_PREVIEW_IMAGE_BAD = 0,
	MATERIAL_PREVIEW_IMAGE_OK,
	MATERIAL_NO_PREVIEW_IMAGE,
};

enum ImageFormat
{
	IMAGE_FORMAT_UNKNOWN = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888,
	IMAGE_FORMAT_R32F, // Single-channel 32-bit floating point
	IMAGE_FORMAT_RGB323232F, // NOTE: D3D9 does not have this format
	IMAGE_FORMAT_RGBA32323232F,
	IMAGE_FORMAT_RG1616F,
	IMAGE_FORMAT_RG3232F,
	IMAGE_FORMAT_RGBX8888,

	IMAGE_FORMAT_NULL, // Dummy format which takes no video memory

	// Compressed normal map formats
	IMAGE_FORMAT_ATI2N, // One-surface ATI2N / DXN format
	IMAGE_FORMAT_ATI1N, // Two-surface ATI1N format

	IMAGE_FORMAT_RGBA1010102, // 10 bit-per component render targets
	IMAGE_FORMAT_BGRA1010102,
	IMAGE_FORMAT_R16F, // 16 bit FP format

	// Depth-stencil texture formats
	IMAGE_FORMAT_D16,
	IMAGE_FORMAT_D15S1,
	IMAGE_FORMAT_D32,
	IMAGE_FORMAT_D24S8,
	IMAGE_FORMAT_LINEAR_D24S8,
	IMAGE_FORMAT_D24X8,
	IMAGE_FORMAT_D24X4S4,
	IMAGE_FORMAT_D24FS8,
	IMAGE_FORMAT_D16_SHADOW, // Specific formats for shadow mapping
	IMAGE_FORMAT_D24X8_SHADOW, // Specific formats for shadow mapping

	// supporting these specific formats as non-tiled for procedural cpu access (360-specific)
	IMAGE_FORMAT_LINEAR_BGRX8888,
	IMAGE_FORMAT_LINEAR_RGBA8888,
	IMAGE_FORMAT_LINEAR_ABGR8888,
	IMAGE_FORMAT_LINEAR_ARGB8888,
	IMAGE_FORMAT_LINEAR_BGRA8888,
	IMAGE_FORMAT_LINEAR_RGB888,
	IMAGE_FORMAT_LINEAR_BGR888,
	IMAGE_FORMAT_LINEAR_BGRX5551,
	IMAGE_FORMAT_LINEAR_I8,
	IMAGE_FORMAT_LINEAR_RGBA16161616,

	IMAGE_FORMAT_LE_BGRX8888,
	IMAGE_FORMAT_LE_BGRA8888,

	NUM_IMAGE_FORMATS
};

enum MaterialVarFlags_t
{
	MATERIAL_VAR_DEBUG = (1 << 0),
	MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
	MATERIAL_VAR_NO_DRAW = (1 << 2),
	MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),

	MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
	MATERIAL_VAR_VERTEXALPHA = (1 << 5),
	MATERIAL_VAR_SELFILLUM = (1 << 6),
	MATERIAL_VAR_ADDITIVE = (1 << 7),
	MATERIAL_VAR_ALPHATEST = (1 << 8),
	//	MATERIAL_VAR_UNUSED					  = (1 << 9),
	MATERIAL_VAR_ZNEARER = (1 << 10),
	MATERIAL_VAR_MODEL = (1 << 11),
	MATERIAL_VAR_FLAT = (1 << 12),
	MATERIAL_VAR_NOCULL = (1 << 13),
	MATERIAL_VAR_NOFOG = (1 << 14),
	MATERIAL_VAR_IGNOREZ = (1 << 15),
	MATERIAL_VAR_DECAL = (1 << 16),
	MATERIAL_VAR_ENVMAPSPHERE = (1 << 17), // OBSOLETE
	//	MATERIAL_VAR_UNUSED					  = (1 << 18),
	MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19), // OBSOLETE
	MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
	MATERIAL_VAR_TRANSLUCENT = (1 << 21),
	MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
	MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23), // OBSOLETE
	MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
	MATERIAL_VAR_ENVMAPMODE = (1 << 25), // OBSOLETE
	MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
	MATERIAL_VAR_HALFLAMBERT = (1 << 27),
	MATERIAL_VAR_WIREFRAME = (1 << 28),
	MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
	MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY = (1 << 30),
	MATERIAL_VAR_VERTEXFOG = (1 << 31),

	// NOTE: Only add flags here that either should be read from
	// .vmts or can be set directly from client code. Other, internal
	// flags should to into the flag enum in IMaterialInternal.h
};

//-----------------------------------------------------------------------------
// Internal flags not accessible from outside the material system. Stored in Flags2
//-----------------------------------------------------------------------------
enum MaterialVarFlags2_t
{
	// NOTE: These are for $flags2!!!!!
	//	UNUSED											= (1 << 0),

	MATERIAL_VAR2_LIGHTING_UNLIT = 0,
	MATERIAL_VAR2_LIGHTING_VERTEX_LIT = (1 << 1),
	MATERIAL_VAR2_LIGHTING_LIGHTMAP = (1 << 2),
	MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP = (1 << 3),
	MATERIAL_VAR2_LIGHTING_MASK =
	(MATERIAL_VAR2_LIGHTING_VERTEX_LIT |
		MATERIAL_VAR2_LIGHTING_LIGHTMAP |
		MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP),

	// FIXME: Should this be a part of the above lighting enums?
	MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL = (1 << 4),
	MATERIAL_VAR2_USES_ENV_CUBEMAP = (1 << 5),
	MATERIAL_VAR2_NEEDS_TANGENT_SPACES = (1 << 6),
	MATERIAL_VAR2_NEEDS_SOFTWARE_LIGHTING = (1 << 7),
	// GR - HDR path puts lightmap alpha in separate texture...
	MATERIAL_VAR2_BLEND_WITH_LIGHTMAP_ALPHA = (1 << 8),
	MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS = (1 << 9),
	MATERIAL_VAR2_USE_FLASHLIGHT = (1 << 10),
	MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING = (1 << 11),
	MATERIAL_VAR2_NEEDS_FIXED_FUNCTION_FLASHLIGHT = (1 << 12),
	MATERIAL_VAR2_USE_EDITOR = (1 << 13),
	MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE = (1 << 14),
	MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE = (1 << 15),
	MATERIAL_VAR2_IS_SPRITECARD = (1 << 16),
	MATERIAL_VAR2_USES_VERTEXID = (1 << 17),
	MATERIAL_VAR2_SUPPORTS_HW_SKINNING = (1 << 18),
	MATERIAL_VAR2_SUPPORTS_FLASHLIGHT = (1 << 19),
	MATERIAL_VAR2_USE_GBUFFER0 = (1 << 20),
	MATERIAL_VAR2_USE_GBUFFER1 = (1 << 21),
	MATERIAL_VAR2_SELFILLUMMASK = (1 << 22),
	MATERIAL_VAR2_SUPPORTS_TESSELLATION = (1 << 23)
};

enum MaterialPropertyTypes_t
{
	MATERIAL_PROPERTY_NEEDS_LIGHTMAP = 0, // bool
	MATERIAL_PROPERTY_OPACITY, // int (enum MaterialPropertyOpacityTypes_t)
	MATERIAL_PROPERTY_REFLECTIVITY, // vec3_t
	MATERIAL_PROPERTY_NEEDS_BUMPED_LIGHTMAPS // bool
};

class IMaterial
{
public:
	// Get the name of the material.  This is a full path to 
	// the vmt file starting from "hl2/materials" (or equivalent) without
	// a file extension.
	virtual const char* GetName() const = 0;
	virtual const char* GetTextureGroupName() const = 0;

	// Get the preferred size/bitDepth of a preview image of a material.
	// This is the sort of image that you would use for a thumbnail view
	// of a material, or in WorldCraft until it uses materials to render.
	// separate this for the tools maybe
	virtual PreviewImageRetVal_t GetPreviewImageProperties(int* width, int* height, ImageFormat* imageFormat, bool* isTranslucent) const = 0;

	// Get a preview image at the specified width/height and bitDepth.
	// Will do resampling if necessary.(not yet!!! :) )
	// Will do color format conversion. (works now.)
	virtual PreviewImageRetVal_t GetPreviewImage(unsigned char* data, int width, int height, ImageFormat imageFormat) const = 0;
	// 
	virtual int GetMappingWidth() = 0;
	virtual int GetMappingHeight() = 0;

	virtual int GetNumAnimationFrames() = 0;

	// For material subrects (material pages).  Offset(u,v) and scale(u,v) are normalized to texture.
	virtual bool InMaterialPage(void) = 0;
	virtual void GetMaterialOffset(float* pOffset) = 0;
	virtual void GetMaterialScale(float* pScale) = 0;
	virtual IMaterial* GetMaterialPage(void) = 0;

	// find a vmt variable.
	// This is how game code affects how a material is rendered.
	// The game code must know about the params that are used by
	// the shader for the material that it is trying to affect.
	virtual IMaterialVar* FindVar(const char* varName, bool* found, bool complain = true) = 0;

	// The user never allocates or deallocates materials.  Reference counting is
	// used instead.  Garbage collection is done upon a call to 
	// IMaterialSystem::UncacheUnusedMaterials.
	virtual void IncrementReferenceCount(void) = 0;
	virtual void DecrementReferenceCount(void) = 0;

	inline void AddRef()
	{
		IncrementReferenceCount();
	}

	inline void Release()
	{
		DecrementReferenceCount();
	}

	// Each material is assigned a number that groups it with like materials
	// for sorting in the application.
	virtual int GetEnumerationID(void) const = 0;

	virtual void GetLowResColorSample(float s, float t, float* color) const = 0;

	// This computes the state snapshots for this material
	virtual void RecomputeStateSnapshots() = 0;

	// Are we translucent?
	virtual bool IsTranslucent() = 0;

	// Are we alphatested?
	virtual bool IsAlphaTested() = 0;

	// Are we vertex lit?
	virtual bool IsVertexLit() = 0;

	// Gets the vertex format
	virtual unsigned __int64 GetVertexFormat() const = 0;

	// returns true if this material uses a material proxy
	virtual bool HasProxy(void) const = 0;

	virtual bool UsesEnvCubemap(void) = 0;

	virtual bool NeedsTangentSpace(void) = 0;

	virtual bool NeedsPowerOfTwoFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;
	virtual bool NeedsFullFrameBufferTexture(bool bCheckSpecificToThisFrame = true) = 0;

	// returns true if the shader doesn't do skinning itself and requires
	// the data that is sent to it to be preskinned.
	virtual bool NeedsSoftwareSkinning(void) = 0;

	// Apply constant color or alpha modulation
	virtual void AlphaModulate(float alpha) = 0;
	virtual void ColorModulate(float r, float g, float b) = 0;

	// Material Var flags...
	virtual void SetMaterialVarFlag(MaterialVarFlags_t flag, bool on) = 0;
	virtual bool GetMaterialVarFlag(MaterialVarFlags_t flag) = 0;

	// Gets material reflectivity
	virtual void GetReflectivity(vec& reflect) = 0;

	// Gets material property flags
	virtual bool GetPropertyFlag(MaterialPropertyTypes_t type) = 0;

	// Is the material visible from both sides?
	virtual bool IsTwoSided() = 0;

	// Sets the shader associated with the material
	virtual void SetShader(const char* pShaderName) = 0;

	// Can't be const because the material might have to precache itself.
	virtual int GetNumPasses(void) = 0;

	// Can't be const because the material might have to precache itself.
	virtual int GetTextureMemoryBytes(void) = 0;

	// Meant to be used with materials created using CreateMaterial
	// It updates the materials to reflect the current values stored in the material vars
	virtual void Refresh() = 0;

	// GR - returns true is material uses lightmap alpha for blending
	virtual bool NeedsLightmapBlendAlpha(void) = 0;

	// returns true if the shader doesn't do lighting itself and requires
	// the data that is sent to it to be prelighted
	virtual bool NeedsSoftwareLighting(void) = 0;

	// Gets at the shader parameters
	virtual int ShaderParamCount() const = 0;
	virtual void** GetShaderParams(void) = 0;

	// Returns true if this is the error material you get back from IMaterialSystem::FindMaterial if
	// the material can't be found.
	virtual bool IsErrorMaterial() const = 0;

	virtual void Unused() = 0;

	// Gets the current alpha modulation
	virtual float GetAlphaModulation() = 0;
	virtual void GetColorModulation(float* r, float* g, float* b) = 0;

	// Is this translucent given a particular alpha modulation?
	virtual bool IsTranslucentUnderModulation(float fAlphaModulation = 1.0f) const = 0;

	// fast find that stores the index of the found var in the string table in local cache
	virtual void* FindVarFast(char const* pVarName, unsigned int* pToken) = 0;

	// Sets new VMT shader parameters for the material
	virtual void SetShaderAndParams(void* pKeyValues) = 0;
	virtual const char* GetShaderName() const = 0;

	virtual void DeleteIfUnreferenced() = 0;

	virtual bool IsSpriteCard() = 0;

	virtual void CallBindProxy(void* proxyData) = 0;

	virtual void RefreshPreservingMaterialVars() = 0;

	virtual bool WasReloadedFromWhitelist() = 0;

	virtual bool SetTempExcluded(bool bSet, int nExcludedDimensionLimit) = 0;

	virtual int GetReferenceCount() const = 0;
};

struct CDemoPlaybackParameters_t {
	uint64_t m_uiCaseID;
	uint32_t m_uiHeaderPrefixLength;
	uint32_t m_uiLockFirstPersonAccountID;
	bool m_bAnonymousPlayerIdentity;
	uint32_t m_numRoundSkip;
	uint32_t m_numRoundStop;
	bool m_bSkipWarmup;
	bool m_bPlayingLiveRemoteBroadcast;
	uint64_t m_uiLiveMatchID;
	uint64_t m_uiLiveMatchID2;
	uint64_t m_uiLiveMatchID3;
	uint64_t m_uiLiveMatchID4;
	uint64_t m_uiLiveMatchID5;
};

class ISPSharedMemory;
class CGamestatsData;
class KeyValues;
class CSteamAPIContext;
struct Frustum_t;
class pfnDemoCustomDataCallback;
typedef struct InputContextHandle_t__* InputContextHandle_t;
struct client_textmessage_t;
struct model_t;
class SurfInfo;
class IMaterial;
class CSentence;
class CAudioSource;
class AudioState_t;
class ISpatialQuery;
class IMaterialSystem;
class CPhysCollide;
class IAchievementMgr;

struct model_t
{
	char name[255];
};

struct player_info_t {
	char __pad0[0x8];
	std::uint64_t xuid;
	char name[128];
	int userid;
	char guid[33];
	unsigned long friendsID;
	char friendsName[128];
	bool fakeplayer;
	bool ishltv;
	unsigned long customFiles[4];
	unsigned char filesDownloaded;
	char __pad1[0xC8];

	std::string getName() {
		return ishltv ? "GOTV" : ((fakeplayer ? std::string("Bot ") : std::string("")) + name);
	}
};

#define FLOW_OUTGOING	0
#define FLOW_INCOMING	1
#define MAX_FLOWS		2		// in & out

class INetChannelInfo {
public:

	enum {
		GENERIC = 0, // must be first and is default group
		LOCALPLAYER, // bytes for local player entity update
		OTHERPLAYERS, // bytes for other players update
		ENTITIES, // all other entity bytes
		SOUNDS, // game sounds
		EVENTS, // event messages
		USERMESSAGES, // user messages
		ENTMESSAGES, // entity messages
		VOICE, // voice data
		STRINGTABLE, // a stringtable update
		MOVE, // client move cmds
		STRINGCMD, // string command
		SIGNON, // various signondata
		TOTAL, // must be last and is not a real group
	};

	virtual const char* GetName(void) const = 0; // get channel name
	virtual const char* GetAddress(void) const = 0; // get channel IP address as string
	virtual float GetTime(void) const = 0; // current net time
	virtual float GetTimeConnected(void) const = 0; // get connection time in seconds
	virtual int GetBufferSize(void) const = 0; // netchannel packet history size
	virtual int GetDataRate(void) const = 0; // send data rate in byte/sec

	virtual bool IsLoopback(void) const = 0; // true if loopback channel
	virtual bool IsTimingOut(void) const = 0; // true if timing out
	virtual bool IsPlayback(void) const = 0; // true if demo playback

	virtual float GetLatency(int flow) const = 0; // current latency (RTT), more accurate but jittering
	virtual float GetAvgLatency(int flow) const = 0; // average packet latency in seconds
	virtual float GetAvgLoss(int flow) const = 0; // avg packet loss[0..1]
	virtual float GetAvgChoke(int flow) const = 0; // avg packet choke[0..1]
	virtual float GetAvgData(int flow) const = 0; // data flow in bytes/sec
	virtual float GetAvgPackets(int flow) const = 0; // avg packets/sec
	virtual int GetTotalData(int flow) const = 0; // total flow in/out in bytes
	virtual int GetSequenceNr(int flow) const = 0; // last send seq number
	virtual bool IsValidPacket(int flow, int frame_number) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float GetPacketTime(int flow, int frame_number) const = 0; // time when packet was send
	virtual int GetPacketBytes(int flow, int frame_number, int group) const = 0; // group size of this packet
	virtual bool GetStreamProgress(int flow, int* received, int* total) const = 0; // TCP progress if transmitting
	virtual float GetTimeSinceLastReceived(void) const = 0; // get time since last recieved packet in seconds
	virtual float GetCommandInterpolationAmount(int flow, int frame_number) const = 0;

	virtual void GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;

	virtual void GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;

	virtual float GetTimeoutSeconds() const = 0;

	BYTE pad[24];
	int OutSequenceNr;
	int InSequenceNr;
	int OutSequenceNrAck;
	int OutReliableState;
	int InReliableState;
	int chokedPackets;
};

struct matrix3x4_t
{
	matrix3x4_t()
	{}

	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;
		m_flMatVal[0][1] = m01;
		m_flMatVal[0][2] = m02;
		m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;
		m_flMatVal[1][1] = m11;
		m_flMatVal[1][2] = m12;
		m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;
		m_flMatVal[2][1] = m21;
		m_flMatVal[2][2] = m22;
		m_flMatVal[2][3] = m23;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	void Init(const vec& xAxis, const vec& yAxis, const vec& zAxis, const vec& vecOrigin)
	{
		m_flMatVal[0][0] = xAxis.x;
		m_flMatVal[0][1] = yAxis.x;
		m_flMatVal[0][2] = zAxis.x;
		m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.z;
		m_flMatVal[1][1] = yAxis.z;
		m_flMatVal[1][2] = zAxis.z;
		m_flMatVal[1][3] = vecOrigin.z;
		m_flMatVal[2][0] = xAxis.y;
		m_flMatVal[2][1] = yAxis.y;
		m_flMatVal[2][2] = zAxis.y;
		m_flMatVal[2][3] = vecOrigin.y;
	}

	//-----------------------------------------------------------------------------
	// Creates a matrix where the X axis = forward
	// the Y axis = left, and the Z axis = up
	//-----------------------------------------------------------------------------
	matrix3x4_t(const vec& xAxis, const vec& yAxis, const vec& zAxis, const vec& vecOrigin)
	{
		Init(xAxis, yAxis, zAxis, vecOrigin);
	}

	inline void SetOrigin(vec const& p)
	{
		m_flMatVal[0][3] = p.x;
		m_flMatVal[1][3] = p.y;
		m_flMatVal[2][3] = p.z;
	}

	inline vec GetOrigin() {
		return vec{
			m_flMatVal[0][3],
			m_flMatVal[1][3],
			m_flMatVal[2][3],
		};
	}

	float* operator[](int i)
	{
		return m_flMatVal[i];
	}

	const float* operator[](int i) const
	{
		return m_flMatVal[i];
	}

	float* Base()
	{
		return &m_flMatVal[0][0];
	}

	const float* Base() const
	{
		return &m_flMatVal[0][0];
	}

	float m_flMatVal[3][4];
};

class IEngine {
public:
	virtual int GetIntersectingSurfaces(const model_t* model, const vec& vCenter, const float radius, const bool bOnlyVisibleSurfaces, SurfInfo* pInfos, const int nMaxInfos) = 0;
	virtual vec GetLightForPoint(const vec& pos, bool bClamp) = 0;
	virtual IMaterial* TraceLineMaterialAndLighting(const vec& start, const vec& end, vec& diffuseLightColor, vec& baseColor) = 0;
	virtual const char* ParseFile(const char* data, char* token, int maxlen) = 0;
	virtual bool CopyFile(const char* source, const char* destination) = 0;
	virtual void GetScreenSize(int& width, int& height) = 0;
	virtual void ServerCmd(const char* szCmdString, bool bReliable = true) = 0;
	virtual void ClientCmd(const char* szCmdString) = 0;
	virtual bool GetPlayerInfo(int ent_num, player_info_t* pinfo) = 0; //8
	virtual int GetPlayerForUserID(int userID) = 0;
	virtual client_textmessage_t* TextMessageGet(const char* pName) = 0; // 10
	virtual bool Con_IsVisible(void) = 0;
	virtual int GetLocalPlayer(void) = 0;
	virtual const model_t* LoadModel(const char* pName, bool bProp = false) = 0;
	virtual float GetLastTimeStamp(void) = 0;
	virtual CSentence* GetSentence(CAudioSource* pAudioSource) = 0; // 15
	virtual float GetSentenceLength(CAudioSource* pAudioSource) = 0;
	virtual bool IsStreaming(CAudioSource* pAudioSource) const = 0;
	virtual void GetViewAngles(angle& va) = 0;
	virtual void SetViewAngles(angle& va) = 0;
	virtual int GetMaxClients(void) = 0; // 20
	virtual const char* Key_LookupBinding(const char* pBinding) = 0;
	virtual const char* Key_BindingForKey(int& code) = 0;
	virtual void Key_SetBinding(int, char const*) = 0;
	virtual void StartKeyTrapMode(void) = 0;
	virtual bool CheckDoneKeyTrapping(int& code) = 0;
	virtual bool IsInGame(void) = 0;
	virtual bool IsConnected(void) = 0;
	virtual bool IsDrawingLoadingImage(void) = 0;
	virtual void HideLoadingPlaque(void) = 0;
	virtual void Con_NPrintf(int pos, const char* fmt, ...) = 0; // 30
	virtual void Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) = 0;
	virtual int IsBoxVisible(const vec& mins, const vec& maxs) = 0;
	virtual int IsBoxInViewCluster(const vec& mins, const vec& maxs) = 0;
	virtual bool CullBox(const vec& mins, const vec& maxs) = 0;
	virtual void Sound_ExtraUpdate(void) = 0;
	virtual const char* GetGameDirectory(void) = 0;
	virtual const Matrix4x4& WorldToScreenMatrix() = 0;
	virtual const Matrix4x4& WorldToViewMatrix() = 0;
	virtual int GameLumpVersion(int lumpId) const = 0;
	virtual int GameLumpSize(int lumpId) const = 0; // 40
	virtual bool LoadGameLump(int lumpId, void* pBuffer, int size) = 0;
	virtual int LevelLeafCount() const = 0;
	virtual ISpatialQuery* GetBSPTreeQuery() = 0;
	virtual void LinearToGamma(float* linear, float* gamma) = 0;
	virtual float LightStyleValue(int style) = 0; // 45
	virtual void ComputeDynamicLighting(const vec& pt, const vec* pNormal, vec& color) = 0;
	virtual void GetAmbientLightColor(vec& color) = 0;
	virtual int GetDXSupportLevel() = 0;
	virtual bool SupportsHDR() = 0;
	virtual void Mat_Stub(IMaterialSystem* pMatSys) = 0; // 50
	virtual void GetChapterName(char* pchBuff, int iMaxLength) = 0;
	virtual char const* GetLevelName(void) = 0;
	virtual char const* GetLevelNameShort(void) = 0;
	virtual char const* GetMapGroupName(void) = 0;
	virtual struct IVoiceTweak_s* GetVoiceTweakAPI(void) = 0;
	virtual void SetVoiceCasterID(unsigned int someint) = 0; // 56
	virtual void EngineStats_BeginFrame(void) = 0;
	virtual void EngineStats_EndFrame(void) = 0;
	virtual void FireEvents() = 0;
	virtual int GetLeavesArea(unsigned short* pLeaves, int nLeaves) = 0;
	virtual bool DoesBoxTouchAreaFrustum(const vec& mins, const vec& maxs, int iArea) = 0; // 60
	virtual int GetFrustumList(Frustum_t** pList, int listMax) = 0;
	virtual bool ShouldUseAreaFrustum(int i) = 0;
	virtual void SetAudioState(const AudioState_t& state) = 0;
	virtual int SentenceGroupPick(int groupIndex, char* name, int nameBufLen) = 0;
	virtual int SentenceGroupPickSequential(int groupIndex, char* name, int nameBufLen, int sentenceIndex, int reset) = 0;
	virtual int SentenceIndexFromName(const char* pSentenceName) = 0;
	virtual const char* SentenceNameFromIndex(int sentenceIndex) = 0;
	virtual int SentenceGroupIndexFromName(const char* pGroupName) = 0;
	virtual const char* SentenceGroupNameFromIndex(int groupIndex) = 0;
	virtual float SentenceLength(int sentenceIndex) = 0;
	virtual void ComputeLighting(const vec& pt, const vec* pNormal, bool bClamp, vec& color, vec* pBoxColors = NULL) = 0;
	virtual void ActivateOccluder(int nOccluderIndex, bool bActive) = 0;
	virtual bool IsOccluded(const vec& vecAbsMins, const vec& vecAbsMaxs) = 0; // 74
	virtual int GetOcclusionViewId(void) = 0;
	virtual void* SaveAllocMemory(size_t num, size_t size) = 0;
	virtual void SaveFreeMemory(void* pSaveMem) = 0;
	virtual INetChannelInfo* GetNetChannelInfo(void) = 0;
	virtual void DebugDrawPhysCollide(const CPhysCollide* pCollide, IMaterial* pMaterial, const matrix3x4_t& transform, const Color& color) = 0; //79
	virtual void CheckPoint(const char* pName) = 0; // 80
	virtual void DrawPortals() = 0;
	virtual bool IsPlayingDemo(void) = 0;
	virtual bool IsRecordingDemo(void) = 0;
	virtual bool IsPlayingTimeDemo(void) = 0;
	virtual int GetDemoRecordingTick(void) = 0;
	virtual int GetDemoPlaybackTick(void) = 0;
	virtual int GetDemoPlaybackStartTick(void) = 0;
	virtual float GetDemoPlaybackTimeScale(void) = 0;
	virtual int GetDemoPlaybackTotalTicks(void) = 0;
	virtual bool IsPaused(void) = 0; // 90
	virtual float GetTimescale(void) const = 0;
	virtual bool IsTakingScreenshot(void) = 0;
	virtual bool IsHLTV(void) = 0;
	virtual bool IsLevelMainMenuBackground(void) = 0;
	virtual void GetMainMenuBackgroundName(char* dest, int destlen) = 0;
	virtual void SetOcclusionParameters(const int /*OcclusionParams_t*/& params) = 0; // 96
	virtual void GetUILanguage(char* dest, int destlen) = 0;
	virtual int IsSkyboxVisibleFromPoint(const vec& vecPoint) = 0;
	virtual const char* GetMapEntitiesString() = 0;
	virtual bool IsInEditMode(void) = 0; // 100
	virtual float GetScreenAspectRatio(int viewportWidth, int viewportHeight) = 0;
	virtual bool REMOVED_SteamRefreshLogin(const char* password, bool isSecure) = 0; // 100
	virtual bool REMOVED_SteamProcessCall(bool& finished) = 0;
	virtual unsigned int GetEngineBuildNumber() = 0; // engines build
	virtual const char* GetProductVersionString() = 0; // mods version number (steam.inf)
	virtual void GrabPreColorCorrectedFrame(int x, int y, int width, int height) = 0;
	virtual bool IsHammerRunning() const = 0;
	virtual void ExecuteClientCmd(const char* szCmdString) = 0; //108
	virtual bool MapHasHDRLighting(void) = 0;
	virtual bool MapHasLightMapAlphaData(void) = 0;
	virtual int GetAppID() = 0;
	virtual vec GetLightForPointFast(const vec& pos, bool bClamp) = 0;
	virtual void ClientCmd_Unrestricted1(char const*, int, bool);
	virtual void ClientCmd_Unrestricted(const char* szCmdString, const char* newFlag = 0) = 0; // 114, new flag, quick testing shows setting 0 seems to work, haven't looked into it.
	//Forgot to add this line, but make sure to format all unrestricted calls now with an extra , 0
	//Ex:
	//	I::Engine->ClientCmd_Unrestricted( charenc( "cl_mouseenable 1" ) , 0);
	//	I::Engine->ClientCmd_Unrestricted( charenc( "crosshair 1" ) , 0);
	virtual void SetRestrictServerCommands(bool bRestrict) = 0;
	virtual void SetRestrictClientCommands(bool bRestrict) = 0;
	virtual void SetOverlayBindProxy(int iOverlayID, void* pBindProxy) = 0;
	virtual bool CopyFrameBufferToMaterial(const char* pMaterialName) = 0;
	virtual void ReadConfiguration(const int iController, const bool readDefault) = 0;
	virtual void SetAchievementMgr(IAchievementMgr* pAchievementMgr) = 0;
	virtual IAchievementMgr* GetAchievementMgr() = 0;
	virtual bool MapLoadFailed(void) = 0;
	virtual void SetMapLoadFailed(bool bState) = 0;
	virtual bool IsLowViolence() = 0;
	virtual const char* GetMostRecentSaveGame(void) = 0;
	virtual void SetMostRecentSaveGame(const char* lpszFilename) = 0;
	virtual void StartXboxExitingProcess() = 0;
	virtual bool IsSaveInProgress() = 0;
	virtual bool IsAutoSaveDangerousInProgress(void) = 0;
	virtual unsigned int OnStorageDeviceAttached(int iController) = 0;
	virtual void OnStorageDeviceDetached(int iController) = 0;
	virtual char* const GetSaveDirName(void) = 0;
	virtual void WriteScreenshot(const char* pFilename) = 0;
	virtual void ResetDemoInterpolation(void) = 0;
	virtual int GetActiveSplitScreenPlayerSlot() = 0;
	virtual int SetActiveSplitScreenPlayerSlot(int slot) = 0;
	virtual bool SetLocalPlayerIsResolvable(char const* pchContext, int nLine, bool bResolvable) = 0;
	virtual bool IsLocalPlayerResolvable() = 0;
	virtual int GetSplitScreenPlayer(int nSlot) = 0;
	virtual bool IsSplitScreenActive() = 0;
	virtual bool IsValidSplitScreenSlot(int nSlot) = 0;
	virtual int FirstValidSplitScreenSlot() = 0; // -1 == invalid
	virtual int NextValidSplitScreenSlot(int nPreviousSlot) = 0; // -1 == invalid
	virtual ISPSharedMemory* GetSinglePlayerSharedMemorySpace(const char* szName, int ent_num = (1 << 11)) = 0;
	virtual void ComputeLightingCube(const vec& pt, bool bClamp, vec* pBoxColors) = 0;
	virtual void RegisterDemoCustomDataCallback(const char* szCallbackSaveID, pfnDemoCustomDataCallback pCallback) = 0;
	virtual void RecordDemoCustomData(pfnDemoCustomDataCallback pCallback, const void* pData, size_t iDataLength) = 0;
	virtual void SetPitchScale(float flPitchScale) = 0;
	virtual float GetPitchScale(void) = 0;
	virtual bool LoadFilmmaker() = 0;
	virtual void UnloadFilmmaker() = 0;
	virtual void SetLeafFlag(int nLeafIndex, int nFlagBits) = 0;
	virtual void RecalculateBSPLeafFlags(void) = 0;
	virtual bool DSPGetCurrentDASRoomNew(void) = 0;
	virtual bool DSPGetCurrentDASRoomChanged(void) = 0;
	virtual bool DSPGetCurrentDASRoomSkyAbove(void) = 0;
	virtual float DSPGetCurrentDASRoomSkyPercent(void) = 0;
	virtual void SetMixGroupOfCurrentMixer(const char* szgroupname, const char* szparam, float val, int setMixerType) = 0;
	virtual int GetMixLayerIndex(const char* szmixlayername) = 0;
	virtual void SetMixLayerLevel(int index, float level) = 0;
	virtual int GetMixGroupIndex(char const* groupname) = 0;
	virtual void SetMixLayerTriggerFactor(int i1, int i2, float fl) = 0;
	virtual void SetMixLayerTriggerFactor(char const* char1, char const* char2, float fl) = 0;
	virtual bool IsCreatingReslist() = 0;
	virtual bool IsCreatingXboxReslist() = 0;
	virtual void SetTimescale(float flTimescale) = 0;
	virtual void SetGamestatsData(CGamestatsData* pGamestatsData) = 0;
	virtual CGamestatsData* GetGamestatsData() = 0;
	virtual void GetMouseDelta(int& dx, int& dy, bool b) = 0; // unknown
	virtual const char* Key_LookupBindingEx(const char* pBinding, int iUserId = -1, int iStartCount = 0, int iAllowJoystick = -1) = 0;
	virtual int Key_CodeForBinding(char const*, int, int, int) = 0;
	virtual void UpdateDAndELights(void) = 0;
	virtual int GetBugSubmissionCount() const = 0;
	virtual void ClearBugSubmissionCount() = 0;
	virtual bool DoesLevelContainWater() const = 0;
	virtual float GetServerSimulationFrameTime() const = 0;
	virtual void SolidMoved(class IClientEntity* pSolidEnt, class ICollideable* pSolidCollide, const vec* pPrevAbsOrigin, bool accurateBboxTriggerChecks) = 0;
	virtual void TriggerMoved(class IClientEntity* pTriggerEnt, bool accurateBboxTriggerChecks) = 0;
	virtual void ComputeLeavesConnected(const vec& vecOrigin, int nCount, const int* pLeafIndices, bool* pIsConnected) = 0;
	virtual bool IsInCommentaryMode(void) = 0;
	virtual void SetBlurFade(float amount) = 0;
	virtual bool IsTransitioningToLoad() = 0;
	virtual void SearchPathsChangedAfterInstall() = 0;
	virtual void ConfigureSystemLevel(int nCPULevel, int nGPULevel) = 0;
	virtual void SetConnectionPassword(char const* pchCurrentPW) = 0;
	virtual CSteamAPIContext* GetSteamAPIContext() = 0;
	virtual void SubmitStatRecord(char const* szMapName, unsigned int uiBlobVersion, unsigned int uiBlobSize, const void* pvBlob) = 0;
	virtual void ServerCmdKeyValues(KeyValues* pKeyValues) = 0; // 203
	virtual void SpherePaintSurface(const model_t* model, const vec& location, unsigned char chr, float fl1, float fl2) = 0;
	virtual bool HasPaintmap(void) = 0;
	virtual void EnablePaintmapRender() = 0;
	//virtual void                TracePaintSurface( const model_t *model, const vec& position, float radius, CUtlvec<Color>& surfColors ) = 0;
	virtual void SphereTracePaintSurface(const model_t* model, const vec& position, const vec& vec2, float radius, /*CUtlvec<unsigned char, CUtlMemory<unsigned char, int>>*/ int& utilVecShit) = 0;
	virtual void RemoveAllPaint() = 0;
	virtual void PaintAllSurfaces(unsigned char uchr) = 0;
	virtual void RemovePaint(const model_t* model) = 0;
	virtual bool IsActiveApp() = 0;
	virtual bool IsClientLocalToActiveServer() = 0;
	virtual void TickProgressBar() = 0;
	virtual InputContextHandle_t GetInputContext(int /*EngineInputContextId_t*/ id) = 0;
	virtual void GetStartupImage(char* filename, int size) = 0;
	virtual bool IsUsingLocalNetworkBackdoor(void) = 0;
	virtual void SaveGame(const char*, bool, char*, int, char*, int) = 0;
	virtual void GetGenericMemoryStats( /* GenericMemoryStat_t */ void**) = 0;
	virtual bool GameHasShutdownAndFlushedMemory(void) = 0;
	virtual int GetLastAcknowledgedCommand(void) = 0;
	virtual void FinishContainerWrites(int i) = 0;
	virtual void FinishAsyncSave(void) = 0;
	virtual int GetServerTick(void) = 0;
	virtual const char* GetModDirectory(void) = 0;
	virtual bool AudioLanguageChanged(void) = 0;
	virtual bool IsAutoSaveInProgress(void) = 0;
	virtual void StartLoadingScreenForCommand(const char* command) = 0;
	virtual void StartLoadingScreenForKeyValues(KeyValues* values) = 0;
	virtual void SOSSetOpvarFloat(const char*, float) = 0;
	virtual void SOSGetOpvarFloat(const char*, float&) = 0;
	virtual bool IsSubscribedMap(const char*, bool) = 0;
	virtual bool IsFeaturedMap(const char*, bool) = 0;
	virtual CDemoPlaybackParameters_t* GetDemoPlaybackParameters(void) = 0;//218
	virtual int GetClientVersion(void) = 0;
	virtual bool IsDemoSkipping(void) = 0;
	virtual void SetDemoImportantEventData(const KeyValues* values) = 0;
	virtual void ClearEvents(void) = 0;
	virtual int GetSafeZoneXMin(void) = 0;
	virtual bool IsVoiceRecording(void) = 0;
	virtual void ForceVoiceRecordOn(void) = 0;
	virtual bool IsReplay(void) = 0;
};

struct RecvProp;

struct RecvTable
{
	RecvProp* m_pProps;
	int m_nProps;
	void* m_pDecoder;
	char* m_pNetTableName;
	bool m_bInitialized;
	bool m_bInMainList;
};

enum ClassID {
	BaseCSGrenadeProjectile = 9,
	BreachChargeProjectile = 29,
	BumpMineProjectile = 33,
	C4,
	Chicken = 36,
	CSPlayer = 40,
	CSPlayerResource,
	CSRagdoll = 42,
	Deagle = 46,
	DecoyProjectile = 48,
	Drone,
	Dronegun,
	DynamicProp = 52,
	EconEntity = 53,
	EconWearable,
	Hostage = 97,
	Inferno = 100,
	Healthshot = 104,
	Cash,
	Knife = 107,
	KnifeGG,
	MolotovProjectile = 114,
	AmmoBox = 125,
	LootCrate,
	RadarJammer,
	WeaponUpgrade,
	PlantedC4,
	PropDoorRotating = 143,
	SensorGrenadeProjectile = 153,
	SmokeGrenadeProjectile = 157,
	SnowballPile = 160,
	SnowballProjectile,
	Tablet = 172,
	Aug = 232,
	Awp,
	Elite = 239,
	FiveSeven = 241,
	G3sg1,
	Glock = 245,
	P2000,
	P250 = 258,
	Scar20 = 261,
	Sg553 = 265,
	Ssg08 = 267,
	Tec9 = 269
};

struct ClientClass
{
	void* m_pCreateFn;//0
	void* m_pCreateEventFn;//4
	char* m_pNetworkName;//8
	RecvTable* m_pRecvTable;//C
	ClientClass* m_pNext;//10
	ClassID m_ClassID;//14
};

struct RecvProp
{
	char* m_pVarName;
	int m_RecvType;
	int m_Flags;
	int m_StringBufferSize;
	bool m_bInsideArray;
	const void* m_pExtraData;
	RecvProp* m_pArrayProp;
	void* m_ArrayLengthProxy;
	void* m_ProxyFn;
	void* m_DataTableProxyFn;
	RecvTable* m_pDataTable;
	int m_Offset;
	int m_ElementStride;
	int m_nElements;
	const char* m_pParentArrayPropName;
};

class IClientEntityList
{
public:
	virtual void Function0() = 0;
	virtual void Function1() = 0;
	virtual void Function2() = 0;

	virtual Entity* GetClientEntity(int iIndex) = 0;
	virtual Entity* GetClientEntityFromHandle(int hHandle) = 0;
	virtual int NumberOfEntities(bool bIncludeNonNetworkable) = 0;
	virtual int GetHighestEntityIndex() = 0;
};

class IInputSystem
{
public:
	void enableInput(bool enable) {
		typedef void(__thiscall* OriginalFn)(void*, bool);
		return GetVFunc< OriginalFn >(this, 11)(this, enable);
	}

	void resetInputState() {
		typedef void(__thiscall* OriginalFn)(void*);
		return GetVFunc< OriginalFn >(this, 39)(this);
	}
};

struct AppSystemInfo_t {
	const char* m_pModuleName;
	const char* m_pInterfaceName;
};

enum InitReturnVal_t {
	INIT_FAILED = 0,
	INIT_OK,

	INIT_LAST_VAL,
};

enum AppSystemTier_t {
	APP_SYSTEM_TIER0 = 0,
	APP_SYSTEM_TIER1,
	APP_SYSTEM_TIER2,
	APP_SYSTEM_TIER3,

	APP_SYSTEM_TIER_OTHER,
};

class IAppSystem3 {
public:
	// Here's where the app systems get to learn about each other
	virtual bool Connect(void* factory) = 0;

	virtual void Disconnect() = 0;

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void* QueryInterface(const char* pInterfaceName) = 0;

	// Init, shutdown
	virtual InitReturnVal_t Init() = 0;

	virtual void Shutdown() = 0;

	// Returns all dependent libraries
	virtual const AppSystemInfo_t* GetDependencies() = 0;

	// Returns the tier
	virtual AppSystemTier_t GetTier() = 0;

	// Reconnect to a particular interface
	virtual void Reconnect(void* factory,
		const char* pInterfaceName) = 0;

	// Returns whether or not the app system is a singleton
	virtual bool IsSingleton() = 0;
};

class KeyValues {
public:
	BYTE unk[36];

	inline const char* GetString(const char* key, const char* def = "") {
		typedef const char* (__thiscall* OriginalFn)(PVOID, const char*, const char*);
		return ((OriginalFn)M::KeyValues_ReadString)(this, key, def);
	}
};

class match_extensions;
class match_events_subscription;
class match_title;
class match_network_msg_controller;
class match_system;

class PlayerLocal {
public:
	int get_xuid_low() {
		return *(int*)((DWORD)this + 0x8);
	}
	int get_xuid_high() {
		return *(int*)((DWORD)this + 0xC);
	}
private:
	virtual void unk_1() = 0; //0
	virtual void unk_2() = 0; //1
public:
	virtual const char* get_name() = 0; //2
};

class PlayerManager {
private:
	virtual void unk_1() = 0; //0
public:
	virtual PlayerLocal* get_local_player(int un) = 0; //1
};

class MatchSession {
public:
	virtual PlayerManager* get_player_manager() = 0; //0
	virtual KeyValues* get_session_settings() = 0; //1
	virtual void UpdateSessionSettings(KeyValues* pSettings) = 0; //2
	virtual void command(KeyValues* command) = 0; //3
	virtual uint64_t get_lobby_id() = 0; //4
};

class MatchFramework : IAppSystem3 {
public:
	virtual void run_frame() = 0; //9
	virtual match_extensions* get_match_extensions() = 0; //10
	virtual match_extensions* get_events_subscription() = 0; //11
	virtual match_title* get_match_title() = 0; //12
	virtual MatchSession* get_match_session() = 0; //13
	virtual match_network_msg_controller* get_msg_controller() = 0; //14
	virtual match_system* get_match_system() = 0; //15
	virtual void create_session(KeyValues* settings) = 0; //16
	virtual void match_sessions(KeyValues* settings) = 0; //17
	virtual void accept_invite(int controller) = 0; //18
	virtual void close_session() = 0; //19
};

class IGameEvent {
public:

	virtual ~IGameEvent() = 0;
	virtual const char* GetName() const = 0;

	virtual bool  IsReliable() const = 0;
	virtual bool  IsLocal() const = 0;
	virtual bool  IsEmpty(const char* keyName = NULL) = 0;

	virtual bool  GetBool(const char* keyName = NULL, bool defaultValue = false) = 0;
	virtual int   GetInt(const char* keyName = NULL, int defaultValue = 0) = 0;
	virtual unsigned long GetUint64(const char* keyName = NULL, unsigned long defaultValue = 0) = 0;
	virtual float GetFloat(const char* keyName = NULL, float defaultValue = 0.0f) = 0;
	virtual const char* GetString(const char* keyName = NULL, const char* defaultValue = "") = 0;
	virtual const wchar_t* GetWString(const char* keyName, const wchar_t* defaultValue = L"") = 0;

	virtual void SetBool(const char* keyName, bool value) = 0;
	virtual void SetInt(const char* keyName, int value) = 0;
	virtual void SetUint64(const char* keyName, unsigned long value) = 0;
	virtual void SetFloat(const char* keyName, float value) = 0;
	virtual void SetString(const char* keyName, const char* value) = 0;
	virtual void SetWString(const char* keyName, const wchar_t* value) = 0;
};

class IGameEventListener {
public:
	virtual ~IGameEventListener() {}
	virtual void FireGameEvent(IGameEvent* Event) = 0;
	virtual int GetEventDebugID() { return 42; }
};

class bf_read;// {
//public:
//	uintptr_t base_address;
//	uintptr_t cur_offset;
//
//	bf_read(uintptr_t addr) {
//		base_address = addr;
//		cur_offset = 0;
//	}
//
//	void SetOffset(uintptr_t offset) {
//		cur_offset = offset;
//	}
//
//	void Skip(uintptr_t length) {
//		cur_offset += length;
//	}
//
//	int ReadByte() {
//		auto val = *reinterpret_cast<char*>(base_address + cur_offset);
//		++cur_offset;
//		return val;
//	}
//
//	bool ReadBool() {
//		auto val = *reinterpret_cast<bool*>(base_address + cur_offset);
//		++cur_offset;
//		return val;
//	}
//
//	std::string ReadString() {
//		char buffer[256];
//		auto str_length = *reinterpret_cast<char*>(base_address + cur_offset);
//		++cur_offset;
//		memcpy(buffer, reinterpret_cast<void*>(base_address + cur_offset), str_length > 255 ? 255 : str_length);
//		buffer[str_length > 255 ? 255 : str_length] = '\0';
//		cur_offset += str_length + 1;
//		return std::string(buffer);
//	}
//};

class bf_write;

class IGameEventManager {
public:
	virtual int __Unknown_1(int* dwUnknown) = 0;

	// load game event descriptions from a file eg "resource\gameevents.res"
	virtual int LoadEventsFromFile(const char* filename) = 0;

	// removes all and anything
	virtual void Reset() = 0;

	// adds a listener for a particular event
	virtual bool AddListener(IGameEventListener* listener, const char* name, bool bServerSide = false) = 0;

	// returns true if this listener is listens to given event
	virtual bool FindListener(IGameEventListener* listener, const char* name) = 0;

	// removes a listener 
	virtual int RemoveListener(IGameEventListener* listener) = 0;

	// create an event by name, but doesn't fire it. returns NULL is event is not
	// known or no listener is registered for it. bForce forces the creation even if no listener is active
	virtual IGameEvent* CreateEvent(const char* name, bool bForce, unsigned int dwUnknown) = 0;

	// fires a server event created earlier, if bDontBroadcast is set, event is not send to clients
	virtual bool FireEvent(IGameEvent* event, bool bDontBroadcast = false) = 0;

	// fires an event for the local client only, should be used only by client code
	virtual bool FireEventClientSide(IGameEvent* event) = 0;

	// create a new copy of this event, must be free later
	virtual IGameEvent* DuplicateEvent(IGameEvent* event) = 0;

	// if an event was created but not fired for some reason, it has to bee freed, same UnserializeEvent
	virtual void FreeEvent(IGameEvent* event) = 0;

	// write/read event to/from bitbuffer
	virtual bool SerializeEvent(IGameEvent* event, bf_write* buf) = 0;

	// create new KeyValues, must be deleted
	virtual IGameEvent* UnserializeEvent(bf_read* buf) = 0;
};

enum CSGOGameUIState_t : int {
	CSGO_GAME_UI_STATE_INVALID = 0,
	CSGO_GAME_UI_STATE_LOADINGSCREEN,
	CSGO_GAME_UI_STATE_INGAME,
	CSGO_GAME_UI_STATE_MAINMENU,
	CSGO_GAME_UI_STATE_PAUSEMENU,
	CSGO_GAME_UI_STATE_INTROMOVIE,
};

class CGameUI {
private:
	BYTE gap0[484];
public:
	CSGOGameUIState_t m_CSGOGameUIState;
};

enum ClientFrameStage_t {
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};

class IBaseClientDll {
public:
	ClientClass* GetAllClasses() {
		typedef ClientClass* (__thiscall* OriginalFn)(PVOID);
		return GetVFunc< OriginalFn >(this, 8)(this);
	}

	bool dispatchUserMessage(int messageType, int arg, int arg1, void* data) {
		typedef bool(__thiscall* OriginalFn)(PVOID, int, int, int, void*);
		return GetVFunc< OriginalFn >(this, 38)(this, messageType, arg, arg1, data);
	}
};

class IUIEvent;
class IUIEngineFrameListener;
class IUIPanelStyle;
class IUIJSObject;
class IUIWindow;
class IUISettings;
class IUITextLayout;
class IUIInputEngine;
class IUILocalize;
class IUISoundSystem;
class IUISettings;
class IUILayoutManager;
class IUIFileSystem;
class IUIPanelClient;

// E Prefix for enums?
class EFontStyle;
class EFontWeight;
class ETextAlign;
class EPanelRepaint;
class EStyleRepaint;
class EStyleFlags;
class EFocusMoveDirection;
class EMouseCanActivate;
class EAnimationTimingFunction;
class EAnimationDirection;

class CUILength;
class CLayoutFile;
typedef unsigned short CPanoramaSymbol;
class PanelHandle_t;
class CJSONWebAPIParams;
class UIEventFactory;
class EPanelEventSource_t;
class CPanel2DFactory;
class RegisterJSType_t;
class RegisterJSScopeInfo_t;
class RegisterJSEntryInfo_t;

class CUtlAbstractDelegate;

struct ScrollBehavior_t;

class IUIPanel {
public:
	virtual void DESTROY1(void) = 0;
	virtual void Initialize(IUIWindow*, IUIPanel*, const char*, unsigned int) = 0;
	virtual void InitClonedPanel(IUIPanel* other) = 0;
	virtual void RegisterEventHandlersOnPanel2DType(CPanoramaSymbol) = 0;
	virtual void Shutdown(void) = 0;
	virtual void FirePanelLoadedEvent(void) = 0;
	virtual void SetClientPtr(IUIPanelClient*) = 0;
	virtual IUIPanelClient* ClientPtr() = 0;
	virtual void SetID(const char* id) = 0;
	virtual const char* GetID(void) = 0; // ID in the XML, (EX: <Panel id="ShopCourierControls">)
	virtual void GetPanelType(void) = 0;
	virtual bool HasID() = 0;
	virtual void unk() = 0;
	virtual void unk2() = 0;
	virtual void unk3() = 0;
	virtual void unk4() = 0;
	virtual void unk5() = 0;
	virtual bool LoadLayout(const char* pathToXML, bool, bool) = 0;
	virtual bool LoadLayoutFromString(const char* layoutXML, bool, bool) = 0;
	virtual void LoadLayoutAsync(const char* pathToXML, bool, bool) = 0;
	virtual void LoadLayoutFromStringAsync(const char* layoutXML, bool, bool) = 0;
	virtual bool CreateChildren(const char*) = 0;
	virtual void UnloadLayout(void) = 0;
	virtual bool IsLoaded(void) = 0;
	virtual void SetParent(IUIPanel* parent) = 0;
	virtual IUIPanel* GetParent(void) = 0;
	virtual IUIWindow* GetParentWindow(void) = 0;
	virtual void SetVisible(bool state) = 0;
	virtual bool IsVisible(void) = 0;
	virtual bool IsTransparent(void) = 0;
	virtual void SetLayoutLoadedFromParent(IUIPanel*) = 0;
	virtual void SetPanelIntoContext(IUIPanel*) = 0;
	virtual void* GetLayoutFile(void) = 0;
	virtual void GetLayoutFileLoadedFrom(void) = 0;
	virtual int GetLayoutFileReloadCount(void) = 0;
	virtual void unk6() = 0;
	virtual void unk7() = 0;
	virtual void unk8() = 0;
	virtual IUIPanel* FindChild(const char*) = 0;
	virtual void unk9() = 0;
	virtual IUIPanel* FindChildTraverse(const char*) = 0;
	virtual void unk10() = 0;
	virtual IUIPanel* FindChildInLayoutFile(const char*) = 0;
	virtual void unk11() = 0;
	virtual IUIPanel* FindPanelInLayoutFile(const char*) = 0;
	virtual bool IsDecendantOf(IUIPanel const* const) = 0;
	virtual void RemoveAndDeleteChildren(void) = 0;
	virtual void RemoveAndDeleteChildrenOfType(CPanoramaSymbol) = 0;
	virtual int GetChildCount(void) = 0;
	virtual IUIPanel* GetChild(int) = 0;
	virtual IUIPanel* GetFirstChild(void) = 0;
	virtual IUIPanel* GetLastChild(void) = 0;
	virtual int GetChildIndex(IUIPanel const* const) = 0;
	virtual int GetChildCountOfType(CPanoramaSymbol) = 0;
	virtual int GetHiddenChildCount(void) = 0;
	virtual IUIPanel* GetHiddenChild(int) = 0;
	virtual IUIPanel* FindAncestor(const char*) = 0;
	virtual void sub_131D10() = 0; // new function, added around august 2018
	virtual void SetRepaint(EPanelRepaint) = 0;
	virtual void SetRepaintUpParentChain(void* unk) = 0;
	virtual bool ShouldDrawChildren(IUIPanel*) = 0;
	virtual void EnableBackgroundMovies(bool state) = 0;
	virtual void* AccessIUIStyle() = 0;
	virtual IUIPanelStyle* AccessIUIStyleDirty() = 0;
	virtual void ApplyStyles(bool apply) = 0;
	virtual void AfterStylesApplied(bool, EStyleRepaint, bool, bool) = 0;
	virtual void SetOnStylesChangedNeeded(void) = 0;
	virtual void* AccessChildren(void) = 0;
	virtual void DesiredLayoutSizeTraverse(float, float) = 0;
	virtual void DesiredLayoutSizeTraverse(float*, float*, float, float, bool) = 0;
	virtual void OnContentSizeTraverse(float*, float*, float, float, bool) = 0;
	virtual void LayoutTraverse(float, float, float, float) = 0;
	virtual void OnLayoutTraverse(float, float) = 0;
	virtual void SetPositionFromLayoutTraverse(CUILength, CUILength, CUILength) = 0;
	virtual void InvalidateSizeAndPosition(void) = 0;
	virtual void InvalidatePosition(void) = 0;
	virtual void SetActiveSizeAndPositionTransition(void) = 0;
	virtual void SetActivePositionTransition(void) = 0;
	virtual bool IsSizeValid(void) = 0;
	virtual bool IsPositionValid(void) = 0;
	virtual bool IsChildSizeValid(void) = 0;
	virtual bool IsChildPositionValid(void) = 0;
	virtual bool IsSizeTransitioning(void) = 0;
	virtual bool IsPositionTransitioning(void) = 0;
	virtual bool IsChildPositionTransitioning(void) = 0;
	virtual bool IsChildSizeTransitioning(void) = 0;
	virtual void TransitionPositionApplied(bool) = 0;
	virtual void sub_140EE0() = 0;
	virtual void sub_140F00() = 0;
	virtual float GetDesiredLayoutWidth(void) = 0; // these might be doubles, need to "double"-check
	virtual float GetDesiredLayoutHeight(void) = 0;
	virtual float GetContentWidth(void) = 0;
	virtual float GetContentHeight(void) = 0;
	virtual float GetActualLayoutWidth(void) = 0;
	virtual float GetActualLayoutHeight(void) = 0;
	virtual float GetActualRenderWidth(void) = 0;
	virtual float GetActualRenderHeight(void) = 0;
	virtual float GetActualXOffset(void) = 0;
	virtual float GetActualYOffset(void) = 0;
	virtual float GetRawActualXOffset(void) = 0;
	virtual float GetRawActualYOffset(void) = 0;
	virtual void unk12() = 0;
	virtual void sub_140FC0() = 0;
	virtual float GetContentsYScrollOffset(void) = 0;
	virtual float GetContentsXScrollOffset(void) = 0;
	virtual void unk_131A10() = 0;
	virtual void unk_152250() = 0;
	virtual void unk_152230() = 0;
	virtual void unk_1522A0() = 0;
	virtual void unk_152270() = 0;
	virtual void unk_1522D0() = 0;
	virtual void unk_1522F0() = 0;
	virtual void unk_152310() = 0;
	virtual void unk_152330() = 0;
	virtual void unk_152350() = 0;
	virtual void unk_152380() = 0;
	virtual void unk_131080() = 0;
	virtual void unk_1310F0() = 0;
	virtual void unk_134E50() = 0;
	virtual void unk_1350C0() = 0;
	virtual void unk_134EF0() = 0;
	virtual void unk_135450() = 0;
	virtual void unk_135470() = 0;
	virtual void unk_131710() = 0;
	virtual void unk_131760() = 0;
	virtual bool CanScrollUp(void) = 0;
	virtual bool CanScrollDown(void) = 0;
	virtual bool CanScrollLeft(void) = 0;
	virtual bool CanScrollRight(void) = 0;
	virtual void AddClass(const char*) = 0;
	virtual void AddClass(CPanoramaSymbol) = 0;
	virtual void AddClasses(CPanoramaSymbol*, unsigned int count) = 0;
	virtual void RemoveClass(const char*) = 0;
	virtual void RemoveClasses(CPanoramaSymbol const* const, unsigned int) = 0;
	virtual void RemoveClasses(const char*) = 0;
	virtual void RemoveAllClasses(void) = 0;
	virtual void sub_137540() = 0;
	virtual void* GetClasses(void) = 0;
	virtual bool HasClass(const char*) = 0;
	virtual bool HasClass(CPanoramaSymbol) = 0;
	virtual bool AscendantHasClass(const char*) = 0;
	virtual bool AscendantHasClass(CPanoramaSymbol) = 0;
	virtual void ToggleClass(const char*) = 0;
	virtual void ToggleClass(CPanoramaSymbol) = 0;
	virtual void SetHasClass(const char*, bool) = 0;
	virtual void SetHasClass(CPanoramaSymbol, bool) = 0;
	virtual void SwitchClass(const char*, const char*) = 0;
	virtual void SwitchClass(const char*, CPanoramaSymbol) = 0;
	virtual void unk_149DF0() = 0;
	virtual void unk_149D50() = 0;
	virtual void unk_149EB0() = 0;
	virtual void unk_149E50() = 0;
	virtual bool AcceptsInput() = 0;
	virtual void SetAcceptsInput(bool state) = 0;
	virtual bool AcceptsFocus() = 0;
	virtual void SetAcceptsFocus(bool state) = 0;
	virtual bool CanAcceptInput() = 0;
	virtual void SetDefaultFocus(const char*) = 0;
	virtual const char* GetDefaultFocus() = 0;
	virtual void SetDisableFocusOnMouseDown(bool state) = 0;
	virtual bool FocusOnMouseDown() = 0;
	virtual void unk_152460() = 0;
	virtual void unk_1523E0() = 0;
	virtual void unk_130E80() = 0;
	virtual void unk_130EB0() = 0;
	virtual void unk_152400() = 0;
	virtual void unk_152420() = 0;
	virtual void unk_152470() = 0;
	virtual void unk_152490() = 0;
	virtual void unk_1318C0() = 0;
	virtual void GetDefaultInputFocus(void) = 0;
	virtual void SetFocus(void) = 0;
	virtual void UpdateFocusInContext(void) = 0;
	virtual void SetFocusDueToHover(void) = 0;
	virtual void SetInputContextFocus(void) = 0;
	virtual unsigned long long GetStyleFlags(void) = 0;
	virtual void AddStyleFlag(EStyleFlags) = 0;
	virtual void RemoveStyleFlag(EStyleFlags) = 0;
	virtual bool IsInspected(void) = 0;
	virtual bool HasHoverStyle(bool) = 0;
	virtual void SetSelected(bool state) = 0;
	virtual bool IsSelected(void) = 0;
	virtual bool HasKeyFocus(void) = 0;
	virtual bool HasDescendantKeyFocus(void) = 0;
	virtual bool IsLayoutLoading(void) = 0;
	virtual void SetEnabled(bool state) = 0;
	virtual bool IsEnabled(void) = 0;
	virtual void unk_146920() = 0;
	virtual void unk_1525B0() = 0;
	virtual void unk13() = 0;
	virtual void sub_125010() = 0;
	virtual void SetAllChildrenActivationEnabled(bool state) = 0;
	virtual void SetHitTestEnabled(bool state) = 0;
	virtual bool HitTestEnabled(void) = 0;
	virtual void SetHitTestEnabledTraverse(bool state) = 0;
	virtual void unk_152630() = 0;
	virtual void unk_152660() = 0;
	virtual void SetDraggable(bool state) = 0;
	virtual bool IsDraggable(void) = 0;
	virtual void SetRememberChildFocus(bool state) = 0;
	virtual bool GetRememberChildFocus(void) = 0;
	virtual void ClearLastChildFocus(void) = 0;
	virtual void SetNeedsIntermediateTexture(bool state) = 0;
	virtual bool GetNeedsIntermediateTexture(void) = 0;
	virtual void unk_152DE0() = 0;
	virtual void unk_152E40() = 0;
	virtual void SetClipAfterTransform(bool state) = 0;
	virtual bool GetClipAfterTransform(void) = 0;
	virtual const char* GetInputNamespace(void) = 0;
	virtual void SetInputNamespace(const char*) = 0;
	virtual void MarkStylesDirty(bool state) = 0;
	virtual bool StylesDirty(void) = 0;
	virtual void sub_12A000() = 0;
	virtual bool ChildStylesDirty() = 0;
	virtual bool ParsePanelEvent(CPanoramaSymbol, const char*) = 0;
	virtual bool IsPanelEventSet(CPanoramaSymbol) = 0;
	virtual bool IsPanelEvent(CPanoramaSymbol) = 0;
	virtual void DispatchPanelEvent(CPanoramaSymbol) = 0;
	virtual void* GetJavaScriptContextParent(void) = 0;
	virtual void* UIImageManager(void) = 0;
	virtual void* UIRenderEngine(void) = 0;
	virtual void* unk_130DF0() = 0; // returns another class like the 2 above
	virtual void PaintTraverse(void) = 0;
	virtual void SetTabIndex(float) = 0;
	virtual float GetTabIndex(void) = 0;
	virtual void SetSelectionPosition(float, float) = 0;
	virtual void SetSelectionPositionX(float) = 0;
	virtual void SetSelectionPositionY(float) = 0;
	virtual float GetSelectionPositionX(void) = 0;
	virtual float GetSelectionPositionY(void) = 0;
	virtual float GetTabIndex_Raw(void) = 0;
	virtual float GetSelectionPositionX_Raw(void) = 0;
	virtual float GetSelectionPositionY_Raw(void) = 0;
	virtual void SetFocusToNextPanel(int, EFocusMoveDirection, bool, float, float, float, float, float) = 0;
	virtual void SetInputFocusToFirstOrLastChildInFocusOrder(EFocusMoveDirection, float, float) = 0;
	virtual void SelectionPosVerticalBoundary(void) = 0;
	virtual void SelectionPosHorizontalBoundary(void) = 0;
	virtual void SetChildFocusOnHover(bool state) = 0;
	virtual bool GetChildFocusOnHover(void) = 0;
	virtual void SetFocusOnHover(bool state) = 0;
	virtual bool GetFocusOnHover(void) = 0;
	virtual void ScrollToTop(void) = 0;
	virtual void ScrollToBottom(void) = 0;
	virtual void ScrollToLeftEdge(void) = 0;
	virtual void ScrollToRightEdge(void) = 0;
	virtual void ScrollParentToMakePanelFit(ScrollBehavior_t, bool) = 0;
	virtual void ScrollToFitRegion(float, float, float, float, ScrollBehavior_t, bool, bool) = 0;
	virtual bool CanSeeInParentScroll(void) = 0;
	virtual void OnScrollPositionChanged(void) = 0;
	virtual void SetSendChildScrolledIntoViewEvents(bool) = 0;
	virtual void OnCheckChildrenScrolledIntoView(void) = 0;
	virtual void FireScrolledIntoViewEvent(void) = 0;
	virtual void FireScrolledOutOfViewEvent(void) = 0;
	virtual bool IsScrolledIntoView(void) = 0;
	virtual void SortChildren() = 0; // (int (*)(panorama::IUIPanelClient * const*,panorama::IUIPanelClient * const*))
	virtual void AddChild(IUIPanel*) = 0;
	virtual void AddChildSorted() = 0; // (bool (*)(panorama::IUIPanelClient * const&,panorama::IUIPanelClient * const&),panorama::IUIPanel *)
	virtual void ResortChild() = 0; // (bool (*)(panorama::IUIPanelClient * const&,panorama::IUIPanelClient * const&),panorama::IUIPanel *)
	virtual void RemoveChild(IUIPanel*) = 0;
	virtual void unk_139310() = 0;
	virtual void unk_1393C0() = 0;
	virtual void unk_131260() = 0;
	virtual void unk_131270() = 0;
	virtual void SetMouseCanActivate(EMouseCanActivate, const char*) = 0;
	virtual EMouseCanActivate GetMouseCanActivate(void) = 0;
	virtual IUIPanel* FindParentForMouseCanActivate(void) = 0;
	virtual bool ReloadLayout(CPanoramaSymbol) = 0;
	virtual void ReloadStyleFileTraverse(CPanoramaSymbol) = 0;
	virtual bool HasOnActivateEvent(void) = 0;
	virtual bool HasOnMouseActivateEvent(void) = 0;
	virtual void SetupJavascriptObjectTemplate(void) = 0;
	virtual void SetLayoutFile(CPanoramaSymbol) = 0;
	virtual void BuildMatchingStyleList() = 0; // (CUtlVector<panorama::CascadeStyleFileInfo_t,CUtlMemory<panorama::CascadeStyleFileInfo_t,int>> *)
	virtual void GetAttribute(const char*, int) = 0;
	virtual void GetAttribute(const char*, const char*) = 0;
	virtual void unk_132680() = 0;
	virtual void GetAttribute(const char*, unsigned int) = 0;
	virtual void GetAttribute(const char*, unsigned long long) = 0;
	virtual void SetAttribute(const char*, int) = 0;
	virtual void SetAttribute(const char*, const char*) = 0;
	virtual void SetAttribute(const char*, unsigned int) = 0;
	virtual void SetAttribute(const char*, unsigned long long) = 0;
	virtual void SetAttribute() = 0;
	virtual void unk_132770() = 0;
	virtual void unk_132830() = 0;
	virtual void unk_132810() = 0;
	virtual void unk_1327C0() = 0;
	virtual void unk_132880() = 0;
	virtual void unk_13D2D0() = 0;
	virtual void unk_13D110() = 0;
	virtual void unk_13D350() = 0;
	virtual void unk_13D3D0() = 0;
	virtual void unk_13D450() = 0;
	virtual void unk_13D950() = 0;
	virtual void unk_13D870() = 0;
	virtual void unk_13D690() = 0;
	virtual void unk_13D420() = 0;
	virtual void SetAnimation(const char*, float, float, EAnimationTimingFunction, EAnimationDirection, float) = 0;
	virtual void UpdateVisibility(bool) = 0;
	virtual void unk14() = 0;
	virtual void SetProperty(CPanoramaSymbol, const char*) = 0;
	virtual void FindChildrenWithClass() = 0; // (char const*,CUtlVector<panorama::IUIPanel *,CUtlMemory<panorama::IUIPanel *,int>> &)
	virtual void FindChildrenWithClassTraverse() = 0; // (char const*,CUtlVector<panorama::IUIPanel *,CUtlMemory<panorama::IUIPanel *,int>> &)
	virtual void PlayFocusChangeSound(int, float) = 0;
	virtual void unk15() = 0;
	virtual void ClearPanelEvents(CPanoramaSymbol) = 0;
	virtual void SetPanelEvent(CPanoramaSymbol, IUIEvent*) = 0;
	virtual void SetPanelEvent2() = 0; // (panorama::CPanoramaSymbol,CUtlVector<panorama::IUIEvent *,CUtlMemory<panorama::IUIEvent *,int>> *)
	virtual bool GetAnalogStickScrollingEnabled(void) = 0;
	virtual void EnableAnalogStickScrolling(bool state) = 0;
	virtual void SetMouseTracking(bool state) = 0;
	virtual void SetInScrollbarConstruction(bool state) = 0;
	virtual void* GetVerticalScrollBar(void) = 0;
	virtual void* GetHorizontalScrollBar(void) = 0;
	virtual void GetPanelEvents(CPanoramaSymbol) = 0;
	virtual bool HasBeenLayedOut(void) = 0;
	virtual void OnStyleTransitionsCleanup(void) = 0;
	virtual void unk_1529D0() = 0;
	virtual void unk_1529F0() = 0;
	virtual void unk_152A00() = 0;
	virtual void unk_152A20() = 0;
	virtual void unk_136B20() = 0;
	virtual void unk_152A30() = 0;
	virtual void unk_139260() = 0;
	virtual void unk_1331A0() = 0;
	virtual void unk_1529A0() = 0;
	virtual void unk_1529B0() = 0;
	virtual void unk_1529C0() = 0;
};

class PanelWrapper {
public:
	void* vtable;
	IUIPanel* panel;
};

class UIEngine {
public:
	virtual void DESTROY() = 0; // 0
	virtual void StartupSubsystems(IUISettings*, void* PlatWindow_t__) = 0;
	virtual void ConCommandInit(void* IConCommandBaseAccessor) = 0;
	virtual void Shutdown(void) = 0;
	virtual void RequestShutdown(void) = 0;
	virtual void Run(void) = 0;
	virtual void RunFrame(void) = 0;
	virtual void SetAggressiveFrameRateLimit(bool) = 0;
	virtual bool IsRunning(void) = 0;
	virtual bool HasFocus(void) = 0;
	virtual double GetCurrentFrameTime(void) = 0;
	virtual void unk0() = 0; // getter
	virtual void unk1() = 0; // setter ^^
	virtual void INT3_WRAPPER() = 0;
	virtual void INT3_WRAPPER2() = 0;
	virtual void INT3_WRAPPER3() = 0; // probably windows specific
	virtual void INT3_WRAPPER4() = 0;
	virtual void INT3_WRAPPER5() = 0;
	virtual void CreateTextLayout(char const*, char const*, float, float, EFontWeight, EFontStyle, ETextAlign, bool, bool, int, float, float) = 0;
	virtual void CreateTextLayout(wchar_t const*, char const*, float, float, EFontWeight, EFontStyle, ETextAlign, bool, bool, int, float, float) = 0;
	virtual void FreeTextLayout(IUITextLayout*) = 0;
	virtual void GetSomeFontThing(void) = 0; //similar to function below
	virtual void GetSortedValidFontNames(void) = 0;
	virtual IUIInputEngine* UIInputEngine(void) = 0;
	virtual IUILocalize* UILocalize(void) = 0;
	virtual IUISoundSystem* UISoundSystem(void) = 0;
	virtual IUISettings* UISettings(void) = 0;
	virtual IUILayoutManager* UILayoutManager(void) = 0;
	virtual IUIFileSystem* UIFileSystem(void) = 0;
	virtual void RegisterFrameFunc(void (*)(void)) = 0;
	virtual void ReloadLayoutFile(CPanoramaSymbol) = 0;
	virtual void ToggleDebugMode(void) = 0;
	virtual const char* GetConsoleHistory(void) = 0;
	virtual PanelWrapper* CreatePanel(void) = 0;
	virtual void PanelDestroyed(IUIPanel*, IUIPanel*) = 0;
	virtual bool IsValidPanelPointer(IUIPanel const*) = 0;
	virtual PanelHandle_t* GetPanelHandle(IUIPanel const*) = 0;
	virtual IUIPanel* GetPanelPtr(PanelHandle_t const&) = 0;
	virtual void CallBeforeStyleAndLayout(IUIPanel*) = 0;
	virtual void RegisterForPanelDestroyed() = 0; //CUtlDelegate<void ()(IUIPanel const*,IUIPanel*)>) = 0;
	virtual void UnregisterForPanelDestroyed() = 0; //CUtlDelegate<void ()(IUIPanel const*,IUIPanel*)>) = 0;
	virtual void RegisterEventHandler(CPanoramaSymbol, IUIPanel*, CUtlAbstractDelegate) = 0;
	virtual void UnregisterEventHandler(CPanoramaSymbol, IUIPanel*, CUtlAbstractDelegate) = 0;
	virtual void RegisterEventHandler(CPanoramaSymbol, IUIPanelClient*, CUtlAbstractDelegate) = 0;
	virtual void UnregisterEventHandler(CPanoramaSymbol, IUIPanelClient*, CUtlAbstractDelegate) = 0;
	virtual void UnregisterEventHandlersForPanel(IUIPanel*) = 0;
	virtual void RegisterForUnhandledEvent(CPanoramaSymbol, CUtlAbstractDelegate) = 0;
	virtual void UnregisterForUnhandledEvent(CPanoramaSymbol, CUtlAbstractDelegate) = 0;
	virtual void UnregisterForUnhandledEvents(void*) = 0;
	virtual bool HaveEventHandlersRegisteredForType(CPanoramaSymbol) = 0;
	virtual void RegisterPanelTypeEventHandler(CPanoramaSymbol, CPanoramaSymbol, CUtlAbstractDelegate, bool) = 0;//50
	virtual void DispatchEvent(IUIEvent*) = 0;
	virtual void DispatchEventAsync(float, IUIEvent*) = 0;
	virtual bool AnyHandlerRegisteredForEvent(CPanoramaSymbol const&) = 0;
	virtual CPanoramaSymbol* GetLastDispatchedEventSymbol(void) = 0;
	virtual IUIPanel* GetLastDispatchedEventTargetPanel(void) = 0; // This function can fail sometimes and you need to check the result/call it later (YUCK!)
	virtual void RegisterEventFilter(CUtlAbstractDelegate) = 0;
	virtual void UnregisterEventFilter(CUtlAbstractDelegate) = 0;
	virtual void LayoutAndPaintWindows(void) = 0;
	virtual const char* GetApplicationInstallPath(void) = 0;
	virtual const char* GetApplicationUserDataPath(void) = 0;
	virtual void RegisterNamedLocalPath(char const*, char const*, bool, bool) = 0;
	virtual void RegisterNamedUserPath(char const*, char const*, bool, bool) = 0;
	virtual void RegisterCustomFontPath(char const*) = 0;
	virtual void GetLocalPathForNamedPath(char const*) = 0;
	virtual void GetLocalPathForRelativePath(char const*, char const*, void* CUtlString) = 0;
	virtual void RegisterNamedRemoteHost(char const*, char const*) = 0;
	virtual void GetRemoteHostForNamedHost(char const*) = 0;
	virtual void RegisterXHeader(char const*, char const*) = 0;
	virtual int GetXHeaderCount(void) = 0;
	virtual void GetXHeader(int, void* CUtlString, void* CUtlString2) = 0;
	virtual void SetCookieHeaderForNamedRemoteHost() = 0; //char const*,CUtlVector<CUtlString,CUtlMemory<CUtlString,int>> const&) = 0;
	virtual void SetCookieHeaderForRemoteHost() = 0; //char const*,CUtlVector<CUtlString,CUtlMemory<CUtlString,int>> const&) = 0;
	virtual void GetCookieHeadersForNamedRemoteHost(char const*) = 0;
	virtual void GetCookieHeadersForRemoteHost(char const*) = 0;
	virtual void GetCookieValueForRemoteHost(char const*, char const*, void* CUtlString) = 0;
	virtual void BSetCookieForWebRequests(char const*, char const*, char const*) = 0;
	virtual void BClearCookieForWebRequests(char const*, char const*, char const*) = 0;
	virtual void* AccessHTMLController(void) = 0;
	virtual void unk() = 0; // _SDL_ShowSimpleMessageBox with yes/no
	virtual void AddFrameListener(IUIEngineFrameListener*) = 0;
	virtual void RemoveFrameListener(IUIEngineFrameListener*) = 0;
	virtual void RegisterMouseCanActivateParent(IUIPanel*, char const*) = 0;
	virtual void UnregisterMouseCanActivateParent(IUIPanel*) = 0;
	virtual void GetMouseCanActivateParent(IUIPanel*) = 0;
	virtual bool AnyWindowHasFocus(void) = 0;
	virtual bool AnyVisibleWindowHasFocus(void) = 0;
	virtual bool AnyOverlayWindowHasFocus(void) = 0;
	virtual IUIWindow* GetFocusedWindow(void) = 0;
	virtual float GetLastInputTime(void) = 0;
	virtual float UpdateLastInputTime(void) = 0;
	virtual void ClearClipboard(void) = 0;
	virtual void CopyToClipboard(char const*, char const*) = 0;
	virtual void GetClipboardText(void* CUtlString, void* CUtlString2) = 0;
	virtual const char* GetDisplayLanguage(void) = 0;
	virtual void unk2() = 0;
	virtual void unk3() = 0;
	virtual void unk4() = 0;
	virtual bool HasOverlayForApp(unsigned long long, unsigned long long) = 0;
	virtual void TrackOverlayForApp(unsigned long long, unsigned long long, void*) = 0;//100
	virtual void DeleteOverlayInstanceForApp(unsigned long long, unsigned long long, void*) = 0;
	virtual void OverlayForApp(unsigned long long, unsigned long long) = 0;
	virtual IUIPanelStyle* AllocPanelStyle(IUIPanel*, float) = 0;
	virtual void FreePanelStyle(IUIPanelStyle*) = 0;
	virtual void SetPanelWaitingAsyncDelete(IUIPanel*) = 0;
	virtual bool IsPanelWaitingAsyncDelete(IUIPanel*) = 0;
	virtual void PulseActiveControllerHaptic() = 0; //IUIEngine::EHapticFeedbackPosition,IUIEngine::EHapticFeedbackStrength) = 0;
	virtual void SomethingControllerHapticRelated() = 0;
	virtual void MarkLayerToRepaintThreadSafe(unsigned long long) = 0;
	virtual void AddDirectoryChangeWatch(char const*) = 0;
	virtual int GetWheelScrollLines(void) = 0; // default 3
	virtual int RunScript(IUIPanel* panel, char const* entireJSCode, char const* pathToXMLContext, int, int, bool alreadyCompiled, bool) = 0;
	virtual void unk5() = 0;
	virtual void ExposeObjectTypeToJavaScript(char const*, CUtlAbstractDelegate&) = 0;
	virtual bool IsObjectTypeExposedToJavaScript(char const*) = 0;
	virtual void ExposeGlobalObjectToJavaScript(char const*, void*, char const*, bool) = 0;
	virtual void ClearGlobalObjectForJavaScript(char const*, void*) = 0;
	virtual void DeleteJSObjectInstance(IUIJSObject*) = 0;
	virtual IUIPanel* GetPanelForJavaScriptContext() = 0; //v8::Context *) = 0;
	virtual void* GetJavaScriptContextForPanel(IUIPanel*) = 0;//120
	virtual void OutputJSExceptionToConsole() = 0; //v8::TryCatch &,IUIPanel *) = 0;
	virtual void AddGlobalV8FunctionTemplate() = 0; //char const*,v8::Handle<v8::FunctionTemplate> *,bool) = 0;
	virtual void* GetV8GlobalContext(void) = 0;
	virtual void GetCurrentV8ObjectTemplateToSetup(void) = 0; //gets offset.
	virtual void GetCurrentV8ObjectTemplateToSetup2(void) = 0; //gets offset and calls some function.
	virtual void Grabber3(void) = 0; // returns offset in class
	virtual void Grabber4(void) = 0; // returns offset in class.
	virtual void UIStyleFactory(void) = 0;
	virtual void GetV8Isolate(void) = 0;//129
	virtual void RunFunction() = 0;
	virtual void RunFunction2() = 0; //different args
	virtual void CreateV8PanelInstance(IUIPanel*) = 0;
	virtual void CreateV8PanelStyleInstance(IUIPanelStyle*) = 0;
	virtual void CreateV8ObjectInstance(char const*, void*, IUIJSObject*) = 0;
	virtual void _unk() = 0;
	virtual void InitiateAsyncJSONWebAPIRequest() = 0; //EHTTPMethod,CUtlString,IUIPanel *,void *,CJSONWebAPIParams *,uint) = 0;
	virtual void InitiateAsyncJSONWebAPIRequest(int overloadMeme) = 0; //EHTTPMethod,CUtlString,CUtlDelegate<void ()(unsigned long long,KeyValues *,void *)>,void *,CJSONWebAPIParams *,uint) = 0;
	virtual void CancelAsyncJSONWebAPIRequest(unsigned int) = 0;
	virtual void ResolvePath(char const*) = 0;
	virtual void RegisterEventWithEngine(CPanoramaSymbol, UIEventFactory) = 0;
	virtual bool IsValidEventName(CPanoramaSymbol) = 0;
	virtual bool IsValidPanelEvent(CPanoramaSymbol, int*) = 0;
	virtual void CreateInputEventFromSymbol(CPanoramaSymbol, IUIPanel*, EPanelEventSource_t, int) = 0;
	virtual void CreateEventFromString(IUIPanel*, char const*, char const**) = 0;
	virtual void unk11() = 0;
	virtual void RegisterPanelFactoryWithEngine(CPanoramaSymbol, CPanel2DFactory*) = 0;
	virtual bool RegisteredPanelType(CPanoramaSymbol) = 0;
	virtual PanelWrapper* CreatePanel(CPanoramaSymbol* panelType, char const* id, IUIPanel* createInThisParent) = 0; // paneltype (ex: Panel, Label), id (ex: <Label id="swag">)
	virtual void CreateDebuggerWindow(void) = 0;
	virtual void CloseDebuggerWindow(void) = 0;
	virtual void RegisterScheduledDelegate() = 0; // double,CUtlDelegate<void ()(void)> ) = 0;
	virtual void CancelScheduledDelegate(int) = 0;
	virtual float GetLastScheduledDelegateRunTime(void) = 0;
	virtual CPanoramaSymbol MakeSymbol(char const*) = 0; // if the symbol already exists, you'll get that one.
	virtual const char* ResolveSymbol(unsigned short) = 0; // if you pass an invalid number into here, it can crash the program.
	virtual void QueueDecrementRefNextFrame() = 0; //CRefCounted<CRefCountServiceBase<true,CRefMT>> *) = 0;
	virtual void RegisterJSGenericCallback() = 0; //IUIPanel *,v8::Handle<v8::Function>) = 0;
	virtual void InvokeJSGenericCallback() = 0; //int,int,v8::Handle<v8::Value> *,v8::Handle<v8::Value> *) = 0;
	virtual void UnregisterJSGenericCallback(int) = 0;
	virtual int GetNumRegisterJSScopes(void) = 0;
	virtual void GetRegisterJSScopeInfo(int, RegisterJSScopeInfo_t*) = 0;
	virtual void GetRegisterJSEntryInfo(int, int, RegisterJSEntryInfo_t*) = 0;
	virtual void StartRegisterJSScope(char const*, char const*) = 0;
	virtual void EndRegisterJSScope(void) = 0;
	virtual void NewRegisterJSEntry(char const*, unsigned int, char const*, RegisterJSType_t) = 0;
	virtual void SetRegisterJSEntryParams(int, unsigned char, RegisterJSType_t*) = 0;
	virtual void _padding() = 0;
	virtual void ClearFileCache(void) = 0;
	virtual void PrintCacheStatus(void) = 0;
	virtual void GetWindowsForDebugger() = 0; //CUtlVector<IUIWindow *,CUtlMemory<IUIWindow *,int>> &) = 0;
	virtual void SetPaintCountTrackingEnabled(bool) = 0;
	virtual bool GetPaintCountTrackingEnabled(void) = 0;
	virtual void IncrementPaintCountForPanel(unsigned long long, bool, double) = 0;
	virtual void GetPanelPaintInfo(unsigned long long, unsigned int&, bool&, double&) = 0;
	virtual bool HasAnyWindows(void) = 0;
	virtual void TextEntryFocusChange(IUIPanel*) = 0;
	virtual void TextEntryInvalid(IUIPanel*) = 0;
	virtual void SetClipboardText(const char* text) = 0; // wrapper for _SDL_SetClipboardText
	virtual const char* GetClipboardText() = 0; // wrapper for _SDL_GetClipboardText
	virtual void OnFileCacheRemoved(CPanoramaSymbol) = 0;
	virtual void RunPlatformFrame(void) = 0;
	virtual void CreateSoundSystem(void) = 0;
};

class IPanoramaUIEngine : IAppSystem3 {
public:
	virtual void SetupUIEngine(void) = 0;
	virtual void ShutdownUIEngine(void) = 0;
	virtual UIEngine* AccessUIEngine(void) = 0;//11
	virtual void HandleInputEvent() = 0;
	virtual void unk1() = 0; // calls this function in PanoramaUIEngine -- call    qword ptr [rax+0A0h]
	virtual void unk2() = 0;
};

class IClientState {
public:
	BYTE		pad0[0x9C];
	void* netChannel;
	int				challengeNr;
	BYTE		pad1[0x04];
	double          m_connect_time;
	int             m_retry_number;
	BYTE		padgay[0x54];
	int				signonState;
	BYTE		pad2[0x8];
	float           nextCmdTime;
	int				serverCount;
	int				currentSequence;
	char			pad99[8];
	struct {
		float		m_clock_offsets[16];
		int			m_cur_clock_offset;
		int			m_server_tick;
		int			m_client_tick;
	} m_clock_drift_mgr;
	int				deltaTick;
	bool			paused;
	BYTE		pad4[0x7];
	int				viewEntity;
	int				playerSlot;
	char			levelName[260];
	char			levelNameShort[80];
	char			groupName[80];
	char			szLastLevelNameShort[80]; // 0x032C
	BYTE		pad5[0xC];
	int				maxClients;
	BYTE pad_030C[4083];
	uint32_t string_table_container;
	BYTE pad_1303[14737];
	float			lastServerTickTime;
	bool			InSimulation;
	BYTE		pad7[0x3];
	int				oldTickcount;
	float			tickRemainder;
	float			frameTime;
	int				lastOutgoingCommand;
	int				chokedCommands;
	int				lastCommandAck;
	int				commandAck;
	int				soundSequence;
	//std::byte		pad8[0x50];
	int                m_last_progress_percent;
	bool            m_is_hltv;

	BYTE padfuck[0x4B];
	vec			angViewPoint;
	BYTE		pad9[0xD0];
	void* pEvents;
};

class CUiComponent_PartyList {
private:
	virtual void unk1() = 0;
public:
	virtual int GetCount() = 0;
};

class CUiComponent_PartyBrowser {
public:
	inline bool HasBeenInvited(int64_t id) {
		typedef void* (__thiscall* OriginalFn)(PVOID, int64_t);
		return ((OriginalFn)M::UiComponent_PartyBrowser_HasBeenInvited)(this, id) != NULL;
	}

};

typedef unsigned short MaterialHandle_t;

class IMaterialSystem
{
public:
	IMaterial* FindMaterial(char const* pMaterialName, const char* pTextureGroupName, bool complain = true, const char* pComplainPrefix = NULL) {
		typedef IMaterial* (__thiscall* OriginalFn)(void*, char const* pMaterialName, const char* pTextureGroupName, bool complain, const char* pComplainPrefix);
		return GetVFunc<OriginalFn>(this, 84)(this, pMaterialName, pTextureGroupName, complain, pComplainPrefix);
	}
	IMaterial* CreateMaterial(const char* pMaterialName, KeyValues* pVMTKeyValues)
	{
		typedef IMaterial* (__thiscall* oCreateMaterial)(PVOID, const char*, KeyValues*);
		return GetVFunc<oCreateMaterial>(this, 83)(this, pMaterialName, pVMTKeyValues);
	}
	MaterialHandle_t FirstMaterial() {
		typedef MaterialHandle_t(__thiscall* oFirstMaterial)(void*);
		return GetVFunc<oFirstMaterial>(this, 86)(this);
	}
	MaterialHandle_t NextMaterial(MaterialHandle_t h) {
		typedef MaterialHandle_t(__thiscall* oNextMaterial)(void*, MaterialHandle_t);
		return GetVFunc<oNextMaterial>(this, 87)(this, h);
	}
	MaterialHandle_t InvalidMaterial() {
		typedef MaterialHandle_t(__thiscall* oInvalidMaterial)(void*);
		return GetVFunc<oInvalidMaterial>(this, 88)(this);
	}
	IMaterial* GetMaterial(MaterialHandle_t h) {
		typedef IMaterial* (__thiscall* oGetMaterial)(void*, MaterialHandle_t);
		return GetVFunc<oGetMaterial>(this, 89)(this, h);
	}
};

//THX valve <3

//
// Describes possible states of an async operation
//
enum AsyncOperationState_t
{
	AOS_RUNNING,
	AOS_ABORTING,
	AOS_ABORTED,
	AOS_FAILED,
	AOS_SUCCEEDED,
};

//
// Interface of an async operation
//
class IMatchAsyncOperation {
public:
	// Poll if operation has completed
	virtual bool IsFinished() = 0;

	// Operation state
	virtual AsyncOperationState_t GetState() = 0;

	// Retrieve a generic completion result for simple operations
	// that return simple results upon success,
	// results are operation-specific, may result in undefined behavior
	// if operation is still in progress.
	virtual uint64_t GetResult() = 0;
	virtual uint64_t GetResultExtraInfo() { return 0; }

	// Request operation to be aborted
	virtual void Abort() = 0;

	// Release the operation interface and all resources
	// associated with the operation. Operation callbacks
	// will not be called after Release. Operation object
	// cannot be accessed after Release.
	virtual void Release() = 0;
};

struct Matrix4x4 {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;

		};
		float m[4][4];
	};
};

template <typename T>
class UtlVector {
public:
	constexpr T& operator[](int i) noexcept { return memory[i]; };
	constexpr const T& operator[](int i) const noexcept { return memory[i]; };

	T* memory;//0
	int allocationCount;//4
	int growSize;//8
	int size;//c
	T* elements;
};

class ConVar
{
public:
	inline void SetValue(const char* value) {
		typedef void(__thiscall* OriginalFn)(void*, const char*);
		return  GetVFunc<OriginalFn>(this, 14)(this, value);
	}

	inline void SetValue(float value) {
		typedef void(__thiscall* OriginalFn)(void*, float);
		return GetVFunc<OriginalFn>(this, 15)(this, value);
	}

	inline void SetValue(int value) {
		typedef void(__thiscall* OriginalFn)(void*, int);
		return GetVFunc<OriginalFn>(this, 16)(this, value);
	}
	float GetFloat(void) const {
		auto temp = *(int*)(&fValue);
		auto temp_result = (int)(temp ^ (DWORD)this);
		return *(float*)(&temp_result);
	}
	inline int GetInt(void) const {
		auto temp = *(int*)(&nValue);
		return (int)(temp ^ (DWORD)this);
	}

	inline const char* GetString(void) const {
		return pParent->pszDefaultValue;
	}

	inline Color GetColor(void) const {
		unsigned char* pColorElement = ((unsigned char*)&pParent->nValue);
		return Color(pColorElement[0], pColorElement[1], pColorElement[2], pColorElement[3]);
	}
	inline void SetValue(Color value) {
		typedef void(__thiscall* OriginalFn)(void*, Color);
		return GetVFunc<OriginalFn>(this, 17)(this, value);
	}

	inline char* GetName() {
		typedef char* (__thiscall* OriginalFn)(void*);
		return GetVFunc<OriginalFn>(this, 5)(this);
	}

	inline char* GetDefault() {
		return pszDefaultValue;
	}

	char pad_0x0000[0x4]; //0x0000
	ConVar* pNext; //0x0004 
	__int32 bRegistered; //0x0008 
	char* pszName; //0x000C 
	char* pszHelpString; //0x0010 
	__int32 nFlags; //0x0014 
	char pad_0x0018[0x4]; //0x0018
	ConVar* pParent; //0x001C 
	char* pszDefaultValue; //0x0020 
	char* strString; //0x0024 
	__int32 StringLength; //0x0028 
	float fValue; //0x002C 
	__int32 nValue; //0x0030 
	__int32 bHasMin; //0x0034 
	float fMinVal; //0x0038 
	__int32 bHasMax; //0x003C 
	float fMaxVal; //0x0040 
	UtlVector<void(__cdecl*)()> fnChangeCallback; //0x0044 
};//Size=0x0048

class ICVar : public IAppSystem3
{
public:
	virtual void func9() = 0;
	virtual void func10() = 0;
	virtual void RegisterConCommand(ConVar* pCommandBase) = 0;
	virtual void UnregisterConCommand(ConVar* pCommandBase) = 0;
	virtual void func13() = 0;
	virtual void func14() = 0;
	virtual ConVar* FindVar(const char* var_name) = 0;

	void const ConsoleColorPrintf(const Color& clr, const char* pFormat, ...) {
		typedef void(__cdecl* OriginalFn)(void*, const Color&, const char*, ...);

		if (pFormat == nullptr)
			return;

		char buf[8192];

		va_list list;
		va_start(list, pFormat);
		_vsnprintf(buf, sizeof(buf) - 1, pFormat, list);
		va_end(list);
		buf[sizeof(buf) - 1] = 0;

		GetVFunc<OriginalFn>(this, 25)(this, clr, buf, list);
	}
};

template<class T> struct CUtlReference {
	CUtlReference* m_pNext;
	CUtlReference* m_pPrev;
	T* m_pObject;
};
template<class T> struct CUtlIntrusiveList {
	T* m_pHead;
};
template<class T> struct CUtlIntrusiveDList : public CUtlIntrusiveList<T> {};
template<class T> struct CUtlReferenceList : public CUtlIntrusiveDList< CUtlReference<T> > {};

enum EAttributeDataType {
	ATTRDATATYPE_NONE = -1,
	ATTRDATATYPE_FLOAT = 0,
	ATTRDATATYPE_4V,
	ATTRDATATYPE_INT,
	ATTRDATATYPE_POINTER,

	ATTRDATATYPE_COUNT,
};

#define MAX_PARTICLE_ATTRIBUTES 24

#define DEFPARTICLE_ATTRIBUTE( name, bit, datatype )			\
	const int PARTICLE_ATTRIBUTE_##name##_MASK = (1 << bit);	\
	const int PARTICLE_ATTRIBUTE_##name = bit;					\
	const EAttributeDataType PARTICLE_ATTRIBUTE_##name##_DATATYPE = datatype;

DEFPARTICLE_ATTRIBUTE(TINT_RGB, 6, ATTRDATATYPE_4V);

DEFPARTICLE_ATTRIBUTE(ALPHA, 7, ATTRDATATYPE_FLOAT);

struct CParticleAttributeAddressTable {
	float* m_pAttributes[MAX_PARTICLE_ATTRIBUTES];
	size_t m_nFloatStrides[MAX_PARTICLE_ATTRIBUTES];

	FORCEINLINE float* FloatAttributePtr(int nAttribute, int nParticleNumber) const {
		int block_ofs = nParticleNumber / 4;
		return m_pAttributes[nAttribute] +
			m_nFloatStrides[nAttribute] * block_ofs +
			(nParticleNumber & 3);
	}

};

struct CUtlString_simple {
	char* buffer;
	int capacity;
	int grow_size;
	int length;
};

class CParticleSystemDefinition {
	BYTE pad_0[308];
public:
	CUtlString_simple m_Name;
};

class CParticleCollection {
	BYTE pad_0[48];//0
public:
	int m_nActiveParticles;//48
private:
	BYTE pad_1[12];//52
public:
	CUtlReference<CParticleSystemDefinition> m_pDef;//64
private:
	BYTE pad_2[60];//80
public:
	CParticleCollection* m_pParent;//136
private:
	BYTE pad_3[84];//140
public:
	CParticleAttributeAddressTable m_ParticleAttributes;//224
};

//constexpr auto test_224 = offsetof(CParticleCollection, m_ParticleAttributes);

class C_INIT_RandomColor {
	BYTE pad_0[92];
public:
	vec	m_flNormColorMin;
	vec	m_flNormColorMax;
	Color	m_ColorMin;//29
	Color	m_ColorMax;
	Color	m_TintMin;
	Color	m_TintMax;
	float	m_flTintPerc;
	float	m_flUpdateThreshold;
	int		m_nTintCP;
	int		m_nFieldOutput;
	int		m_nTintBlendMode;
	float	m_flLightAmplification;
};

class CVoiceStatus;

class LZSSLagger2 {

public:
	bool init = false;
	unsigned char* LZSSData;
	unsigned int output_size = 0;
	int SizeOfFakePacket = 0;
	int lastPacketSize = 0;

	LZSSLagger2(int size) { Init(size); }
	void Init(int size) {

		lastPacketSize = size;
		CLZSS Compressor;
		char* buffer = (char*)malloc(size + sizeof(unsigned int));
		if (!buffer)
			return;

		for (int i = 0; i < size; i += 8)
			*(int8_t*)(&buffer[i] + sizeof(unsigned int)) = (int8_t)rand() % 0x7FFFFFFFF;

		LZSSData = Compressor.CompressLeave8((unsigned char*)buffer, size, &output_size);
		output_size += sizeof(unsigned int);
		init = true;

	}

	int CreateLZSSData(char** buffer_to_write) {
		*buffer_to_write = (char*)LZSSData;
		return output_size;
	}
};

class CGlobalVarsBase
{
public:
	float     realtime;                     // 0x0000
	int       framecount;                   // 0x0004
	float     absoluteframetime;            // 0x0008
	float     absoluteframestarttimestddev; // 0x000C
	float     curtime;                      // 0x0010
	float     frametime;                    // 0x0014
	int       maxClients;                   // 0x0018
	int       tickcount;                    // 0x001C
	float     interval_per_tick;            // 0x0020
	float     interpolation_amount;         // 0x0024
	int       simTicksThisFrame;            // 0x0028
	int       network_protocol;             // 0x002C
	void* pSaveData;                    // 0x0030
	bool      m_bClient;                    // 0x0031
	bool      m_bRemoteClient;              // 0x0032

private:
	// 100 (i.e., tickcount is rounded down to this base and then the "delta" from this base is networked
	int       nTimestampNetworkingBase;
	// 32 (entindex() % nTimestampRandomizeWindow ) is subtracted from gpGlobals->tickcount to Set the networking basis, prevents
	//  all of the entities from forcing a new PackedEntity on the same tick (i.e., prevents them from getting lockstepped on this)
	int       nTimestampRandomizeWindow;
};

class CEconGameAccountClient {
public:
	void** vftable; //0
	void** CSOEconGameAccountClient_vftable; //4
	DWORD unk_1; //8
	uint32 additional_backpack_slots; //12
	uint32 bonus_xp_timestamp_refresh; //16
	uint32 bonus_xp_usedflags; //20
	uint32 elevated_state; //24
	uint32 elevated_timestamp; //28
	DWORD unk_2; //32
	DWORD unk_3; //36
};