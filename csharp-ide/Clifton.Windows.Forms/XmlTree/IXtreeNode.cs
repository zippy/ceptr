using System;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
{
	public interface IXtreeNode
	{
		string Name {get; set;}
		string TypeData { get; set;}
		IXtreeNode Parent {get; set;}
		int IconIndex { get;}
		int SelectedIconIndex { get;}
		object Item { get; set; }

		bool AddNode(IXtreeNode parentInstance, string tag);
		bool DeleteNode(IXtreeNode parentInstance);
		bool AutoDeleteNode(IXtreeNode parentInstance);
		void Select(TreeNode tn);
		bool IsEnabled(string tag, bool defaultState);
		void MoveTo(IXtreeNode newParent, IXtreeNode oldParent, int idx, TreeNode movingNode);
		int Index(object obj);
	}
}
