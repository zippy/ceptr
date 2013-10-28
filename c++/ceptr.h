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
     Named Patterns: implement semantic data by combining a medium with a structural geometry (Carrier)
     and translating the variants possible in that Carrier via a Protocol into a SemanticGeometry.
     */
    typedef int patternID;
    enum {_P,BIT_P,INT_P,BOOL_P,FLOAT_P,STR_P,XADDR_P,SCAPE_P,CEPTR_P,_LAST_P}; // primitive pattern ids
    //ARRAY_P,HASH_P

    typedef int nameID;


    enum {X_S,NAME_S,SEQ_S,_LAST_S};  // primitive scapes

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

    struct Segment {
	unsigned char byte1;
	unsigned char byte2;
	unsigned char byte3;
	unsigned char byte4;
	unsigned char byte5;
	unsigned char byte6;
	unsigned char byte7;
	unsigned char byte8;
    };
    class FixedStore {
	vector<Segment> store;
	void rangechk(storageIdx idx) {
	    if (idx >= store.size() || idx < 0) throw(exception());
	}
    public:
	void set(storageIdx idx,int segments,void *data) {
	    Segment *sP = reinterpret_cast<Segment *>(data);
	    rangechk(idx);
	    for (int i=0;i<segments;i++){
		store[idx+i] = *sP;
		sP++;
	    }
	}
	//void *get(storageIdx idx,int segments,void *data){	}
    	storageIdx set(int data){
	    Segment s;
	    int *iP;
	    storageIdx i=store.size();
	    iP = reinterpret_cast<int *>(&s.byte1);
	    *iP = data;
	    store.push_back(s);
	    return i;
	}
    	int get(storageIdx idx){
	    Segment s;
	    rangechk(idx);
	    s = store[idx];
	    return *reinterpret_cast<int *>(&s.byte1);
	}
	bool valid_idx(storageIdx idx) {return (idx < store.size() && idx >= 0);}
    };

    /*
     XAddrs: provide semantic addresses into the ceptr "memory" space
    */

    class Scape;
    class XAddr {
	nameID name_id_;
	storageIdx idx_;
	static StorageHandler<int>* intHandlerP;
	static StorageHandler<string>* strHandlerP;
	static StorageHandler<void*>* ptrHandlerP;
    public:
	XAddr(int i){name_id_ = INT_P;idx_ = intHandlerP->set(i);}
	XAddr(string s){name_id_ = STR_P;idx_ = strHandlerP->set(s);}
	XAddr(Scape* sP){name_id_ = SCAPE_P;idx_ = ptrHandlerP->set(sP);}
	XAddr(nameID wid, storageIdx idx){
	    name_id_ = wid;
	    idx_ = idx;
	}
	bool operator==(XAddr& a){return name_id_ == a.name_id_ && idx_==a.idx_;}
	inline nameID name_id() const {return name_id_;}
	inline void* value() const {
	    switch(name_id_){
	    case INT_P: return intHandlerP->getP(idx_);
	    case STR_P: return strHandlerP->getP(idx_);
	    case SCAPE_P: return ptrHandlerP->get(idx_);
	    default: throw("NO HANDLER");
	    };
	}
	inline storageIdx idx() const {return idx_;}
    };

    /*
     Scapes:
    */
    class ScapeType : public IdentifiedBase {
	auto(ScapeType,int)
	string quality_;
	string name_;
	nameID key_source_;
	nameID data_source_;
    public:
        ScapeType(nameID id,string name,string quality,nameID key_source,nameID data_source) : ScapeType(id) {
	    name_ = name;quality_ = quality;key_source_ = key_source;data_source_ = data_source;
	}
        const string name() {return name_;}
	const string quality() {return quality_;};
	const nameID key_source() {return key_source_;};
	const nameID data_source() {return data_source_;};
    };
    auto_init(ScapeType,int,_LAST_S)

    class Scape {
	ScapeType& spec_;
    public:
        Scape(ScapeType& spec) : spec_(spec) {}
	ScapeType& spec() {return spec_;}
	//	int size() {return}
    };

    class Existence : public Scape {
    public:
    Existence(ScapeType& spec) : Scape(spec) {}
	XAddr& seek(XAddr& key) {
	    key.value(); // get the value so as to see if it really exists and throw exception if it doesn't
	    return key;
	}
    };

    class Pattern : public IdentifiedBase {
	auto(Pattern,int)
	string name_;
    public:
       Pattern(patternID id,string name) : Pattern(id) {
	    name_ = name;
	}
	const string name() {return name_;};
    };
    auto_init(Pattern,patternID,_LAST_P)

    class NamedPattern : public IdentifiedBase {
	auto(NamedPattern,nameID)
	string name_;
	Pattern* pattern_;
    public:
    NamedPattern(nameID id,string name,Pattern& pattern)  : pattern_(&pattern)  {
	    name_ = name;
	    id_ = id; init();
	    // for non- c++11 we have to do this manually
	}
	const string name() {return name_;}
	Pattern& pattern() {return *pattern_;}
    };
    auto_init(NamedPattern,nameID,_LAST_P)

    // VM operands
    namespace Op {
	XAddr* New(int val) {return new XAddr(val);}
	XAddr* New(ScapeType& s) {return new XAddr(new Scape(s));}
    }

    /*Implementation stuff (to be moved to .cpp files later)*/
    StorageHandler<int>* XAddr::intHandlerP = new StorageHandler<int>;
    StorageHandler<string>* XAddr::strHandlerP = new StorageHandler<string>;
    StorageHandler<void*>* XAddr::ptrHandlerP = new StorageHandler<void*>;

    //  ScapeType sequenceSS = ScapeType(SEQ_S,"sequence","linear order",0,0);
    //Scape sequenceS = Scape(sequenceSS);
    //ScapeType xSS = ScapeType(X_S,"existence","embodiment",XADDR_P,XADDR_P);
    //Existence xS = Existence(xSS);
    Pattern bitP = Pattern(BIT_P,"bit");
    Pattern intP = Pattern(INT_P,"int");
    Pattern xaddrP = Pattern(XADDR_P,"xaddr");

   }
#endif
