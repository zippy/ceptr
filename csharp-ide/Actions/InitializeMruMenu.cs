using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using Clifton.Windows.Forms;

using csharp_ide.Controllers;

namespace csharp_ide.Actions
{
	public class InitializeMruMenu : DeclarativeAction, IMruMenu
	{
		public ToolStripMenuItem MenuFile { get; protected set; }
		public ToolStripMenuItem MenuRecentFiles { get; protected set; }
		public ApplicationFormController ApplicationController { get; protected set; }
		
		protected MruStripMenuInline mruMenu;

		public override void EndInit()
		{
			string mruRegKey = "SOFTWARE\\ceptr\\Config";
			mruMenu = new MruStripMenuInline(MenuFile, MenuRecentFiles, new MruStripMenu.ClickedHandler(OnMruFile), mruRegKey + "\\MRU", true, 9);
		}

		public void AddFile(string filename)
		{
			mruMenu.AddFile(filename);
			mruMenu.SaveToRegistry();
		}

		protected void OnMruFile(int idx, string filename)
		{
			// TODO: Check saving existing model before loading a new model.
			//if (ApplicationController.CheckDirtyModel())
			//{
			ApplicationController.LoadSymbols(filename);
			//}
		}
	}
}
