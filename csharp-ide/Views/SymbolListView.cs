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
		}

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		// TODO: This should be responding to changes in the model that the controller is affecting!
		public void AddDistinctSymbol(string name)
		{
			if (Model.IncrementSymbolReference(name) == 1)
			{
				SymbolList.Items.Add(new ListViewItem(name));
			}
		}

		public void ShowDistinctSymbol(string name)
		{
			if (SymbolList.Items.Cast<ListViewItem>().None(lvi => lvi.Text == name))
			{
				SymbolList.Items.Add(new ListViewItem(name));
			}
		}

		// TODO: This should be responding to changes in the model that the controller is affecting!
		public void RemoveSymbol(string name)
		{
			if (Model.DecrementSymbolReference(name) == 0)
			{
				foreach (ListViewItem lvi in SymbolList.Items)
				{
					if (lvi.Text == name)
					{
						SymbolList.Items.Remove(lvi);
						break;
					}
				}
			}
		}

		protected override void WhenHandleDestroyed(object sender, EventArgs e)
		{
			Closing.IfNotNull().Then(() => Closing());
			base.WhenHandleDestroyed(sender, e);
		}
	}
}
