#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include "HashTable.h"
#include "anqi.hh"
#include "Search.h"
#ifdef _WINDOWS
#include<windows.h>
#else
#include<ctime>
#endif

#define INF 1000001
#define WIN 1000000

#define DEBUG 0
using namespace std;


const int STAT_VAL[]={12863,6431,3215,1607,803,401,200};
int DYN_VAL[]={12863,6431,3215,1607,803,401,200,12863,6431,3215,1607,803,401,200};

FILE* flog = fopen("mylog.txt", "w+");
#ifdef _WINDOWS
	DWORD sTick;     // 開始時刻
	int   sTimeOut;  // 時限
#else
	clock_t sTick;     // 開始時刻
	clock_t sTimeOut;  // 時限
#endif
int DEFAULTTIME_S = 10;
int DEFAULTTIME_F = 5;
int SEARCH_MV_DEP=8;
int SEARCH_FLIP_DEP=4;
bool isEndgame=false;

bool searchTimesUp() {
#ifdef _WINDOWS
	return GetTickCount()-sTick>=sTimeOut;
#else
	return clock() - sTick > sTimeOut;
#endif
}

SCORE getPawnValue(const BOARD &board){
	int s[2]={0,0};

	for(int i=0;i<14;i++)
		s[GetColor(FIN(i))]+=(board.cnt[i]+board.brightCnt[i])*DYN_VAL[FIN(i)];
	
	
	if(DEBUG){
		fprintf(flog, "s=%d (%d)\n", s[board.who]-s[board.who^1], board.who);
		fflush(flog);
	}
	return s[board.who]-s[board.who^1];
}

SCORE bfs(const BOARD &board, POS s){
	if(GetLevel(board.fin[s])==LVL_C) return 0;

	queue<POS> que;
	SCORE score=0;
	int dist[32];
	POS nowP, nextP;
	FIN nowF, nextF;
	for(int i=0;i<32;i++) dist[i]=INF;
	
	que.push(s), dist[s]=0;
	while(!que.empty()){
		nowP = que.front(), que.pop();
		nowF=board.fin[nowP];
		for(int i=0;i<4;i++) { //adjacent location
			nextP=ADJ[nowP][i];
			if(nextP==-1 || dist[nextP]!=INF) continue; //boarder
			nextF=board.fin[nextP]; //adjacent piece
			if((nowF<14 && !ChkEats(nowF,nextF)) || nextF==FIN_X) continue;
			dist[nextP]=dist[nowP]+1;
			que.push(nextP);

			if(ChkEats(nowF,nextF)) score+=(10-dist[nextP])*2*DYN_VAL[nextF]/100;
		}
	}
	return score;
}

SCORE getAttackValue(const BOARD &board){
	if(!isEndgame) return 0;
	//printf("%d %d\n", board.totalDark, board.totalDark+board.totalBright);
	POS pos[2][32];
	int cnt[2]={0};
	SCORE s[2]={0};

	for(int i=0;i<32;i++){ //put fin into corresponding chess array
		if(board.fin[i]>=14) continue; //is FIN_E or FIN_X
		CLR clr = GetColor(board.fin[i]);
		pos[clr][cnt[clr]++]=i;
	}

	int self=board.who, oppo=board.who^1;
	for(int i=0;i<cnt[self];i++)
		s[self]+=bfs(board, pos[self][i]);
	for(int i=0;i<cnt[oppo];i++)
		s[oppo]+=bfs(board, pos[oppo][i]);	
	
	return s[self]-s[oppo];
}

SCORE getPositionValue(const BOARD &board){
	SCORE s[2]={0};
	for(int nowP=0;nowP<32;nowP++){
		FIN f=board.fin[nowP];
		if(f>=14 ||GetLevel(f)==LVL_C) continue; //is FIN_E or FIN_X

		CLR clr = GetColor(f);
		for(int i=0;i<4;i++) { //adjacent location
			POS nextP=ADJ[nowP][i];
			if(nextP==-1) continue; //boarder

			FIN nextF=board.fin[nextP];
			if(nextF==FIN_X || GetColor(nextF)==GetColor(f) || !ChkEats(f,nextF)) continue;//cant move to this position

			bool safe=true;
			for(int j=0;j<4;j++){ //adjacent location after move i
				POS nnP=ADJ[nextP][j];
				if(nnP==nowP || nnP==-1) continue;
				FIN nnF=board.fin[nnP];
				if(nnF>=14) continue;
				if(GetColor(nnF)!=GetColor(f) && GetLevel(nnF)!=LVL_C && ChkEats(nnF,f)){
					safe=false; //not safe after move to this position
					break;
				}
			}
			if(safe) s[clr]+=(7-GetLevel(f));
		}
	}
	return s[board.who]-s[board.who^1];
}

SCORE evaluate(const BOARD &board, MOVLST &hist){ //evaluate by poit of "board.who"
	//todo: advanced evaluating function
	if(DEBUG){
		for(int i=0;i<hist.num;i++) fprintf(flog, "(%d-%d) ", hist.mov[i].st, hist.mov[i].ed);
		fprintf(flog,"\n");
		board.Display(flog);
	}

	
	if (board.ChkLose()) return -WIN; //check lose
	SCORE pawnS = getPawnValue(board);
	SCORE attackS = getAttackValue(board);
	SCORE positionS = getPositionValue(board);
	//printf("%d %d %d\n", pawnS, attackS, positionS);
	//printf("%d %d\n", board.totalDark, board.totalBright);
	return pawnS+attackS+positionS;
}

SCORE SearchEngine::NegaScout(BOARD &board, int depth, SCORE alpha, SCORE beta, MOVLST &hist, int unflipCnt){
	MOVLST lst;
	board.MoveGen(lst, false);
	if(depth>0 && unflipCnt>0) lst.mov[lst.num++]=MOV(-1,-1); //null move (assume flip)

	//todo: timing constraint
	if(depth<=0 || lst.num==0) //quiescent search
		board.MoveGen(lst, true); //reproduce move list : true for quiescent search
	
	if(lst.num==0) return evaluate(board, hist); //terminate 

	SCORE m=-INF, n=beta; //fail soft
	
	if(depth>0){ //check hash
		EVal e = p_hashTbl->find(board.hashVal, board.fin, board.who);
		if(e.valid && e.layer>=depth){ //hash hit
			if(e.type==ETYP_E) return e.value; //is exact value: return
			else m=e.value; //failed high value : use as bound
		}
	}

	//search deeper
	for(int i=0;i<lst.num;i++){
		BOARD tmpBoard(board);
		int isFlip=0;
		if(!(lst.mov[i].st==-1 && lst.mov[i].ed==-1))
			tmpBoard.Move(lst.mov[i]); //do move
		else{ //assume flip
			tmpBoard.who^=1;
			isFlip=1;
		}
		hist.mov[hist.num++]=lst.mov[i];

		int t= -NegaScout(tmpBoard, depth-1, -n, -max(alpha,m), hist, unflipCnt-isFlip); //null window search

		if(t>m){
			if(n==beta || (depth>0 && depth<3) || t>=beta) m=t; //first branch || depth<3(scout returns exact value) || fali high 
			else m= -NegaScout(tmpBoard, depth-1, -beta, -t, hist, unflipCnt-isFlip); //re-search
		}
		hist.num--;

		if(m>=beta){
			if(DEBUG) fprintf(flog, "beta cutoff %d %d\n", m, beta);
			if(depth>0) p_hashTbl->insert(board, m, depth, ETYP_H);
			return m; //beta cut off
		}

		n=max(alpha,m)+1; //set value for null window search
	}
	if(depth>0) p_hashTbl->insert(board, m, depth, ETYP_E);
	return m;
}

MOV SearchEngine::genMove(const BOARD &board, int remain_time){
	if(remain_time<80000){
		printf("goint to timeout -> search 1 s\n");
		DEFAULTTIME_S=DEFAULTTIME_F=4; //prevent timeout
	}

#ifdef _WINDOWS
	sTick=GetTickCount();
	sTimeOut = (DEFAULTTIME_S-3)*1000;
#else
	sTick=clock();
	sTimeOut = (DEFAULTTIME_S-3)*CLOCKS_PER_SEC;
#endif

	POS p;
	POS firstMove[4]={5,6,25,26};
	int sdep=0;
	if(board.who==-1){ //new game
		p=firstMove[rand()%4];
		return MOV(p,p);
	}

	//search for unfilp
	vector<FIN> unflipFin;
	for(int i=0;i<14;i++)
		if(board.cnt[i]>0) unflipFin.push_back(FIN(i));
	int unflipCnt=unflipFin.size();

	//check endgame
	if(!board.HasDark() || board.totalDark+board.totalBright<12) {
		isEndgame=true;
		DEFAULTTIME_S=5;
		printf("Enter end game mode\n");
	}

	//dyn piece val
	for(int i=board.who*7;i<board.who*7+7;i++){
		bool useless=true;
		if(board.cnt[i]==0 && board.brightCnt[i]==0) continue;
		for(int j=(board.who^1)*7;j<=(board.who^1)*7+7;j++){
			if(board.cnt[j]==0 && board.brightCnt[j]==0) continue;
			if(ChkEats(FIN(i),FIN(j))){
				useless=false;
				break;
			}
		}
		if(useless){
			printf("%s useless\n", nam[i]);
			DYN_VAL[i]=0;
		}
	}

	//nega scout (search for move)
	MOVLST lst, hist;
	SCORE mvScore=-INF;
	MOV best_mv, best_flip;
	int best_d=0;

	board.MoveGen(lst, false);

	for(int d=0;d<=SEARCH_MV_DEP && ! searchTimesUp();d++){ //iterative deepening
		if(d>0) lst.sortScore();
		mvScore=-INF;
		for(int i=0;i<lst.num;i++) {
			BOARD tmpBoard(board);
			tmpBoard.Move(lst.mov[i]);
			
			hist.mov[hist.num++]=lst.mov[i];
			SCORE s=-NegaScout(tmpBoard, d, -INF, INF, hist, unflipCnt);
			hist.num--;
			lst.mov[i].s=s;

			if(DEBUG) fprintf(flog, "d:%d, mv=%d-%d, score=%d\n", d, lst.mov[i].st, lst.mov[i].ed, s);
			if(s>mvScore){
				mvScore=s;
				best_mv=lst.mov[i];
				best_d=d;
			}
			if(s==WIN) break;
		}
		sdep++;
		printf("search %d depth done\n", d);

		if(DEBUG) fprintf(flog, "d:%d, best score:%d, best_mv=(%d,%d)\n", d, mvScore, best_mv.st, best_mv.ed);
		if(mvScore==WIN){
			printf("#mv:%d, best score:%d, best_mv=(%d,%d)\n", lst.num, mvScore, best_mv.st, best_mv.ed);
			printf("search depth:%d\n", sdep);
			return best_mv; //cant flip || win , return the best move
		}
	}
	printf("#mv:%d, best score:%d, best_mv=(%d,%d)\n", lst.num, mvScore, best_mv.st, best_mv.ed);
	printf("search depth:%d\n", sdep);
	if(unflipCnt==0) return best_mv;
	
	//nega scout (search for flip)
#ifdef _WINDOWS
	sTick=GetTickCount();
	sTimeOut = (DEFAULTTIME_F-3)*1000;
#else
	sTick=clock();
	sTimeOut = (DEFAULTTIME_F-3)*CLOCKS_PER_SEC;
#endif
	int best_gt,best_sum;
	SCORE flipScore;
	MOVLST flst;

	for(p=0;p<32;p++)
		if(board.fin[p]==FIN_X) flst.mov[flst.num++]=MOV(p,p);
	
	sdep=0;
	for(int d=0;d<=SEARCH_FLIP_DEP && !searchTimesUp();d++){ //iterative deepening
		if(d>0) lst.sortScore();
		best_gt=best_sum=flipScore=-INF;
		for(int i=0;i<flst.num;i++) { //for each unflip position
			p=flst.mov[i].st;
			int gt=0, sum=0;
			for(int j=0;j<unflipCnt;j++){ //for each type of FIN
				int cnt=board.cnt[unflipFin[j]];
				BOARD tmpBoard(board);
				tmpBoard.Flip(p, unflipFin[j]);

				hist.mov[hist.num++]=MOV(p,p);
				SCORE s = -NegaScout(tmpBoard, d, -INF, INF, hist, unflipCnt-1);
				hist.num--;
				if(DEBUG){
					MOVLST tmpLst;
					tmpBoard.MoveGen(tmpLst, false);
					fprintf(flog, "mv=%d-%d, score=%d, #move=%d\n", p, unflipFin[j], s, tmpLst.num);
				}
				if(s>mvScore) gt+=cnt;
				else if(s<mvScore) gt-=cnt;
				sum+=(s-mvScore)*cnt;
			}
			flst.mov[i].s=sum;
			if(gt>best_gt || (gt==best_gt && sum>best_sum)){
				best_gt=gt, best_sum=sum, best_flip=MOV(p,p);
				flipScore=sum;
			}
		}
		if(DEBUG){
			fprintf(flog, "best flip score:%d, best_flip=(%d,%d)\n", flipScore, best_flip.st, best_flip.ed);
			fflush(flog);
		}
		printf("flip %d depth done\n", d);
		sdep++;
	}
	printf("best_gt:%d, best_flip=(%d,%d)\n", best_gt, best_flip.st, best_flip.ed);
	printf("filp depth:%d\n", sdep);
	return best_gt>=0 ? best_flip : best_mv;
}