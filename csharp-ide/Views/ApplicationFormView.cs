using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Clifton.MycroParser;

namespace csharp_ide.Views
{
	public class ApplicationFormView : Form, IMycroParserInstantiatedObject
	{
		public Dictionary<string, object> ObjectCollection { get; set; }
	}
}
