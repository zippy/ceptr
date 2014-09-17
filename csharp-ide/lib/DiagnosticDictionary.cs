using System;
using System.Collections.Generic;

namespace Clifton.Collections.Generic
{
	/// <summary>
	/// A dictionary with an indexer that produces an informative KeyNotFoundException message.
	/// </summary>
	public class DiagnosticDictionary<TKey, TValue> : Dictionary<TKey, TValue>
	{
		protected object tag;
		protected string name = "unknown";

		/// <summary>
		/// Gets/sets an object that you can associate with the dictionary.
		/// </summary>
		public object Tag
		{
			get { return tag; }
			set { tag = value; }
		}

		/// <summary>
		/// The dictionary name.  The default is "unknown".  Used to enhance the KeyNotFoundException.
		/// </summary>
		public string Name
		{
			get { return name; }
			set { name = value; }
		}

		/// <summary>
		/// Parameterless constructor.
		/// </summary>
		public DiagnosticDictionary()
		{
		}

		/// <summary>
		/// Constructor that takes a name.
		/// </summary>
		public DiagnosticDictionary(string name)
		{
			this.name = name;
		}

		public DiagnosticDictionary(Dictionary<TKey, TValue> copyFrom)
		{
			foreach (KeyValuePair<TKey, TValue> kvp in copyFrom)
			{
				this[kvp.Key] = kvp.Value;
			}
		}

		/// <summary>
		/// Indexer that produces a more useful KeyNotFoundException.
		/// </summary>
		public new TValue this[TKey key]
		{
			get
			{
				try
				{
					return base[key];
				}
				catch (KeyNotFoundException)
				{
					throw new KeyNotFoundException("The key '" + key.ToString() + "' was not found in the dictionary '"+name+"'");
				}
			}

			set	{ base[key] = value; }
		}
	}

/*
	// extension method example:
	public static class DiagnosticDictionary
	{
		public static string SafeItem(this Dictionary<string, string> d, string key)
	    {
	        try
	        {
	            return d[key];
	        }
	        catch (KeyNotFoundException)
	        {
	            throw new KeyNotFoundException("The key '" + key + "' was not found in the dictionary.");
	        }
	    }
	}
 */ 
}
