
public class CeptrDataEngine {
    Map<String, Surface> surfaces
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

public class SurfaceNounPair {
    public Surface surface;
    public Noun noun;
    SurfaceNounPair next;
}
public class SemanticStack {
    SurfaceNounPair head = null
    void push(Noun n, Surface s) {
	head = new SurfaceNounPair(n, s, head)
    }
    private SurfaceNounPair privatePop() {
	if (head == null){
	    throw new NoSuchElementException();}
	SurfaceNounPair ret = head
	head = head.next
	return ret
    }
    SurfaceNounPair popBySpec(Class c) {
	if (c.name != head.noun.nounSpec.name)
	    throw new SemanticFault("requested that noun names a $c instead is a ${head.noun.nounSpec}")
	return privatePop()
    }
    SurfaceNounPair pop(Noun n) {
	if (n != head.noun)
	    throw new SemanticFault("requested $n expecting ${head.noun}")
	return privatePop()
    }
}

public class Receptor {
    SemanticStack stack
    CeptrDataEngine host
    Executor ex
    Set<Scape> scapes
    Scape scapeScape
    void invoke(Xaddr x, long processName) {
	CeptrProcess toInvoke = x.getSpec().getProcesses()[processName]
	Surface arg = host.get(x)
	toInvoke.call(this, arg)
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

public interface CeptrProcess {
    private Closure c
    public CeptrProcess(Closure c) {
	this.c = c
    }
    public void call(Receptor r, Surface s) {
	c.delegate = s
	c.call(r)
    }
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
    Map<String, CeptrProcess> getProcesses()
    Noun getName()
}

public interface Surface {

}

public interface PatternSpec
         extends Element {
    Pattern[] getChildren()
    long getSize()
}

class CeptrInteger extends PatternSpec {
    Pattern[] getChildren() {return []}
    long getSize() {Integer.SIZE}
    Map<String, CeptrProcess> getProcesses() {
	[add: new CeptrProcess{Receptor r ->
	      SurfaceNounPair pair = r.stack.popBySpec(CeptrInteger.class)
	      def finished = new BigInteger(pair.surface).add(new BigInteger(delegate))
	      ByteBuffer byteSurface = ByteBuffer.allocate(4).putInt(finished.intValue())
	      r.stack.push(pair.noun, new Surface(byteSurface))
	    }
	 , subtract: new CeptrProcess{}
	 , inc: new CeptrProcess{
	      def finished = new BigInteger(delegate).add(BigInteger.valueOf(1))
	      ByteBuffer byteSurface = ByteBuffer.allocate(4).putInt(finished.intValue())
	      delegate.set(byteSurface)
	    }
	    ]
    }
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
