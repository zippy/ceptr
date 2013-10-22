#ifndef _CEPTR_H
#define _CEPTR_H
using namespace std;
namespace Ceptr {
    typedef Version int;
    typedef Name string;
    class Process {
	FunctionPointer process;
    };
    class StreamType {
    };
    class LengthRepStream : StreamType {
	int count;
	PatternSpec* pattern;
    };
    class TerminatedRepStream : StreamType {
	PatternSpec* pattern;
    };

    const size_t UNKNOWN_SIZE = 0xFFFFFFFF;
    class EnvelopeItem {
	Name name; // name of pattern or stream
	size_t size;
    };
    class EnvelopeStream : StreamType {
	size_t size;
	vector<EnvelopeItem> items;
    };
    class UnspecifiedStream : StreamType {
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
    class SurfaceItem {
	Name pattern;
	size_t size;
    };
    class PatternSurface {
	size_t size;
	Tree<SurfaceItem> surface;
    };
    class PatternProcess : Process {
    };
    class PatternSpec : Stream {
	Name name;
	Version version;
	PatternSurface surface;
	PatternProcess process;
    };
    class Pattern {
	PatternSpec* spec;
    };
}
#endif
