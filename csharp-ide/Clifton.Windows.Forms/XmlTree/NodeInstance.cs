using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
{
	public class NodeInstance
	{
		protected TreeNode tn;
		protected NodeDef nodeDef;
		protected IXtreeNode instance;
		
		/// <summary>
		/// Gets/sets nodeDef
		/// </summary>
		public NodeDef NodeDef
		{
			get { return nodeDef; }
			set { nodeDef = value; }
		}

		/// <summary>
		/// Gets/sets instance
		/// </summary>
		public IXtreeNode Instance
		{
			get { return instance; }
			set { instance = value; }
		}

		public NodeInstance(TreeNode tn, NodeDef def)
		{
			this.tn = tn;
			nodeDef = def;
		}

		public NodeInstance(TreeNode tn, NodeDef def, IXtreeNode instance)
		{
			this.tn = tn;
			nodeDef = def;
			this.instance = instance;
		}
	}
}
