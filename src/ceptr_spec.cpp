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

Context(storage){
    Context(handlers){
	Spec(handlers_store_data){
	    StorageHandler<int> s;
	    storageIdx idx = s.set(1);
	    int i = s.get(idx);
	    Assert::That(i,Equals<int>(1));
	    Assert::That(*static_cast<int*>(s.getP(idx)),Equals<int>(1));
	}
    };
    Context(existence_addressing)
    {
	XAddr x = XAddr(1);
	XAddr y = XAddr("fish");
	Spec(xaddrs_have_word_ids) {
	    Assert::That(x.word_id(),Equals<wordID>(INT_W));
	    Assert::That(y.word_id(),Equals<wordID>(STR_W));
	}
	Spec(xaddrs_have_indexes){
	    //	    Assert::That(x.idx(),Equals(1));
	}
	Spec(xaddrs_point_to_values){
	    Assert::That( *static_cast<int*>(x.value()),Equals(1));
	    Assert::That( *static_cast<string*>(y.value()),Equals<string>("fish"));
	}
    };
};
Context(vm_operands){
    Context(NEW){
	Spec(takes_a_word_id) {
	    XAddr* xaddrP = Op::New(INT_W);
	    Assert::That(xaddrP->word_id(),Equals<wordID>(INT_W));
	}
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

Describe(scapes){
It(can parse a scape spec)
It(comes with default scapes: existence,scape,word(def/dec/spec),variable,process)
It(can create scapes, which adds them to the existence scape)
}


Describe(inter-process communication){
It(can plant a listener on a scape)
It(can wake up process to look at "this" (i.e. wake up to handle the listener you planted))
It(has a required entry point for a respond function for passing responses back with request ids from planted listeners)
It(has a required entry point for a request function for planting listeners on request by other receptors)
It(has an event queue that a process can be woken up to service)
}

Describe(storage){
It(has storage handlers for different types of data, i.e. buckets for different sizes/chunkability types)
It(has different indexing portions of x-addresses depending on the storage handler) //implemented as polymorphic types by each handler
It(throws errors when you access something using an incorrect semantic type)
It(has a permission/in-use tree to mark receptor access to data objects) // also part of garbage collection
It(can set the storage attributes (permanence level, recyclability, latency, throughput[r/w], growth-rate needs, private/shared, redundancy, tolerance), for a data object, which indicates which medium to use: ram,ram-disk,hdd,network shared,etc
It(can choose the appropriate storage medium based on balancing the requests for the storage item and what's available in the media pool, closest fit based on tolerances for specific attributes)
}

Describe(execution context){
It(is initiated, i.e. there is a wake-up call to the aspect, by a log entry being written)
It(has a round-robin process scheduler based on active items written to log(s): input, fork (for independent processes), scape update queue)
It(has a sematic data stack on which to push "words")
It(has a calling stack onto whcih are pushed stack-frames)
It(has an "index addressing" mode where you can get portions of a semantic unit back out, especially used by the program counter tuple to reference code-blocks/routines and operations within that routine)
It(has a log (as std input/output/err+))
Describe(log){
It(has immutable log entries)
It(has attached meta-data for each log entry that open loggers (connected receptors) can update)
It(had an open logger tree to manage permissions for accessing the log entry and writing meta-data about results by "called" receptors that continue to process the log entry)
It(has scapes on the meta-data state, like an error scape so folks can listen to errors)
It(automatically ends execution if there is an error and no explicit catch code was supplied)
}
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


/*    void SetUp()
    {
	guitar.AddEffect(fuzzbox);
    }

    It(starts_in_clean_mode)
    {
	Assert::That(guitar.Sound(), Equals(Clean));
    }

    Describe(in_distorted_mode)
    {
	void SetUp()
	{
	    Parent().fuzzbox.Switch();
	}

	It(sounds_distorted)
	{
	    Assert::That(Parent().guitar.Sound(), Equals(Distorted));
	}

	It(sounds_clean_when_I_switch_the_fuzzbox)
	{
	    Parent().fuzzbox.Switch();
	    Assert::That(Parent().guitar.Sound(), Equals(Clean));
	}
    };

    Fuzzbox fuzzbox;
    Guitar guitar
    };*/

int main(int argc, const char** argv)
{
    return TestRunner::RunAllTests(argc, argv);
}
