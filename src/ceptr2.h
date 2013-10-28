#ifndef _CEPTR_H
#define _CEPTR_H
#include <initializer_list>
using namespace std;

namespace Ceptr {
    typedef int Version;
    typedef string Name;
    const size_t UNKNOWN_SIZE = 0xFFFFFFFF;

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

    class PArray {
	int count_;
	PatternSpec* pattern_;
    public:
    PArray(int count, PatternSpec* pattern) : count_(count), pattern_(pattern){};
	int count() {return count_;}
	size_t size() {return pattern_->surface().size()*count_;}
	PatternSpec& pattern() {return *pattern_;}
    };

    class PList {
	size_t size_;
	PatternSpec* pattern_;
    public:
    PList(PatternSpec* pattern) : pattern_(pattern) , size_(pattern->surface().size()){};
    PList(PArray* array) : pattern_(&array->pattern()), size_(array->pattern().surface().size()* array->count()){};
	PatternSpec& pattern() {return *pattern_;}
	size_t element_size() {return size_;}
    };

    class EnvelopeItem {
	Name name_; // name of pattern or stream
	size_t size_;
    public:
    EnvelopeItem(PatternSpec *pattern) : name_(pattern->surface().name()), size_(pattern->surface().size()) {}
    EnvelopeItem(PArray *array) : EnvelopeItem(&array->pattern()) {
	    int count = array->count();
	    if (count == 0) {
		size_ = UNKNOWN_SIZE;
	    } else size_ = count * size_;
	}
	Name name() {return name_;}
	size_t size() {return size_;}
    };
    class EnvelopeStream {
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
	StreamProcess process;
    };
    class Stream {
	StreamSpec* spec;
    };

}
#endif
