#include <cstring>
#include "anqi.hh"

#ifndef HASHTABLE
#define HASHTABLE
	#define BKT_SIZE (1<<16)
	extern long long hashKey[16][32]; //[piece][location]: use (piece+location)-bit key
	enum ETYP {
		ETYP_E=0, // exact value
		ETYP_H=1, // fail high
	};

	struct EVal{
		int value, layer, who;
		bool valid;
		ETYP type;
		EVal(){valid=false;};
		EVal(int value, int layer, int who, ETYP type, bool valid){
			this->value=value;
			this->layer=layer;
			this->who=who;
			this->type=type;
			this->valid=valid;
		};
	};

	struct Entry{
		EVal eval;
		FIN fin[32];

		Entry(){
			eval=EVal();
		}
		Entry(int value, ETYP type, int who, int layer ,FIN (&fin)[32]) {
			eval=EVal(value, layer, who, type, true);
			memcpy(&(this->fin[0]), &fin[0], sizeof(fin));
		};

		bool finEquals(FIN (&fin)[32]) { return memcmp(&fin[0], &(this->fin[0]), sizeof(fin))==0; }
	};

	struct HashTbl{
		Entry entries[BKT_SIZE];
		void init();
		void insert(BOARD &board, int value, int layer, ETYP type);
		void insert(int hashIdx, Entry& entry);
		EVal find(long long hashVal, FIN (&fin)[32], int who);
	};

	long long getHashVal(FIN (&fin)[32]);
	long long modHashVal(long long oriVal, int piece, int location);

#endif