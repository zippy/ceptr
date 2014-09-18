using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
{
	public class XmlTreeException : ApplicationException
	{
		public XmlTreeException(string msg)
			: base(msg)
		{
		}
	}

	public abstract class XtreeNodeController : IXtreeNode
	{
		protected IXtreeNode parent;
		protected string name;
		
		/// <summary>
		/// Gets/sets name.  This is the tree nodel label.
		/// TODO: rename to "Label".
		/// </summary>
		public virtual string Name
		{
			get { return name; }
			set { name = value; }
		}

		public virtual string TypeData
		{
			get { return null; }
			set {}
		}

		/// <summary>
		/// Gets/sets parent
		/// </summary>
		public IXtreeNode Parent
		{
			get { return parent; }
			set { parent = value; }
		}

		public virtual int IconIndex
		{
			get { return 0; }
		}

		public virtual int SelectedIconIndex
		{
			get { return 0; }
		}

		public abstract object Item
		{
			get;
			set;
		}

		public virtual bool IsEnabled(string tag, bool defaultValue)
		{
			return defaultValue;
		}

		public virtual void MoveTo(IXtreeNode newParent, IXtreeNode oldParent, int idx, TreeNode movingNode)
		{
			int oldIdx = oldParent.Index(this);
			idx=AdjustIndex(newParent, movingNode, idx);

			// Make sure indexing is supported by the controller.
			if (oldIdx != -1)
			{
				// If we're moving the node internally to our own parent...
				if (newParent == oldParent)
				{
					// Get the old index.
					bool ret=AutoDeleteNode(oldParent);

					// If this is before the new insert point, we can delete the old index
					// and insert at idx-1, since everything is shifted back one entry.
					// If the controller did not delete the node, then the insertion point
					// is "idx", not "idx-1"
					if ( (oldIdx < idx) && (ret) )
					{
						InsertNode(oldParent, idx - 1);
					}
					else
					{
						// the oldIdx occurs after the new point, so we can delete the old entry
						// and insert the new one without changing the new index point.
						InsertNode(oldParent, idx);
					}
				}
				else
				{
					// parent is different, so delete our node...
					AutoDeleteNode(oldParent);
					// Insert our field in the new parent.
					InsertNode(newParent, idx);
				}
			}
		}

		public abstract int Index(object item);
		public abstract void InsertNode(IXtreeNode parentInstance, int idx);
		public abstract bool AddNode(IXtreeNode parentInstance, string tag);
		public abstract bool DeleteNode(IXtreeNode parentInstance);
		public abstract bool AutoDeleteNode(IXtreeNode parentInstance);
		public abstract void Select(TreeNode tn);
		public virtual int AdjustIndex(IXtreeNode newParent, TreeNode movingNode, int idx) { return idx; }
	}
}


/*

		public override void MoveTo(IXtreeNode newParent, IXtreeNode oldParent, int idx)
		{
			// If we're moving the node internally to our own parent...
			if (newParent == oldParent)
			{
				// Initialize, if not already set.
				tableDef = ((TableController)oldParent).TableDef;
				// Get the old index.
				int oldIdx = tableDef.Fields.IndexOf(tableFieldDef);

				// If this is before the new insert point, we can delete the old index
				// and insert at idx-1, since everything is shifted back one entry.
				if (oldIdx < idx)
				{
					tableDef.Fields.RemoveAt(oldIdx);
					tableDef.Fields.Insert(idx - 1, tableFieldDef);
				}
				else
				{
					// the oldIdx occurs after the new point, so we can delete the old entry
					// and insert the new one without changing the new index point.
					tableDef.Fields.RemoveAt(oldIdx);
					tableDef.Fields.Insert(idx, tableFieldDef);
				}
			}
			else
			{
				// parent is different, so delete our node...
				((TableController)oldParent).TableDef.Fields.Remove(tableFieldDef);
				// Insert our field in the new parent.
				((TableController)newParent).TableDef.Fields.Insert(idx, tableFieldDef);
				// Reset our parent.
				tableDef = ((TableController)oldParent).TableDef;
			}
		}
 
 */