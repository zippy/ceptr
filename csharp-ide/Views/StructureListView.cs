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
	public class StructureListView : PaneView
	{
		protected ListView structureList;

		public delegate void NotificationDlgt();

		public event NotificationDlgt Opening;
		public event NotificationDlgt Closing;

		public ApplicationModel Model { get; protected set; }
		public override string MenuName { get { return "mnuStructureList"; } }

		public ListView StructureList
		{
			get { return structureList; }
			set
			{
				structureList = value;
				structureList.Columns.Add("Structures", -2, HorizontalAlignment.Center);
			}
		}

		public StructureListView()
		{
		}

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		// TODO: This should be responding to changes in the model that the controller is affecting!
		public void AddDistinctStructure(string name)
		{
			if (Model.IncrementStructureReference(name) == 1)
			{
				StructureList.Items.Add(new ListViewItem(name));
			}
		}

		public void ShowDistinctStructure(string name)
		{
			if (StructureList.Items.Cast<ListViewItem>().None(lvi => lvi.Text == name))
			{
				StructureList.Items.Add(new ListViewItem(name));
			}
		}

		// TODO: This should be responding to changes in the model that the controller is affecting!
		public void RemoveStructure(string name)
		{
			if (Model.DecrementStructureReference(name) == 0)
			{
				foreach (ListViewItem lvi in StructureList.Items)
				{
					if (lvi.Text == name)
					{
						StructureList.Items.Remove(lvi);
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
