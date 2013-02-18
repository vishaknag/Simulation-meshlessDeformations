#include "texture.h"


GLuint front_face_id, back_face_id, right_face_id;		
GLuint left_face_id, top_face_id, bottom_face_id;

// skybox limit
int SKYBOXXLIM = 2;
int SKYBOXYLIM = 2;
int SKYBOXZLIM = 2;

Snap::Snap(char* pA, int width, int height) : pixelArray(pA), w(width), h(height) 
{}

Snap::~Snap() {
	delete[] pixelArray;
}

GLuint ImageToGLTexture(Snap* snap) 
{
	GLuint textureResult;	// stores the resulting texture Id

	glGenTextures(1, &textureResult); 

	glBindTexture(GL_TEXTURE_2D, textureResult);	// Bind the texture Id, i.e; indicate openGL about this
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, snap->w, snap->h, 0, GL_RGB, GL_UNSIGNED_BYTE, snap->pixelArray);               

	return textureResult;					   
}

;		

// Convert a char array of two chars into short value
short charArrayToShort(const char* chars) {
	
	return (short)(
					((unsigned char)chars[1] << 8) |
					(unsigned char)chars[0]
				   );
}


short readShort(std::ifstream &in) {

	char store[2];
	in.read(store, 2);
	return charArrayToShort(store);
}


// Convert a char array of four chars ainto an integer value
int charArrayToInt(const char* chars) {
	
	return (int)(
				 ((unsigned char)chars[3] << 24) | ((unsigned char)chars[2] << 16) |
				 ((unsigned char)chars[1] << 8) | (unsigned char)chars[0]
				 );

}

int readInteger(std::ifstream &in) {

	char store[4];

	in.read(store, 4);
	return charArrayToInt(store);

}


Snap* storeBitmap(const char* fname) 
{
	// Open the file to read the bitmap snap content
	std::ifstream in;
	in.open(fname, std::ifstream::binary);
	assert(!in.fail() || !"Could not find file");

	char store[2];
	in.read(store, 2);

	// Check for the first two characters of the snap file, if it has 
	// "B" and "M" then its a bmp file
	assert(store[0] == 'B' && store[1] == 'M' || !"Not a bitmap file");
	in.ignore(8);
	int info = readInteger(in);
	
	// Fetch the header content
	int sizeH = readInteger(in);
	int w, h;
	
	w = readInteger(in);
	h = readInteger(in);
	in.ignore(2);
	assert(readShort(in) == 24 || !"Image is not 24 bits per pixel");
	assert(readShort(in) == 0 || !"Image is compressed");
	
	int BPerRCount = ((w * 3 + 3) / 4) * 4 - (w * 3 % 4);
	int sizeA = BPerRCount * h;
	
	vishakArray<char> pixelArrayObj(new char[sizeA]);
	
	in.seekg(info, std::ios_base::beg);
	in.read(pixelArrayObj.get(), sizeA);
	
	//Get the data into the right format
	vishakArray<char> pixelArrayObj2(new char[w * h * 3]);

	for(int y = 0; y < h; y++) {
		for(int x = 0; x < w; x++) {
			for(int c = 0; c < 3; c++) {
				pixelArrayObj2[3 * (w * y + x) + c] =
					pixelArrayObj[BPerRCount * y + 3 * x + (2 - c)];
			}
		}
	}
	
	in.close();
	return new Snap(pixelArrayObj2.release(), w, h);
}


void renderSkyBox(GLuint front_face_id, GLuint back_face_id, GLuint right_face_id, GLuint left_face_id, GLuint top_face_id, GLuint bottom_face_id)
{
  
	glColor4f(1,1,1,1);

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	// LEFT FACE
	
	  glBindTexture(GL_TEXTURE_2D, left_face_id);


	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  
	  glBegin(GL_QUADS);
	  
	  glNormal3f(1.0, 0.0f, 0.0f);
	  glTexCoord2f(0.0f, 0.0f);
	  glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	  glTexCoord2f(1.0f, 0.0f);
	  glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	  glTexCoord2f(1.0f, 1.0f);
	  glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	  glTexCoord2f(0.0f, 1.0f);
	  glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	  glEnd();

	// RIGHT FACE
	
	  glBindTexture(GL_TEXTURE_2D, right_face_id);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(-1.0, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);

	glEnd();

	// BACK FACE
	glBindTexture(GL_TEXTURE_2D, back_face_id);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 0.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();

	// FRONT FACE

	glBindTexture(GL_TEXTURE_2D, front_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 0.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);

	glEnd();


	// BOTTOM FACE
	glBindTexture(GL_TEXTURE_2D, bottom_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-SKYBOXXLIM, -SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();


	// TOP FACE
	glBindTexture(GL_TEXTURE_2D, top_face_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBegin(GL_QUADS);

	glNormal3f(0.0, -1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(SKYBOXXLIM, SKYBOXYLIM, -SKYBOXZLIM);

	glEnd();
	glEnable(GL_LIGHTING);

	return;
}
