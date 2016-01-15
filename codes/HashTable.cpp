#include <cstdlib>
#include <cstdio>
#include "HashTable.h"
#include "anqi.hh"

long long hashKey[16][32]; //[piece][location]: use (piece+location)-bit key

long long getHashVal(FIN (&fin)[32]){
	long long val=0;
	for(int i=0;i<32;i++){
		val^=(long long)hashKey[fin[i]][i];
	}
	return val;
}

long long modHashVal(long long oriVal, int piece, int location){
	if(piece==FIN_E) return oriVal;
	return oriVal^hashKey[piece][location];
}

void HashTbl::init(){
	for(int i=0;i<BKT_SIZE;i++) entries[i]=Entry();
	//init hash key
	for(int i=0;i<16;i++){
		for(int j=0;j<32;j++){ //32-bit: 15 15 2
			hashKey[i][j]=((long long)rand()<<17) | ((long long)rand()<<2) | ((long long)rand()%4);
		}
	}
}

void HashTbl::insert(BOARD & board,int value, int layer, ETYP type){
	Entry entry(value, type, board.who, layer, board.fin);
	int hashIdx = board.hashVal%BKT_SIZE;
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

EVal HashTbl::find(long long hashVal, FIN (&fin)[32], int who){
	int hashIdx = hashVal%BKT_SIZE;
	Entry oriEntry=entries[hashIdx];
	if(oriEntry.eval.valid==true && oriEntry.eval.who==who && oriEntry.finEquals(fin)) return oriEntry.eval;
	return EVal(0,0,0,ETYP_E,false);
}



