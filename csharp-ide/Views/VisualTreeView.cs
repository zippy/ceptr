using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using CefSharp;
using CefSharp.WinForms;

using csharp_ide.Controllers;

namespace csharp_ide.Views
{
	public class VisualTreeView : UserControl
	{
		protected IWinFormsWebBrowser browser;

		public ApplicationFormController ApplicationController { get; protected set; }
		public VisualTreeController Controller { get; protected set; }
		public IWinFormsWebBrowser Browser
		{
			get { return browser; }
			set { browser = value; }
		}

		public VisualTreeView()
		{
		}

		public void Clear()
		{
		}

		public void RefreshBrowser()
		{
			browser.Load("http://localhost/");
		}
	}
}
