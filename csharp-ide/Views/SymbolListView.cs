using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using csharp_ide.Controls;
using csharp_ide.Controllers;
using csharp_ide.Models;

using Clifton.ExtensionMethods;
using Clifton.Tools.Data;

namespace csharp_ide.Views
{
	public class SymbolListView : PaneView
	{
		protected ListView symbolList;
		protected Dictionary<string, int> symbolRefCount;

		public delegate void NotificationDlgt();

		public event NotificationDlgt Opening;
		public event NotificationDlgt Closing;

		public ApplicationModel Model { get; protected set; }
		public ListView SymbolList 
		{
			get {return symbolList;}
			set 
			{
				symbolList=value;
				symbolList.Columns.Add("Symbols", -2, HorizontalAlignment.Center);
			}
		}

		public override string MenuName { get { return "mnuSymbolList"; } }

		public SymbolListView()
		{
			symbolRefCount = new Dictionary<string, int>();
		}

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		public void AddDistinctSymbol(string name)
		{
			if (ApplicationFormController.IncrementReference(symbolRefCount, name) == 1)
			{
				SymbolList.Items.Add(new ListViewItem(name));
			}
		}

		protected override void WhenHandleDestroyed(object sender, EventArgs e)
		{
			Closing.IfNotNull().Then(() => Closing());
			base.WhenHandleDestroyed(sender, e);
		}
	}
}
