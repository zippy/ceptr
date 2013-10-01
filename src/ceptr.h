#ifndef _CEPTR_H
#define _CEPTR_H
using namespace std;
namespace Ceptr {
    /*
     Words: implement semantic data by combining a medium with a structural geometry (Carrier)
     and translating the variants possible in that Carrier via a Protocol into a SemanticGeometry.
     */
    typedef int wordID;
    enum {_W,BIT_W,INT_W,BOOL_W,STR_W}; // primitive words
    enum {SEQ_S};  // primitive structures

    class IdentifiedBase {
    public:
	virtual const string name() = 0;
	virtual const int id() = 0;
    };

    class Structure : public IdentifiedBase {
    };

    class Sequence : public Structure {
    public:
	const string name() {return "sequence";}
	virtual const int id() {return SEQ_S;};
    };

    class Word : public IdentifiedBase {
	static map<wordID,Word*> init_words() {map<wordID,Word*> w; return w;};
    public:
	static map<wordID,Word*> words;
	Word(wordID id){
	    words[id] = this;
	}
    };
    class Carrier : public IdentifiedBase {
	int id_;
	string name_;
	Structure* structureP_;
	wordID medium;
	static map<int,Structure*> init_structures() {
	    map<int,Structure*> m;
	    m[SEQ_S] = new Sequence;
	    return m;
	};
	static map<int,Structure*> structures;
	static int ids;
    public:
	Carrier(int structure_id,wordID medium){
	    structureP_ = structures[structure_id];
	    if (structureP_ == 0) {throw("UNKNOWN STRUCTURE");}
	    id_ = ids++;
	    name_ = structureP_->name() + " of " + Word::words[medium]->name()+"s";
	}
	const int id() {return id_;}
	const string name() {return name_;};

	Structure& structure() {return *structureP_;}
    };


    class BitWord : public Word {
    public:
    BitWord() : Word(BIT_W){};
	const int id() {return BIT_W;}
	const string name() {return "bit";}
	Carrier& carrier() {return *(new Carrier(SEQ_S,_W));}
    };

    class IntWord : public Word {
    public:
    IntWord() : Word(INT_W) {};
	const int id() {return INT_W;}
	const string name() {return "int";}
	Carrier& carrier() {return *(new Carrier(SEQ_S,BIT_W));}
    };
    /*
      Storage Handlers: implement storage of data with different structural geometries
     */
    typedef void* storageIdx;
    template<class T> class StorageHandler{
    public:
	void* set(T data){return new T(data);}
	T get(void *p){return *static_cast<T*>(p);}
	T* getP(void *p){return static_cast<T*>(p);}
    };

    /*
     XAddrs: provide semantic addresses into the ceptr "memory" space
    */
    class XAddr {
	wordID word_id_;
	storageIdx idx_;
	static StorageHandler<int>* intHandlerP;
	static StorageHandler<string>* strHandlerP;
    public:
	XAddr(int i){word_id_ = INT_W;idx_ = intHandlerP->set(i);}
	XAddr(string s){word_id_ = STR_W;idx_ = strHandlerP->set(s);}
	XAddr(wordID wid, storageIdx idx){
	    word_id_ = wid;
	    idx_ = idx;
	}
	bool operator==(XAddr& a){return word_id_ == a.word_id_ && idx_==a.idx_;}
	inline wordID word_id() const {return word_id_;}
	inline void* value() const {
	    switch(word_id_){
	    case INT_W: return intHandlerP->getP(idx_);
	    case STR_W: return strHandlerP->getP(idx_);
	    default: throw("NO HANDLER");
	    };
	}
	inline storageIdx idx() const {return idx_;}
    };

    // VM operands
    namespace Op {
	XAddr* New(wordID w_id) {
	    return new XAddr(w_id);
	}
    }


    /*Implementation stuff (to be moved to .cpp files later)*/
    StorageHandler<int>* XAddr::intHandlerP = new StorageHandler<int>;
    StorageHandler<string>* XAddr::strHandlerP = new StorageHandler<string>;
    map<int,Structure*> Carrier::structures = init_structures();
    int Carrier::ids = 0;
    map<wordID,Word*> Word::words= init_words();
    BitWord bitW = BitWord();
    IntWord intW = IntWord();
}
#endif
