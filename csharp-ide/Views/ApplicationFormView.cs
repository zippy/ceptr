using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Clifton.MycroParser;

using csharp_ide.Controllers;
using csharp_ide.Models;

namespace csharp_ide.Views
{
	public class ApplicationFormView : Form, IMycroParserInstantiatedObject
	{
		public ApplicationFormController ApplicationController { get; protected set; }
		public ApplicationModel Model { get; protected set; }
		public Dictionary<string, object> ObjectCollection { get; set; }
	}
}
