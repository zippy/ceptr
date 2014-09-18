/*
Copyright (c) 2006, Marc Clifton
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list
  of conditions and the following disclaimer. 

* Redistributions in binary form must reproduce the above copyright notice, this 
  list of conditions and the following disclaimer in the documentation and/or other
  materials provided with the distribution. 
 
* Neither the name Marc Clifton nor the names of contributors may be
  used to endorse or promote products derived from this software without specific
  prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Reflection;
using System.Resources;
using System.Text;					// -- used by drag & drop stuff.
using System.Windows.Forms;
using System.Xml;

using Clifton.MycroParser;
using Clifton.Windows.Forms.XmlTree;

namespace Clifton.Windows.Forms
{
	public static class Helper
	{
		/// <summary>
		/// The main purpose of this method is to support legacy apps that have put their icon resources
		/// into XTreeResources assembly.
		/// </summary>
		/// <param name="treeDefinitionResourceName"></param>
		/// <param name="assyName"></param>
		/// <param name="className"></param>
		/// <param name="resName"></param>
		public static void GetResourceInfo(string treeDefinitionResourceName, out string assyName, out string className, out string resName)
		{
			string[] res = treeDefinitionResourceName.Split('.');
			assyName = String.Empty;
			className = String.Empty;
			resName = String.Empty;

			switch (res.Length)
			{
				case 1:
					assyName = "XTreeResources";
					className = "ApplicationIcons";
					resName = res[0];
					break;

				case 3:
					assyName = res[0];
					className = res[1];
					resName = res[2];
					break;

				case 4:
					assyName = res[0] + "." + res[1];
					className = res[2];
					resName = res[3];
					break;
			}

		}
	}

	/// <summary>
	/// Used to qualify identical controller instances by varying the data value.
	/// </summary>
	public struct ControllerInfo
	{
		public Type type;
		public string data;

		public ControllerInfo(Type type, string data)
		{
			this.type = type;
			this.data = data;
		}
	}

	public class GTreeEventArgs : EventArgs
	{
		protected bool handled;

		public bool Handled
		{
			get { return handled; }
			set { handled = value; }
		}

		public GTreeEventArgs()
		{
		}
	}

	public class NewNodeEventArgs
	{
		protected TreeNode treeNode;
		protected IXtreeNode instanceNode;
		protected NodeDef nodeDef;
		
		public NodeDef NodeDef
		{
			get { return nodeDef; }
		}
		
		public IXtreeNode InstanceNode
		{
			get { return instanceNode; }
		}

		public TreeNode TreeNode
		{
			get { return treeNode; }
		}
		
		public NewNodeEventArgs(IXtreeNode instanceNode, NodeDef nodeDef, TreeNode treeNode)
		{
			this.instanceNode = instanceNode;
			this.nodeDef = nodeDef;
			this.treeNode = treeNode;
		}
	}

	public class CustomPopupEventArgs : EventArgs
	{
		protected NodeInstance node;
		protected string tag;

		public NodeInstance Node
		{
			get { return node; }
		}

		public string Tag
		{
			get { return tag; }
		}

		public CustomPopupEventArgs(NodeInstance node, string tag)
		{
			this.node = node;
			this.tag = tag;
		}
	}

	/// <summary>
	/// Can be used by controller nodes to indicate that one type of node can be
	/// a child of the other type of node, even when the controllers are different types.
	/// </summary>
	public class CommonNodeAttribute : Attribute
	{
		protected string commonId;
		
		/// <summary>
		/// Returns commonId
		/// </summary>
		public string CommonId
		{
			get { return commonId; }
		}

		public CommonNodeAttribute(string commonId)
		{
			this.commonId = commonId;
		}
	}

	// Was derived from MultiSelectTreeView
	public class XTree : TreeView, ISupportInitialize
	{
		public delegate void NewNodeDlgt(object sender, NewNodeEventArgs args);
		public delegate void CustomPopupCommandDlgt(object sender, CustomPopupEventArgs args);

		public event EventHandler DisplayContextMenu;
		public event NewNodeDlgt NewNode;
		public event CustomPopupCommandDlgt CustomPopupCommand;
		public event EventHandler TreeDoubleClick;

		protected RootNode rootNode;
		protected Point mousePos;
		protected TreeNode selNode;
		protected string treeDefFileName;
		protected IXtreeNode selectedNodeInstance;

		protected Dictionary<string, NodeDef> nodeList;
		protected Dictionary<ControllerInfo, NodeDef> controllerToNodeDefMap;
		protected Dictionary<string, NodeDef> nameToNodeDefMap;

		protected string treeDefinitionResourceName;

		protected ToolStripMenuItem contextMenu;

		public ToolStripMenuItem XTreeContextMenu
		{
			get { return contextMenu; }
			set { contextMenu = value; }
		}
		
		/// <summary>
		/// Gets/sets treeDefinitionResourceName
		/// </summary>
		public string TreeDefinitionResourceName
		{
			get { return treeDefinitionResourceName; }
			set { treeDefinitionResourceName = value; }
		}

		public Point MousePos
		{
			get { return mousePos; }
			set { mousePos = value; }
		}							

		public string TreeDefinitionFileName
		{
			get { return treeDefFileName; }
			set { treeDefFileName = value; }
		}

		public RootNode RootNode
		{
			get { return rootNode; }
		}

		public XTree()
		{
		}

		public virtual void BeginInit()
		{
		}

		public virtual void EndInit()
		{
			if (treeDefFileName != null)
			{
				XmlDocument xdoc = new XmlDocument();
				xdoc.Load(treeDefFileName);
				Initialize(xdoc);
			}
			else if (treeDefinitionResourceName != null)
			{
				System.Diagnostics.Debug.WriteLine("TreeDefinitionResourceName==" + treeDefinitionResourceName);
				string assyName;
				string className;
				string resName;
				Helper.GetResourceInfo(treeDefinitionResourceName, out assyName, out className, out resName);
				Assembly resAssy = Assembly.Load(assyName);
				ResourceManager rm = new ResourceManager(assyName+"."+className, resAssy);
				string xml = (string)rm.GetObject(resName);
				//System.Diagnostics.Debug.WriteLine(xml);
				XmlDocument xdoc = new XmlDocument();
				xdoc.LoadXml(xml);
				Initialize(xdoc);
			}

			AllowDrop = true;

			DragOver += new DragEventHandler(OnXTreeDragOver);
			DragEnter += new DragEventHandler(OnXTreeDragEnter);
			ItemDrag += new ItemDragEventHandler(OnXTreeItemDrag);
			DragDrop += new DragEventHandler(OnXTreeDragDrop);
			AfterSelect += new TreeViewEventHandler(OnXTreeAfterSelect);
		}

		public virtual void Clear()
		{
			Nodes.Clear();
		}

		public void Initialize(XmlDocument xdoc)
		{
			nodeList = new Dictionary<string, NodeDef>();
			controllerToNodeDefMap = new Dictionary<ControllerInfo, NodeDef>();
			nameToNodeDefMap = new Dictionary<string, NodeDef>();
//			LabelEdit = true;
			ImageList = new ImageList();
			Clifton.MycroParser.MycroParser mp = new Clifton.MycroParser.MycroParser();
			mp.Load(xdoc, null, null);
			// Type t = typeof(RootNode);
			// string qname = t.AssemblyQualifiedName;
			// TODO: Fix this by getting the correct qname (minus the ".RootNode") from the above commented-out code.
			mp.NamespaceMap[""] = "Clifton.Windows.Forms.XmlTree, Clifton.Windows.Forms";
			// object ret = mp.Process();
			// MessageBox.Show(ret.GetType().AssemblyQualifiedName+"\r\n"+typeof(RootNode).AssemblyQualifiedName);
			rootNode=(RootNode)mp.Process();
			nodeList[rootNode.Name] = rootNode;
			BuildFlatNodeList(rootNode);
//			TreeNode tn=CreateNodeAndRequiredChildren(rootNode);
//			Nodes.Add(tn);
		}

		public TreeNode AddNode(IXtreeNode inst, TreeNode parent)
		{
			NodeDef nodeDef = null;
			// System.Diagnostics.Debug.WriteLine("Searching for TypeName: "+inst.GetType().AssemblyQualifiedName);
			// System.Diagnostics.Debug.WriteLine("controllerToNodeDefMap.Count==" + controllerToNodeDefMap.Count);
			ControllerInfo ci = new ControllerInfo(inst.GetType(), inst.TypeData);
			bool found = controllerToNodeDefMap.TryGetValue(ci, out nodeDef);

			if (!found)
			{
				throw new ApplicationException("The controller instance "+inst.GetType().AssemblyQualifiedName+" is expected but not defined in a TypeName attribute of the tree schema.");
			}

			TreeNode tn = CreateNodeAndRequiredChildren(nodeDef, inst);

			if (parent != null)
			{
				parent.Nodes.Add(tn);
			}
			else
			{
				Nodes.Add(tn);
			}

			// Fire event that a node has been created.
			OnNewNode(new NewNodeEventArgs(inst, nodeDef, tn));

			return tn;
		}

		public TreeNode CreateNode(NodeDef nodeDef, IXtreeNode inst)
		{
			TreeNode tn = new TreeNode();
			tn.Text = nodeDef.Text;

			// If no controller is specified, use the text in the NodeDef.
			if (!(inst is PlaceholderInstance))
			{
				// Otherwise, get the text from the controller...
				if (inst.Name != null)
				{
					// if it's not null!
					tn.Text = inst.Name;
				}
			}

			tn.Tag = new NodeInstance(tn, nodeDef, inst);
			UpdateImageList(tn, nodeDef, inst);

			return tn;
		}

		public TreeNode CreateNodeAndRequiredChildren(NodeDef nodeDef, IXtreeNode parentInst)
		{
			TreeNode tn = CreateNode(nodeDef, parentInst);
			UpdateImageList(tn, nodeDef, parentInst);

			foreach (NodeDef child in nodeDef.Nodes)
			{
				// If the current node is recursive, then ignore nodes from the parent
				// that are NOT recursive.  In other words, the current node should only
				// select itself.  This prevents required nodes of the parent from being added.
				if ((nodeDef.Recurse) && (!child.Recurse))
				{
					continue;
				}

				if (child.IsRequired)
				{
					IXtreeNode inst = child.CreateImplementingType(parentInst);
					// Set the qualifier to distinguish a generic controller with a concrete type that it manages.
					inst.TypeData = child.TypeData;
					TreeNode tnChild = CreateNodeAndRequiredChildren(child, inst);
					bool success = inst.AddNode(parentInst, null);
					OnNewNode(new NewNodeEventArgs(inst, nodeDef, tnChild));
					tn.Nodes.Add(tnChild);
					tn.Expand();
				}
			}

			return tn;
		}

        protected void BuildChildPopups(NodeMenuItem nmi, Popup popup, TreeNode tn, NodeDef n, NodeDef refNode)
        {
            foreach (Popup childPopup in popup.PopupItems)
            {
                NodeMenuItem nmiChild = new NodeMenuItem(childPopup.Text, tn, n, refNode, childPopup);

                if (childPopup.PopupItems.Count == 0)
                {
                    nmiChild.Click += new EventHandler(OnContextItem);
                    nmiChild.Enabled = ((NodeInstance)tn.Tag).Instance.IsEnabled(childPopup.Tag, childPopup.Enabled);
                }

                BuildChildPopups(nmiChild, childPopup, tn, n, refNode);
                nmi.MenuItems.Add(nmiChild);
            }
        }

		public ContextMenu BuildContextMenu(TreeNode tn, NodeDef n)
		{
			ContextMenu cm = new ContextMenu();
			bool first = true;

			// Populate from this node's popup collection.
			if (n.PopupItems.Count != 0)
			{
				foreach (Popup popup in n.PopupItems)
				{
					NodeMenuItem nmi = new NodeMenuItem(popup.Text, tn, n, null, popup);

                    if (popup.PopupItems.Count == 0)
                    {
                        nmi.Click += new EventHandler(OnContextItem);
                        nmi.Enabled = ((NodeInstance)tn.Tag).Instance.IsEnabled(popup.Tag, popup.Enabled);
                    }

                    BuildChildPopups(nmi, popup, tn, n, null);
					cm.MenuItems.Add(nmi);
				}

				first = false;
			}

			// For each child node, populate from the child's ParentPopupItems collection.
			foreach (NodeDef child in n.Nodes)
			{
				NodeDef refNode = child;

				// Resolve any referenced node.
				if (child.IsRef)
				{
					if (!nodeList.ContainsKey(child.RefName))
					{
						throw new ApplicationException("referenced node does not exist.");
					}

					refNode = nodeList[child.RefName];
				}

				if (refNode.ParentPopupItems.Count != 0)
				{
					if (!first)
					{
						if (child.Separator)
						{
							cm.MenuItems.Add(new NodeMenuItem("-", tn, n, null, null));
						}
					}

					first = false;

					// Populate the items.
					foreach (Popup popup in refNode.ParentPopupItems)
					{

                        NodeMenuItem nmi = new NodeMenuItem(popup.Text, tn, n, refNode, popup);

                        if (popup.PopupItems.Count == 0)
                        {
                            nmi.Click += new EventHandler(OnContextItem);
                            nmi.Enabled = ((NodeInstance)tn.Tag).Instance.IsEnabled(popup.Tag, popup.Enabled);
                        }

                        BuildChildPopups(nmi, popup, tn, n, refNode);
                        cm.MenuItems.Add(nmi);

                        //NodeMenuItem nmi = new NodeMenuItem(popup.Text, tn, n, refNode, popup);
                        //nmi.Click += new EventHandler(OnContextItem);
                        //nmi.Enabled = ((NodeInstance)tn.Tag).Instance.IsEnabled(popup.Tag, popup.Enabled);
                        //cm.MenuItems.Add(nmi);
					}
				}
			}

			// Add custom items.
			NodeMenuItem spacer = new NodeMenuItem("-", tn, n, null, null);
			cm.MenuItems.Add(spacer);
			NodeMenuItem collapse = new NodeMenuItem("Collapse", tn, n, null, null);
			collapse.Click += new EventHandler(OnCollapse);
			cm.MenuItems.Add(collapse);
			NodeMenuItem expand = new NodeMenuItem("Expand", tn, n, null, null);
			expand.Click += new EventHandler(OnExpand);
			cm.MenuItems.Add(expand);
			NodeMenuItem expandAll = new NodeMenuItem("Expand All", tn, n, null, null);
			expandAll.Click += new EventHandler(OnExpandAll);
			cm.MenuItems.Add(expandAll);

			return cm;														  
		}

		/// <summary>
		/// Show everyting in the popup menu as children to the specified menu item.
		/// </summary>
		/// <param name="cm"></param>
		protected void ShowPopupMenuInMenuBar(ContextMenu cm, string nodeName)
		{
			if (contextMenu != null)
			{
				contextMenu.DropDownItems.Clear();
				contextMenu.Text = "&"+nodeName;

				foreach (NodeMenuItem mi in cm.MenuItems)
				{
					if (mi.Text == "-")
					{
						contextMenu.DropDownItems.Add(new ToolStripSeparator());
					}
					else
					{
						ToolStripMenuItem tsmi = new ToolStripMenuItem("&"+mi.Text);
						tsmi.Tag = mi;
						tsmi.Click += new EventHandler(OnContextMenuClick);
						contextMenu.DropDownItems.Add(tsmi);
					}
				}
			}
		}

		protected void OnContextMenuClick(object sender, EventArgs e)
		{
			ToolStripMenuItem tsmi = (ToolStripMenuItem)sender;
			NodeMenuItem nmi = (NodeMenuItem)tsmi.Tag;
			nmi.RaiseClick();			
		}

		protected void OnCollapse(object sender, EventArgs e)
		{
			SelectedNode.Collapse();
		}

		protected void OnExpand(object sender, EventArgs e)
		{
			SelectedNode.Expand();
		}

		protected void OnExpandAll(object sender, EventArgs e)
		{
			SelectedNode.ExpandAll();
		}

		public void Serialize(string fn)
		{
			XmlDocument xdoc = new XmlDocument();
			XmlDeclaration xmlDeclaration = xdoc.CreateXmlDeclaration("1.0", "utf-8", null);
			xdoc.InsertBefore(xmlDeclaration, xdoc.DocumentElement);
			XmlNode xnode = xdoc.CreateElement("XTree");
			xdoc.AppendChild(xnode);

			foreach (TreeNode tn in Nodes)
			{
				WriteNode(xdoc, xnode, tn);
			}

			xdoc.Save("tree.xml");

		}

		public void Deserialize(string fn)
		{
			Nodes.Clear();
			XmlDocument xdoc = new XmlDocument();
			xdoc.Load("tree.xml");
			XmlNode node = xdoc.DocumentElement;
			ReadNode(xdoc, node, Nodes, null);
		}

		protected void OnXTreeAfterSelect(object sender, TreeViewEventArgs e)
		{
			// Update the context menu in the menubar.
			NodeDef nodeDef = ((NodeInstance)SelectedNode.Tag).NodeDef;
			ContextMenu cm = BuildContextMenu(SelectedNode, nodeDef);
			ShowPopupMenuInMenuBar(cm, nodeDef.Name);

			((NodeInstance)SelectedNode.Tag).Instance.Select(SelectedNode);
		}

		protected void UpdateImageList(TreeNode tn, NodeDef nodeDef, IXtreeNode inst)
		{
			if (nodeDef.ImageList != null)
			{
				// Have images been loaded into the TreeView's image list?
				if (nodeDef.ImageOffset == -1)
				{
					// No.  Load them and save the offset.
					nodeDef.ImageOffset = ImageList.Images.Count;

					foreach (Image img in nodeDef.ImageList.Images)
					{
						ImageList.Images.Add(img);
					}

				}

				// Yes, just update the indices.
				tn.ImageIndex = nodeDef.ImageOffset + inst.IconIndex;
				tn.SelectedImageIndex = nodeDef.ImageOffset + inst.SelectedIconIndex;
			}
		}

		protected void ReadNode(XmlDocument xdoc, XmlNode node, TreeNodeCollection nodes, IXtreeNode parent)
		{
			foreach (XmlNode xn in node.ChildNodes)
			{
				IXtreeNode inst = nodeList[xn.Name].CreateImplementingType(parent);
				// Set the qualifier to distinguish a generic controller with a concrete type that it manages.
				inst.TypeData = nodeList[xn.Name].TypeData;
				TreeNode tn = CreateNode(nodeList[xn.Name], inst);
				nodes.Add(tn);
				
				ReadNode(xdoc, xn, tn.Nodes, inst);

				if (Convert.ToBoolean(xn.Attributes["IsExpanded"].Value))
				{
					tn.Expand();
				}
			}
		}

		protected void WriteNode(XmlDocument xdoc, XmlNode xnode, TreeNode tn)
		{
			XmlNode xn = xdoc.CreateElement(((NodeInstance)tn.Tag).NodeDef.Name);
			xn.Attributes.Append(xdoc.CreateAttribute("Text"));
			xn.Attributes.Append(xdoc.CreateAttribute("IsExpanded"));
			xn.Attributes["Text"].Value = tn.Text;
			xn.Attributes["IsExpanded"].Value = tn.IsExpanded.ToString();
			xnode.AppendChild(xn);

			foreach (TreeNode child in tn.Nodes)
			{
				WriteNode(xdoc, xn, child);
			}
		}

		protected void BuildFlatNodeList(NodeDef node)
		{
			foreach (NodeDef child in node.Nodes)
			{
				if (child.Name == null)
				{
					throw new ApplicationException("NodeDef Name cannot be null.");
				}

				// System.Diagnostics.Debug.WriteLine("Child='" + child.Text + "'");
				// System.Diagnostics.Debug.WriteLine("Adding controller node map:" + (child.ImplementingType==null ? "null" : child.ImplementingType.ToString()) + ", " + (child.TypeData==null ? "null" : child.TypeData.ToString()));
				nodeList[child.Name] = child;
				controllerToNodeDefMap[new ControllerInfo(child.ImplementingType, child.TypeData)] = child;
				nameToNodeDefMap[child.Name] = child;

				if (!child.Recurse)
				{
					BuildFlatNodeList(child);
				}
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);

			if (e.KeyCode == Keys.F2)
			{
				if (!((NodeInstance)SelectedNode.Tag).NodeDef.IsReadOnly)
				{
					if (!SelectedNode.IsEditing)
					{
						LabelEdit = true;
						SelectedNode.BeginEdit();
					}
				}
			}
		}

		/// <summary>
		/// Handles clicking on a node and either editing the label (double click) or
		/// calling the controller's Select method.
		/// </summary>
		/// <param name="e"></param>
		protected override void OnMouseDown(MouseEventArgs e)
		{
			base.OnMouseDown(e);
			selNode = GetNodeAt(e.Location);

			if (selNode != null)
			{
				if (e.Button == MouseButtons.Left)
				{
					if (e.Clicks == 2)
					{
						if (TreeDoubleClick != null)
						{
							TreeDoubleClick(this, EventArgs.Empty);
						}
					}

					// clicked twice on the same node?
					if (selNode == SelectedNode)
					{
						// Is writeable?
						if (!((NodeInstance)selNode.Tag).NodeDef.IsReadOnly)
						{
							// Not currently editing?
							if (!selNode.IsEditing)
							{
								// Then begin edit of the label.
								LabelEdit = true;
								selNode.BeginEdit();
							}
						}
						else
						{
							// Re-select the same node, in case we're binding to a property grid that has
							// displayed some other information, we now want to update any dependent controls.
							((NodeInstance)selNode.Tag).Instance.Select(selNode);
						}
					}
					else
					{
						// Clicked on a different node.  Select it and call
						// the controller's Select method.
						SelectedNode = selNode;
						((NodeInstance)selNode.Tag).Instance.Select(selNode);
					}
				}
				else
				{
					// Other mouse button.  Still select the node.
					SelectedNode = selNode;
					((NodeInstance)selNode.Tag).Instance.Select(selNode);
				}

				// Get the backing node instance of the selected node.
				selectedNodeInstance = ((NodeInstance)SelectedNode.Tag).Instance;
			}
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			base.OnMouseUp(e);

			if (e.Button == MouseButtons.Right)
			{
				mousePos = new Point(e.X, e.Y);
				ContextMenu cm = OnDisplayContextMenu();
			}
		}

		protected override void OnAfterLabelEdit(NodeLabelEditEventArgs e)
		{
			if (e.Label != null)
			{
				base.OnAfterLabelEdit(e);
				LabelEdit = false;
				// Update the text for the label in the associated controller.
				((IXtreeNode)((NodeInstance)e.Node.Tag).Instance).Name = e.Label;
			}
		}

		protected virtual ContextMenu OnDisplayContextMenu()
		{
			GTreeEventArgs ea = new GTreeEventArgs();
			ContextMenu cm = null;
			
			if (DisplayContextMenu != null)
			{
				DisplayContextMenu(this, ea);
			}

			if (!ea.Handled)
			{
				TreeNode tn = GetNodeAt(mousePos);
				NodeDef nodeDef;

				if (tn == null)
				{
					nodeDef = rootNode;
				}
				else
				{
					nodeDef = ((NodeInstance)tn.Tag).NodeDef;
				}

				cm=BuildContextMenu(tn, nodeDef);
				cm.Show(this, mousePos);

			}

			return cm;
		}

		private void OnContextItem(object sender, EventArgs e)
		{
			NodeMenuItem nmi = (NodeMenuItem)sender;

			if (nmi.PopupInfo.IsAdd)
			{
				IXtreeNode parentInst = null;

				if (nmi.TreeNode != null)
				{
					parentInst = ((NodeInstance)nmi.TreeNode.Tag).Instance;
				}

				IXtreeNode inst = nmi.ChildNode.CreateImplementingType(parentInst);
				// Set the qualifier to distinguish a generic controller with a concrete type that it manages.
				inst.TypeData = nmi.ChildNode.TypeData;
				bool success = inst.AddNode(parentInst, nmi.PopupInfo.Tag);

				if (success)
				{
					TreeNode tn = CreateNodeAndRequiredChildren(nmi.ChildNode, inst);
					OnNewNode(new NewNodeEventArgs(inst, nmi.ChildNode, tn));

					// This wouldn't be necessary if there was something like an ITreeNode interface
					// which both TreeView and TreeNode implemented to give you access to the Nodes collection!
					if (nmi.TreeNode == null)
					{
						Nodes.Add(tn);
					}
					else
					{
						nmi.TreeNode.Nodes.Add(tn);
						tn.Parent.Expand();
					}

					tn.TreeView.SelectedNode = tn;
					inst.Select(tn);
				}
			}
			else if (nmi.PopupInfo.IsRemove)
			{
				NodeInstance ni = (NodeInstance)nmi.TreeNode.Tag;
				IXtreeNode inst = ni.Instance;
				IXtreeNode parentInst = null;
				TreeNode parentNode = nmi.TreeNode.Parent;

				if (parentNode != null)
				{
					parentInst = ((NodeInstance)parentNode.Tag).Instance;
				}

				bool success = inst.DeleteNode(parentInst);

				if (success)
				{
					nmi.TreeNode.Remove();
				}
			}
			else
			{
				// if not add or remove, raise an event to hangle the custom popup command.
				NodeInstance ni = (NodeInstance)nmi.TreeNode.Tag;
				RaiseCustomPopupCommand(ni, nmi.PopupInfo.Tag);
			}
		}

		protected virtual void RaiseCustomPopupCommand(NodeInstance ni, string tag)
		{
			if (CustomPopupCommand != null)
			{
				CustomPopupEventArgs args = new CustomPopupEventArgs(ni, tag);
				CustomPopupCommand(this, args);
			}
		}
		
		// ------------ Drag & Drop Events --------------

		// Modified from Gabe Anguiano's work here: http://www.codeproject.com/cs/miscctrl/TreeViewReArr.asp

		private string NodeMap;
		private const int MAPSIZE = 128;
		private StringBuilder NewNodeMap = new StringBuilder(MAPSIZE);

		private void OnXTreeItemDrag(object sender, System.Windows.Forms.ItemDragEventArgs e)
		{
			DataObject dobj = new DataObject("TableDef", e.Item.ToString());
			DoDragDrop(dobj, DragDropEffects.Copy);
			DoDragDrop(e.Item, DragDropEffects.Move);
		}

		private void OnXTreeDragEnter(object sender, System.Windows.Forms.DragEventArgs e)
		{
			e.Effect = DragDropEffects.Move;
		}
		private void OnXTreeDragDrop(object sender, System.Windows.Forms.DragEventArgs e)
		{
			if (e.Data.GetDataPresent("System.Windows.Forms.TreeNode", false) && this.NodeMap != "")
			{
				TreeNode MovingNode = (TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode");
				string[] NodeIndexes = this.NodeMap.Split('|');
				TreeNodeCollection InsertCollection = Nodes;
				TreeNode newParent=null;

				for (int i = 0; i < NodeIndexes.Length - 1; i++)
				{
					newParent=InsertCollection[Int32.Parse(NodeIndexes[i])];
					InsertCollection = newParent.Nodes;
				}

				if (InsertCollection != null)
				{
					try
					{
						int idx = Int32.Parse(NodeIndexes[NodeIndexes.Length - 1]);

						// Get the node controller for the moving node.
						NodeInstance ni = (NodeInstance)MovingNode.Tag;
						IXtreeNode inst = ni.Instance;
						// Get the node controller for the new parent.
						NodeInstance parent = (NodeInstance)newParent.Tag;
						IXtreeNode parentInst = parent.Instance;

						if (MovingNode.Parent != null)
						{
							IXtreeNode movingNodeParentInst = ((NodeInstance)MovingNode.Parent.Tag).Instance;
							// Can only move to another parent of the same type.
							if ( (parentInst.GetType() == movingNodeParentInst.GetType()) ||
								(EquivalentNodes(parentInst, movingNodeParentInst)) )
							{
								inst.MoveTo(parentInst, movingNodeParentInst, idx, MovingNode);
								InsertCollection.Insert(idx, (TreeNode)MovingNode.Clone());
								SelectedNode = InsertCollection[idx];
								MovingNode.Remove();
							}
						}
						else
						{
							// Remove markers.
							Refresh();
						}
					}
					catch (Exception ex)
					{
						MessageBox.Show(ex.Message);
					}
				}
			}
		}

		protected bool EquivalentNodes(object inst1, object inst2)
		{
			Type t1 = inst1.GetType();
			Type t2 = inst1.GetType();
			string id1 = GetID(t1);
			string id2 = GetID(t2);
			bool ret = (id1 != String.Empty) && (id1 == id2);

			return ret;
		}

		protected string GetID(Type t)
		{
			string id = String.Empty;
			object[] attr = t.GetCustomAttributes(typeof(CommonNodeAttribute), false);

			if (attr.Length==1)
			{
				id = ((CommonNodeAttribute)attr[0]).CommonId;
			}

			return id;
		}

		private void OnXTreeDragOver(object sender, System.Windows.Forms.DragEventArgs e)
		{
			TreeNode NodeOver = GetNodeAt(PointToClient(Cursor.Position));
			TreeNode NodeMoving = (TreeNode)e.Data.GetData("System.Windows.Forms.TreeNode");


			// A bit long, but to summarize, process the following code only if the nodeover is null
			// and either the nodeover is not the same thing as nodemoving UNLESSS nodeover happens
			// to be the last node in the branch (so we can allow drag & drop below a parent branch)
			if (NodeOver != null && (NodeOver != NodeMoving || (NodeOver.Parent != null && NodeOver.Index == (NodeOver.Parent.Nodes.Count - 1))))
			{
				int OffsetY = PointToClient(Cursor.Position).Y - NodeOver.Bounds.Top;
//				int NodeOverImageWidth = ImageList.Images[NodeOver.ImageIndex].Size.Width + 8;
				Graphics g = CreateGraphics();

				// Image index of 1 is the non-folder icon
//				if (NodeOver.ImageIndex == 1)
				// MTC
				// If there are subnodes already, or we're at the last node, then draw the "before/after" markers.
				// What this prevents happening is the ability to add subnodes to a last node that has no children.
				// So we need to add an AND expression, that if we're at the last node AND the mouse Y is at the top third or bottom third of the node,
				// THEN we want to treat this as a "before/after" selection.  Otherwise, treat it as adding a child node.
				if ( (NodeOver.Nodes.Count > 0) || 
					( (NodeOver.Index == (NodeOver.Parent.Nodes.Count - 1)) &&
					  ( (OffsetY < NodeOver.Bounds.Height / 3) || (OffsetY > 2*NodeOver.Bounds.Height/3) ) )
					)
				{
					#region Standard Node
					if (OffsetY < (NodeOver.Bounds.Height / 2))
					{
						//this.lblDebug.Text = "top";

						#region If NodeOver is a child then cancel
						TreeNode tnParadox = NodeOver;
						while (tnParadox.Parent != null)
						{
							if (tnParadox.Parent == NodeMoving)
							{
								this.NodeMap = "";
								return;
							}

							tnParadox = tnParadox.Parent;
						}
						#endregion
						#region Store the placeholder info into a pipe delimited string
						SetNewNodeMap(NodeOver, false);
						if (SetMapsEqual() == true)
							return;
						#endregion
						#region Clear placeholders above and below
						this.Refresh();
						#endregion
						#region Draw the placeholders
						this.DrawLeafTopPlaceholders(NodeOver);
						#endregion
					}
					else
					{
						//this.lblDebug.Text = "bottom";

						#region If NodeOver is a child then cancel
						TreeNode tnParadox = NodeOver;
						while (tnParadox.Parent != null)
						{
							if (tnParadox.Parent == NodeMoving)
							{
								this.NodeMap = "";
								return;
							}

							tnParadox = tnParadox.Parent;
						}
						#endregion
						#region Allow drag drop to parent branches
						TreeNode ParentDragDrop = null;
						// If the node the mouse is over is the last node of the branch we should allow
						// the ability to drop the "nodemoving" node BELOW the parent node
						if (NodeOver.Parent != null && NodeOver.Index == (NodeOver.Parent.Nodes.Count - 1))
						{
							int XPos = PointToClient(Cursor.Position).X;
							if (XPos < NodeOver.Bounds.Left)
							{
								ParentDragDrop = NodeOver.Parent;

								// MTC - commented out--maybe there isn't an image list!!!
								//if (XPos < (ParentDragDrop.Bounds.Left - ImageList.Images[ParentDragDrop.ImageIndex].Size.Width))
								//{
								//    if (ParentDragDrop.Parent != null)
								//        ParentDragDrop = ParentDragDrop.Parent;
								//}
							}
						}
						#endregion
						#region Store the placeholder info into a pipe delimited string
						// Since we are in a special case here, use the ParentDragDrop node as the current "nodeover"
						SetNewNodeMap(ParentDragDrop != null ? ParentDragDrop : NodeOver, true);
						if (SetMapsEqual() == true)
							return;
						#endregion
						#region Clear placeholders above and below
						this.Refresh();
						#endregion
						#region Draw the placeholders
						DrawLeafBottomPlaceholders(NodeOver, ParentDragDrop);
						#endregion
					}
					#endregion
				}
				else
				{
					#region Folder Node
					if (OffsetY < (NodeOver.Bounds.Height / 3))
					{
						//this.lblDebug.Text = "folder top";

						#region If NodeOver is a child then cancel
						TreeNode tnParadox = NodeOver;
						while (tnParadox.Parent != null)
						{
							if (tnParadox.Parent == NodeMoving)
							{
								this.NodeMap = "";
								return;
							}

							tnParadox = tnParadox.Parent;
						}
						#endregion
						#region Store the placeholder info into a pipe delimited string
						SetNewNodeMap(NodeOver, false);
						if (SetMapsEqual() == true)
							return;
						#endregion
						#region Clear placeholders above and below
						this.Refresh();
						#endregion
						#region Draw the placeholders
						this.DrawFolderTopPlaceholders(NodeOver);
						#endregion
					}
					else if ((NodeOver.Parent != null && NodeOver.Index == 0) && (OffsetY > (NodeOver.Bounds.Height - (NodeOver.Bounds.Height / 3))))
					{
						//this.lblDebug.Text = "folder bottom";

						#region If NodeOver is a child then cancel
						TreeNode tnParadox = NodeOver;
						while (tnParadox.Parent != null)
						{
							if (tnParadox.Parent == NodeMoving)
							{
								this.NodeMap = "";
								return;
							}

							tnParadox = tnParadox.Parent;
						}
						#endregion
						#region Store the placeholder info into a pipe delimited string
						SetNewNodeMap(NodeOver, true);
						if (SetMapsEqual() == true)
							return;
						#endregion
						#region Clear placeholders above and below
						this.Refresh();
						#endregion
						#region Draw the placeholders
						DrawFolderTopPlaceholders(NodeOver);
						#endregion
					}
					else
					{
						//this.lblDebug.Text = "folder over";

						if (NodeOver.Nodes.Count > 0)
						{
							NodeOver.Expand();
							//this.Refresh();
						}
						else
						{
							#region Prevent the node from being dragged onto itself
							if (NodeMoving == NodeOver)
								return;
							#endregion
							#region If NodeOver is a child then cancel
							TreeNode tnParadox = NodeOver;
							while (tnParadox.Parent != null)
							{
								if (tnParadox.Parent == NodeMoving)
								{
									this.NodeMap = "";
									return;
								}

								tnParadox = tnParadox.Parent;
							}
							#endregion
							#region Store the placeholder info into a pipe delimited string
							SetNewNodeMap(NodeOver, false);
							NewNodeMap = NewNodeMap.Insert(NewNodeMap.Length, "|0");

							if (SetMapsEqual() == true)
								return;
							#endregion
							#region Clear placeholders above and below
							this.Refresh();
							#endregion
							#region Draw the "add to folder" placeholder
							DrawAddToFolderPlaceholder(NodeOver);
							#endregion
						}
					}
					#endregion
				}
			}
		}

		private void SetNewNodeMap(TreeNode tnNode, bool boolBelowNode)
		{
			NewNodeMap.Length = 0;

			if (boolBelowNode)
				NewNodeMap.Insert(0, (int)tnNode.Index + 1);
			else
				NewNodeMap.Insert(0, (int)tnNode.Index);
			TreeNode tnCurNode = tnNode;

			while (tnCurNode.Parent != null)
			{
				tnCurNode = tnCurNode.Parent;

				if (NewNodeMap.Length == 0 && boolBelowNode == true)
				{
					NewNodeMap.Insert(0, (tnCurNode.Index + 1) + "|");
				}
				else
				{
					NewNodeMap.Insert(0, tnCurNode.Index + "|");
				}
			}
		}//oem

		private bool SetMapsEqual()
		{
			if (this.NewNodeMap.ToString() == this.NodeMap)
				return true;
			else
			{
				this.NodeMap = this.NewNodeMap.ToString();
				return false;
			}
		}//oem

		private void DrawLeafTopPlaceholders(TreeNode NodeOver)
		{
			Graphics g = CreateGraphics();

			// MTC
//			int NodeOverImageWidth = ImageList.Images[NodeOver.ImageIndex].Size.Width + 8;
			int LeftPos = NodeOver.Bounds.Left; // MTC -NodeOverImageWidth;
			int RightPos = Width - 4;

			Point[] LeftTriangle = new Point[5]{
												   new Point(LeftPos, NodeOver.Bounds.Top - 4),
												   new Point(LeftPos, NodeOver.Bounds.Top + 4),
												   new Point(LeftPos + 4, NodeOver.Bounds.Y),
												   new Point(LeftPos + 4, NodeOver.Bounds.Top - 1),
												   new Point(LeftPos, NodeOver.Bounds.Top - 5)};

			Point[] RightTriangle = new Point[5]{
													new Point(RightPos, NodeOver.Bounds.Top - 4),
													new Point(RightPos, NodeOver.Bounds.Top + 4),
													new Point(RightPos - 4, NodeOver.Bounds.Y),
													new Point(RightPos - 4, NodeOver.Bounds.Top - 1),
													new Point(RightPos, NodeOver.Bounds.Top - 5)};


			g.FillPolygon(System.Drawing.Brushes.Black, LeftTriangle);
			g.FillPolygon(System.Drawing.Brushes.Black, RightTriangle);
			g.DrawLine(new System.Drawing.Pen(Color.Black, 2), new Point(LeftPos, NodeOver.Bounds.Top), new Point(RightPos, NodeOver.Bounds.Top));

		}//eom

		private void DrawLeafBottomPlaceholders(TreeNode NodeOver, TreeNode ParentDragDrop)
		{
			Graphics g = CreateGraphics();

			// MTC
//			int NodeOverImageWidth = ImageList.Images[NodeOver.ImageIndex].Size.Width + 8;
			// Once again, we are not dragging to node over, draw the placeholder using the ParentDragDrop bounds
			int LeftPos, RightPos;
			// MTC
			//if (ParentDragDrop != null)
			//    LeftPos = ParentDragDrop.Bounds.Left - (ImageList.Images[ParentDragDrop.ImageIndex].Size.Width + 8);
			//else
			LeftPos = NodeOver.Bounds.Left; // MTC -NodeOverImageWidth;
			RightPos = Width - 4;

			Point[] LeftTriangle = new Point[5]{
												   new Point(LeftPos, NodeOver.Bounds.Bottom - 4),
												   new Point(LeftPos, NodeOver.Bounds.Bottom + 4),
												   new Point(LeftPos + 4, NodeOver.Bounds.Bottom),
												   new Point(LeftPos + 4, NodeOver.Bounds.Bottom - 1),
												   new Point(LeftPos, NodeOver.Bounds.Bottom - 5)};

			Point[] RightTriangle = new Point[5]{
													new Point(RightPos, NodeOver.Bounds.Bottom - 4),
													new Point(RightPos, NodeOver.Bounds.Bottom + 4),
													new Point(RightPos - 4, NodeOver.Bounds.Bottom),
													new Point(RightPos - 4, NodeOver.Bounds.Bottom - 1),
													new Point(RightPos, NodeOver.Bounds.Bottom - 5)};


			g.FillPolygon(System.Drawing.Brushes.Black, LeftTriangle);
			g.FillPolygon(System.Drawing.Brushes.Black, RightTriangle);
			g.DrawLine(new System.Drawing.Pen(Color.Black, 2), new Point(LeftPos, NodeOver.Bounds.Bottom), new Point(RightPos, NodeOver.Bounds.Bottom));
		}//eom

		private void DrawFolderTopPlaceholders(TreeNode NodeOver)
		{
			Graphics g = CreateGraphics();
			// MTC
//			int NodeOverImageWidth = ImageList.Images[NodeOver.ImageIndex].Size.Width + 8;

			int LeftPos, RightPos;
			LeftPos = NodeOver.Bounds.Left; // MTC -NodeOverImageWidth;
			RightPos = Width - 4;

			Point[] LeftTriangle = new Point[5]{
												   new Point(LeftPos, NodeOver.Bounds.Top - 4),
												   new Point(LeftPos, NodeOver.Bounds.Top + 4),
												   new Point(LeftPos + 4, NodeOver.Bounds.Y),
												   new Point(LeftPos + 4, NodeOver.Bounds.Top - 1),
												   new Point(LeftPos, NodeOver.Bounds.Top - 5)};

			Point[] RightTriangle = new Point[5]{
													new Point(RightPos, NodeOver.Bounds.Top - 4),
													new Point(RightPos, NodeOver.Bounds.Top + 4),
													new Point(RightPos - 4, NodeOver.Bounds.Y),
													new Point(RightPos - 4, NodeOver.Bounds.Top - 1),
													new Point(RightPos, NodeOver.Bounds.Top - 5)};


			g.FillPolygon(System.Drawing.Brushes.Black, LeftTriangle);
			g.FillPolygon(System.Drawing.Brushes.Black, RightTriangle);
			g.DrawLine(new System.Drawing.Pen(Color.Black, 2), new Point(LeftPos, NodeOver.Bounds.Top), new Point(RightPos, NodeOver.Bounds.Top));

		}//eom
		private void DrawAddToFolderPlaceholder(TreeNode NodeOver)
		{
			Graphics g = CreateGraphics();
			int RightPos = NodeOver.Bounds.Right + 6;
			Point[] RightTriangle = new Point[5]{
													new Point(RightPos, NodeOver.Bounds.Y + (NodeOver.Bounds.Height / 2) + 4),
													new Point(RightPos, NodeOver.Bounds.Y + (NodeOver.Bounds.Height / 2) + 4),
													new Point(RightPos - 4, NodeOver.Bounds.Y + (NodeOver.Bounds.Height / 2)),
													new Point(RightPos - 4, NodeOver.Bounds.Y + (NodeOver.Bounds.Height / 2) - 1),
													new Point(RightPos, NodeOver.Bounds.Y + (NodeOver.Bounds.Height / 2) - 5)};

			this.Refresh();
			g.FillPolygon(System.Drawing.Brushes.Black, RightTriangle);
		}//eom

		protected virtual void OnNewNode(NewNodeEventArgs args)
		{
			if (NewNode != null)
			{
				NewNode(this, args);
			}
		}
	}
}
