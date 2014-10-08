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
using Clifton.Windows.Forms;
using Clifton.Windows.Forms.XmlTree;

using XTreeController;
using XTreeInterfaces;

namespace csharp_ide.Controllers
{
	public class SymbolEditorController : ViewController<SymbolEditorView>
	{
		protected TreeNode currentNode;
		protected string symbolName;
		protected string structureName;

		public void UpdateView()
		{
			PopulateTree();
		}

		public void UpdateNodeText(string text)
		{
			if (currentNode != null)
			{
				object item = ((NodeInstance)currentNode.Tag).Instance.Item;

				if (item is IHasFullyQualifiedName)
				{
					currentNode.Text = ((IHasFullyQualifiedName)item).FullyQualfiedName;
				}
				else
				{
					currentNode.Text = text;
				}

				if (item is Symbol)
				{
					string newSymbolName = ((Symbol)item).Name;
					string newStructureName = ((Symbol)item).Structure;

					// Update the symbol list if it has changed.
					if (symbolName != newSymbolName)
					{
						ApplicationController.SymbolListController.IfNotNull(ctrl => ctrl.ReplaceSymbol(symbolName, newSymbolName));
						symbolName = newSymbolName;
					}

					if (structureName != newStructureName)
					{
						ApplicationController.StructureListController.IfNotNull(ctrl => ctrl.ReplaceStructure(structureName, newStructureName));
						structureName = newStructureName;
					}
				}
			}
		}

		public void PopulateTree()
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

		protected void NodeSelected(object sender, TreeViewEventArgs e)
		{
			currentNode = e.Node;
			object item = ((NodeInstance)e.Node.Tag).Instance.Item;

			ApplicationController.PropertyGridController.IfNotNull(t =>
			{
				t.ShowObject(item);

				if (item is Symbol)
				{
					// Save the current symbol and structure name so that, if it changes, we can update the symbol and structure lists.
					symbolName = ((Symbol)item).Name;
					structureName = ((Symbol)item).Structure;
				}
			});

			ApplicationController.DumpOutputController.IfNotNull(t => 
				{
					if (item is Symbol)
					{
						t.ShowSymbolDump((Symbol)item);
					}
				});
		}

		protected void DeletingNode(object sender, DeletingNodeEventArgs args)
		{
			object item = args.InstanceNode.Item;

			if (item is Symbol)
			{
				symbolName = ((Symbol)item).Name;
				structureName = ((Symbol)item).Structure;
				ApplicationController.SymbolListController.IfNotNull(ctrl => ctrl.RemoveSymbol(symbolName));
				ApplicationController.StructureListController.IfNotNull(ctrl => ctrl.RemoveStructure(structureName));
			}
		}

		protected void Opening()
		{
		}

		protected void Closing()
		{
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

						// If the item is a symbol, then we want to also show in the symbol and structure lists the associated symbol and structure!
						if (item is Symbol)
						{
							ApplicationController.SymbolListController.IfNotNull(ctrl => ctrl.AddSymbol(item.Name));
							ApplicationController.StructureListController.IfNotNull(ctrl => ctrl.AddStructure(item.Structure));
						}
						// string name = ((IHasCollection)item).Name;
						// tn.Text = (String.IsNullOrWhiteSpace(name) ? tn.Text : name);
						RecurseCollection(nodeDef, item, tn);
					}
				}
			}
		}
	}
}
