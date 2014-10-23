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

using csharp_ide.Models;
using csharp_ide.Views;

using ceptrlib;

namespace csharp_ide.Controllers
{
	public class SemtrexUIController : ViewController<SemtrexUIView>
	{
		public SemtrexUIController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.SemtrexUIController = this;
			base.EndInit();
		}
	}
}