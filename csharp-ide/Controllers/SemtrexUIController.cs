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

using Clifton.Tools.Strings.Extensions;
using Clifton.Windows.Forms.XmlTree;

using csharp_ide.Models;
using csharp_ide.Views;

using ceptrlib;

namespace csharp_ide.Controllers
{
	public class SemtrexUIController : ViewController<SemtrexUIView>
	{
		public SemtrexUIController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.SemtrexUIController = this;
			base.EndInit();
		}

		public void Go(object sender, EventArgs args)
		{
			ApplicationController.CeptrInterface.CreateStructureAndSymbolNodes();
			Dictionary<string, SemanticID> symbolMap = new Dictionary<string, SemanticID>();
			Dictionary<string, SemanticID> structureMap = new Dictionary<string, SemanticID>();

			foreach (string symbolName in ApplicationModel.SymbolRefCount.Keys)
			{
				if (!symbolMap.ContainsKey(symbolName))
				{
					// Find the symbol in the tree.
					Symbol symbol = FindSymbolInTree(symbolName, ApplicationController.SymbolEditorController.View.TreeView.Nodes);

					if (symbol == null)
					{
						throw new Exception("The symbol " + symbolName + " should have been found in the tree.");
					}

					SemanticID topStructure = ApplicationController.Recurse(symbol, structureMap, symbolMap);
					SemanticID symbolID = ApplicationController.CeptrInterface.DeclareSymbol(ApplicationController.CeptrInterface.RootSymbolsNode, topStructure, symbolName);
					symbolMap[symbolName] = symbolID;
				}
			}

			Guid treeID = ApplicationController.CeptrInterface.ParseSemtrex(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				View.tbMatchExpression.Text);

			string dump = ApplicationController.CeptrInterface.Dump(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				treeID);

			View.tbSemtrexTree.Text = ApplicationController.FormatDump(dump);
		}

		// TODO: As per other comments, we need an actual model separate from the view!
		/// <summary>
		/// Recursively try to find where the symbol is defined in the tree.
		/// </summary>
		protected Symbol FindSymbolInTree(string symbol, TreeNodeCollection nodes)
		{
			Symbol ret = null;

			foreach (System.Windows.Forms.TreeNode node in nodes)
			{
				if (node.Text.LeftOf(':').Trim() == symbol)
				{
					NodeInstance inst = (NodeInstance)node.Tag;
					ret = (Symbol)inst.Instance.Item;
					break;
				}
				else
				{
					ret = FindSymbolInTree(symbol, node.Nodes);

					if (ret != null)
					{
						break;
					}
				}
			}

			return ret;
		}
	}
}

/*
    stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/{HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT})";
    s = parseSemtrex(&test_HTTP_defs,stx);
    spec_is_str_equal(_dump_semtrex(test_HTTP_defs,s,buf),stx);
    spec_is_str_equal(__t_dump(&test_HTTP_defs,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENTS (SEMTREX_GROUP:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENT))))))");
*/


