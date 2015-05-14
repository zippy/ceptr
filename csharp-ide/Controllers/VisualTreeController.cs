using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using System.Xml.Linq;

using Clifton.Tools.Strings.Extensions;
using Clifton.Windows.Forms.XmlTree;

using csharp_ide.Models;
using csharp_ide.Views;

using ceptrlib;

namespace csharp_ide.Controllers
{
	public class VisualTreeController : ViewController<VisualTreeView>
	{
		public VisualTreeController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.VisualTreeController = this;
			base.EndInit();
		}

		public void ShowTree(string json)
		{
			// Write the file to where my webserver is running.  Kludge for now!
			File.WriteAllText(@"C:\BasicWebServer\ConsoleWebServer\Website\test.json", json);
			View.RefreshBrowser();
		}
	}
}
