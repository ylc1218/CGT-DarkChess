#ifndef ANQI
#define ANQI
#include <algorithm>
#include <cstdlib>
using namespace std;

// (color)
//  0 = 紅方 (大寫字母)
//  1 = 黑方 (小寫字母)
// -1 = 都不是
typedef int CLR;

// (level)
enum LVL {
	LVL_K=0, // 帥將 King
	LVL_G=1, // 仕士 Guard
	LVL_M=2, // 相象 Minister
	LVL_R=3, // 硨車 Rook     // BIG5 沒有人部的車
	LVL_N=4, // 傌馬 kNight
	LVL_C=5, // 炮砲 Cannon
	LVL_P=6  // 兵卒 Pawn
};

enum FIN {
	FIN_K= 0 /* 'K' 帥 */ , FIN_k= 7 /* 'k' 將 */ , FIN_X=14 /* 'X' 未翻 */ ,
	FIN_G= 1 /* 'G' 仕 */ , FIN_g= 8 /* 'g' 士 */ , FIN_E=15 /* '-' 空格 */ ,
	FIN_M= 2 /* 'M' 相 */ , FIN_m= 9 /* 'm' 象 */ ,
	FIN_R= 3 /* 'R' 硨 */ , FIN_r=10 /* 'r' 車 */ ,
	FIN_N= 4 /* 'N' 傌 */ , FIN_n=11 /* 'n' 馬 */ ,
	FIN_C= 5 /* 'C' 炮 */ , FIN_c=12 /* 'c' 砲 */ ,
	FIN_P= 6 /* 'P' 兵 */ , FIN_p=13 /* 'p' 卒 */
};


// (position)
//  0  1  2  3
//  4  5  6  7
//  8  9 10 11
// 12 13 14 15
// 16 17 18 19
// 20 21 22 23
// 24 25 26 27
// 28 29 30 31
typedef int POS;
typedef int SCORE;

static const POS ADJ[32][4]={
	{ 1,-1,-1, 4},{ 2,-1, 0, 5},{ 3,-1, 1, 6},{-1,-1, 2, 7},
	{ 5, 0,-1, 8},{ 6, 1, 4, 9},{ 7, 2, 5,10},{-1, 3, 6,11},
	{ 9, 4,-1,12},{10, 5, 8,13},{11, 6, 9,14},{-1, 7,10,15},
	{13, 8,-1,16},{14, 9,12,17},{15,10,13,18},{-1,11,14,19},
	{17,12,-1,20},{18,13,16,21},{19,14,17,22},{-1,15,18,23},
	{21,16,-1,24},{22,17,20,25},{23,18,21,26},{-1,19,22,27},
	{25,20,-1,28},{26,21,24,29},{27,22,25,30},{-1,23,26,31},
	{29,24,-1,-1},{30,25,28,-1},{31,26,29,-1},{-1,27,30,-1}
};

static const char *nam[16]={
	"帥","仕","相","硨","傌","炮","兵",
	"將","士","象","車","馬","砲","卒",
	"Ｏ","　"
};

struct MOV {
	POS st; // 起點
	POS ed; // 終點 // 若 ed==st 表示是翻子
	SCORE s;

	MOV() {s=0;}
	MOV(POS s,POS e):st(s),ed(e) {s=0;}
	MOV(POS s,POS e,SCORE score):st(s),ed(e) {this->s=score;}
	bool operator==(const MOV &x) const {return st==x.st&&ed==x.ed;}
	MOV operator=(const MOV &x) {st=x.st;ed=x.ed;s=x.s;return MOV(x.st, x.ed, x.s);}
};

int sortScore_cmp(const void* a, const void* b);

struct MOVLST {
	int num;     // 走法數(移動+吃子,不包括翻子)
	MOV mov[68];
	MOVLST(){num=0;}

	void sortScore(){ 
		//std::sort(&mov[0],&mov[num], &cmp);
		qsort(mov, num, sizeof(mov[0]), sortScore_cmp);
	}

};



struct BOARD {
	CLR who;     // 現在輪到那一方下
	FIN fin[32]; // 各個位置上面擺了啥
	int cnt[14]; // 各種棋子的未翻開數量
	int brightCnt[14]; //count of each type of chess faced-up
	long long hashVal; //hashVal % BKT_SIZE = hashIdx
	int totalDark, totalBright;

	void NewGame();              // 開新遊戲
	int  LoadGame(const char*);  // 載入遊戲並傳回時限(單位:秒)
	void Display() const;        // 顯示到 stderr 上
	void Display(FILE* flog) const;
	int  MoveGen(MOVLST&, bool onlyEat) const; // 列出所有走法(走子+吃子,不包括翻子)
	                             // 回傳走法數量
	bool ChkLose() const;        // 檢查當前玩家(who)是否輸了
	bool HasDark() const{return totalDark>0;}; //check if still has dark chess
	bool ChkValid(MOV) const;    // 檢查是否為合法走法
	void Flip(POS,FIN=FIN_X);    // 翻子
	void Move(MOV);              // 移動 or 吃子
	void DoMove(MOV m, FIN f) ;
	void Init(int Board[32], int Piece[14], int Color);
	void Init(char Board[32], int Piece[14], int Color);
};

CLR  GetColor(FIN);    // 算出棋子的顏色
LVL  GetLevel(FIN);    // 算出棋子的階級
bool ChkEats(FIN,FIN); // 判斷第一個棋子能否吃第二個棋子
void Output (MOV);     // 將答案傳給 GUI

#endif
