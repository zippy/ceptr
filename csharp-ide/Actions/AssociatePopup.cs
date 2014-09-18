using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Clifton.Windows.Forms;

using csharp_ide.Controls;
using csharp_ide.Controllers;

namespace csharp_ide.Actions
{
	public class AssociatePopup : DeclarativeAction
	{
		public object Control { get; protected set; }
		public ContextMenuStrip ContextMenu { get; protected set; }

		public override void EndInit()
		{
			if (Control is TreeViewControl)
			{
				// ((TreeViewControl)Control).NodeRightClick += ContextMenuPopup;
			}
		}

		// TODO: Implement using the TreeViewControl (Windows.Form wrapper)
		protected void ContextMenuPopup(object sender, object tag, Point mousePosition)
		{
			ContextMenu.Show((Control)Control, mousePosition);
		}
	}
}
