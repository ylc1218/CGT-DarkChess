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
using namespace std;

typedef int SCORE;
const int STAT_VAL[]={12863,6431,3215,1607,803,401,200};
FILE* flog = fopen("mylog.txt", "w+");


SCORE evaluate(const BOARD &board, CLR view, vector<MOV> mvlist){ //evaluate by poit of "view"
	//printf("----------------\n");
	for(int i=0;i<mvlist.size();i++) fprintf(flog, "(%d-%d) ", mvlist[i].st, mvlist[i].ed);
	fprintf(flog,"\n");
	board.Display(flog);

	//todo: advanced evaluating function
	int cnt[2]={0,0};
	for(POS p=0;p<32;p++){
		const CLR c=GetColor(board.fin[p]);
		if(c!=-1) cnt[c]+=STAT_VAL[GetLevel(board.fin[p])];
	}
	for(int i=0;i<14;i++)
		cnt[GetColor(FIN(i))]+=board.cnt[i]*STAT_VAL[GetLevel(FIN(i))];
	
	fprintf(flog, "s=%d (%d)\n", cnt[view]-cnt[view^1], view);
	fflush(flog);
	return cnt[view]-cnt[view^1];
}

SCORE NegaScout(BOARD &board, int depth, SCORE alpha, SCORE beta, CLR view, vector<MOV> mvlist, int unflipCnt){
	fprintf(flog, "abc\n");
	//todo: hash
	MOVLST lst;
	board.MoveGen(lst, false);
	
	//terminate //todo: timing constraint
	if(depth<=0){ 
		//quiescent search
		board.MoveGen(lst, true); //reproduce move list : true for quiescent search
	}
	if(lst.num==0) return evaluate(board, view, mvlist);

	//search deeper
	SCORE m=-INF, n=beta; //fail soft
	for(int i=0;i<=lst.num;i++){
		vector<MOV> tmpList=mvlist;
		BOARD tmpBoard(board);
		int isFlip=0;
		if(i<lst.num){
			tmpBoard.Move(lst.mov[i]); //do move
			tmpList.push_back(lst.mov[i]);
		}
		else{ //assume flip
			if(depth>0 && unflipCnt>0){ //not quiescent search & can flip
				tmpBoard.who^=1;
				tmpList.push_back(MOV(-1,-1));
				isFlip=1;
			}
			else continue;
		}

		int t= -NegaScout(tmpBoard, depth-1, -n, -max(alpha,m), view^1, tmpList, unflipCnt-isFlip); //null window search

		if(t>m){
			if(n==beta || (depth>0 && depth<3) || t>=beta) m=t; //first branch || depth<3(scout returns exact value) || fali high 
			else m= -NegaScout(tmpBoard, depth-1, -beta, -t, view^1, tmpList, unflipCnt-isFlip); //re-search
		}

		if(m>=beta){
			fprintf(flog, "beta cutoff %d %d\n", m, beta);
			return m; //beta cut off
		}

		n=max(alpha,m)+1; //set value for null window search
	}
	return m;
}

MOV genMove(const BOARD &board){
	POS p;
	POS firstMove[4]={5,6,25,26};
	if(board.who==-1){ //new game
		p=firstMove[rand()%4];
		printf("%d\n",p);
		return MOV(p,p);
	}
	//search for unfilp
	vector<FIN> unflipFin;
	for(int i=0;i<14;i++)
		if(board.cnt[i]>0) unflipFin.push_back(FIN(i));

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
		SCORE s=-NegaScout(tmpBoard, SEARCH_MV_DEP, -INF, INF, tmpBoard.who, mvlist, unflipFin.size());
		fprintf(flog, "mv=%d-%d, score=%d\n", lst.mov[i].st, lst.mov[i].ed, s);
		if(s>mvScore){
			mvScore=s;
			best_mv=lst.mov[i];
		}
	}
	fprintf(flog, "best score:%d, best_mv=(%d,%d)\n", mvScore, best_mv.st, best_mv.ed);
	fflush(flog);
	//if(mvScore > evaluate(board, board.who, mvlist))return best_mv;
	
	//nega scout (search for flip)
	int best_gt=-INF, best_sum=-INF;
	SCORE fiipScore=-INF;
	
	for(p=0;p<32;p++){
		int gt=0, sum=0;
		if(board.fin[p]==FIN_X){
			for(int j=unflipFin.size()-1;j>=0;j--){
				int cnt=board.cnt[unflipFin[j]];
				BOARD tmpBoard(board);
				tmpBoard.Flip(p, unflipFin[j]);

				vector<MOV> mvlist;
				mvlist.push_back(MOV(p,p));
				SCORE s = -NegaScout(tmpBoard, SEARCH_FLIP_DEP, -INF, INF, tmpBoard.who, mvlist, unflipFin.size()-1);
				fprintf(flog, "mv=%d-%d, score=%d\n", p, unflipFin[j], s);
				if(s>mvScore) gt+=cnt;
				else if(s<mvScore) gt-=cnt;
				sum+=(s-mvScore)*cnt;

				if(gt>best_gt || (gt==best_gt && sum>best_sum)){
					best_gt=gt, best_sum=sum, best_flip=MOV(p,p);
					fiipScore=s;
				}
			}
		}
	}
	fprintf(flog, "best flip score:%d, best_flip=(%d,%d)\n", fiipScore, best_flip.st, best_flip.ed);

	return best_gt>=0 ? best_flip : best_mv;
}