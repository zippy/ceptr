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
		protected Dictionary<string, SemanticID> symbolMap;
		protected Dictionary<string, SemanticID> structureMap;
		protected Guid asciiTreeID;
		protected Guid parseExprID;
		protected Guid matchResultTreeID;
		protected Guid embodyID;
		protected Guid matchAgainstID;
		protected string fnPlayground;

		public SemtrexUIController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.SemtrexUIController = this;
			base.EndInit();
		}

		public void CreateStructuresAndSymbols()
		{
			symbolMap = new Dictionary<string, SemanticID>();
			structureMap = new Dictionary<string, SemanticID>();

			ApplicationController.CeptrInterface.CreateStructureAndSymbolNodes();

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
		}
		
		public void ToTree(object sender, EventArgs args)
		{
			CreateStructuresAndSymbols();

			asciiTreeID = ApplicationController.CeptrInterface.GetTree(View.tbInputString.Text);
			DumpOutput(asciiTreeID);
		}

		public void ToSemtrex(object sender, EventArgs args)
		{
			parseExprID = ApplicationController.CeptrInterface.ParseSemtrex(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				View.tbParseExpr.Text
				);
			DumpOutput(parseExprID);
		}

		public void Match(object sender, EventArgs args)
		{
			Tuple<bool, Guid> result = ApplicationController.CeptrInterface.Match(parseExprID, asciiTreeID);

			if (result.Item1)
			{
				View.tbMatchResult.Text = "True";
				matchResultTreeID = result.Item2;
				DumpOutput(matchResultTreeID);
			}
			else
			{
				View.tbMatchResult.Text = "False";
				View.tbSemtrexTree.Text = "";
			}
		}

		public void Embody(object sender, EventArgs args)
		{
			embodyID = ApplicationController.CeptrInterface.Embody(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				matchResultTreeID,
				asciiTreeID
				);

			DumpOutput(embodyID);
		}

		public void ToMatchAgainst(object sender, EventArgs args)
		{
			matchAgainstID = ApplicationController.CeptrInterface.ParseSemtrex(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				View.tbMatchAgainst.Text
				);
			DumpOutput(matchAgainstID);
		}

		public void MatchAgainstMatches(object sender, EventArgs args)
		{
			bool ret = ApplicationController.CeptrInterface.MatchTest(matchAgainstID, embodyID);

			View.tbMatchResult2.Text=(ret ? "True" : "False");
		}
/*
		public void AsciiParserToTree(object swender, EventArgs args)
		{
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
*/
		// Formatted output of the resulting tree.
		public void DumpOutput(Guid id)
		{
			string dump = ApplicationController.CeptrInterface.Dump(
				ApplicationController.CeptrInterface.RootSymbolsNode,
				ApplicationController.CeptrInterface.RootStructuresNode,
				id);

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

		// UI 

		protected void New(object sender, EventArgs args)
		{
			View.Clear();
		}

		protected void Load(object sender, EventArgs args)
		{
			OpenFileDialog ofd = new OpenFileDialog();
			DialogResult ret = ofd.ShowDialog();

			if (ret == DialogResult.OK)
			{
				fnPlayground = ofd.FileName;

				string[] lines = File.ReadAllLines(fnPlayground);
				// string text = View.tbInputString.Text + "\r\n" + View.tbParseExpr.Text + "\r\n" + View.tbMatchAgainst.Text;
				View.tbInputString.Text = lines[0];
				View.tbParseExpr.Text = lines[1];
				View.tbMatchAgainst.Text = lines[2];
			}
		}

		protected void Save(object sender, EventArgs args)
		{
			if (String.IsNullOrEmpty(fnPlayground))
			{
				SaveAs(sender, args);
			}
			else
			{
				InternalSave();
			}
		}

		protected void SaveAs(object sender, EventArgs args)
		{
			SaveFileDialog sfd = new SaveFileDialog();
			DialogResult ret = sfd.ShowDialog();

			if (ret == DialogResult.OK)
			{
				fnPlayground = sfd.FileName;
				InternalSave();
			}
		}

		protected void InternalSave()
		{
			string text = View.tbInputString.Text + "\r\n" + View.tbParseExpr.Text + "\r\n" + View.tbMatchAgainst.Text;
			File.WriteAllText(fnPlayground, text);
		}
	}
}

/*
    stx = "/HTTP_REQUEST/(.,.,HTTP_REQUEST_PATH/HTTP_REQUEST_PATH_SEGMENTS/{HTTP_REQUEST_PATH_SEGMENT:HTTP_REQUEST_PATH_SEGMENT})";
    s = parseSemtrex(&test_HTTP_defs,stx);
    spec_is_str_equal(_dump_semtrex(test_HTTP_defs,s,buf),stx);
    spec_is_str_equal(__t_dump(&test_HTTP_defs,s,0,buf)," (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_ANY) (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENTS (SEMTREX_GROUP:HTTP_REQUEST_PATH_SEGMENT (SEMTREX_SYMBOL_LITERAL:HTTP_REQUEST_PATH_SEGMENT))))))");
*/


