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
	public class SymbolOutputController : ViewController<SymbolOutputView>
	{
		public SymbolOutputController()
		{
		}

		public override void EndInit()
		{
			ApplicationController.SymbolOutputController = this;
			base.EndInit();
		}

		public void ShowSymbolDump(Symbol symbol)
		{
			Guid structures = ApplicationController.CeptrInterface.CreateRootNode(SystemSymbol.STRUCTURES);
			Guid symbols = ApplicationController.CeptrInterface.CreateRootNode(SystemSymbol.SYMBOLS);

			Dictionary<string, uint> structureMap = new Dictionary<string, uint>();
			uint topStructure = Recurse(structures, symbols, symbol, structureMap);
			ApplicationController.CeptrInterface.DeclareSymbol(symbols, topStructure, symbol.Name);

			string ret = ApplicationController.CeptrInterface.DumpSymbols(symbols, structures);
			View.Output.Text=FormatDump(ret);
		}

		// Recurse into a symbol, creating the child symbols first and then defining the structure composed of the child symbols.
		protected uint Recurse(Guid structures, Guid symbols, Symbol symbol, Dictionary<string, uint> structureMap)
		{
			List<uint> syms = new List<uint>();
			uint structure = GetStructureID(symbol.Structure, structureMap);
			
			foreach (Symbol child in symbol.Symbols)
			{
				// We need to recurse to get to native types, from which more complex symbols are constructed.
				Recurse(structures, symbols, child, structureMap);

				uint id = GetStructureID(child.Structure, structureMap);
				uint uchild = ApplicationController.CeptrInterface.DeclareSymbol(symbols, id, child.Name);
				syms.Add(uchild);
			}

			if (syms.Count > 0)
			{
				structure = ApplicationController.CeptrInterface.DefineStructure(structures, symbol.Structure, syms.ToArray());
				structureMap[symbol.Structure] = structure;
			}

			return structure;
		}

		protected uint GetStructureID(string structure, Dictionary<string, uint> structureMap)
		{
			uint id = 0;

			switch (structure.ToLower())
			{
				case "float":
					id = (UInt32)SystemStructure.FLOAT;
					break;

				case "string":
					id = (UInt32)SystemStructure.CSTRING;
					break;

				case "int":
					id = (UInt32)SystemStructure.INTEGER;
					break;

				default:
					structureMap.TryGetValue(structure, out id);
					break;
			}

			return id;
		}


			//uint latitude = ApplicationController.CeptrInterface.DeclareSymbol(symbols, (UInt32)SystemStructure.FLOAT, "latitude");
			//uint longitude = ApplicationController.CeptrInterface.DeclareSymbol(symbols, (UInt32)SystemStructure.FLOAT, "longitude");
			//uint latlong = ApplicationController.CeptrInterface.DefineStructure(structures, "latlong", new uint[] { latitude, longitude });

		/// <summary>
		/// Format the dump so that () are indented and separated on new lines.
		/// </summary>
		/// <param name="dump"></param>
		/// <returns></returns>
		protected string FormatDump(string dump)
		{
			StringBuilder sb = new StringBuilder();
			int idx = 0;
			int indent = 0;

			// Brute force approach.
			while (idx < dump.Length)
			{
				char c = dump[idx];

				switch (c)
				{
					case '(':
						sb.Append("\r\n");
						sb.Append(new string(' ', indent));
						sb.Append(c);
						indent += 2;
						sb.Append("\r\n");
						sb.Append(new string(' ', indent));
						break;
					case ')':
						sb.Append("\r\n");
						indent -= 2;
						sb.Append(new string(' ', indent));
						sb.Append(c);
						break;
					default:
						sb.Append(c);
						break;
				}

				++idx;
			}

			return sb.ToString();
		}
	}
}