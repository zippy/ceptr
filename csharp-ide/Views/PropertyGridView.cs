using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using csharp_ide.Controls;
using csharp_ide.Models;

using Clifton.ExtensionMethods;
using Clifton.Tools.Data;

namespace csharp_ide.Views
{
	public class PropertyGridView : PaneView
	{
		public delegate void NotificationDlgt();

		public event NotificationDlgt Opening;
		public event NotificationDlgt Closing;

		public ApplicationModel Model { get; protected set; }
		public PropertyGrid PropertyGrid { get; protected set; }
		public override string MenuName { get { return "mnuPropertyGrid"; } }

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		public void ShowObject(object obj)
		{
			PropertyGrid.SelectedObject = obj;
			// PropertyGrid.Refresh();
		}

		public void Clear()
		{
			PropertyGrid.SelectedObject = null;
		}

		protected override void WhenHandleDestroyed(object sender, EventArgs e)
		{
			Closing.IfNotNull().Then(() => Closing());
			base.WhenHandleDestroyed(sender, e);
		}
	}
}
