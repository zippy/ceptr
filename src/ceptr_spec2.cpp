#include <igloo/igloo.h>
#include <ceptr2.h>

using namespace igloo;
using namespace Ceptr;

Context(all) {
Context(surface_tree){
    Spec(create_leaf) {
	SurfaceTree s = SurfaceTree("xaddr",8);
	Assert::That(s.size(),Equals<size_t>(8));
	Assert::That(s.name(),Equals("xaddr"));
	Assert::That(s.children(),Equals(0));
    }
    Spec(create_tree) {
	SurfaceTree s = SurfaceTree("t",{new SurfaceTree("t1",4),new SurfaceTree("t2",new SurfaceTree("t2/t1",8))});
	Assert::That(s.children(),Equals(2));
	Assert::That(s.child(0).name(),Equals("t1"));
	Assert::That(s.child(0).children(),Equals(0));
	Assert::That(s.child(0).size(),Equals(4));
	Assert::That(s.child(1).name(),Equals("t2"));
  	Assert::That(s.child(1).children(),Equals(1));
	Assert::That(s.child(1).size(),Equals(8));
	Assert::That(s.child(1).child(0).name(), Equals("t2/t1"));
	Assert::That(s.size(),Equals(12));
    }
};
Context(pattern_spec) {

    Spec(create) {
	PatternSpec p = PatternSpec(new SurfaceTree("xaddr",8) );
	Assert::That(p.surface().name(),Equals("xaddr"));
    }
};
Context(stream_type){
    PatternSpec *p = new PatternSpec(new SurfaceTree("xaddr",8));
    Spec(pattern_type) {
	PatternStream s = PatternStream(5,p);
	Assert::That(s.count(),Equals(5));
	Assert::That(s.pattern().surface().name(),Equals("xaddr"));
    }
    Spec(envelope_type) {
	EnvelopeStream e = EnvelopeStream({
		new EnvelopeItem(p),
		    new EnvelopeItem(new PatternStream(10,p))});
	Assert::That(e.size(),Equals(88));
    }
    Spec(envelope_type_unknown_size) {
	EnvelopeStream e = EnvelopeStream({
		new EnvelopeItem(p),
		    new EnvelopeItem(new PatternStream(UNKNOWN_LENGTH,p))});
	Assert::That(e.size(),Equals(UNKNOWN_SIZE));
    }
    };
};
int main(int argc, const char** argv)
{
    return TestRunner::RunAllTests(argc, argv);
}
