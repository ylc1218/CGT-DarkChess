#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include "HashTable.h"
#include "anqi.hh"
#include "Search.h"

#define INF 1000001
#define WIN 1000000
#define SEARCH_MV_DEP 8
#define SEARCH_FLIP_DEP 4
#define DEBUG 0
using namespace std;


const int STAT_VAL[]={12863,6431,3215,1607,803,401,200};
FILE* flog = fopen("mylog.txt", "w+");


SCORE getPawnValue(const BOARD &board){
	int s[2]={0,0};
	for(POS p=0;p<32;p++){
		const CLR c=GetColor(board.fin[p]);
		if(c!=-1) s[c]+=STAT_VAL[GetLevel(board.fin[p])];
	}
	for(int i=0;i<14;i++)
		s[GetColor(FIN(i))]+=board.cnt[i]*STAT_VAL[GetLevel(FIN(i))];
	
	if(DEBUG){
		fprintf(flog, "s=%d (%d)\n", s[board.who]-s[board.who^1], board.who);
		fflush(flog);
	}
	return s[board.who]-s[board.who^1];
}

int bfs(const BOARD &board, POS s){
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
			if(nowF<14 && !ChkEats(nowF,nextF)) continue;
			dist[nextP]=dist[nowP]+1;
			que.push(nextP);

			if(ChkEats(nowF,nextF)) score+=(10-dist[nextP])*2*STAT_VAL[nextF]/100;
		}
	}

}

SCORE getAttackValue(const BOARD &board){
	if(board.HasDark() || board.totalDark+board.totalBright>12) return 0;
	POS pos[2][32];
	int cnt[2]={0};
	SCORE s[2]={0};

	for(int i=0;i<32;i++){ //put fin into corresponding chess array
		if(board.fin[i]>=14) continue; //is FIN_E or FIN_X
		CLR clr = GetColor(board.fin[i]);
		pos[clr][cnt[clr]++]=i;
	}

	int self=board.who, oppo=board.who^1;
	for(int i=0;i<cnt[self];i++){
		/*for(int j=0;j<cnt[oppo];j++){
			POS selfP=pos[self][i], oppoP=pos[oppo][j];
			int dist = abs(selfP/4-oppoP/4)+abs(selfP%4-oppoP%4);
			if(GetLevel(board.fin[selfP])!=LVL_C && ChkEats(board.fin[selfP], board.fin[oppoP]))
				s[self]+=(10-dist)*2*STAT_VAL[board.fin[oppoP]]/100;
			if(GetLevel(board.fin[oppoP])!=LVL_C && ChkEats(board.fin[oppoP], board.fin[selfP]))
				s[oppo]+=(10-dist)*2*STAT_VAL[board.fin[selfP]]/100;
		}*/
		s[self]+=bfs(board, pos[self][i]);
	}
	for(int i=0;i<cnt[oppo];i++)
		s[oppo]+=bfs(board, pos[oppo][i]);	
	
	return s[self]-s[oppo];
}

SCORE evaluate(const BOARD &board, MOVLST &hist){ //evaluate by poit of "board.who"
	//todo: advanced evaluating function
	if(DEBUG){
		for(int i=0;i<hist.num;i++) fprintf(flog, "(%d-%d) ", hist.mov[i].st, hist.mov[i].ed);
		fprintf(flog,"\n");
		board.Display(flog);
	}

	
	if (board.ChkLose()) return -WIN; //check lose
	SCORE ps = getPawnValue(board);
	SCORE as = getAttackValue(board);
	return ps+as;
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

MOV SearchEngine::genMove(const BOARD &board){
	POS p;
	POS firstMove[4]={5,6,25,26};
	if(board.who==-1){ //new game
		p=firstMove[rand()%4];
		return MOV(p,p);
	}

	//search for unfilp
	vector<FIN> unflipFin;
	for(int i=0;i<14;i++)
		if(board.cnt[i]>0) unflipFin.push_back(FIN(i));
	int unflipCnt=unflipFin.size();

	//nega scout (search for move)
	MOVLST lst, hist;
	SCORE mvScore=-INF;
	MOV best_mv, best_flip;
	int best_d=0;

	board.MoveGen(lst, false);

	for(int d=0;d<=SEARCH_MV_DEP;d++){ //iterative deepening
		for(int i=0;i<lst.num;i++) {
			BOARD tmpBoard(board);
			tmpBoard.Move(lst.mov[i]);
			
			hist.mov[hist.num++]=lst.mov[i];
			SCORE s=-NegaScout(tmpBoard, d, -INF, INF, hist, unflipCnt);
			hist.num--;
			lst.mov[i].s=s;

			if(DEBUG) fprintf(flog, "d:%d, mv=%d-%d, score=%d\n", d, lst.mov[i].st, lst.mov[i].ed, s);
			if(s>=mvScore){
				mvScore=s;
				best_mv=lst.mov[i];
				best_d=d;
			}
			if(s==WIN) break;
		}
		if(DEBUG) fprintf(flog, "d:%d, best score:%d, best_mv=(%d,%d)\n", d, mvScore, best_mv.st, best_mv.ed);
		if(unflipCnt==0 || mvScore==WIN) return best_mv; //cant flip || win , return the best move
		lst.sortScore();
	}
	printf("best score:%d, best_mv=(%d,%d)\n", mvScore, best_mv.st, best_mv.ed);

	//nega scout (search for flip)
	int best_gt=-INF, best_sum=-INF;
	SCORE flipScore=-INF;
	
	for(p=0;p<32;p++){
		int gt=0, sum=0;
		if(board.fin[p]==FIN_X){
			for(int j=0;j<unflipCnt;j++){
				int cnt=board.cnt[unflipFin[j]];
				BOARD tmpBoard(board);
				tmpBoard.Flip(p, unflipFin[j]);

				hist.mov[hist.num++]=MOV(p,p);
				SCORE s = -NegaScout(tmpBoard, SEARCH_FLIP_DEP, -INF, INF, hist, unflipCnt-1);
				hist.num--;
				if(DEBUG){
					MOVLST tmpLst;
					tmpBoard.MoveGen(tmpLst, false);
					fprintf(flog, "mv=%d-%d, score=%d, #move=%d\n", p, unflipFin[j], s, tmpLst.num);
				}
				if(s>mvScore) gt+=cnt;
				else if(s<mvScore) gt-=cnt;
				sum+=(s-mvScore)*cnt;

				if(gt>best_gt || (gt==best_gt && sum>best_sum)){
					best_gt=gt, best_sum=sum, best_flip=MOV(p,p);
					flipScore=s;
				}
			}
		}
	}
	if(DEBUG){
		fprintf(flog, "best flip score:%d, best_flip=(%d,%d)\n", flipScore, best_flip.st, best_flip.ed);
		fflush(flog);
	}
	printf("best flip score:%d, best_flip=(%d,%d)\n", flipScore, best_flip.st, best_flip.ed);

	return best_gt>=0 ? best_flip : best_mv;
}