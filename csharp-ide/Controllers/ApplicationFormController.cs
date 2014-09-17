using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using WeifenLuo.WinFormsUI.Docking;

using Clifton.ApplicationStateManagement;
using Clifton.Assertions;
using Clifton.Tools.Strings.Extensions;

using csharp_ide.Actions;
using csharp_ide.Views;

namespace csharp_ide.Controllers
{
	public class ApplicationFormController : ViewController<ApplicationFormView>
	{
		public IMruMenu MruMenu { get; protected set; }

		public ApplicationFormController()
		{
			RegisterUserStateOperations();
		}

		public override void EndInit()
		{
			// Restore window position and size when last closed.
			Assert.SilentTry(() => Program.AppState.RestoreState("Form"));
		}

		/// <summary>
		/// Register our application state getter and setter.
		/// </summary>
		protected void RegisterUserStateOperations()
		{
			Program.AppState.Register("Form", () =>
			{
				return new List<State>()
						{
							new State("X", View.Location.X),
							new State("Y", View.Location.Y),
							new State("W", View.Size.Width),
							new State("H", View.Size.Height),
							new State("WindowState", View.WindowState.ToString()),
							// new State("Last Opened", CurrentFilename),
						};

			},
				state =>
				{
					// Silently handle exceptions for when we add state items that are part of the state file until we 
					// save the state.  This allows us to add new state information without crashing the app on startup.
					Assert.SilentTry(() => View.Location = new Point(state.Single(t => t.Key == "X").Value.to_i(), state.Single(t => t.Key == "Y").Value.to_i()));
					Assert.SilentTry(() => View.Size = new Size(state.Single(t => t.Key == "W").Value.to_i(), state.Single(t => t.Key == "H").Value.to_i()));
					Assert.SilentTry(() => View.WindowState = state.Single(t => t.Key == "WindowState").Value.ToEnum<FormWindowState>());
				});
		}

		// Form Events

		protected void Closing(object sender, CancelEventArgs args)
		{
			Program.AppState.SaveState("Form");
		}

		protected void New(object sender, EventArgs args)
		{
		}

		protected void Load(object sender, EventArgs args)
		{
		}

		protected void Save(object sender, EventArgs args)
		{
		}

		protected void SaveAs(object sender, EventArgs args)
		{
		}

		protected void Exit(object sender, EventArgs args)
		{
			// TODO: If the model is dirty, check if user wants to save changes before exiting.
			View.Close();
		}

		// Docking panel events

		protected void ActiveDocumentChanged(object sender, EventArgs args)
		{
		}

		protected void ContentRemoved(object sender, DockContentEventArgs e)
		{
		}



	}
}
