using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace csharp_ide.Models
{
	public class ApplicationModel
	{
		public Dictionary<string, int> StructureRefCount { get; protected set; }
		public Dictionary<string, int> SymbolRefCount { get; protected set; }

		public ApplicationModel()
		{
			StructureRefCount = new Dictionary<string, int>();
			SymbolRefCount = new Dictionary<string, int>();
		}

		public int IncrementStructureReference(string name)
		{
			return IncrementReference(StructureRefCount, name);
		}

		public int IncrementSymbolReference(string name)
		{
			return IncrementReference(SymbolRefCount, name);
		}

		public int DecrementStructureReference(string name)
		{
			return DecrementReference(StructureRefCount, name);
		}

		public int DecrementSymbolReference(string name)
		{
			return DecrementReference(SymbolRefCount, name);
		}

		/// <summary>
		/// Increment the reference count to the specified name, returning the new count.
		/// </summary>
		protected int IncrementReference(Dictionary<string, int> dict, string name)
		{
			int count;

			if (!dict.TryGetValue(name, out count))
			{
				dict[name] = 0;
				count = 0;
			}

			dict[name] = ++count;

			return count;
		}

		/// <summary>
		/// Decrement the reference.  If it doesn't exist, which is possible, return -1.
		/// If it does exist, return the decremented count.
		/// The reference count however is always guaranteed to be >= 0.
		/// </summary>
		protected int DecrementReference(Dictionary<string, int> dict, string name)
		{
			int count = 0;

			if (dict.TryGetValue(name, out count))
			{
				// Do not store a negative count.
				if (--count >= 0)
				{
					dict[name] = count;
				}
			}
			else
			{
				dict[name] = 0;
				count = -1;			// Indicate an error situation.
			}

			return count;
		}


	}
}
