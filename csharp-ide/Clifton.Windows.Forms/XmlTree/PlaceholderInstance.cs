using System;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
{
	public class PlaceholderInstance : IXtreeNode
	{
		protected string name;
		protected IXtreeNode parent;
		
		/// <summary>
		/// Gets/sets parent
		/// </summary>
		public IXtreeNode Parent
		{
			get { return parent; }
			set { parent = value; }
		}

		/// <summary>
		/// Gets/sets name
		/// </summary>
		public string Name
		{
			get { return name; }
			set { name = value; }
		}

		public object Item { get; set; }

		public string TypeData
		{
			get { return null; }
			set { }
		}

		public int IconIndex
		{
			get { return 0; }
		}

		public int SelectedIconIndex
		{
			get { return 0; }
		}

		public override string ToString()
		{
			return name;
		}

		public bool AddNode(IXtreeNode parentInstance, string tag)
		{
			return true;
		}

		public bool DeleteNode(IXtreeNode parentInstance)
		{
			return true;
		}

		public bool AutoDeleteNode(IXtreeNode parentInstance)
		{
			return true;
		}

		public void Select(TreeNode tn)
		{
		}

		public bool IsEnabled(string tag, bool defaultValue)
		{
			return defaultValue;
		}


		public void MoveTo(IXtreeNode newParent, IXtreeNode oldParent, int idx, TreeNode movingNode)
		{
		}

		public int Index(object obj)
		{
			return -1;
		}
	}
}
