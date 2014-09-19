using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

using XTreeInterfaces;

namespace csharp_ide.Models
{
	public class Symbol : IHasCollection, IHasFullyQualifiedName
	{
		[Category("Symbol")]
		[XmlAttribute()]
		public string Name { get; set; }

		[Category("Symbol")]
		[XmlAttribute()]
		//[DisplayName("Native Type")]
		//[TypeConverter(typeof(ImplementingTypeNameConverter))]			// TODO: Implement as a lookup
		public string Structure { get; set; }

		// Serializable list.
		[Browsable(false)]
		public List<Symbol> Symbols { get; set; }

		[XmlIgnore]
		[Browsable(false)]
		public Dictionary<string, dynamic> Collection { get; set; }

		[XmlIgnore]
		[Browsable(false)]
		public string FullyQualfiedName
		{
			get { return Name + " : " + Structure; }
		}

		public Symbol()
		{
			Name = String.Empty;
			Structure = "(undefined)";
			Symbols = new List<Symbol>();
			Collection = new Dictionary<string, dynamic>() 
			{ 
				{"Models.Symbol", Symbols},
			};
		}
	}

	public class SymbolNamespaceContainer : IHasCollection
	{
		[XmlAttribute()]
		public string Name { get; set; }

		[XmlIgnore]
		[Browsable(false)]
		public Dictionary<string, dynamic> Collection { get; protected set; }

		// Serializable list.
		[Browsable(false)]
		public List<Symbol> Symbols { get; set; }

		public SymbolNamespaceContainer()
		{
			Symbols = new List<Symbol>();
			Collection = new Dictionary<string, dynamic>() 
			{ 
				{"Models.Symbol", Symbols},
			};
		}
	}

	public class Schema : IHasCollection
	{
		[Category("Name")]
		[XmlAttribute()]
		public string Name { get; set; }

		[XmlIgnore]
		[Browsable(false)]
		public Dictionary<string, dynamic> Collection { get; set; }

		[Browsable(false)]
		public List<SymbolNamespaceContainer> SymbolNamespaceContainer { get; set; }


		[Browsable(false)]

		[XmlIgnore]
		public static Schema Instance { get; protected set; }

		public Schema()
		{
			SymbolNamespaceContainer = new List<SymbolNamespaceContainer>();
			Collection = new Dictionary<string, dynamic>() 
			{
				{"Models.SymbolNamespaceContainer", SymbolNamespaceContainer},
			};

			Instance = this;
		}
	}
}