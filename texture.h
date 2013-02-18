#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "render.h"
#include <fstream>
#include <assert.h>

extern GLuint front_face_id, back_face_id, right_face_id;		
extern GLuint left_face_id, top_face_id, bottom_face_id;		

extern int SKYBOXXLIM, SKYBOXYLIM, SKYBOXZLIM;

//Represents a snap to be texture mapped onto the bounding box wall
class Snap {
	public:
		Snap(char* ps, int w, int h);
		~Snap();

		char* pixelArray;
		int w;
		int h;
};

//Just like auto_ptr, but for arrays
template<class T>
class vishakArray {
	private:
		T* array;
		mutable bool isReleased;
	public:
		explicit vishakArray(T* array_ = NULL) :
			array(array_), isReleased(false) {
		}
		
		vishakArray(const vishakArray<T> &aarray) {
			array = aarray.array;
			isReleased = aarray.isReleased;
			aarray.isReleased = true;
		}
		
		~vishakArray() {
			if (!isReleased && array != NULL) {
				delete[] array;
			}
		}
		
		T* get() const {
			return array;
		}
		
		T &operator*() const {
			return *array;
		}
		
		void operator=(const vishakArray<T> &aarray) {
			if (!isReleased && array != NULL) {
				delete[] array;
			}
			array = aarray.array;
			isReleased = aarray.isReleased;
			aarray.isReleased = true;
		}
		
		T* operator->() const {
			return array;
		}
		
		T* release() {
			isReleased = true;
			return array;
		}
	
		T* operator+(int i) {
			return array + i;
		}
		
		T &operator[](int i) {
			return array[i];
		}
};

// fetches the bitmap snap content from the bitmap file
Snap* storeBitmap(const char* fname);

GLuint ImageToGLTexture(Snap* snap);

short charArrayToShort(const char* chars);

short readShort(std::ifstream &in);

int charArrayToInt(const char* chars);

int readInteger(std::ifstream &in);

void renderSkyBox(GLuint front_face_id, GLuint back_face_id, GLuint right_face_id, GLuint left_face_id, GLuint top_face_id, GLuint bottom_face_id);

#endif