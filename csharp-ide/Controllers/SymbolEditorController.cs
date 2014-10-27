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
						// TODO: Yuck.  Clean this up so the model is king, and drives any controller, and remove the inc/dec from the respective view!
						ApplicationController.SymbolListController.
							IfNotNull(ctrl => ctrl.ReplaceSymbol(symbolName, newSymbolName)).
							Else(() => 
								{
									ApplicationModel.DecrementSymbolReference(symbolName);
									ApplicationModel.IncrementSymbolReference(newSymbolName);
								});
						symbolName = newSymbolName;
					}

					if (structureName != newStructureName)
					{
						ApplicationController.StructureListController.
							IfNotNull(ctrl => ctrl.ReplaceStructure(structureName, newStructureName)).
							Else(() =>
							{
								ApplicationModel.DecrementStructureReference(structureName);
								ApplicationModel.IncrementStructureReference(newStructureName);
							});
						structureName = newStructureName;
					}

					// If the symbol being selected already exists, copy over the current structure here as well.
					// TODO: What happens if the referencing structure changes?  We need to update all the types with that reference.
					// TODO: Verify that we're at a child node -- we can't have duplicate top level symbols.
					if (ApplicationModel.SymbolRefCount[newSymbolName] > 1)
					{
						CopySymbolStructure(currentNode, newSymbolName);
					}
				}
			}
		}

		// TODO: Is this a new structure, are we re-using an existing structure, and if so, what happens if the user changes this structure?
		// We then have the situation where the symbol-structure graph is different for two graphs of the same name.  Should the graph be read-only?
		/// <summary>
		/// Copy the symbol structure of a previously defined symbol to this node.
		/// </summary>
		protected void CopySymbolStructure(TreeNode node, string symbolName)
		{
			// Find an instance in the view model that actually has children.
			// TODO: This should be implemented in an actual view model rather than by scanning through the list view control!
			TreeNode sourceNode = FindDeclaringNode(View.TreeView.Nodes, symbolName);

			// If we have an actual structure.
			if (sourceNode != null)
			{
				// Set the structure name in the new node.  
				// TODO: Clean up all this casting stuff.
				((Symbol)((NodeInstance)node.Tag).Instance.Item).Structure = ((Symbol)((NodeInstance)sourceNode.Tag).Instance.Item).Structure;
				node.Text = ((IHasFullyQualifiedName)((Symbol)((NodeInstance)node.Tag).Instance.Item)).FullyQualfiedName;
				CopyNodes(node, sourceNode);
			}
			else
			{
				// No child nodes, just get an instance that defines the structure because the structure is a native type.
				sourceNode = FindStructure(View.TreeView.Nodes, symbolName);

				// Set the structure name in the new node.  
				// TODO: Clean up all this casting stuff.
				((Symbol)((NodeInstance)node.Tag).Instance.Item).Structure = ((Symbol)((NodeInstance)sourceNode.Tag).Instance.Item).Structure;
				node.Text = ((IHasFullyQualifiedName)((Symbol)((NodeInstance)node.Tag).Instance.Item)).FullyQualfiedName;
			}
		}

		/// <summary>
		/// Recursively copy the tree at src into dest.
		/// </summary>
		protected void CopyNodes(TreeNode dest, TreeNode src)
		{
			foreach (TreeNode childSrcNode in src.Nodes)
			{
				NodeInstance childInst = (NodeInstance)childSrcNode.Tag;
				Symbol childInstSymbol = (Symbol)childInst.Instance.Item;

				// Create a new controller.
				IXtreeNode controller = (IXtreeNode)Activator.CreateInstance(Type.GetType(childInst.NodeDef.TypeName), new object[] { false });
				// Create a new symbol.
				Symbol item = new Symbol() { Name = childInstSymbol.Name, Structure = childInstSymbol.Structure };
				controller.Item = item;

				// Add the new symbol to the destination symbol collection.
				((Symbol)((NodeInstance)dest.Tag).Instance.Item).Symbols.Add(item);

				TreeNode childDestNode = View.AddNode(controller, dest);

				// TODO: Yuck.  Clean this up so the model is king, and drives any controller, and remove the inc/dec from the respective view!
				ApplicationController.SymbolListController.IfNotNull(ctrl => ctrl.AddSymbol(item.Name)).Else(() => ApplicationModel.IncrementSymbolReference(item.Name));
				ApplicationController.StructureListController.IfNotNull(ctrl => ctrl.AddStructure(item.Structure)).Else(() => ApplicationModel.IncrementStructureReference(item.Structure));

				// Recurse.
				CopyNodes(childDestNode, childSrcNode);
			}
		}

		/// <summary>
		/// Recurse through the tree to find a declaring node (one with children) of the specified symbol.
		/// </summary>
		protected TreeNode FindDeclaringNode(TreeNodeCollection nodes, string symbol)
		{
			TreeNode ret = null;

			foreach (TreeNode node in nodes)
			{
				if ((node.Text.LeftOf(':').Trim() == symbol) && (node.Nodes.Count > 0))
				{
					ret = node;
					break;
				}
				else
				{
					ret = FindDeclaringNode(node.Nodes, symbol);

					if (ret != null)
					{
						break;
					}
				}
			}

			return ret;
		}

		/// <summary>
		/// Recurse through the tree to find a declaring node with an actual defined structure.
		/// </summary>
		protected TreeNode FindStructure(TreeNodeCollection nodes, string symbol)
		{
			TreeNode ret = null;

			foreach (TreeNode node in nodes)
			{
				// TODO: Change the literal string to a global const.
				if ((node.Text.LeftOf(':').Trim() == symbol) && (node.Text.RightOf(':').Trim() != "(undefined)"))
				{
					ret = node;
					break;
				}
				else
				{
					ret = FindStructure(node.Nodes, symbol);

					if (ret != null)
					{
						break;
					}
				}
			}

			return ret;
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

			ApplicationController.SymbolOutputController.IfNotNull(t => 
				{
					if (item is Symbol)
					{
						t.ShowSymbolDump((Symbol)item);
					}
				});

			ApplicationController.StructureOutputController.IfNotNull(t =>
			{
				if (item is Symbol)
				{
					t.ShowStructureDump((Symbol)item);
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
							// TODO: Yuck.  Clean this up so the model is king, and drives any controller, and remove the inc/dec from the respective view!
							ApplicationController.SymbolListController.IfNotNull(ctrl => ctrl.AddSymbol(item.Name)).Else(() => ApplicationModel.IncrementSymbolReference(item.Name));
							ApplicationController.StructureListController.IfNotNull(ctrl => ctrl.AddStructure(item.Structure)).Else(() => ApplicationModel.IncrementStructureReference(item.Structure));
						}
						else
						{
							if (!String.IsNullOrEmpty(item.Name))
							{
								tn.Text = item.Name;
							}
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
