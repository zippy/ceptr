using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using csharp_ide.Controllers;

namespace csharp_ide.Actions
{
	public class SetDocumentText : DeclarativeAction
	{
		public ITextController Controller { get; set; }
		public string Text { get; set; }

		public override void EndInit()
		{
			Controller.SetDocumentText(Text);
		}
	}
}
