#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include "HashTable.h"
#include "anqi.hh"

#define INF 1000001
#define WIN 1000000
#define SEARCH_MV_DEP 6
#define SEARCH_FLIP_DEP 3
#define DEBUG 0
using namespace std;

typedef int SCORE;
const int STAT_VAL[]={12863,6431,3215,1607,803,401,200};
FILE* flog = fopen("mylog.txt", "w+");


SCORE evaluate(const BOARD &board, vector<MOV> mvlist){ //evaluate by poit of "board.who"
	//todo: advanced evaluating function
	if(DEBUG){
		for(int i=0;i<mvlist.size();i++) fprintf(flog, "(%d-%d) ", mvlist[i].st, mvlist[i].ed);
		fprintf(flog,"\n");
		MOVLST lst;
		board.MoveGen(lst, false);
		fprintf(flog, "lstnum=%d\n", lst.num);
		board.Display(flog);
	}

	//check lose
	if (board.ChkLose()) return -WIN;

	int cnt[2]={0,0};
	for(POS p=0;p<32;p++){
		const CLR c=GetColor(board.fin[p]);
		if(c!=-1) cnt[c]+=STAT_VAL[GetLevel(board.fin[p])];
	}
	for(int i=0;i<14;i++)
		cnt[GetColor(FIN(i))]+=board.cnt[i]*STAT_VAL[GetLevel(FIN(i))];
	
	if(DEBUG){
		fprintf(flog, "s=%d (%d)\n", cnt[board.who]-cnt[board.who^1], board.who);
		fflush(flog);
	}
	return cnt[board.who]-cnt[board.who^1];
}

SCORE NegaScout(BOARD &board, int depth, SCORE alpha, SCORE beta, vector<MOV> mvlist, int unflipCnt, HashTbl &hashTbl){
	//todo: hash
	MOVLST lst;
	board.MoveGen(lst, false);
	if(depth>0 && unflipCnt>0) lst.mov[lst.num++]=MOV(-1,-1); //null move (assume flip)

	//todo: timing constraint
	if(depth<=0 || lst.num==0){ //quiescent search
		board.MoveGen(lst, true); //reproduce move list : true for quiescent search
	}
	if(lst.num==0) return evaluate(board, mvlist); //terminate 

	SCORE m=-INF, n=beta; //fail soft
	//check hash
	if(depth>0){
		EVal e = hashTbl.find(board.hashVal, board.fin, board.who);
		if(e.valid && e.layer>=depth){ //hash hit
			if(e.type==ETYP_E) return e.value; //is exact value: return
			else m=e.value; //failed high value : use as bound
		}
	}

	//search deeper
	for(int i=0;i<lst.num;i++){
		vector<MOV> tmpList=mvlist;
		BOARD tmpBoard(board);
		int isFlip=0;
		if(!(lst.mov[i].st==-1 && lst.mov[i].ed==-1)){
			tmpBoard.Move(lst.mov[i]); //do move
		}
		else{ //assume flip
			tmpBoard.who^=1;
			isFlip=1;
		}
		tmpList.push_back(lst.mov[i]);

		int t= -NegaScout(tmpBoard, depth-1, -n, -max(alpha,m), tmpList, unflipCnt-isFlip, hashTbl); //null window search

		if(t>m){
			if(n==beta || (depth>0 && depth<3) || t>=beta) m=t; //first branch || depth<3(scout returns exact value) || fali high 
			else m= -NegaScout(tmpBoard, depth-1, -beta, -t, tmpList, unflipCnt-isFlip, hashTbl); //re-search
		}

		if(m>=beta){
			if(DEBUG) fprintf(flog, "beta cutoff %d %d\n", m, beta);
			if(depth>0) hashTbl.insert(board, m, depth, ETYP_H);
			return m; //beta cut off
		}

		n=max(alpha,m)+1; //set value for null window search
	}
	if(depth>0) hashTbl.insert(board, m, depth, ETYP_E);
	return m;
}

MOV genMove(const BOARD &board, HashTbl &hashTbl){
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
	MOVLST lst;
	SCORE mvScore=-INF;
	MOV best_mv, best_flip;

	board.MoveGen(lst, false);
	for(int i=0;i<lst.num;i++) {
		BOARD tmpBoard(board);
		tmpBoard.Move(lst.mov[i]);
		vector<MOV> mvlist;
		mvlist.push_back(lst.mov[i]);
		SCORE s=-NegaScout(tmpBoard, SEARCH_MV_DEP, -INF, INF, mvlist, unflipCnt, hashTbl);
		if(DEBUG) fprintf(flog, "mv=%d-%d, score=%d\n", lst.mov[i].st, lst.mov[i].ed, s);
		if(s>mvScore){
			mvScore=s;
			best_mv=lst.mov[i];
		}
		if(s==WIN) break;
	}
	if(DEBUG) fprintf(flog, "best score:%d, best_mv=(%d,%d)\n", mvScore, best_mv.st, best_mv.ed);
	if(unflipCnt==0 || mvScore==WIN) return best_mv; //cant flip || win , return the best move

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

				vector<MOV> mvlist;
				mvlist.push_back(MOV(p,p));
				SCORE s = -NegaScout(tmpBoard, SEARCH_FLIP_DEP, -INF, INF, mvlist, unflipCnt-1, hashTbl);
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

	return best_gt>=0 ? best_flip : best_mv;
}