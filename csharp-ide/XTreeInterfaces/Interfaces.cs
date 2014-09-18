using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace XTreeInterfaces
{
	public interface IHasCollection
	{
		string Name { get; set; }
		Dictionary<string, dynamic> Collection { get; }
	}
}
