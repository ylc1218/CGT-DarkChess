#include <cstdlib>
#include <cstdio>
#include "HashTable.h"
#include "anqi.hh"

double hashKey[16][32]; //[piece][location]: use (piece+location)-bit key

void initHashKey(){
	for(int i=0;i<16;i++){
		for(int j=0;j<32;j++){ //32-bit: 15 15 2
			hashKey[i][j]=(rand()<<17) | (rand()<<2) | (rand()%4);
		}
	}
}

int getHashValue(FIN (&fin)[32]){
	int val=0;
	for(int i=0;i<32;i++){
		val^=(long long)hashKey[fin[i]][i];
	}
	return val;
}

void HashTbl::insert(BOARD &board, int value, int layer, ETYP type){
	Entry entry(value, type, board.who, layer, board.fin);
	int hashIdx = getHashValue(board.fin) % BKT_SIZE;
	insert(hashIdx, entry);
}

void HashTbl::insert(int hashIdx, Entry &entry){
	Entry oriEntry=entries[hashIdx];
	if(oriEntry.eval.valid==true){
		if(entry.finEquals(oriEntry.fin) && entry.eval.who==oriEntry.eval.who){
			if(oriEntry.eval.layer>entry.eval.layer) return;
			else if(oriEntry.eval.layer==entry.eval.layer && oriEntry.eval.type==ETYP_E) return;
		}
	}
	entries[hashIdx]=entry;
}

EVal HashTbl::find(FIN (&fin)[32], int who){
	int hashIdx = getHashValue(fin) % BKT_SIZE;
	Entry oriEntry=entries[hashIdx];
	if(oriEntry.eval.who==who && oriEntry.finEquals(fin)) return oriEntry.eval;
	return EVal(0,0,0,ETYP_E,false);
}



