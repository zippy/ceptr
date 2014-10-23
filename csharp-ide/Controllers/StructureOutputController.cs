using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using System.Xml.Linq;

using csharp_ide.Models;
using csharp_ide.Views;

using ceptrlib;

namespace csharp_ide.Controllers
{
	public class StructureOutputController : ViewController<StructureOutputView>
	{
		public StructureOutputController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.StructureOutputController = this;
			base.EndInit();
		}

		public void ShowStructureDump(Symbol symbol)
		{
			ApplicationController.CeptrInterface.CreateStructureAndSymbolNodes();
			Dictionary<string, SemanticID> structureMap = new Dictionary<string, SemanticID>();
			SemanticID topStructure = ApplicationController.Recurse(symbol, structureMap);
			ApplicationController.CeptrInterface.DeclareSymbol(ApplicationController.CeptrInterface.RootSymbolsNode, topStructure, symbol.Name);

			string ret = ApplicationController.CeptrInterface.DumpStructures(ApplicationController.CeptrInterface.RootSymbolsNode, ApplicationController.CeptrInterface.RootStructuresNode);
			View.Output.Text = ApplicationController.FormatDump(ret);
		}
	}
}