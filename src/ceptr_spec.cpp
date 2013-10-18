#include <igloo/igloo.h>
#include <ceptr.h>

using namespace igloo;
using namespace Ceptr;

/* CEPTR HOST Kernel  "UberKernel"

   This file exercises the basic capabilities provided by the Ceptr kernel.

   What the kernel Manages:
   - memory (DataEngine)
   - processes (VMs/receptors <-> Processes/thread)
   - device bindings (streams)
*/
class X : public IdentifiedBase {
    auto(X,int)
    const string name() {return "my name";}
};
auto_init(X,int,1)

Context(base_classes) {
    Context(indentified) {
	class I : IdentifiedBase {
	public:
	    const string name() {return "my name";}
	    const int id() {return 1;}
	};
	I my_i;
	Spec(creates_identification_by_name){
	    Assert::That(my_i.name(),Equals("my name"));
	}
	Spec(creates_identification_by_id){
	    Assert::That(my_i.id(),Equals(1));
	}
    };
    Context(auto_classes) {
	Spec(creates_identification_by_id){
	    X i1(7),i2;
	    Assert::That(i1.id(),Equals(7));
	    Assert::That(i2.id(),Equals(2));
	}
	Spec(creates_access_to_objects_by_id){
	    X x1,x2(99);
	    Assert::That(X::instances[x1.id()],Equals<X*>(&x1));
	    Assert::That(X::instances[99],Equals<X*>(&x2));
	}
	Spec(throws_an_exception_if_multiply_defined){
	    AssertThrows(exception,new X(99));
	}
    };
};

Context(storage){
    /*It(has storage handlers for different types of data, i.e. buckets for different sizes/chunkability types)
      It(has different indexing portions of x-addresses depending on the storage handler) //implemented as polymorphic types by each handler
      It(throws errors when you access something using an incorrect semantic type)
      It(has a permission/in-use tree to mark receptor access to data objects) // also part of garbage collection
      It(can set the storage attributes (permanence level, recyclability, latency, throughput[r/w], growth-rate needs, private/shared, redundancy, tolerance), for a data object, which indicates which medium to use: ram,ram-disk,hdd,network shared,etc
      It(can choose the appropriate storage medium based on balancing the requests for the storage item and what's available in the media pool, closest fit based on tolerances for specific attributes)*/
    Context(fixed_size_store) {
	Spec(uses_64_bit_segments) {
	    Segment s;
	    Assert::That(sizeof(s),Equals(8));
	}
	Spec(storing_and_retrieving_data) {
	    FixedStore s;
	    storageIdx idx1 = s.set(1);
	    storageIdx idx2 = s.set(2);
	    int t;
	    int i1 = s.get(idx1);
	    int i2 = s.get(idx2);
	    Assert::That(i1,Equals<int>(1));
	    Assert::That(i2,Equals<int>(2));
	    Assert::That(idx1+1,Equals(idx2));
	    Assert::That(s.valid_idx(5),Is().False());
	    AssertThrows(exception,s.get(5));
	    t = 3;
	    s.set(idx1,1,static_cast<void*>(&t));
	    //i1 = *reinterpret_cast<int*>(s.get(idx1,1));
	    //Assert::That(i1,Equals(3));
	    Assert::That(s.get(idx1),Equals(3));
	    Assert::That(i2,Equals<int>(2));
	    }
    };

    Context(handlers){
	//It(has storage handlers for different types of data, i.e. buckets for different sizes/chunkability types)
	Spec(handlers_store_data){
	    StorageHandler<int> s;
	    storageIdx idx = s.set(1);
	    int i = s.get(idx);
	    Assert::That(i,Equals<int>(1));
	    Assert::That(*static_cast<int*>(s.getP(idx)),Equals<int>(1));
	    Assert::That(s.valid_idx(5),Is().False());
	    AssertThrows(exception,s.get(5));
	}
    };
    Context(existence_addressing)
    {
	XAddr x = XAddr(1);
	XAddr y = XAddr("fish");
	Spec(xaddrs_have_name_ids) {
	    Assert::That(x.name_id(),Equals<nameID>(INT_P));
	    Assert::That(y.name_id(),Equals<nameID>(STR_P));
	}
	Spec(xaddrs_have_indexes){
	    Assert::That(static_cast<int>(x.idx()),IsGreaterThan(0));
	}
	Spec(xaddrs_point_to_values){
	    Assert::That( *static_cast<int*>(x.value()),Equals(1));
	    Assert::That( *static_cast<string*>(y.value()),Equals<string>("fish"));
	}
	Spec(xaddrs_can_be_constructed_explicitly) {
	    XAddr z = XAddr(STR_P,y.idx());
	    Assert::That(z.name_id(),Equals<nameID>(STR_P));
	    Assert::That( *static_cast<string*>(z.value()),Equals<string>("fish"));
	    Assert::That(z == y,Is().True());
	    Assert::That(z == x,Is().False());
	}
    };


/*
  Carrier/Medium: the embodiment of some <dimensions of variability/scapes> such that an embodied <element/word> of the carrier/medium can take on/have/be of a specific <attribute/quality>, i.e. be a particular variant within the range of variability.
  Dimension of variability/Scape: a <attribute/quality> that varies over a range as specified by a <geometric process> that can be used to relate <elements/words>
  Element/Word:
  Geometric Process: a process which takes an <element/word> as an argument, and yields its <attribute/quality> and/or vice-versa.
  Signal/Message: A set of variations on a <carrier> according a <protocol>


use of a specific <geometry> of the <dimensions of variability/scapes> of a <substrate/meduim> to manipulate the parts (<words>) of the  <medium> to encode <words> at a new level. [Alternate: a <geometric arrangement/geometry> of <variants (which are also <word(s)...>)> in a <substrate/medium>] [Carriers are level jumpers in that they produces semantic words at a new level from semantic words at a low level. Carriers are the level at which composition is occurring.]

Dimension of variability/Scape: an <attribute/quality> that can vary over some range created by <process> applied to a <medium>. (A dimension of variability creates a medium when applied to a specific set of words.  E.g. RAM (as a medium) is concretizing a set of bits according to the DoV Sequence..)
Example: Guitar as music medium: DoV mass of string, material of string, length of string, resting tension of string, amplitude of displacement of string, size and shape echo chamber, material and structure of plucking

Structural aspects of the medium & semantic aspects.  What DoV's are built into medium?

Substrate/Medium: a realization of a collection of <dimensions of variability> applied to a collection of <media>

Geometry: a form/subset of variation within a <dimension of variability> indicated by a <process>
Variant: a single variation within a <dimension of variability>
Word: <primitive word> | <protocol> to translate/indicate which <variants> of <carrier> map to variants of semantic meaning of the word spec
Protocol: a <process> (including grammar etc) to map of <variants> on a <carrier> to <word variants> at the new level. [Protocols identify, where in the semantic geometry an particular variant lives.  They translate from the structural geometry to the semantic geometry]
Process: a unit of function which takes <words> as parameters and produces or changes <words>
*/
/*
    (verb
     "executable instruction"
     ());
    (noun
     "named pattern"
     ());
    (pattern
     ""
     (name.(process.pattern)));
    (xaddr
     ""
     ());
    (process
     ""
     (many verb));
    (scape
     ""
     ());
    (sequence
     ""
     ());
    (group
     ""
     ());
    (many
     ""
     ());
    (enum
     ""
     ());
    (label
     ""
     ());

    // (def seq _ _); sequence is undefineable!!!
    // (def group ()); group is undefineable!!!
    // (def many _ _ _); many is undefineable!!!


    (prime makePattern )

    (defpat name _symbol);
    (defpat bit (or 0 1));
    (defpat int (seq _bit _bit _bit _bit));
    (defpat pattern (defpat _name (seq _process _surface)));
    (defpat process (many _verb));
    (defpat surface (or (many _noun) (group _noun)));
    (defpat noun (seq _name _pattern));

    (pattern.((<many verb>).(latlong.(aLat:lat.aLong:long))));
    (pattern.(latlong.((name.(aLat.lat)).(name.(aLong.long)))));

    (pattern name (process [(or (many))]));
    (surface (or (many noun) (group noun)));
    (many (patternX (or patternX nil)));
    (ref symbol target);

    (context
     (ref X pattern)
      (many _ (or _ nil)));

    //bit int char str
    */

    Context(patterns) {
	Context(built_in) {
	    Spec(integer) {
		Assert::That(intP.id(),Equals<patternID>(INT_P));
		Assert::That(intP.name(),Equals("int"));
		//		Assert::That(intP.structure().to_s(SWEET),Equals("BITP...32"));
	    }
	    Spec(xaddr) {
		Assert::That(xaddrP.id(),Equals<patternID>(XADDR_P));
		Assert::That(xaddrP.name(),Equals("xaddr"));
		//		Assert::That(intW.structure().name(),Equals("sequence of bits"));
	    }
	   //  Spec() {
	    // 	Pattern four_bit_int = Pattern(432,"4 bit int",
	    //
// 						       "(.4bitint (bit bit bit bit)) (def inc (...))");
// /* this is kind of like
// 		class 4bitint {
// 		    bit bits[4];
// 		    void inc() {...};
// 		};
// */

// 		WordType age = WordType(....432);
// 		XAddr wills_age = Op::New(432,"(1 0 1 1)");
// 		Op::Exec("inc",wills_age);
// 		Assert::That(value of wills_age, Equal "(1 1 0 0)");
// 	    }
	};
	Spec(named_pattern) {
	    NamedPattern age = NamedPattern(1,"age",intP);
	    int a = age.id();
	    Assert::That(a,Equals(1));
	    Assert::That(age.name(),Equals("age"));
	}
	/*	Context(are_embodied_in_a_carrier){
		Carrier& c = intW.carrier();
		Spec(which_have_a_name){
		Assert::That(c.name(),Equals("sequence of bits"));
		Assert::That(xaddrW.carrier().name(),Equals("collection "));
		}
		Spec(have_scapes){
		ScapeType& s = c.scape();
		Assert::That(s.name(),Equals("sequence"));
		}
		};*/
    };


    //    Context(scapes)//  {
// /*
//   It(can parse a scape spec)
//   It(comes with default scapes: existence,scape,word(def/dec/spec),variable,process)
//   It(can create scapes, which adds them to the existence scape)

//Scape
//    Name name
//    Name keySource // the name of a pattern that we want to use to organize data
//    Name dataSource // the name of the pattern of the data that is to be organized
//    Name dataPattern  // the name of the pattern of what we actually want to store if different from data-pattern
//   Process transforms // if needed to instantiate data pattern
//    Vector<Condition> conditions  // conditions that are have been planted by listeners on this scape


// 	Privacy scape:
// 	    Key source: Step level scale: share-bed,share-room,...
// 	    Data source: person or the room, or whatever depending on the content_spec
// 	    Indexing fn: use key source as the key data (no transform)
// 	    Key Geometry: order_by(step level scale)
// */
// 	Context(built_in) {
// 	    Context(existence) {
// 		Spec(has_a_name_and_quality){
// 		    Assert::That(xS.spec().name(),Equals("existence"));
// 		    Assert::That(xS.spec().quality(),Equals("embodiment"));
// 		    Assert::That(xS.spec().id(),Equals<int>(X_S));
// 		    Assert::That(ScapeType::instances[X_S],Equals<ScapeType *>(&xSS));
// 		}
// 		Spec(uses_xaddrs_as_the_key_source) {
// 		    Assert::That(xS.spec().key_source(),Equals<nameID>(XADDR_P));
// 		}
// 		Spec(uses_xaddrs_as_the_data_source) {
// 		    Assert::That(xS.spec().data_source(),Equals<nameID>(XADDR_P));
// 		}
// 		Spec(creating_words_adds_them_to_existence) {
// 		    //	    int size = xS.size();
// 		    XAddr* xaddrP = Op::New(314);
// 		    //Assert::That(size+1,Equals(xS.size()));
// 		}
// 		Spec(seeking_for_existing_words) {
// 		    XAddr* xaddrP = Op::New(314);
// 		    XAddr q = xS.seek(*xaddrP);
// 		    Assert::That(q == *xaddrP, Is().True());
// 		}
//  		Spec(seeking_for_address_that_dont_exist){
// 		    XAddr a = XAddr(INT_P,99);
// 		    AssertThrows(exception,xS.seek(a));
// 		}
// 	    };
// 	};
// 	Spec(has_a_name){
// 	    Assert::That(sequenceSS.name(),Equals("sequence"));
// 	}
// 	Spec(has_identification){
// 	    Assert::That(sequenceSS.id(),Equals<int>(SEQ_S));
// 	    Assert::That(ScapeType::instances[SEQ_S],Equals<ScapeType *>(&sequenceSS));
// 	}
// 	Spec(has_a_quality_name){
// 	    Assert::That(sequenceSS.quality(),Equals("linear order"));
// 	}
// 	/*
// 	  process for producing or capturing the variants
// 	  structure
// 	  grammar
// 	*/
//     }
    };
    Context(protocol) {
	/*have names
	  and ids
	  and variations
	*/
    };

Context(virtual_machine){
    Context(code_execution){
	/*It(is initiated, i.e. there is a wake-up call to the aspect, by a log entry being written)
	  It(has a round-robin process scheduler based on active items written to log(s): input, fork (for independent processes), scape update queue)
	  It(has a sematic data stack on which to push "words")
	  It(has a calling stack onto which are pushed stack-frames)
	  It(has an "index addressing" mode where you can get portions of a semantic unit back out, especially used by the program counter tuple to reference code-blocks/routines and operations within that routine)
	  It(has a log (as std input/output/err+))
	  Describe(log){
	  It(has immutable log entries)
	  It(has attached meta-data for each log entry that open loggers (connected receptors) can update)
	  It(had an open logger tree to manage permissions for accessing the log entry and writing meta-data about results by "called" receptors that continue to process the log entry)
	  It(has scapes on the meta-data state, like an error scape so folks can listen to errors)
	  It(automatically ends execution if there is an error and no explicit catch code was supplied)
	  }
	*/
	Spec(happens_in_a_context){

	}
    };
    Context(operands){
	Context(NEW){
	    Spec(you_can_create_ints) {
		XAddr* xaddrP = Op::New(314);
		Assert::That(xaddrP->name_id(),Equals<nameID>(INT_P));
		Assert::That( *static_cast<int*>(xaddrP->value()),Equals(314));
	    }
	    //	    Spec(you_can_create_scapes) {
	    //	XAddr* xaddrP = Op::New(sequenceSS);
	    //	Assert::That(xaddrP->word_type_id(),Equals<nameID>(SCAPE_P));
	    //	Assert::That( static_cast<Scape*>(xaddrP->value())->spec().id(),Equals(SEQ_S));
	    //}
	};
    };
};

/*

  ---- TO-DO specs to be implemented-----
  It(provides ceptr network addressing for receptors to refer to external receptors)
  //    It(provides existence addressing for receptors to refer to items they contain)

It (can bridge external devices by creating binds which provide a ceptr stream)
It (maintains a mapping between process and data)

It (can do CRUD on: data-matrix/process space pairs)
It (can report mcube crc and "size" which is its resource use (file system size, perhaps or also actual xaddr counts))
It (can trigger execution of a process at a code-point due to a planted listener)

It(can install "elevator shaft" publicly available services that are available to all receptors in the host, i.e. domain-name resolution, email-parsing, http-server, smtp server)



Describe(inter-process communication){
It(can plant a listener on a scape)
It(can wake up process to look at "this" (i.e. wake up to handle the listener you planted))
It(has a required entry point for a respond function for passing responses back with request ids from planted listeners)
It(has a required entry point for a request function for planting listeners on request by other receptors)
It(has an event queue that a process can be woken up to service)
}

It(can_calculate_the_size_of_itself)
It(can_crc_content)


NOTES:

Receptors always talk to two bucket sets, the inside one, and the outside one, except that the only bucket on the outside it can talk to is itself.  Talking to yourself is NOT a sign of impending mental collapse.

Data engine can connect directly to network devices for handling distributed storage without going back through the UberKernel. This might allow two hands writing because if both the Kernel and the DE can access third parties they can independently verify stuff.

transaction rollback can be the result of taking advantage of using persistent memory for log entries and marking attempts to process them in the ephemeral space, which if fail are just abandoned and restarted from the persistent entry.

Maintenance of processes/data/devices is all CRUD at the control level.  Note: this is not about writing to devices or data, that's at a different level

Sending content to devices:
-Devices

-Memory
  only through executing bycoded instructions

*/


int main(int argc, const char** argv)
{
    return TestRunner::RunAllTests(argc, argv);
}
