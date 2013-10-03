#ifndef _CEPTR_H
#define _CEPTR_H

#define auto(CLASS_NAME,ID_TYPE)					\
    int id_;								\
    static int id_gen_;							\
    static map<ID_TYPE,CLASS_NAME*>& init_instances_store() {return *(new map<ID_TYPE,CLASS_NAME*>);}; \
    void init() {							\
	ID_TYPE i = id();						\
	CLASS_NAME* c = instances[i];					\
	if(c != 0) throw(exception());					\
	instances[id()]=this;}						\
    public: 								\
    static map<ID_TYPE,CLASS_NAME*>& instances;		         	\
    CLASS_NAME() {id_ = id_gen_++;init();}				\
    CLASS_NAME(ID_TYPE id) {id_ = id;init();}				\
    const int id() {return id_;}					\
    CLASS_NAME* get_instance_by_id(ID_TYPE id) {return instances[id];}  \
private:

#define auto_init(CLASS_NAME,ID_TYPE,FIRST)	                        \
    int CLASS_NAME::id_gen_=FIRST;		                        \
    map<ID_TYPE,CLASS_NAME*>& CLASS_NAME::instances= init_instances_store();


using namespace std;
namespace Ceptr {
    /*
     Words: implement semantic data by combining a medium with a structural geometry (Carrier)
     and translating the variants possible in that Carrier via a Protocol into a SemanticGeometry.
     */
    typedef int wordID;
    enum {_W,BIT_W,INT_W,BOOL_W,STR_W,_LAST_W}; // primitive words
    enum {SEQ_S,_LAST_S};  // primitive structures

    class IdentifiedBase {
    public:
	virtual const string name() = 0;
	virtual const int id() = 0;
    };

    class Scape : public IdentifiedBase {
	auto(Scape,int)
    public:
	virtual const string quality() = 0;
    };
    auto_init(Scape,int,_LAST_S)

    class Sequence : public Scape {
    public:
    Sequence():Scape(SEQ_S){};
	const string name() {return "sequence";}
	const string quality() {return "linear order";}
	virtual const int id() {return SEQ_S;};
    };

    class Word : public IdentifiedBase {
	auto(Word,wordID)
    public:
    };
    auto_init(Word,wordID,_LAST_W)

    class Carrier : public IdentifiedBase {
	auto(Carrier,int)
	string name_;
	Scape* scapeP_;
	wordID medium;
    public:
	Carrier(int scape_id,wordID medium){
	    scapeP_ = Scape::instances[scape_id];
	    if (scapeP_ == 0) {throw("UNKNOWN SCAPE");}
	    name_ = scapeP_->name() + " of " + Word::instances[medium]->name()+"s";
	}
	const string name() {return name_;};
	Scape& scape() {return *scapeP_;}
    };
    auto_init(Carrier,int,1)

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

    Sequence sequenceS = Sequence();
    BitWord bitW = BitWord();
    IntWord intW = IntWord();

}
#endif
