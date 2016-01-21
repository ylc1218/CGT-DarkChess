#ifndef SEARCH
#define SEARCH
	#include "anqi.hh"
	#include "HashTable.h"
	#include <vector>
	#define REP_SIZE 10
	using namespace std;
	
	typedef int SCORE;

	/*struct BfsNode{
		POS p;
		int d;
		bool operator<(const BfsNode& a, const BfsNode& b) { //reversed the comparison to achieve min heap wihtout passing extra arguments to the priority queue.
		  return a.d > b.d;
		}
		BfsNode(){}
		BfsNode(POS p, int d){
			this->p=p;
			this->d=d;
		}
	};*/

	struct RepeatHistory{
		int p;
		MOV mov[REP_SIZE];

		RepeatHistory(){
			p=0;
		}
		void add(MOV mv){
			mov[p%REP_SIZE]=mv;
			p++;
			if(p>10000) p=p%REP_SIZE+REP_SIZE;
			printf("p=%d\n", p);
		}
		MOV repeatMov(){
			if(p<REP_SIZE) return MOV(-1,-1); //less than REP_SIZE moves, will not repeat
			for(int i=p;i<p+REP_SIZE;i++)
				printf("(%d-%d) ", mov[i%REP_SIZE].st, mov[i%REP_SIZE].ed);
			printf("\n");
			if(!(mov[(p-1)%REP_SIZE]==mov[(p-5)%REP_SIZE]) || !(mov[(p-5)%6]==mov[(p-9)%REP_SIZE])) return MOV(-1,-1);
			if(!(mov[(p-2)%REP_SIZE]==mov[(p-6)%REP_SIZE]) || !(mov[(p-6)%3]==mov[(p-10)%REP_SIZE])) return MOV(-1,-1);
			if(!(mov[(p-3)%REP_SIZE]==mov[(p-7)%REP_SIZE]) || !(mov[(p-4)%3]==mov[(p-8)%REP_SIZE])) return MOV(-1,-1);
			
			return mov[(p-4)%REP_SIZE];
		}
	};

	struct SearchEngine{
		RepeatHistory history;
		HashTbl *p_hashTbl; 

		SearchEngine(){
			history.p=0;
			p_hashTbl=new HashTbl();
			p_hashTbl->init();
		}

		MOV genMove(const BOARD &board, int remain_time);
		MOV searchForMove(const BOARD &board, int unflipCnt);
		MOV searchForFlip(const BOARD &board, vector<FIN> &unflipFin, SCORE mvScore);
		SCORE NegaScout(BOARD &board, int depth, SCORE alpha, SCORE beta, MOVLST &hist, int unflipCnt);
		void addHistory(MOV mov){
			history.add(mov);
		}
	};

#endif