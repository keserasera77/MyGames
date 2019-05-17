//製作時間::ちょうど5時間ほど
//改良::1時間  元々のmoveの役割をmoveWithInputへ変更し、moveWithInputの部品としてmoveを追加。さらにその部品にleaveを追加
//サンプルコードを参考に、ステージの表現方法の改変、Array2D,Stateの追加、移動時の範囲チェックの追加、readFileの追加
//Stateのメンバ変数mPx,mPyを追加

#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

void readFile(char** buffer, int* size, const char* filename);

template<class T> class Array2D {
public:
	Array2D() : mArray(0){}
	~Array2D(){
		delete[] mArray;
		mArray = 0;
	}
	void setSize(int width,int height){
		mWidth = width;
		mHeight = height;
		mArray = new T[width * height];
	}
	T& operator()(int x,int y){
		return mArray[y*mWidth + x];
	}
	const T& operator()(int x,int y) const {
		return mArray[y*mWidth + x];
	}
private:
	T* mArray;
	int mWidth;
	int mHeight;
};

class State{
public:
	State(const char* stage, int size);
	void update(char input);
	void drawStage() const;
	void clearCheck() const;
private:
	enum Object{
		OBJ_SPACE,
		OBJ_WALL,
		OBJ_GOAL,
		OBJ_BLOCK,
		OBJ_MAN,

		OBJ_UNKNOWN,
	};
	int mWidth;
	int mHeight;
	int mPx;
	int mPy;
	Array2D<Object> mObjects;
	Array2D<bool> mGoalFlags;
	void setSize(const char* stage, int size);
};

int main() {
  char* stageData;
  int fileSize;
  readFile(&stageData, &fileSize, "stageData.txt");
  State stage(stageData,fileSize);

  stage.drawStage();

  while(1){
    char input;
    cin >> input;
    stage.update(input);
    stage.drawStage();
    stage.clearCheck();
  }
}

void readFile(char** buffer, int* size, const char* filename) {
	ifstream in(filename);
	if (!in) {
	  *buffer = 0;
		*size = 0;
	}
	else {
		in.seekg(0, ifstream::end);
		*size = static_cast<int>(in.tellg());
		in.seekg(0, ifstream::beg);
		*buffer = new char[*size];
		in.read(*buffer, *size);
	}
}


State::State(const char* stage, int size){

	setSize(stage,size);
	mObjects.setSize(mWidth,mHeight);
	mGoalFlags.setSize(mWidth,mHeight);

  //mObjects,mGoalFlagsの初期化 いる？？
	for ( int y = 0; y < mHeight; ++y ){
		for ( int x = 0; x < mWidth; ++x ){
			mObjects( x, y ) = OBJ_WALL; 
			mGoalFlags( x, y ) = false; 
		}
	}

	int x = 0;
	int y = 0;
	for(int i=0; i < size; i++){
		Object t;
		bool goalFlag = false;
		switch (stage[i]) {
			case '#': t = OBJ_WALL; break;
			case ' ': t = OBJ_SPACE; break;
			case 'o': t = OBJ_BLOCK; break;
			case 'O': t = OBJ_BLOCK; goalFlag = true; break;
			case '.': t = OBJ_SPACE; goalFlag = true; break;
			case 'p': t = OBJ_MAN; break;
			case 'P': t = OBJ_MAN; goalFlag = true; break;
			case '\n': x = 0; ++y; t = OBJ_UNKNOWN; break; //���s����
			default: t = OBJ_UNKNOWN; break;
		}

		if (t == OBJ_MAN) {
			mPx = x;
			mPy = y;
		}

		if (t != OBJ_UNKNOWN) {
			mObjects(x,y) = t;
			mGoalFlags(x,y) = goalFlag;
			x++;
		}
	}
}

void State::setSize(const char* stage, int size){
	int x = 0;
	int y = 0;
	for(int i=0; i < size; i++){
		switch (stage[i]) {
			case '#': case 'o': case 'O' : case '.':
			case 'p': case 'P': case ' ':
			  x++;
			  break;
			case '\n':
			  y++;
        mWidth = max(mWidth,x);
			  mHeight = max(mHeight,y);
			  x=0;
			  break;
		}
	}
}

void State::clearCheck() const {
  for(int y=0;y<mHeight;y++){
    for(int x=0;x<mWidth;x++){
      if(mObjects(x,y) == OBJ_BLOCK && mGoalFlags(x,y) == false) return;
    }
  }
  cout << "Congratulation!!" << endl;
}

void State::drawStage() const {
  for(int y=0;y<mHeight;y++){
    for(int x=0;x<mWidth;x++){
			char obj = ' ';
			bool goalFlag = mGoalFlags(x, y);
			switch (mObjects(x, y)) {
			case OBJ_WALL: obj = '#'; break;
			case OBJ_SPACE: (goalFlag == true) ? obj = '.' : obj = ' '; break;
			case OBJ_BLOCK: (goalFlag == true) ? obj = 'O' : obj = 'o'; break;
			case OBJ_MAN: (goalFlag == true) ? obj = 'P' : obj = 'p'; break;
			case OBJ_UNKNOWN: break;
			default: break;
			}
			cout << obj;
    }
		cout << endl;
  }
}

void State::update(char input){
	int dPx = 0, dPy = 0;

	switch (input) {
	case 'a': dPx = -1; break;
	case 's': dPy = 1 ; break;
	case 'd': dPx = 1 ; break;
	case 'w': dPy = -1; break;
	}

	//範囲チェック
	if (mPx + dPx < 0 || mWidth <= mPx + dPx || mPy + dPy < 0 || mWidth <= mPy + dPy) {
		return;
	}
  switch (mObjects(mPx + dPx, mPy + dPy)){
    case OBJ_SPACE:
			mObjects(mPx + dPx, mPy + dPy) = OBJ_MAN;
			mObjects(mPx, mPy) = OBJ_SPACE;
			mPx += dPx; mPy += dPy;
			break;
    case OBJ_BLOCK:
			//範囲チェック
			if (mPx + 2 * dPx < 0 || mWidth <= mPx + 2 * dPx || mPy + 2 * dPy < 0 || mWidth <= mPy + 2 * dPy) {
				return;
			}
      switch (mObjects(mPx + 2*dPx, mPy + 2*dPy)) {
        //'#','o','O'の場合はその場に残る
        case OBJ_SPACE:
					mObjects(mPx + 2*dPx, mPy + 2*dPy) = OBJ_BLOCK;
					mObjects(mPx + dPx, mPy + dPy) = OBJ_MAN;
					mObjects(mPx, mPy) = OBJ_SPACE;
					mPx += dPx; mPy += dPy;
					break;
      }
			break;
    }
}
