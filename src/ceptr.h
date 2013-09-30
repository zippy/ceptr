#ifndef _CEPTR_H
#define _CEPTR_H
using namespace std;
namespace Ceptr {
    /*
     Words
     */
    typedef int wordID;
    enum {INT_W,BOOL_W,STR_W};
    class Word {
	wordID id_;
    public:
	Word(wordID id) {id_=id;}
	wordID id() {return id_;}
    };

    /*
      Storage Handlers
     */
    typedef void* storageIdx;
    template<class T> class StorageHandler{
    public:
	void* set(T data){return new T(data);}
	T get(void *p){return *static_cast<T*>(p);}
	T* getP(void *p){return static_cast<T*>(p);}
    };

    /*
     XAddrs
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

}
#endif
