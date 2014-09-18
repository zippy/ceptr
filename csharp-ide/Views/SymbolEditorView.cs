using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using csharp_ide.Controls;
using csharp_ide.Models;

using Clifton.ExtensionMethods;
using Clifton.Tools.Data;
using Clifton.Windows.Forms;
using Clifton.Windows.Forms.XmlTree;

namespace csharp_ide.Views
{
	public class SymbolEditorView : PaneView
	{
		public delegate void NotificationDlgt();

		public event NotificationDlgt Opening;
		public event NotificationDlgt Closing;

		public ApplicationModel Model { get; protected set; }
		public XTree TreeView { get; protected set; }
		public override string MenuName { get { return "mnuSymbolTypeEditor"; } }

		public override void EndInit()
		{
			Opening.IfNotNull().Then(() => Opening());
			base.EndInit();
		}

		protected override void WhenHandleDestroyed(object sender, EventArgs e)
		{
			Closing.IfNotNull().Then(() => Closing());
			base.WhenHandleDestroyed(sender, e);
		}

		public void Clear()
		{
			TreeView.Clear();
		}

		public TreeNode AddNode(IXtreeNode inst, TreeNode parent)
		{
			TreeNode node = TreeView.AddNode(inst, parent);

			return node;
		}
	}
}
