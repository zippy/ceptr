
public class CeptrDataEngine {
    Map<long, Surface> surfaces
    Surface get(Xaddr x) {
	if (existenceScape.get(x.key).equals( x.noun))
	    throw new SemanticFault()
	return surfaces.get(x.key)
    }
    Map<long, long> existenceScape
    void set(Receptor r, Xaddr x, Surface s) {
	surfaces.put(x.key, s)
	existenceScape.put(x.key, x.noun)
    }
}

public interface Scape<S extends Surface> {
    Collection get(S s)
    CeptrType containedType
}

public class Receptor {
    CeptrDataEngine host;
    Executor ex
    Set<Scape> scapes;
    Scape scapeScape
    void invoke(Xaddr x, long processName) {
	CeptrProcess toInvoke = x.getSpec().getProcesses()[processName]
	Surface arg = host.get(x)
	toInvoke.call(arg)
    }
}
class XaddrInstructionRunnable implements Runnable {
    private Xaddr x
    public XaddrInstructionRunnable(Xaddr x){
	this.x = x
    }
    public void run() {
	
    }
}

public interface CeptrProcess
   extends Runnable {

}

public interface CeptrType {
    
}

public class Xaddr {
    long key
    long noun
}

public class Path {
    
}

public interface Element {
    Process[] getProcesses()
    Noun getName()
}

public interface Surface {

}

public interface PatternSpec
         extends Element {
    Pattern[] getChildren()
    long getSize()
}

public class ArraySpec
  implements Element {
    Noun getArrayType()
    long getInstanceSize(Xaddr x)
}

interface InstructionSpec {}
enum Spec1 implements InstructionSpec {
    NEW(new Runnable(){
	    pop() + pop()
	}),
    EXEC(new Runnable(){})
    private Runnable r
    private Spec1(Runnable r) {
	this.r = r
    }
    public void runInstruction(Receptor rec) {rec.run()}
}
ADD.run()
enum Spec2 implements InstructionSpec {
    ADD, SUBTRACT
}

public class Instruction {
    Surface
    Noun
    InstructionSpec processName
    Xaddr operands
}
PatternSpec instruction = new PatternSpec() {
    long processName
    Pattern[] getChildren()
    long getSize()    
}

public void test() {
    new Age(7)
    new Age(9)
    // 2 xaddrs on the stack
    invoke(ADD )
}


age1 : Age = 7
age2 : Age = 9
age1 + age2
