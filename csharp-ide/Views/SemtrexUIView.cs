using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using csharp_ide.Controllers;

namespace csharp_ide.Views
{
	public class SemtrexUIView : UserControl
	{
		public TextBox Output { get; protected set; }

		public ApplicationFormController ApplicationController { get; protected set; }
		public SemtrexUIController Controller { get; protected set; }

		public TextBox tbInputString { get; set; }
		public TextBox tbParseExpr { get; set; }
		public TextBox tbMatchExpression { get; set; }
		public TextBox tbSemtrexTree { get; set; }
		public TextBox tbMatchResult { get; set; }
		public TextBox tbMatchResult2 { get; set; }
		public TextBox tbMatchAgainst { get; set; }

		public SemtrexUIView()
		{
		}
	}
}
