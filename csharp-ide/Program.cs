using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

using Clifton.ApplicationStateManagement;
using Clifton.MycroParser;

namespace csharp_ide
{
	static class Program
	{
		public static Form MainForm;
		public static StatePersistence AppState;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main()
		{
			Application.EnableVisualStyles();
			Application.SetCompatibleTextRenderingDefault(false);

			try
			{
				AppState = new StatePersistence();
				AppState.ReadState("appState.xml");																	// Load the last application state.
				MainForm = MycroParser.InstantiateFromFile<Form>("mainform.xml", null);
				Application.Run(MainForm);
				AppState.WriteState("appState.xml");
			}
			catch (Exception ex)
			{
				MessageBox.Show(ex.Message + "\r\n"+ex.StackTrace, "Critical Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}
	}
}
