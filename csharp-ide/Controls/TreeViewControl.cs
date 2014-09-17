using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using Clifton.ExtensionMethods;

namespace csharp_ide.Controls
{
	public class TreeViewControl : TreeView
	{
		public void Clear()
		{
			Nodes.Clear();
		}

		public object AddNode(object parent, string name, object tag)
		{
			TreeNode node = null;

			if (parent == null)
			{
				node = Nodes.Add(name);
				node.Tag = tag;
			}
			else
			{
				node = ((TreeNode)parent).Nodes.Add(name);
				node.Tag = tag;
			}

			return node;
		}
	}
}
