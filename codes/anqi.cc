/*****************************************************************************\
 * Theory of Computer Games: Fall 2013
 * Chinese Dark Chess Library by You-cheng Syu
 *
 * This file may not be used out of the class unless asking
 * for permission first.
 *
 * Modify by Hung-Jui Chang, December 2013
\*****************************************************************************/
#include<cassert>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"anqi.hh"
#include"HashTable.h"
#ifdef _WINDOWS
#include<windows.h>
#endif
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


static const char *tbl="KGMRNCPkgmrncpX-";





CLR GetColor(FIN f) {
	return f<FIN_X?f/7:-1;
}

LVL GetLevel(FIN f) {
	assert(f<FIN_X);
	return LVL(f%7);
}

bool ChkEats(FIN fa,FIN fb) {
	if(fa>=FIN_X)return false;
	if(fb==FIN_X)return false;
	if(fb==FIN_E)return true ;
	if(GetColor(fb)==GetColor(fa))return false;

	const LVL la=GetLevel(fa);
	if(la==LVL_C)return true ;

	const LVL lb=GetLevel(fb);
	if(la==LVL_K)return lb!=LVL_P;
	if(la==LVL_P)return lb==LVL_P||lb==LVL_K;

	return la<=lb;
}

static void Output(FILE *fp,POS p) {
	fprintf(fp,"%c%d\n",'a'+p%4,8-(p/4));
}

void Output(MOV m) {
	FILE *fp=fopen("move.txt","w");
	assert(fp!=NULL);
	if(m.ed!=m.st) {
		fputs("0\n",fp);
		Output(fp,m.st);
		Output(fp,m.ed);
		fputs("0\n",fp);
	} else {
		fputs("1\n",fp);
		Output(fp,m.st);
		fputs("0\n",fp);
		fputs("0\n",fp);
	}
	fclose(fp);
}

void BOARD::NewGame() {
	static const int tbl[]={1,2,2,2,2,2,5};
	who=-1;
	for(POS p=0;p<32;p++)fin[p]=FIN_X;
	for(int i=0;i<14;i++){
		cnt[i]=tbl[GetLevel(FIN(i))];
		brightCnt[i]=0;
	}
	hashVal = getHashVal(fin);
	totalDark=32, totalBright=0;
}

static FIN find(char c) {
	return FIN(strchr(tbl,c)-tbl);
}

static POS LoadGameConv(const char *s) {
	return (8-(s[1]-'0'))*4+(s[0]-'a');
}

static void LoadGameReplay(BOARD &brd,const char *cmd) {
	if(cmd[2]!='-')brd.Flip(LoadGameConv(cmd),find(cmd[3]));
	else brd.Move(MOV(LoadGameConv(cmd),LoadGameConv(cmd+3)));
}

static POS mkpos(int x,int y) {
	return x*4+y;
}

void BOARD::Init(int Board[32], int Piece[14], int Color) {
    for (int i = 0 ; i < 14; ++i) {
		cnt[i] = Piece[i];
		brightCnt[i]=0;
    }
    for (int i = 0 ; i < 32; ++i) {
		switch(Board[i]) {
		    case  0: fin[i] = FIN_E;break;
		    case  1: fin[i] = FIN_K;cnt[FIN_K]--, brightCnt[FIN_K]++;break;
		    case  2: fin[i] = FIN_G;cnt[FIN_G]--, brightCnt[FIN_G]++;break;
		    case  3: fin[i] = FIN_M;cnt[FIN_M]--, brightCnt[FIN_M]++;break;
		    case  4: fin[i] = FIN_R;cnt[FIN_R]--, brightCnt[FIN_R]++;break;
		    case  5: fin[i] = FIN_N;cnt[FIN_N]--, brightCnt[FIN_N]++;break;
		    case  6: fin[i] = FIN_C;cnt[FIN_C]--, brightCnt[FIN_C]++;break;
		    case  7: fin[i] = FIN_P;cnt[FIN_P]--, brightCnt[FIN_P]++;break;
		    case  8: fin[i] = FIN_X;break;
		    case  9: fin[i] = FIN_k;cnt[FIN_k]--, brightCnt[FIN_k]++;break;
		    case 10: fin[i] = FIN_g;cnt[FIN_g]--, brightCnt[FIN_g]++;break;
		    case 11: fin[i] = FIN_m;cnt[FIN_m]--, brightCnt[FIN_m]++;break;
		    case 12: fin[i] = FIN_r;cnt[FIN_r]--, brightCnt[FIN_r]++;break;
		    case 13: fin[i] = FIN_n;cnt[FIN_n]--, brightCnt[FIN_n]++;break;
		    case 14: fin[i] = FIN_c;cnt[FIN_c]--, brightCnt[FIN_c]++;break;
		    case 15: fin[i] = FIN_p;cnt[FIN_p]--, brightCnt[FIN_p]++;break;
		}
    }
    who = Color;
    hashVal=getHashVal(fin);
    totalDark=totalBright=0;
    for(int i=0;i<14;i++){
    	totalDark+=cnt[i];
    	totalBright+=brightCnt[i];
    }
}

void BOARD::Init(char Board[32], int Piece[14], int Color) {
    for (int i = 0 ; i < 14; ++i) {
		cnt[i] = Piece[i];
		brightCnt[i]=0;
    }
    for (int i = 0 ; i < 32; ++i) {
		switch(Board[i]) {
		    case '-': fin[i] = FIN_E;break;
		    case 'K': fin[i] = FIN_K;cnt[FIN_K]--, brightCnt[FIN_K]++;break;
		    case 'G': fin[i] = FIN_G;cnt[FIN_G]--, brightCnt[FIN_G]++;break;
		    case 'M': fin[i] = FIN_M;cnt[FIN_M]--, brightCnt[FIN_M]++;break;
		    case 'R': fin[i] = FIN_R;cnt[FIN_R]--, brightCnt[FIN_R]++;break;
		    case 'N': fin[i] = FIN_N;cnt[FIN_N]--, brightCnt[FIN_N]++;break;
		    case 'C': fin[i] = FIN_C;cnt[FIN_C]--, brightCnt[FIN_C]++;break;
		    case 'P': fin[i] = FIN_P;cnt[FIN_P]--, brightCnt[FIN_P]++;break;
		    case 'X': fin[i] = FIN_X;break;
		    case 'k': fin[i] = FIN_k;cnt[FIN_k]--, brightCnt[FIN_k]++;break;
		    case 'g': fin[i] = FIN_g;cnt[FIN_g]--, brightCnt[FIN_g]++;break;
		    case 'm': fin[i] = FIN_m;cnt[FIN_m]--, brightCnt[FIN_m]++;break;
		    case 'r': fin[i] = FIN_r;cnt[FIN_r]--, brightCnt[FIN_r]++;break;
		    case 'n': fin[i] = FIN_n;cnt[FIN_n]--, brightCnt[FIN_n]++;break;
		    case 'c': fin[i] = FIN_c;cnt[FIN_c]--, brightCnt[FIN_c]++;break;
		    case 'p': fin[i] = FIN_p;cnt[FIN_p]--, brightCnt[FIN_p]++;break;
		}
    }
    who = Color;
    hashVal=getHashVal(fin);
    totalBright=totalDark=0;
    for(int i=0;i<14;i++){
    	totalDark+=cnt[i];
    	totalBright+=brightCnt[i];
    }
}

int BOARD::LoadGame(const char *fn) {
	FILE *fp=fopen(fn,"r");
	assert(fp!=NULL);

	while(fgetc(fp)!='\n');

	while(fgetc(fp)!='\n');

	fscanf(fp," %*c");
	for(int i=0;i<14;i++)fscanf(fp,"%d",cnt+i);

	for(int i=0;i<8;i++) {
		fscanf(fp," %*c");
		for(int j=0;j<4;j++) {
			char c;
			fscanf(fp," %c",&c);
			fin[mkpos(i,j)]=find(c);
		}
	}

	int r;
	fscanf(fp," %*c%*s%d" ,&r);
	who=(r==0||r==1?r:-1);
	fscanf(fp," %*c%*s%d ",&r);

	for(char buf[64];fgets(buf,sizeof(buf),fp);) {
		if(buf[2]<'0'||buf[2]>'9')break;
		char xxx[16],yyy[16];
		const int n=sscanf(buf+2,"%*s%s%s",xxx,yyy);
		if(n>=1)LoadGameReplay(*this,xxx);
		if(n>=2)LoadGameReplay(*this,yyy);
	}

	fclose(fp);
	hashVal=getHashVal(fin);
	return r;
}

void BOARD::Display() const {
#ifdef _WINDOWS
	HANDLE hErr=GetStdHandle(STD_ERROR_HANDLE);
	for(int i=0;i<8;i++) {
		SetConsoleTextAttribute(hErr,8);
		for(int j=0;j<4;j++)fprintf(stderr,"[%02d]",mkpos(i,j));
		if(i==2) {
			SetConsoleTextAttribute(hErr,12);
			fputs("  ",stderr);
			for(int j=0;j<7;j++)for(int k=0;k<cnt[j];k++)fputs(nam[j],stderr);
		}
		fputc('\n',stderr);
		for(int j=0;j<4;j++) {
			const FIN f=fin[mkpos(i,j)];
			const CLR c=GetColor(f);
			SetConsoleTextAttribute(hErr,(c!=-1?12-c*2:7));
			fprintf(stderr," %s ",nam[fin[mkpos(i,j)]]);
		}
		if(i==0) {
			SetConsoleTextAttribute(hErr,7);
			fputs("  輪到 ",stderr);
			if(who==0) {
				SetConsoleTextAttribute(hErr,12);
				fputs("紅方",stderr);
			} else if(who==1) {
				SetConsoleTextAttribute(hErr,10);
				fputs("黑方",stderr);
			} else {
				fputs("？？",stderr);
			}
		} else if(i==1) {
			SetConsoleTextAttribute(hErr,7);
			fputs("  尚未翻出：",stderr);
		} else if(i==2) {
			SetConsoleTextAttribute(hErr,10);
			fputs("  ",stderr);
			for(int j=7;j<14;j++)for(int k=0;k<cnt[j];k++)fputs(nam[j],stderr);
		}
		fputc('\n',stderr);
	}
	SetConsoleTextAttribute(hErr,7);
#else
	for(int i=0;i<8;i++) {
		for(int j=0;j<4;j++)fprintf(stderr,"[%02d]",mkpos(7-i,j));
		if(i==2) {
			fputs("  ",stderr);
			for(int j=0;j<7;j++)for(int k=0;k<cnt[j];k++)
				fprintf(stderr,ANSI_COLOR_RED "%s" ANSI_COLOR_RESET ,nam[j]);
			/*fprintf(stderr,"\n");
			for(int j=7;j<14;j++)for(int k=0;k<cnt[j];k++)
				fprintf(stderr,ANSI_COLOR_RED "%s" ANSI_COLOR_RESET ,nam[j]);*/
		}
		fputc('\n',stderr);
		for(int j=0;j<4;j++) {
			const FIN f=fin[mkpos(7-i,j)];
			const CLR c=GetColor(f);
		if(c==0)
			fprintf(stderr, ANSI_COLOR_RED " %s " ANSI_COLOR_RESET,nam[fin[mkpos(7-i,j)]]);
		else if(c==1)
			fprintf(stderr, ANSI_COLOR_BLUE " %s " ANSI_COLOR_RESET,nam[fin[mkpos(7-i,j)]]);
		else
			fprintf(stderr, " %s " ,nam[fin[mkpos(7-i,j)]]);

		}
		if(i==0) {
			fputs("  輪到 ",stderr);
			if(who==0) {
				fprintf(stderr,ANSI_COLOR_RED "%s" ANSI_COLOR_RESET ,"紅方");
			} else if(who==1) {
				fprintf(stderr,ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET ,"黑方");
			} else {
				fputs("？？",stderr);
			}
		} else if(i==1) {
			fputs("  尚未翻出：",stderr);
		} else if(i==2) {
			fputs("  ",stderr);
			for(int j=7;j<14;j++)for(int k=0;k<cnt[j];k++)fprintf(stderr,ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET ,nam[j]);
		}
		fputc('\n',stderr);
	}
#endif
}



void BOARD::Display(FILE* flog) const {
#ifdef _WINDOWS
	HANDLE hErr=GetStdHandle(STD_ERROR_HANDLE);
#endif
	for(int i=0;i<8;i++) {
#ifdef _WINDOWS
		SetConsoleTextAttribute(hErr,8);
#endif
		for(int j=0;j<4;j++)fprintf(flog,"[%02d]",mkpos(i,j));
		if(i==2) {
#ifdef _WINDOWS
			SetConsoleTextAttribute(hErr,12);
#endif
			fputs("  ",flog);
			for(int j=0;j<7;j++)for(int k=0;k<cnt[j];k++)fputs(nam[j],flog);
		}
		fputc('\n',flog);
		for(int j=0;j<4;j++) {
			const FIN f=fin[mkpos(i,j)];
			const CLR c=GetColor(f);
#ifdef _WINDOWS
			SetConsoleTextAttribute(hErr,(c!=-1?12-c*2:7));
#endif
			fprintf(flog," %s ",nam[fin[mkpos(i,j)]]);
		}
		if(i==0) {
#ifdef _WINDOWS
			SetConsoleTextAttribute(hErr,7);
#endif
			fputs("  輪到 ",flog);
			if(who==0) {
#ifdef _WINDOWS
				SetConsoleTextAttribute(hErr,12);
#endif
				fputs("紅方",flog);
			} else if(who==1) {
#ifdef _WINDOWS
				SetConsoleTextAttribute(hErr,10);
#endif
				fputs("黑方",flog);
			} else {
				fputs("？？",flog);
			}
		} else if(i==1) {
#ifdef _WINDOWS
			SetConsoleTextAttribute(hErr,7);
#endif
			fputs("  尚未翻出：",flog);
		} else if(i==2) {
#ifdef _WINDOWS
			SetConsoleTextAttribute(hErr,10);
#endif
			fputs("  ",flog);
			for(int j=7;j<14;j++)for(int k=0;k<cnt[j];k++)fputs(nam[j],flog);
		}
		fputc('\n',flog);
	}
#ifdef _WINDOWS
	SetConsoleTextAttribute(hErr,7);
#endif
}

int sortScore_cmp(const void* a, const void* b) {
	MOV lhs = *(MOV*)a, rhs=*(MOV*)b;
	if(lhs.s==rhs.s) return 0;
	return lhs.s>rhs.s?-1:1;
}


int BOARD::MoveGen(MOVLST &lst, bool onlyEat) const {
	if(who==-1)return false;
	lst.num=0;
	int x=rand()%32;
	for(POS p=x;p<x+32;p++) {
		const FIN pf=fin[p%32];
		if(GetColor(pf)!=who)continue; //not self piece
		const LVL pl=GetLevel(pf);
		if(pl!=LVL_C){ //not cannon: check eat
			for(int z=0;z<4;z++) { //adjacent location
				const POS q=ADJ[p%32][z];
				if(q==-1)continue; //boarder
				const FIN qf=fin[q]; //adjacent piece
				
				if(qf!=FIN_E && ChkEats(pf,qf)) lst.mov[lst.num++]=MOV(p%32,q);
			}
		}
		else{ //for cannon: check eat
			for(int z=0;z<4;z++) { 
				int c=0;
				for(POS q=p%32;(q=ADJ[q][z])!=-1;) {
					const FIN qf=fin[q];
					if(qf==FIN_E||++c!=2)continue;
					if(qf!=FIN_X&&GetColor(qf)!=who)lst.mov[lst.num++]=MOV(p%32,q);
					break;
				}
			}
		}
	}
	if(onlyEat) return lst.num;

	x=rand()%32;
	for(POS p=x;p<x+32;p++) {
		const FIN pf=fin[p%32];
		if(GetColor(pf)!=who)continue; //not self piece
		for(int z=0;z<4;z++) { //adjacent location
			const POS q=ADJ[p%32][z];
			if(q==-1)continue; //boarder
			const FIN qf=fin[q]; //adjacent piece
			if(qf==FIN_E) lst.mov[lst.num++]=MOV(p%32,q);
		}
	}
	return lst.num;
}

bool BOARD::ChkLose() const {
	/*if(who==-1)return false;

	bool fDark=false;
	for(int i=0;i<14;i++) {
		if(cnt[i]==0)continue;
		if(GetColor(FIN(i))==who)return false;
		fDark=true;
	}

	bool fLive=false;
	for(POS p=0;p<32;p++)if(GetColor(fin[p])==who){fLive=true;break;}
	if(!fLive)return true;

	MOVLST lst;
	return !fDark&&MoveGen(lst, false)==0;*/
	for(int i=0;i<14;i++){
		if(GetColor(FIN(i))!=who) continue;
		if(cnt[i]>0 || brightCnt[i]>0) return false;
	}
	return true;
}

bool BOARD::ChkValid(MOV m) const {
	if(m.ed!=m.st) {
		MOVLST lst;
		MoveGen(lst, false);
		for(int i=0;i<lst.num;i++)if(m==lst.mov[i])return true;
	} else {
		if(m.st<0||m.st>=32)return false;
		if(fin[m.st]!=FIN_X)return false;
		for(int i=0;i<14;i++)if(cnt[i]!=0)return true;
	}
	return false;
}


void BOARD::Flip(POS p,FIN f) {
	if(f==FIN_X) {
		int i,sum=0;
		for(i=0;i<14;i++) sum+=cnt[i];
		sum=rand()%sum;
		for(i=0;i<14;i++)if((sum-=cnt[i])<0)break;
		f=FIN(i);
	}
	fin[p]=f;
	cnt[f]--, brightCnt[f]++;
	totalDark--, totalBright++;

	if(who==-1)who=GetColor(f);
	who^=1;
	hashVal= modHashVal(hashVal, FIN_X, p);
	hashVal= modHashVal(hashVal, f, p);
}

void BOARD::Move(MOV m) {
	if(m.ed!=m.st) {
		if(fin[m.ed]!=FIN_E){
			brightCnt[fin[m.ed]]--;
			totalBright--;
		}
		hashVal = modHashVal(hashVal, fin[m.st], m.st);
		hashVal = modHashVal(hashVal, fin[m.ed], m.ed);
		hashVal = modHashVal(hashVal, fin[m.st], m.ed);
		fin[m.ed]=fin[m.st];
		fin[m.st]=FIN_E;
		who^=1;
	} else {
		Flip(m.st);
	}
}

void BOARD::DoMove(MOV m, FIN f) {
    if (m.ed!=m.st) {
    	if(fin[m.ed]!=FIN_E){
    		brightCnt[fin[m.ed]]--;
    		totalBright--;
    	}
    	hashVal = modHashVal(hashVal, fin[m.st], m.st);
		hashVal = modHashVal(hashVal, fin[m.ed], m.ed);
		hashVal = modHashVal(hashVal, fin[m.st], m.ed);
		fin[m.ed]=fin[m.st];
		fin[m.st]=FIN_E;
		who^=1;
    }
    else {
		Flip(m.st, f);
    }
}
