#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include "HashTable.h"
#include "anqi.hh"

#define INF 1000001
#define WIN 1000000
#define SEARCH_MV_DEP 6
using namespace std;

typedef int SCORE;
FILE* flog = fopen("mylog.txt", "w+");


SCORE evaluate(const BOARD &board, CLR view, vector<MOV> mvlist){ //evaluate by poit of "view"
	//printf("----------------\n");
	for(int i=0;i<mvlist.size();i++) fprintf(flog, "(%d-%d) ", mvlist[i].st, mvlist[i].ed);
	fprintf(flog,"\n");
	board.Display(flog);
	//todo: advanced evaluating function
	int cnt[2]={0,0};
	for(POS p=0;p<32;p++){const CLR c=GetColor(board.fin[p]);if(c!=-1)cnt[c]++;}
	for(int i=0;i<14;i++)cnt[GetColor(FIN(i))]+=board.cnt[i];
	fprintf(flog, "s=%d (%d)\n", cnt[view]-cnt[view^1], view);
	fflush(flog);
	return cnt[view]-cnt[view^1];
}

SCORE NegaScout(BOARD &board, int depth, SCORE alpha, SCORE beta, CLR view, vector<MOV> mvlist){
	fprintf(flog, "abc\n");
	//todo: hash
	MOVLST lst;
	board.MoveGen(lst); //todo: better move ordering
	
	if(depth==0 || lst.num==0){ //terminate
		//todo: quiescent search, timing constraint
		return evaluate(board, view, mvlist);
	}
	
	SCORE m=-INF, n=beta; //fail soft
	for(int i=0;i<=lst.num;i++){
		vector<MOV> tmpList=mvlist;
		BOARD tmpBoard(board);
		if(i<lst.num){
			tmpBoard.Move(lst.mov[i]); //do move
			tmpList.push_back(lst.mov[i]);
		}
		else{
			tmpBoard.who^=1; //assume flip //todo: check if can flip
			tmpList.push_back(MOV(-1,-1));
		}

		int t= -NegaScout(tmpBoard, depth-1, -n, -max(alpha,m), view^1, tmpList); //null window search

		if(t>m){
			if(n==beta || depth<3 || t>=beta) m=t; //first branch || depth<3(scout returns exact value) || fali high 
			else m= -NegaScout(tmpBoard, depth-1, -beta, -t, view, tmpList); //research
		}

		if(m>=beta){
			fprintf(flog, "beta cutoff %d %d\n", m, beta);
			return m; //beta cut off
		}

		n=max(alpha,m)+1; //set value for null windows search
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

	//nega scout (search for move)
	MOVLST lst;
	SCORE mvScore=-INF;
	MOV best_mv;

	board.MoveGen(lst);
	for(int i=0;i<lst.num;i++) {
		BOARD tmpBoard(board);
		tmpBoard.Move(lst.mov[i]);
		vector<MOV> mvlist;
		mvlist.push_back(lst.mov[i]);
		SCORE s=-NegaScout(tmpBoard, SEARCH_MV_DEP, -INF, INF, board.who^1, mvlist);
		fprintf(flog, "mv=%d-%d, score=%d\n", lst.mov[i].st, lst.mov[i].ed, s);
		if(s>mvScore){
			mvScore=s;
			best_mv=lst.mov[i];
		}
	}
	fprintf(flog, "best score:%d, best_mv=(%d,%d)\n", mvScore, best_mv.st, best_mv.ed);
	fflush(flog);
	//todo: search for flip
	vector<MOV> mvlist;
	if(mvScore> evaluate(board, board.who, 	mvlist))return best_mv;

	// flip (check if still can flip)
	int c=0;
	for(p=0;p<32;p++)if(board.fin[p]==FIN_X)c++;
	if(c==0)return best_mv; //cant flip
	c=rand()%c;
	for(p=0;p<32;p++)if(board.fin[p]==FIN_X&&--c<0)break;
	return MOV(p,p);
}