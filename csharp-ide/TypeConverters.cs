using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;

using csharp_ide.Controllers;

namespace csharp_ide
{
	public class StructureNameConverter : StringConverter
	{
		public override bool GetStandardValuesSupported(ITypeDescriptorContext context)
		{
			return true;
		}

		public override TypeConverter.StandardValuesCollection GetStandardValues(ITypeDescriptorContext context)
		{
			// Native types are listed first.
			List<string> names = new List<string>() { "string", "int", "float", "list" };
			names.Sort();

			// Next, we get the names of all existing structures, sorted.
			List<string> structs = ApplicationFormController.Instance.ApplicationModel.StructureRefCount.Keys.ToList();
			names.ForEach(nt => structs.Remove(nt));
			structs.Sort();

			names.Add("--------");  // Add a separator.  TODO: This is selectable, and should not be.

			// Append structs to names, excluding native types.
			names.AddRange(structs);

			return new StandardValuesCollection(names);
		}
	}
}
