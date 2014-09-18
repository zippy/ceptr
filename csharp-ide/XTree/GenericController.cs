using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

using Clifton.Tools.Strings.Extensions;
using Clifton.Windows.Forms;
using Clifton.Windows.Forms.XmlTree;

using XTreeInterfaces;

namespace XTreeController
{
	// Used in the tree definition XML to provide a class at the terminal point of a hierarchy.
	public class NullInstance : IHasCollection
	{
		public string Name { get; set; }
		public Dictionary<string, dynamic> Collection { get { return null; } }
	}

	// Other examples:
	// SchemaDef has a collection of TableDef objects.
	// The parent controller of a TableDef object is the GenericController<SchemaDef, TableDef>

	/// <summary>
	/// Generic controller for a backing class that implements a single collection to which child nodes can be added.
	/// </summary>
	/// <typeparam name="T">Backing class that implements the instance.  For example, TableDef</typeparam>
	/// <typeparam name="U">The collection that this controller manages, for example, TableFieldDef</typeparam>
	public class GenericController<T> :
		XtreeNodeController, IGenericController
		where T : IHasCollection, new()
	{
		public T Instance { get; set; }

		public override string Name { get; set; }

		public override object Item
		{
			get { return Instance; }
			set { Instance = (T)value; }
		}

		public Dictionary<string, dynamic> Collection { get { return Instance.Collection; } }

		public string GenericTypeName
		{
			get
			{
				// Example: XTreeDemo.GenericController`1[[ROPLib.Entity, ROPLib, Version=1.0.0.0, Culture=neutral, PublicKeyToken=null]]
				string fulltype = this.GetType().FullName;
				string typeName = fulltype.RightOf('.').Between('.', ',');

				return typeName;
			}
		}

		public GenericController()
		{
			Instance = new T();
		}

		public GenericController(bool createInstance)
		{
			if (createInstance)
			{
				Instance = new T();
			}
		}

		public override int Index(object item)
		{
			return Instance.Collection[GenericTypeName].IndexOf((T)item);
		}

		public override bool AddNode(IXtreeNode parentInstance, string tag)
		{
			IGenericController ctrl = (IGenericController)parentInstance;
			ctrl.Collection[GenericTypeName].Add(Instance);

			return true;
		}

		public override bool DeleteNode(IXtreeNode parentInstance)
		{
			// TODO: Inject the ability to confirm the delete operation.

			IGenericController ctrl = (IGenericController)parentInstance;
			ctrl.Collection[GenericTypeName].Remove(Instance);

			return true;
		}

		public override bool AutoDeleteNode(IXtreeNode parentInstance)
		{
			IGenericController ctrl = (IGenericController)parentInstance;
			ctrl.Collection[GenericTypeName].Remove(Instance);

			return true;
		}

		public override void InsertNode(IXtreeNode parentInstance, int idx)
		{
			throw new NotImplementedException();
		}

		public override void Select(TreeNode tn)
		{
			// !!!TODO: This updates the property grid.
			// Program.Properties.SelectedObject = Instance;
		}
	}
}
