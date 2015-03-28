using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Serialization;

using WeifenLuo.WinFormsUI.Docking;

using Clifton.ApplicationStateManagement;
using Clifton.Assertions;
using Clifton.ExtensionMethods;
using Clifton.MycroParser;
using Clifton.Tools.Strings.Extensions;

using XTreeController;
													   
using csharp_ide.Actions;
using csharp_ide.Models;
using csharp_ide.Views;

using ceptrlib;

namespace csharp_ide.Controllers
{
	public class ApplicationFormController : ViewController<ApplicationFormView>
	{
		public static ApplicationFormController Instance { get; protected set; }

		public IMruMenu MruMenu { get; protected set; }
		public SymbolEditorController SymbolEditorController { get; protected set; }
		public PropertyGridController PropertyGridController { get; protected set; }
		public SymbolListController SymbolListController { get; protected set; }
		public StructureListController StructureListController { get; protected set; }
		public SymbolOutputController SymbolOutputController { get; set; }
		public StructureOutputController StructureOutputController { get; set; }
		public SemtrexUIController SemtrexUIController { get; set; }
		public VisualTreeController VisualTreeController { get; set; }
		public GenericController<Schema> schemaController;
		public string SchemaFilename { get; set; }

		public Schema Schema
		{
			get { return schema; }
			set
			{
				schema = value;
				schemaController.Instance = Schema;
				// ((Schema)((GenericController<Schema>)sc).Instance) = Schema;
			}
		}

		protected Schema schema;
		public CeptrInterface CeptrInterface;

		public ApplicationFormController()
		{
			Instance = this;
			RegisterUserStateOperations();
			CeptrInterface = new CeptrInterface();
			CeptrInterface.Initialize();
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
			SchemaFilename = null;
			CreateSymbolsRootNode();
		}

		protected void Load(object sender, EventArgs args)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.RestoreDirectory = true;
			ofd.CheckFileExists = true;
			ofd.Filter = "ceptr files (*.ceptr)|*.xml|All files (*.*)|*.*";
			ofd.Title = "Load Ceptr Symbols";
			DialogResult res = ofd.ShowDialog();

			if (res == DialogResult.OK)
			{
				SchemaFilename = ofd.FileName;
				LoadSymbols();
			}
		}

		protected void Save(object sender, EventArgs args)
		{
			if (String.IsNullOrEmpty(SchemaFilename))
			{
				SaveAs(sender, args);
			}
			else
			{
				SaveSymbols();
			}
		}

		protected void SaveAs(object sender, EventArgs args)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.OverwritePrompt = true;
			sfd.Filter = "ceptr files (*.ceptr)|*.ceptr|All files (*.*)|*.*";
			sfd.Title = "Save Ceptr Symbols";
			DialogResult res = sfd.ShowDialog();

			if (res == DialogResult.OK)
			{
				SchemaFilename = sfd.FileName;
				SaveSymbols();
			}
		}

		protected void Exit(object sender, EventArgs args)
		{
			// TODO: If the model is dirty, check if user wants to save changes before exiting.
			View.Close();
		}

		/// <summary>
		/// The first time the form is displayed, do some initialization of internal models and exposed UI's.
		/// </summary>
		protected void Shown(object sender, EventArgs args)
		{
			// Because I get tired of doing this manually.
			CreateSymbolsRootNode();
			SymbolEditorController.IfNotNull(ctrl => ctrl.UpdateView());
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
			else if (pane is SymbolListView)
			{
				SymbolListController = null;
			}
			else if (pane is StructureListView)
			{
				StructureListController = null;
			}
			else
			{
				throw new ApplicationException("Unknown pane : " + pane.GetType().FullName);
			}
		}

		// View management (menu handlers)

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

		protected void ShowSymbolList(object sender, EventArgs args)
		{
			SymbolListController.IfNull(() =>
			{
				NewDocument("symbolListPane.xml");
				ApplicationModel.SymbolRefCount.Keys.ToList().ForEach(k => SymbolListController.ShowSymbol(k));
			});
		}

		protected void ShowStructureList(object sender, EventArgs args)
		{
			StructureListController.IfNull(() =>
			{
				NewDocument("structureListPane.xml");
				ApplicationModel.StructureRefCount.Keys.ToList().ForEach(k => StructureListController.ShowStructure(k));
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

		protected void ShowSymbols(object sender, EventArgs args)
		{
			SymbolOutputController.IfNull(() =>
			{
				NewDocument("symbolOutput.xml");
			});
		}

		protected void ShowStructures(object sender, EventArgs args)
		{
			StructureOutputController.IfNull(() =>
			{
				NewDocument("structureOutput.xml");
			});
		}

		protected void ShowSemtrexUI(object sender, EventArgs args)
		{
			SemtrexUIController.IfNull(() =>
			{
				NewDocument("SemtrexUI.xml");
			});
		}

		protected void ShowVisualTree(object sender, EventArgs args)
		{
			VisualTreeController.IfNull(() => NewDocument("VisualTree.xml"));
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

		protected void CreateSymbolsRootNode()
		{
			SymbolEditorController.IfNotNull(ctrl => ctrl.View.Clear());
			schemaController = new GenericController<Schema>();
			Schema = (Schema)schemaController.Instance;

			SymbolEditorController.IfNotNull(ctrl =>
			{
				ctrl.View.AddNode(schemaController, null);
				PropertyGridController.IfNotNull(pgrid => pgrid.View.ShowObject(Schema));
			});
		}

		// Helper functions

		/// <summary>
		/// Called from MRU menu selection.
		/// </summary>
		public void LoadSymbols(string fn)
		{
			SchemaFilename = fn;
			LoadSymbols();
		}

		protected void LoadSymbols()
		{
			XmlSerializer xs = new XmlSerializer(typeof(Schema));
			StreamReader sr = new StreamReader(SchemaFilename);
			Schema = (Schema)xs.Deserialize(sr);
			sr.Close();
			SymbolEditorController.IfNotNull(ctrl => ctrl.PopulateTree());
			PropertyGridController.IfNotNull(pgrid => pgrid.View.ShowObject(Schema));
			MruMenu.AddFile(SchemaFilename);
		}

		protected void SaveSymbols()
		{
			XmlSerializer xs = new XmlSerializer(typeof(Schema));
			TextWriter tw = new StreamWriter(SchemaFilename);
			xs.Serialize(tw, Schema);
			tw.Close();
			MruMenu.AddFile(SchemaFilename);
		}

		// Ceptr menu events
/*
		protected void BuildSemanticTree(object sender, EventArgs args)
		{
			CeptrInterface.CreateStructureAndSymbolNodes();
			SemanticID floatID = CeptrInterface.GetFloat();
			SemanticID latitude = CeptrInterface.DeclareSymbol(CeptrInterface.RootSymbolsNode, floatID, "latitude");
			SemanticID longitude = CeptrInterface.DeclareSymbol(CeptrInterface.RootSymbolsNode, floatID, "longitude");

			// Structure latlong = _d_define_structure(structures,"latlong",RECEPTOR_CONTEXT, 2, lat, lon);
			SemanticID latlong = CeptrInterface.DefineStructure(CeptrInterface.RootStructuresNode, "latlong", new SemanticID[] { latitude, longitude });
			
			string retSymbols = CeptrInterface.DumpSymbols(CeptrInterface.RootSymbolsNode, CeptrInterface.RootStructuresNode);
			string retStructures = CeptrInterface.DumpStructures(CeptrInterface.RootSymbolsNode, CeptrInterface.RootStructuresNode);
		}
*/
		// Common helpers for symbol and structure controllers:

		// Recurse into a symbol, creating the child symbols first and then defining the structure composed of the child symbols.
		public SemanticID Recurse(Symbol symbol, Dictionary<string, SemanticID> structureMap, Dictionary<string, SemanticID> symbolMap)
		{
			List<SemanticID> syms = new List<SemanticID>();
			SemanticID structure = GetStructureID(symbol.Structure, structureMap);

			foreach (Symbol child in symbol.Symbols)
			{
				// We need to recurse to get to native types, from which more complex symbols are constructed.
				Recurse(child, structureMap, symbolMap);
				SemanticID uchild;

				if (!symbolMap.ContainsKey(child.Name))
				{
					SemanticID id = GetStructureID(child.Structure, structureMap);
					uchild = CeptrInterface.DeclareSymbol(CeptrInterface.RootSymbolsNode, id, child.Name);
					syms.Add(uchild);
					symbolMap[child.Name] = uchild;
				}
				else
				{
					// Re-use the symbol ID.
					uchild = symbolMap[child.Name];
					syms.Add(uchild);
				}
			}

			if (syms.Count > 0)
			{
				structure = CeptrInterface.DefineStructure(CeptrInterface.RootStructuresNode, symbol.Structure, syms.ToArray());
				structureMap[symbol.Structure] = structure;
			}
			return structure;
		}

		public SemanticID GetStructureID(string structure, Dictionary<string, SemanticID> structureMap)
		{
			SemanticID id;

			switch (structure.ToLower())
			{
				case "float":
					id = CeptrInterface.GetFloat();
					break;

				case "string":
					id = CeptrInterface.GetString();
					break;

				case "int":
				case "integer":
					id = CeptrInterface.GetInteger();
					break;

				case "list":
					id = CeptrInterface.GetList();
					break;

				default:
					structureMap.TryGetValue(structure, out id);
					break;
			}

			return id;
		}


		/// <summary>
		/// Format the dump so that () are indented and separated on new lines.
		/// </summary>
		public string FormatDump(string dump)
		{
			StringBuilder sb = new StringBuilder();
			int idx = 0;
			int indent = 0;

			// Brute force approach.
			while (idx < dump.Length)
			{
				char c = dump[idx];

				switch (c)
				{
					case '(':
						sb.Append("\r\n");
						sb.Append(new string(' ', indent));
						sb.Append(c);
						indent += 2;
						sb.Append("\r\n");
						sb.Append(new string(' ', indent));
						break;
					case ')':
						sb.Append("\r\n");
						indent -= 2;

						if (indent == 2)
						{
							sb.Append(new string(' ', indent));
							sb.Append(c);
							sb.Append("\r\n\r\n");
						}
						else
						{
							sb.Append(new string(' ', indent));
							sb.Append(c);
						}
						break;
					default:
						sb.Append(c);
						break;
				}

				++idx;
			}

			return sb.ToString();
		}

	}
}
