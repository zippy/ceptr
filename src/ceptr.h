#ifndef _CEPTR_H
#define _CEPTR_H
namespace Ceptr {
    typedef int wordID;
    enum {INT_W,BOOL_W,STR_W};
    class XAddr {
	wordID word_id_;
    public:
	XAddr(wordID w_id){word_id_ = w_id;}
	wordID word_id() const {return word_id_;}
    };
    namespace Op {
	XAddr* New(wordID w_id) {
	    return new XAddr(w_id);
	}
    }
}
#endif
