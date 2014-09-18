using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace csharp_ide.Actions
{
	public class BindControlProperty : DeclarativeAction
	{
		public Control Control { get; protected set; }
		public string ControlProperty { get; protected set; }
		public object Object { get; protected set; }
		public string ObjectProperty { get; protected set; }

		public override void EndInit()
		{
			Control.DataBindings.Add(ControlProperty, Object, ObjectProperty);
		}
	}
}
