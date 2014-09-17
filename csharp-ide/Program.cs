using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

using Clifton.MycroParser;

namespace csharp_ide
{
	static class Program
	{
		public static Form MainForm;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);
			MainForm = MycroParser.InstantiateFromFile<Form>("mainform.xml", null);
			Application.Run(MainForm);
		}
	}
}
