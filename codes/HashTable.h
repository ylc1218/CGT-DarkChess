#include <cstring>
#include "anqi.hh"

#ifndef HASHTABLE
#define HASHTABLE
	#define BKT_SIZE (2<<16)

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
			memcpy(&(this->fin), &fin, sizeof(fin));
		};

		bool finEquals(const FIN (&fin)[32]) { return memcmp(&fin, &(this->fin), sizeof(fin))==0; }
	};

	struct HashTbl{
		Entry entries[BKT_SIZE];
		HashTbl(){
			for(int i=0;i<BKT_SIZE;i++) entries[i]=Entry();
		};
		void insert(BOARD &board, int value, int layer, ETYP type);
		void insert(int hashIdx, Entry& entry);
		EVal find(FIN (&fin)[32], int who);
	};

#endif