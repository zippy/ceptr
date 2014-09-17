using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace Clifton.Tools.Strings.Extensions
{
	public static class StringHelpersExtensions
	{
		public static bool IsInt32(this String src)
		{
			int result;
			bool ret = Int32.TryParse(src, out result);

			return ret;
		}

		public static string ParseQuote(this String src)
		{
			return src.Replace("\"", "'");
		}

		public static string SingleQuote(this String src)
		{
			return "'" + src + "'";
		}

		public static string Quote(this String src)
		{
			return "\"" + src + "\"";
		}

		public static string Brace(this String src)
		{
			return "[" + src + "]";
		}

		public static string Between(this String src, char c1, char c2)
		{
			return Clifton.Tools.Strings.StringHelpers.Between(src, c1, c2);
		}

		public static string Between(this String src, string s1, string s2)
		{
			return src.RightOf(s1).LeftOf(s2);
		}

		/// <summary>
		/// Return a new string that is "around" (left of and right of) the specified string.
		/// Only the first occurance is processed.
		/// </summary>
		public static string Surrounding(this String src, string s)
		{
			return src.LeftOf(s) + src.RightOf(s);
		}

		public static string RightOf(this String src, char c)
		{
			return Clifton.Tools.Strings.StringHelpers.RightOf(src, c);
		}

		public static bool BeginsWith(this String src, string s)
		{
			return src.StartsWith(s);
		}

		public static string RightOf(this String src, string s)
		{
			string ret = String.Empty;
			int idx = src.IndexOf(s);

			if (idx != -1)
			{
				ret = src.Substring(idx + s.Length);
			}

			return ret;
		}

		public static string RightOfRightmostOf(this String src, char c)
		{
			return Clifton.Tools.Strings.StringHelpers.RightOfRightmostOf(src, c);
		}

		public static string LeftOf(this String src, char c)
		{
			return Clifton.Tools.Strings.StringHelpers.LeftOf(src, c);
		}

		public static string LeftOf(this String src, string s)
		{
			string ret = src;
			int idx = src.IndexOf(s);

			if (idx != -1)
			{
				ret = src.Substring(0, idx);
			}

			return ret;
		}

		public static string LeftOfRightmostOf(this String src, char c)
		{
			return Clifton.Tools.Strings.StringHelpers.LeftOfRightmostOf(src, c);
		}

		public static string LeftOfRightmostOf(this String src, string s)
		{
			string ret = src;
			int idx = src.IndexOf(s);
			int idx2 = idx;

			while (idx2 != -1)
			{
				idx2 = src.IndexOf(s, idx + s.Length);

				if (idx2 != -1)
				{
					idx = idx2;
				}
			}

			if (idx != -1)
			{
				ret = src.Substring(0, idx);
			}

			return ret;
		}

		public static string RightOfRightmostOf(this String src, string s)
		{
			string ret = src;
			int idx = src.IndexOf(s);
			int idx2 = idx;

			while (idx2 != -1)
			{
				idx2 = src.IndexOf(s, idx + s.Length);

				if (idx2 != -1)
				{
					idx = idx2;
				}
			}

			if (idx != -1)
			{
				ret = src.Substring(idx + s.Length, src.Length - (idx + s.Length));
			}

			return ret;
		}

		public static char Rightmost(this String src)
		{
			return Clifton.Tools.Strings.StringHelpers.Rightmost(src);
		}

		public static string TrimLastChar(this String src)
		{
			string ret = String.Empty;
			int len = src.Length;

			if (len > 1)
			{
				ret = src.Substring(0, len - 1);
			}

			return ret;
		}

		public static bool IsBlank(this string src)
		{
			return String.IsNullOrEmpty(src) || (src.Trim()==String.Empty);
		}

		/// <summary>
		/// Returns the first occurance of any token given the list of tokens.
		/// </summary>
		public static string Contains(this String src, string[] tokens)
		{
			string ret = String.Empty;
			int firstIndex=9999;

			// Find the index of the first index encountered.
			foreach (string token in tokens)
			{
				int idx = src.IndexOf(token);

				if ( (idx != -1) && (idx < firstIndex) )
				{
					ret = token;
					firstIndex = idx;
				}
			}

			return ret;
		}

		public static int to_i(this string src)
		{
			return Convert.ToInt32(src);
		}

		public static bool to_b(this string src)
		{
			return Convert.ToBoolean(src);
		}

		public static T ToEnum<T>(this string src)
		{
			T enumVal = (T)Enum.Parse(typeof(T), src);

			return enumVal;
		}

		public static string SafeToString(this Object src)
		{
			string ret = String.Empty;

			if (src != null)
			{
				ret = src.ToString();
			}

			return ret;
		}

		/// <summary>
		/// Returns a list of substrings separated by the specified delimiter,
		/// ignoring delimiters inside quotes.
		/// </summary>
		public static List<string> DelimitedSplit(this string src, char delimeter, char quote = '\"')
		{
			List<string> ret = new List<string>();
			int idx = 0;
			int start = 0;
			bool inQuote = false;

			while (idx < src.Length)
			{
				if ( (!inQuote) && (src[idx]==delimeter) )
				{
					ret.Add(src.Substring(start, idx - start).Trim());
					start = idx + 1;		// Ignore the comma.
				}

				if (src[idx] == quote)
				{
					inQuote = !inQuote;
				}

				++idx;
			}

			// The last part.
			if (!inQuote)
			{
				ret.Add(src.Substring(start, idx - start).Trim());
			}

			return ret;
		}

		public static string SplitCamelCase(this string input)
		{
			return Regex.Replace(input, "([A-Z])", " $1", System.Text.RegularExpressions.RegexOptions.Compiled).Trim();
		}

		/// <summary>
		/// Searches for all occurances of < > and removes everything between them.
		/// </summary>
		public static string StripHtml(this string s)
		{
			string ret = s;

			int idx1 = s.IndexOf('<');
			int idx2 = s.IndexOf('>');

			while (idx1 < idx2) 
			{
				s = s.LeftOf('<') + s.RightOf('>');
				idx1 = s.IndexOf('<');
				idx2 = s.IndexOf('>');
			} 

			return s;
		}
	}
}
