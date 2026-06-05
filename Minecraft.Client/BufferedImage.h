#pragma once
using namespace std;

class Graphics;
class DLCPack;

class BufferedImage
{
private:
	int *data[10];	// Arrays for mipmaps - NULL if not used
	int width;
	int height;
	void ByteFlip4(unsigned int &data);	// 
public:
	static const int TYPE_INT_ARGB = 0;
	static const int TYPE_INT_RGB = 1;
	BufferedImage(int width,int height,int type);
	BufferedImage(const wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture=false, const wstring &drive =L"");	// 
	BufferedImage(DLCPack *dlcPack, const wstring& File, bool filenameHasExtension = false ); // 
	BufferedImage(BYTE *pbData, DWORD dwBytes);	// 
	~BufferedImage();

	int getWidth();	
	int getHeight();
	void getRGB(int startX, int startY, int w, int h, intArray out,int offset,int scansize, int level = 0); //  level param
	int *getData();	// 
	int *getData(int level);	// 
	Graphics *getGraphics();
	int getTransparency();
	BufferedImage *getSubimage(int x, int y, int w, int h);

	void preMultiplyAlpha();
};