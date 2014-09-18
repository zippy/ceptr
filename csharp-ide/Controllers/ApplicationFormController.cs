using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using WeifenLuo.WinFormsUI.Docking;

using Clifton.ApplicationStateManagement;
using Clifton.Assertions;
using Clifton.ExtensionMethods;
using Clifton.MycroParser;
using Clifton.Tools.Strings.Extensions;

using csharp_ide.Actions;
using csharp_ide.Views;

namespace csharp_ide.Controllers
{
	public class ApplicationFormController : ViewController<ApplicationFormView>
	{
		public IMruMenu MruMenu { get; protected set; }
		public SymbolEditorController SymbolEditorController { get; protected set; }
		public PropertyGridController PropertyGridController { get; protected set; }

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
			SaveLayout();
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

		// Pane management

		public void PaneClosed(PaneView pane)
		{
			if (pane is SymbolEditorView)
			{
				SymbolEditorController = null;
			}
			else if (pane is PropertyGridView)
			{
				PropertyGridController = null;
			}
			else
			{
				throw new ApplicationException("Unknown pane : " + pane.GetType().FullName);
			}
		}

		protected void ShowPropertyGrid(object sender, EventArgs args)
		{
			PropertyGridController.IfNull(() =>
			{
				NewDocument("propertyGrid.xml");
			});
		}

		protected void ShowSymbolEditor(object sender, EventArgs args)
		{
			SymbolEditorController.IfNull(() =>
			{
				NewDocument("symbolEditorTree.xml");
			});
		}

		protected void LoadTheLayout(string layoutFilename)
		{
			View.DockPanel.LoadFromXml(layoutFilename, ((string persistString) =>
			{
				string typeName = persistString.LeftOf(',').Trim();
				string contentMetadata = persistString.RightOf(',').Trim();
				IDockContent container = InstantiateContainer(typeName, contentMetadata);
				InstantiateContent(container, contentMetadata);

				return container;
			}));
		}

		protected void LoadLayout(object sender, EventArgs args)
		{
			if (File.Exists("layout.xml"))
			{
				LoadTheLayout("layout.xml");
			}
			else
			{
				RestoreLayout(sender, args);
			}
		}

		protected void RestoreLayout(object sender, EventArgs args)
		{
			if (File.Exists("defaultLayout.xml"))
			{
				CloseAllDockContent();
				LoadTheLayout("defaultLayout.xml");
			}
		}

		protected void SaveLayout()
		{
			// Close documents first, so we don't get dummy documents when we reload the layout.
			CloseAllDocuments();
			View.DockPanel.SaveAsXml("layout.xml");
		}

		protected IDockContent InstantiateContainer(string typeName, string metadata)
		{
			IDockContent container = null;

			if (typeName == typeof(GenericPane).ToString())
			{
				container = new GenericPane(metadata);
			}
			else if (typeName == typeof(GenericDocument).ToString())
			{
				container = new GenericDocument(metadata);
			}

			return container;
		}

		protected void InstantiateContent(object container, string filename)
		{
			MycroParser.InstantiateFromFile<object>(filename, ((MycroParser mp) =>
			{
				mp.AddInstance("Container", container);
				mp.AddInstance("ApplicationFormController", this);
				mp.AddInstance("ApplicationModel", ApplicationModel);
			}));
		}

		protected void CloseAllDockContent()
		{
			View.CloseAll();
		}

		protected void CloseAllDocuments()
		{
			View.CloseDocuments();
		}

		protected void NewDocument(string filename)
		{
			GenericDocument doc = new GenericDocument(filename);
			InstantiateContent(doc, filename);
			doc.Show(View.DockPanel);
		}

		protected void NewPane(string filename)
		{
			GenericPane pane = new GenericPane(filename);
			InstantiateContent(pane, filename);
			pane.Show(View.DockPanel);
		}

		// Menu management

		public void SetMenuCheckedState(string menuName, bool state)
		{
			View.SetMenuCheckState(menuName, state);
		}

		public void SetMenuEnabledState(string menuName, bool state)
		{
			View.SetMenuEnabledState(menuName, state);
		}
	}
}
