using System;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
{
	public class NodeMenuItem : MenuItem
	{
		protected TreeNode tn;
		protected NodeDef parentNode;
		protected NodeDef childNode;
		protected Popup popup;

		public TreeNode TreeNode
		{
			get { return tn; }
		}

		public NodeDef ParentNode
		{
			get { return parentNode; }
		}

		public NodeDef ChildNode
		{
			get { return childNode; }
		}

		public Popup PopupInfo
		{
			get { return popup; }
		}

		public NodeMenuItem(string text, TreeNode tn, NodeDef parentNode, NodeDef childNode, Popup popup)
			: base(text)
		{
			this.tn = tn;
			this.parentNode = parentNode;
			this.childNode = childNode;
			this.popup = popup;
		}

		public void RaiseClick()
		{
			OnClick(EventArgs.Empty);
		}

		public void RaiseClick(EventArgs e)
		{
			OnClick(e);
		}
	}
}
