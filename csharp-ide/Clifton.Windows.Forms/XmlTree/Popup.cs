using System;
using System.Collections.Generic;

namespace Clifton.Windows.Forms.XmlTree
{
	public class Popup
	{
		protected string text;
		protected bool isAdd;
		protected bool isRemove;
		protected bool enabled;
		protected string tag;
        protected List<Popup> popupItems;

		public string Text
		{
			get { return text; }
			set { text = value; }
		}

		public bool IsAdd
		{
			get { return isAdd; }
			set { isAdd = value; }
		}

		public bool IsRemove
		{
			get { return isRemove; }
			set { isRemove = value; }
		}

		public bool Enabled
		{
			get { return enabled; }
			set { enabled = value; }
		}

		public string Tag
		{
			get { return tag; }
			set { tag = value; }
		}

        public List<Popup> PopupItems
        {
            get { return popupItems; }
        }

		public Popup()
		{                                   
            popupItems = new List<Popup>();
			enabled = true;
		}
	}
}
