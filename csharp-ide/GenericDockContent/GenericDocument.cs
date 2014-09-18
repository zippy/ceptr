using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using WeifenLuo.WinFormsUI.Docking;

namespace csharp_ide
{
	public class GenericDocument : DockContent, IGenericDock
	{
		public string ContentMetadata { get; set; }

		public GenericDocument()
		{
			ContentMetadata = String.Empty;
		}

		public GenericDocument(string contentMetadata)
		{
			ContentMetadata = contentMetadata;
		}

		protected override string GetPersistString()
		{
			return GetType().ToString() + "," + ContentMetadata;
		}
	}
}
