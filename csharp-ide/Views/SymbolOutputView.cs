using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using csharp_ide.Controllers;

namespace csharp_ide.Views
{
	public class SymbolOutputView : UserControl
	{
		public TextBox Output { get; protected set; }

		public ApplicationFormController ApplicationController { get; protected set; }
		public SymbolOutputController Controller { get; protected set; }

		public SymbolOutputView()
		{
		}
	}
}
