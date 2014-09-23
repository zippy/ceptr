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
		protected Dictionary<string, int> structureRefCount;

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
			structureRefCount = new Dictionary<string, int>();
		}

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		public void AddDistinctStructure(string name)
		{
			if (ApplicationFormController.IncrementReference(structureRefCount, name) == 1)
			{
				StructureList.Items.Add(new ListViewItem(name));
			}
		}

		public void RemoveStructure(string name)
		{
			if (ApplicationFormController.DecrementReference(structureRefCount, name) == 0)
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
