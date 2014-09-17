using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Intertexti.lib
{
	public interface IMycroParserInstantiatedObject
	{
		Dictionary<string, object> ObjectCollection { get; set; }
	}
}
