using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ceptrlib
{
	// Emulates typedefs
	using Symbol = System.UInt32;
	using Process = System.UInt32;
	using Structure = System.UInt32;

	public enum SystemSymbol
	{
		//-----  Basic symbols for underlying data types
		NULL_SYMBOL = 0x7fff0000,
		TRUE_FALSE,

		//-----  Symbols for the different semantic parts of semtrexes
		SEMTREX_MATCHED_PATH,              ///< path to symbol matched by semtrex
		SEMTREX_SYMBOL_LITERAL,            ///< This system symbol matches on the semantic type.         Ex: /TEST_SYMBOL
		SEMTREX_SEQUENCE,                  ///< Match on a sequence of child nodes which are any valid semtrex's.  Ex: comma separated nodes
		SEMTREX_OR,                        ///< Logical OR between two Semtrex expressions.      Ex: |
		SEMTREX_SYMBOL_ANY,                ///< Match any symbol or value of the node.           Ex: .
		SEMTREX_ZERO_OR_MORE,              ///< Requires one child Semtrex and matches on zero or more of that Semtrex.  Ex: /0/TestSemtrex*
		SEMTREX_ONE_OR_MORE,               ///< Requires one child Semtrex and matches on one or more of that Semtrex.   Ex: /0/TestSemtrex+
		SEMTREX_ZERO_OR_ONE,               ///< Requires one child Semtrex and matches on zero or one of that Semtrex.   Ex: /0/TestSemtrex?
		SEMTREX_VALUE_LITERAL,	       ///< Matches on the semantic type and the data value.
		SEMTREX_GROUP,                     ///< Grouping                                 Ex: (...)operator
		SEMTREX_MATCH,                     ///< Returns result and sibling count.        Ex: {name:expr} (verify this is what it's supposed to do)
		SEMTREX_MATCH_RESULTS,             ///< In the FSA, keeps track of which part matches so it can be referenced
		SEMTREX_MATCH_SIBLINGS_COUNT,      ///< In the FSA, it's the length of the match

		//-----  Symbols for receptors
		RECEPTOR_XADDR,                    ///< An Xaddr that points to a receptor
		FLUX,                              ///< tree to hold all incoming and in process signals on the various aspects
		DEFINITIONS,
		STRUCTURES,
		STRUCTURE_DEFINITION,
		STRUCTURE_LABEL,
		STRUCTURE_PARTS,
		STRUCTURE_PART,
		SYMBOLS,
		SYMBOL_DECLARATION,
		SYMBOL_STRUCTURE,
		SYMBOL_LABEL,
		SCAPE_SPEC,
		ASPECT,
		SIGNALS,                           ///< list of signals on an aspect in the flux
		SIGNAL,                            ///< a signal on the flux.  It's first child is the contents of the signal
		LISTENERS,                         ///< list of carrier/expectation/action tress that "listen" to changes on the flux
		LISTENER,                          ///< surface of the listener is the carrier symbol, and it has two children, expectation semtrex and action code tree
		EXPECTATION,                       ///< expectation is a semtrex (thus has one child which is the first part of the semtrex)
		ACTION,                            ///< code tree, which specifies the action to perform when an expectation's semtrex matches
		INTERPOLATE_SYMBOL,                ///< a place holder to indicate which symbol to insert into this part of the three
		PROCESSES,
		PROCESS_CODING,
		PROCESS_NAME,
		PROCESS_INTENTION,
		INPUT,
		INPUT_SIGNATURE,
		INPUT_LABEL,
		SIGNATURE_STRUCTURE,
		OUTPUT_SIGNATURE,
		RUN_TREE,                         ///< think about this as a stack frame and it's code
		PARAM_REF,                        ///< used in a code tree as a reference to a parameter
		PARAMS,
		SCAPES,

		//-----  Symbols for the virtual machine host
		VM_HOST_RECEPTOR,
		COMPOSITORY,                      ///< receptor that holds available receptor packages for installation
		MANIFEST,                         ///< configuration template to be filled out for the installation of a receptor
		MANIFEST_PAIR,
		MANIFEST_LABEL,                   ///< a label in the manifest to identify a binding
		MANIFEST_SPEC,                    ///< a symbol to specify what type of data must be provided for a given manifest label
		RECEPTOR_PACKAGE,                 ///< a manifest, a symbol declaration tree, a structure definition tree, and an identifier
		RECEPTOR_IDENTIFIER,              ///< uuid that identifies receptors
		INSTALLED_RECEPTOR,               ///< contains the installed receptor as well as state information (enabled,disabled, etc..)
		ACTIVE_RECEPTORS,                 ///< list of currently active INSTALLED_RECEPTORS
		BINDINGS,                         ///< specifics that match a MANIFEST and allow a receptor to be installed
		BINDING_PAIR,                     ///< a pair that matches a MANIFEST_LABEL with a given binding
		_LAST_SYS_SYMBOL
	}

	public enum TestSymbol
	{
		TEST_INT_SYMBOL = 0x7ffff000, 
		TEST_INT_SYMBOL2, 
		TEST_STR_SYMBOL, 
		TEST_TREE_SYMBOL, 
		TEST_TREE_SYMBOL2,
		TEST_NAME_SYMBOL, 
		TEST_FIRST_NAME_SYMBOL, 
		TEST_RECEPTOR_SYMBOL,
		_LAST_TEST_SYMBOL
	}

	public enum SystemStructure
    {
		NULL_STRUCTURE = 0x7fff0000,
		BOOLEAN,
		INTEGER,
		FLOAT,
		CSTRING,
		SYMBOL,
		TREE_PATH,
		XADDR,
		LIST,
		SURFACE,
		TREE,
		RECEPTOR,
		PROCESS,
		STRUCTURE,
		SCAPE,
		_LAST_SYS_STRUCTURE
    }

	[StructLayout(LayoutKind.Sequential, Pack=1), Serializable]
	public unsafe struct Tstruct
	{
		public Tnode* node;
		public int child_count;
		public Tnode** children;
		
		// public Tnode* node;
		// public int child_count;
		// public Tnode** children;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1), Serializable]
	public unsafe struct Tcontents
	{
		public Symbol symbol;
		public int size;
		public IntPtr surface;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1), Serializable]
	public unsafe struct Tcontext
	{
		public int flags;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1), Serializable]
	public unsafe struct Tnode
	{
		public Tstruct structure;
		public Tcontext context;
		public Tcontents contents;
	}

	[StructLayout(LayoutKind.Sequential, Pack = 1), Serializable]
	public unsafe struct Defs 
	{
		public Tnode *structures;
		public Tnode *symbols;   
		public Tnode *processes; 
		public Tnode *scapes;    
	};

	public class CeptrInterface
	{
		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe void testGetSize();

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Tnode* _t_new(IntPtr parent, Symbol symbol, IntPtr surface, int size);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Tnode* _t_new_root(Symbol symbol);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Symbol _d_declare_symbol(Tnode* symbols, Structure st, string label);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Symbol _d_define_structure(Tnode* structures, [MarshalAs(UnmanagedType.LPStr)] string label, int num_params, __arglist);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Symbol _dv_define_structure(Tnode* structures, [MarshalAs(UnmanagedType.LPStr)] string label, int num_params, __arglist);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		extern static unsafe Structure _t_children(Tnode* structures);

		// extern static unsafe Symbol _d_define_structure(Tnode* structures, char* label, int num_params, UInt32 p1, UInt32 p2);

		[DllImport("libceptrlib.dll", CallingConvention = CallingConvention.Cdecl)]
		// [return: MarshalAs(UnmanagedType.LPStr)]
		extern static unsafe void __t_dump(Defs* defs, Tnode* t, int level, char* buf);

		protected Dictionary<Guid, IntPtr> nodes = new Dictionary<Guid, IntPtr>();

		/// <summary>
		/// Create a root node.
		/// </summary>
		/// <param name="symbol">The node symbol.</param>
		/// <returns>A GUID associated with this node instance.</returns>
		public unsafe Guid CreateRootNode(SystemSymbol symbol)
		{
			Tnode *node = _t_new_root((Symbol)symbol);
			Guid guid = RegisterNode(node);

			return guid;
		}

		/// <summary>
		/// Declare a symbol having the specified structure.
		/// </summary>
		public unsafe uint DeclareSymbol(Guid symbols, Structure st, string label)
		{
			Tnode *pnode = (Tnode*)nodes[symbols];
			Symbol symbol = _d_declare_symbol(pnode, st, label);

			return symbol;
		}

		public unsafe uint DefineStructure(Guid structures, string name, uint[] symbolArray)
		{
			Tnode *structs = (Tnode*)nodes[structures];

			_dv_define_structure(structs, name, symbolArray.Length, __arglist(symbolArray));
			Structure st = _t_children(structs);

			return st;
		}

		public unsafe string Dump(Guid g_symbols, Guid g_structures)
		{
			//testGetSize();

			//return "";

			Defs defs = new Defs() 
			{ 
				structures = (Tnode*)nodes[g_structures], 
				symbols = (Tnode*)nodes[g_symbols], 
				processes = (Tnode*)0, 
				scapes = (Tnode*)0 
			};

			string ret = String.Empty;

			try
			{
				fixed (char* buf = new char[10000])
				{
					// __t_dump(&defs, defs.structures, 0, buf);
					__t_dump(&defs, defs.structures, 0, buf);
					ret = Marshal.PtrToStringAnsi((IntPtr)buf);
				}
			}
			catch (Exception ex)
			{
				System.Diagnostics.Debugger.Break();
			}

			return ret;
		}

		/// <summary>
		/// Return a Guid associated with the Tnode* instance.
		/// </summary>
		protected unsafe Guid RegisterNode(Tnode* node)
		{
			Guid guid = Guid.NewGuid();
			nodes[guid] = (IntPtr)node;

			return guid;
		}
	}
}

/*
 * 
 * Converting a char* to a string (return value)
 * http://stackoverflow.com/questions/9041094/char-to-a-string-in-c-sharp
 * 
using System.Runtime.InteropServices;
...
string s = Marshal.PtrToStringAnsi((IntPtr)c);
  
or 

[DllImport ( "MyDll.dll", CharSet = CharSet.Ansi, EntryPoint = "Func" )]
[return : MarshalAs( UnmanagedType.LPStr )]
string Func ( ... );
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
*/