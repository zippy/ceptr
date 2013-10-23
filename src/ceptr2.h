#ifndef _CEPTR_H
#define _CEPTR_H
#include <initializer_list>
using namespace std;

namespace Ceptr {
    typedef int Version;
    typedef string Name;
    const size_t UNKNOWN_SIZE = 0xFFFFFFFF;
    const int UNKNOWN_LENGTH = 0xFFFFFFFF;

    class Process {
	void (*process) ();
    };
    class Stream;
    class SurfaceTree {
	const Name name_;
	size_t size_;
	vector<SurfaceTree *> *children_;
	void init(SurfaceTree *tree) {
	    if (tree->children_ == NULL) {
		size_ += tree->size();
	    } else {
		for (auto it : *tree->children_) {
		    size_ += it->size();
		}
	    }
	    children_->push_back(tree);
	}
    public:
    SurfaceTree(Name name,size_t size) : name_(name), size_(size), children_(NULL) {};
    SurfaceTree(Name name, SurfaceTree *tree) : name_(name), children_(new vector<SurfaceTree *>){
	    size_ = 0;
	    init(tree);
	}
    SurfaceTree(Name name,initializer_list<SurfaceTree*> trees) : name_(name), children_(new vector<SurfaceTree *>) {
	    size_ = 0;
	    for (auto tree : trees) {
		init(tree);
	    }
	};
	const size_t size() {return size_;}
	const Name name() {return name_;}
	unsigned int children() {return (children_==0) ? 0 : children_->size();}
	SurfaceTree& child(int index) {return *(*children_)[index];}
    };
    class PatternProcess : public Process {
    };
    class PatternSpec {
	SurfaceTree* surface_;
	PatternProcess process_;
    public:
    PatternSpec(SurfaceTree *surface) : surface_(surface){};
	SurfaceTree& surface() {return *surface_;}
    };
    class Pattern {
	PatternSpec* spec;
    };
    class StreamType {
    };

    class PatternStream : public StreamType {
	int count_;
	PatternSpec* pattern_;
    public:
    PatternStream(int count, PatternSpec* pattern) : count_(count),pattern_(pattern){};
	int count() {return count_;}
	PatternSpec& pattern() {return *pattern_;}
    };

    class EnvelopeItem {
	Name name_; // name of pattern or stream
	size_t size_;
    public:
    EnvelopeItem(PatternSpec *pattern) : name_(pattern->surface().name()), size_(pattern->surface().size()) {}
    EnvelopeItem(PatternStream *pstream) : EnvelopeItem(&pstream->pattern()) {
	    int count = pstream->count();
	    if (count == UNKNOWN_LENGTH) {
		size_ = UNKNOWN_SIZE;
	    } else size_ = count * size_;
	}
	Name name() {return name_;}
	size_t size() {return size_;}
    };
    class EnvelopeStream : public StreamType {
	size_t size_;
	vector<EnvelopeItem *> items_;
    public:
	EnvelopeStream(initializer_list<EnvelopeItem*> items) {
	    size_ = 0;
	    for (auto item : items) {
		if (size_ != UNKNOWN_SIZE) {
		    size_t s = item->size();
		    size_ = (s == UNKNOWN_SIZE) ? UNKNOWN_SIZE : size_+s;
		}
		items_.push_back(item);
	    }
	}
	size_t size() {return size_;}
    };

    class StreamProcess : Process {
    };
    class StreamSpec {
	Name name;
	Version version;
	StreamType type;
	StreamProcess process;
    };
    class Stream {
	StreamSpec* spec;
    };

}
#endif
