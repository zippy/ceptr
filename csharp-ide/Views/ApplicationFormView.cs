using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using WeifenLuo.WinFormsUI.Docking;

using Clifton.ExtensionMethods;
using Clifton.MycroParser;

using csharp_ide.Controllers;
using csharp_ide.Models;

namespace csharp_ide.Views
{
	public class ApplicationFormView : Form, IMycroParserInstantiatedObject
	{
		public DockPanel DockPanel { get; protected set; }
		public ApplicationFormController ApplicationController { get; protected set; }
		public ApplicationModel Model { get; protected set; }
		public Dictionary<string, object> ObjectCollection { get; set; }

		public void CloseAll()
		{
			// ToArray, so we get a copy rather than iterating through the original list, which is being modified.
			DockPanel.Contents.ToArray().ForEach(t => t.DockHandler.Close());
		}

		public void CloseDocuments()
		{
			// This works too.
			// DockPanel.DocumentsToArray().ForEach(t => t.DockHandler.Close());
			// ToArray, so we get a copy rather than iterating through the original list, which is being modified.
			// DockPanel.Contents.Where(t => t is GenericDocument).ToArray().ForEach(t => ((IDockContent)t).DockHandler.Close());
		}

		public void SetMenuCheckState(string menuName, bool checkedState)
		{
			((ToolStripMenuItem)ObjectCollection[menuName]).Checked = checkedState;
		}

		public void SetMenuEnabledState(string menuName, bool enabledState)
		{
			((ToolStripMenuItem)ObjectCollection[menuName]).Enabled = enabledState;
		}

		public void SetCaption(string text)
		{
			//if (!String.IsNullOrEmpty(text))
			//{
			//	string filename = Path.GetFileNameWithoutExtension(text);
			//	string path = Path.GetDirectoryName(text);

			//	if (String.IsNullOrEmpty(path))
			//	{
			//		// Use the application path if we don't have a formal path defined.
			//		path = Path.GetDirectoryName(Path.GetFullPath(text));
			//	}

			//	Text = /*"Intertexti - " +*/ filename + " (" + path + ")";
			//}
			//else
			//{
			//	Text = "- new"; // Intertexti - new";
			//}

			//WhenModelIsDirty(Model.IsDirty);

			Text = text;
		}
	}
}
