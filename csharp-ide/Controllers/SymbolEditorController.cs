using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.IO;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using System.Xml.Serialization;

using csharp_ide.Controllers;
using csharp_ide.Models;
using csharp_ide.Views;

using Clifton.ApplicationStateManagement;
using Clifton.ExtensionMethods;
using Clifton.Tools.Data;
using Clifton.Tools.Strings.Extensions;
using Clifton.Windows.Forms.XmlTree;

using XTreeController;
using XTreeInterfaces;

namespace csharp_ide.Controllers
{
	public class SymbolEditorController : ViewController<SymbolEditorView>
	{
		protected TreeNode currentNode;

		public void UpdateView()
		{
			// PopulateTree();
		}

		public void UpdateNodeText(string text)
		{
			if (currentNode != null)
			{
				currentNode.Text = text;
			}
		}

		protected void NodeSelected(object sender, TreeViewEventArgs e)
		{
			ApplicationController.PropertyGridController.IfNotNull(t =>
			{
				currentNode = e.Node;
				t.ShowObject(((NodeInstance)e.Node.Tag).Instance.Item);
			});
		}

		protected void Opening()
		{
		}

		protected void Closing()
		{
		}
/*
		protected void OnNew(object sender, EventArgs e)
		{
			ApplicationController.SchemaFilename = null;
			// ClearAll();
			// pgProperties.SelectedObject = schemaDef;
			// sdTree.SelectedNode = sdTree.Nodes[0];
		}

		protected void OnOpen(object sender, EventArgs e)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			ofd.RestoreDirectory = true;
			ofd.CheckFileExists = true;
			ofd.Filter = "xml files (*.xml)|*.xml|All files (*.*)|*.*";
			ofd.Title = "Load Schema";
			DialogResult res = ofd.ShowDialog();

			if (res == DialogResult.OK)
			{
				ApplicationController.SchemaFilename = ofd.FileName;
				Load();
			}
		}

		protected void OnSave(object sender, EventArgs e)
		{
			if (String.IsNullOrEmpty(ApplicationController.SchemaFilename))
			{
				OnSaveAs(sender, e);
			}
			else
			{
				Save();
			}
		}

		protected void OnSaveAs(object sender, EventArgs e)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			sfd.OverwritePrompt = true;
			sfd.Filter = "xml files (*.xml)|*.xml|All files (*.*)|*.*";
			sfd.Title = "Save Schema";
			DialogResult res = sfd.ShowDialog();

			if (res == DialogResult.OK)
			{
				ApplicationController.SchemaFilename = sfd.FileName;
				Save();
			}
		}

		protected void Save()
		{
			XmlSerializer xs = new XmlSerializer(typeof(Schema));
			TextWriter tw = new StreamWriter(ApplicationController.SchemaFilename);
			xs.Serialize(tw, ApplicationController.Schema);
			tw.Close();
		}

		protected void Load()
		{
			ApplicationController.LoadSchema();
			PopulateTree();
		}

		protected void RecurseCollection(NodeDef node, dynamic collection, TreeNode tnCurrent)
		{
			if (node.Nodes.Count > 0)
			{
				// Collection is a Dictionary<string, dynamic> where dynamic is a List<T>
				// obj is a KeyValuePair<string, dynamic>
				foreach (var kvp in collection.Collection)
				{
					string collectionName = kvp.Key;
					var collectionItems = kvp.Value;
					// Doesn't matter what nodeDef we find, this is only to get the TypeName and number of child nodes on recursion.
					// But it does allow us to separate the serialization order from the tree definition order.
					List<NodeDef> matchingNodes = node.Nodes.FindAll(t => t.TypeName.Contains(collectionName));
					NodeDef nodeDef = node.Nodes.Find(t => t.TypeName.Contains(collectionName));

					foreach (var item in collectionItems)
					{
						// Do not create new instances for the items, as they have already been created!
						IXtreeNode controller = (IXtreeNode)Activator.CreateInstance(Type.GetType(nodeDef.TypeName), new object[] { false });
						controller.Item = item;
						TreeNode tn = View.AddNode(controller, tnCurrent);
						string name = ((IHasCollection)item).Name;
						tn.Text = (String.IsNullOrWhiteSpace(name) ? tn.Text : name);
						RecurseCollection(nodeDef, item, tn);
					}
				}
			}
		}

		protected void PopulateTree()
		{
			// TODO: For some reason, suspending / resuming layout stops the Dock mode "Fill" from working correctly.
			// To replicate, uncomment the suspend/resume lines, load a schema, then increase the size of the pane.  The tree view
			// control's size will no longer automatically adjust.
			// View.SuspendLayout();
			// Remove all existing (such as required) nodes.
			View.TreeView.Nodes[0].Nodes.Clear();
			NodeDef nodeDef = View.TreeView.RootNode.Nodes[0];		// Get the child of the top level node.
			RecurseCollection(nodeDef, ApplicationController.Schema, View.TreeView.Nodes[0]);

			View.TreeView.CollapseAll();
			View.TreeView.Nodes[0].Expand();
			// View.TreeView.ResumeLayout();
			currentNode = View.TreeView.Nodes[0];

			ApplicationController.PropertyGridController.IfNotNull(t =>
			{
				t.ShowObject(((NodeInstance)currentNode.Tag).Instance.Item);
			});

			View.TreeView.SelectedNode = currentNode;
		}
 */
	}
}
