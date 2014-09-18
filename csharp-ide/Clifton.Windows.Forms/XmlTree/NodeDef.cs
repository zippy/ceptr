using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Reflection;
using System.Resources;
using System.Windows.Forms;

namespace Clifton.Windows.Forms.XmlTree
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

	public class NodeDef : ISupportInitialize
	{
		protected string name;
		protected string refName;
		protected bool isReadOnly;
		protected string text;
		protected bool isRequired;
		protected string iconFilename;
		private string typeName;
		private Type type;
		protected ImageList imgList;
		protected int imgOffset;
		protected bool separator;
		
		protected List<Popup> parentPopupItems;
		protected List<Popup> popupItems;
		protected List<NodeDef> nodes;
		protected NodeDef parent;
		protected bool recurse;
		protected string typeData;
		protected string iconResourceName;

		public bool Separator
		{
			get { return separator; }
			set { separator = value; }
		}
		
		/// <summary>
		/// Gets/sets iconResourceName
		/// </summary>
		public string IconResourceName
		{
			get { return iconResourceName; }
			set { iconResourceName = value; }
		}

		/// <summary>
		/// Gets/sets typeData
		/// </summary>
		public string TypeData
		{
			get { return typeData; }
			set { typeData = value; }
		}
		
		/// <summary>
		/// Gets/sets recurse flag.
		/// </summary>
		public bool Recurse
		{
			get { return recurse; }
			set { recurse = value; }
		}
		
		/// <summary>
		/// Gets/sets parent
		/// </summary>
		public NodeDef Parent
		{
			get { return parent; }
			set { parent = value; }
		}

		public string Name
		{
			get { return name; }
			set { name = value; }
		}

		public string RefName
		{
			get { return refName; }
			set { refName = value; }
		}

		public bool IsReadOnly
		{
			get { return isReadOnly; }
			set { isReadOnly = value; }
		}

		public string Text
		{
			get { return text; }
			set { text = value; }
		}

		public bool IsRequired
		{
			get { return isRequired; }
			set { isRequired = value; }
		}

		public List<Popup> ParentPopupItems
		{
			get { return parentPopupItems; }
		}

		public List<Popup> PopupItems
		{
			get { return popupItems; }
		}

		public List<NodeDef> Nodes
		{
			get
			{
				if ((parent != null) && (recurse))
				{
					return parent.Nodes;
				}
				else
				{
					return nodes;
				}
			}
		}

		public bool IsRef
		{
			get { return refName != null; }
		}

		public string IconFilename
		{
			get { return iconFilename; }
			set { iconFilename = value; }
		}

		/// <summary>
		/// Gets/sets typeName
		/// </summary>
		public string TypeName
		{
			get { return typeName; }
			set { typeName = value; }
		}

		/// <summary>
		/// Gets/sets type
		/// </summary>
		public Type ImplementingType
		{
			get { return type; }
			set { type = value; }
		}

		public ImageList ImageList
		{
			get { return imgList; }
		}

		public int ImageOffset
		{
			get { return imgOffset; }
			set { imgOffset = value; }
		}

		public NodeDef()
		{
			parentPopupItems = new List<Popup>();
			popupItems = new List<Popup>();
			nodes = new List<NodeDef>();
			imgOffset = -1;
			separator = true;
		}

		public virtual void BeginInit()
		{
		}

		public virtual void EndInit()
		{
			imgList = new ImageList();
			type = typeof(PlaceholderInstance);

			if (iconFilename != null)
			{
				string[] icons = iconFilename.Split(',');

				foreach (string ifn in icons)
				{
					string fn=ifn.Trim();
					Icon icon = new Icon(fn);
					imgList.Images.Add(fn, icon);
				}
			}

			if (iconResourceName != null)
			{
				string[] icons = iconResourceName.Split(',');

				foreach (string irn in icons)
				{
					string assyName;
					string className;
					string resName;
					Helper.GetResourceInfo(irn, out assyName, out className, out resName);
					Assembly resAssy = Assembly.Load(assyName);
					ResourceManager rm = new ResourceManager(assyName + "." + className, resAssy);
					object obj = rm.GetObject(resName.Trim());
					imgList.Images.Add(irn + ".ico", (Icon)obj);
				}
			}

			if (typeName != null)
			{
				try
				{
					type = Type.GetType(typeName);
					System.Diagnostics.Trace.WriteLine("Typename " + TypeName + (type == null ? " null" : " found"));
				}
				catch (Exception e)
				{
					System.Diagnostics.Debug.WriteLine("Loading typename " + typeName + " failed.");
					throw e;
				}
			}

			foreach (NodeDef child in nodes)
			{
				child.Parent = this;
			}
		}

		public IXtreeNode CreateImplementingType(IXtreeNode parent)
		{
			IXtreeNode inst = null;
			inst = (IXtreeNode)Activator.CreateInstance(ImplementingType);
			inst.Parent = parent;
			return inst;
		}
	}
}
