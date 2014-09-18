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

	public class Schema : IHasCollection
	{
		[Category("Name")]
		[XmlAttribute()]
		public string Name { get; set; }

		[XmlIgnore]
		[Browsable(false)]
		public Dictionary<string, dynamic> Collection { get { return null; } }

		[Browsable(false)]

		[XmlIgnore]
		public static Schema Instance { get; protected set; }

		public Schema()
		{
			Instance = this;
		}
	}
}