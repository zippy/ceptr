using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using WeifenLuo.WinFormsUI.Docking;

namespace csharp_ide
{
	public class GenericPane : ToolWindow, IGenericDock
	{
		public string ContentMetadata { get; set; }

		public GenericPane()
		{
			ContentMetadata = String.Empty;
		}

		public GenericPane(string contentMetadata)
		{
			ContentMetadata = contentMetadata;
		}

		protected override string GetPersistString()
		{
			return GetType().ToString() + "," + ContentMetadata;
		}
	}
}
