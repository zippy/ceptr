#ifndef _CEPTR_H
#define _CEPTR_H


/* this is my quick and dirty way to implement reflection on the classes I create.
   It allows for creating different id types on inherited classes either automatically,
   or manually by calling the explicit constructor.
*/
#define auto(CLASS_NAME,ID_TYPE)					\
    int id_;								\
    static int id_gen_;							\
    static map<ID_TYPE,CLASS_NAME*>& init_instances_store() {return *(new map<ID_TYPE,CLASS_NAME*>);}; \
    void init() {							\
	ID_TYPE i = id();						\
	CLASS_NAME* c = instances[i];					\
	if(c != 0) throw(exception());					\
	instances[i]=this;}						\
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
    enum {_W,XADDR_W,BIT_W,INT_W,BOOL_W,STR_W,_LAST_W}; // primitive words
    enum {X_S,SEQ_S,_LAST_S};  // primitive scapes

    class IdentifiedBase {
    public:
	virtual const string name() = 0;
	virtual const int id() = 0;
     };

    /*
      Storage Handlers: implement storage of data with different structural geometries
     */
    typedef int storageIdx;
    template<class T> class StorageHandler{
	vector<T> store;
	void rangechk(storageIdx idx) {
	    if (idx >= store.size() || idx < 0) throw(exception());
	}
	public:
	bool valid_idx(storageIdx idx) {return (idx < store.size() && idx >= 0);}

	storageIdx set(T data){
	    storageIdx i=store.size();
	    store.push_back(data);
	    return i;
	}
	T get(storageIdx idx){
	    rangechk(idx);
	    return store[idx];
	}
	T* getP(storageIdx idx){
	    rangechk(idx);
	    return &store[idx];
	}
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

    /*
     Scapes:
    */
    class Scape : public IdentifiedBase {
	auto(Scape,int)
    public:
	virtual const string quality() = 0;
	virtual const wordID key_source() = 0;
	virtual const wordID data_source() = 0;
    };
    auto_init(Scape,int,_LAST_S)

    class Sequence : public Scape {
    public:
    Sequence():Scape(SEQ_S){};
	const string name() {return "sequence";}
	const string quality() {return "linear order";}
	const wordID key_source() {return 0;};
	const wordID data_source() {return 0;};
	const int id() {return SEQ_S;};
    };

    class Existence : public Scape {
    public:
        Existence():Scape(X_S){};
	const string name() {return "existence";}
	const string quality() {return "embodiment";}
	const int id() {return X_S;}
	const wordID key_source() {return XADDR_W;};
	const wordID data_source() {return XADDR_W;};
	XAddr& seek(XAddr& key) {
	    key.value(); // get the value so as to see if it really exists and throw exception if it doesn't
	    return key;
	}
    };

    class Carrier;

    class WordSpec : public IdentifiedBase {
	auto(WordSpec,wordID)
	string name_;
	Carrier* carrier_;
    public:
	WordSpec(wordID id,string name,Carrier& carrier) {
	    name_ = name;
	    carrier_ = &carrier;
	    id_ = id;
	    init();
	}
	const string name() {return name_;}
	Carrier& carrier() {return *carrier_;}
    };
    auto_init(WordSpec,wordID,_LAST_W)

    class Carrier : public IdentifiedBase {
	auto(Carrier,int)
	string name_;
	Scape* scapeP_;
	wordID medium;
    public:
	Carrier(int scape_id,wordID medium){
	    scapeP_ = Scape::instances[scape_id];
	    if (scapeP_ == 0) {throw("UNKNOWN SCAPE");}
	    name_ = scapeP_->name() + " of " +
		(medium == _W ? "memory" : WordSpec::instances[medium]->name ())+"s";
	}
	const string name() {return name_;};
	Scape& scape() {return *scapeP_;}
    };
    auto_init(Carrier,int,1)

    // VM operands
    namespace Op {
	XAddr* New(int val) {return new XAddr(val);}
    }

    /*Implementation stuff (to be moved to .cpp files later)*/
    StorageHandler<int>* XAddr::intHandlerP = new StorageHandler<int>;
    StorageHandler<string>* XAddr::strHandlerP = new StorageHandler<string>;

    Sequence sequenceS = Sequence();
    Existence xS = Existence();
    Carrier primSeqC = Carrier(SEQ_S,_W);
    WordSpec bitW = WordSpec(BIT_W,"bit",primSeqC);
    Carrier bitSeqC = Carrier(SEQ_S,BIT_W);
    WordSpec intW = WordSpec(INT_W,"int",bitSeqC);
    WordSpec xaddrW = WordSpec(XADDR_W,"xaddr",bitSeqC);
}
#endif
