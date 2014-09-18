using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Forms;

using csharp_ide.Models;

namespace csharp_ide.Controllers
{
	public abstract class ViewController<T> : ISupportInitialize
	{
		public ApplicationModel ApplicationModel { get; protected set; }
		public ApplicationFormController ApplicationController { get; protected set; }

		public T View { get; protected set; }

		public ViewController()
		{
		}

		public virtual void BeginInit()
		{
		}

		public virtual void EndInit()
		{
		}
	}
}
