using System;
using System.Collections.Generic;
using System.Xml;

using Clifton.Windows.Forms.XmlTree;

namespace XTreeController
{
	public interface IGenericController
	{
		Dictionary<string, dynamic> Collection { get; }
	}
}
