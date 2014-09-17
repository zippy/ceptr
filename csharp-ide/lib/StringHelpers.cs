/*
Copyright (c) 2005, 2006 Marc Clifton
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list
  of conditions and the following disclaimer. 

* Redistributions in binary form must reproduce the above copyright notice, this 
  list of conditions and the following disclaimer in the documentation and/or other
  materials provided with the distribution. 
 
* Neither the name of Marc Clifton nor the names of its contributors may be
  used to endorse or promote products derived from this software without specific
  prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

using System;
using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;

namespace Clifton.Tools.Strings
{
	/// <summary>
	/// Helpers for string manipulation.
	/// </summary>
	public static class StringHelpers
	{
		/// <summary>
		/// Left of the first occurance of c
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">Return everything to the left of this character.</param>
		/// <returns>String to the left of c, or the entire string.</returns>
		public static string LeftOf(string src, char c)
		{
			string ret=src;

			int idx=src.IndexOf(c);

			if (idx != -1)
			{
				ret=src.Substring(0, idx);
			}

			return ret;
		}

		/// <summary>
		/// Left of the n'th occurance of c.
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">Return everything to the left n'th occurance of this character.</param>
		/// <param name="n">The occurance.</param>
		/// <returns>String to the left of c, or the entire string if not found or n is 0.</returns>
		public static string LeftOf(string src, char c, int n)
		{
			string ret=src;
			int idx=-1;
			
			while (n > 0)
			{
				idx=src.IndexOf(c, idx+1);
				
				if (idx==-1)
				{
					break;
				}
				
				--n;
			}
			
			if (idx != -1)
			{
				ret=src.Substring(0, idx);
			}
			
			return ret;
		}

		/// <summary>
		/// Right of the first occurance of c
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">The search char.</param>
		/// <returns>Returns everything to the right of c, or an empty string if c is not found.</returns>
		public static string RightOf(string src, char c)
		{
			string ret=String.Empty;
			int idx=src.IndexOf(c);
			
			if (idx != -1)
			{
				ret=src.Substring(idx+1);
			}
			
			return ret;
		}

		/// <summary>
		/// Returns all the text to the right of the specified string.
		/// Returns an empty string if the substring is not found.
		/// </summary>
		/// <param name="src"></param>
		/// <param name="substr"></param>
		/// <returns></returns>
		public static string RightOf(string src, string substr)
		{
			string ret = String.Empty;
			int idx = src.IndexOf(substr);

			if (idx != -1)
			{
				ret = src.Substring(idx + substr.Length);
			}

			return ret;
		}

		/// <summary>
		/// Returns the last character in the string.
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		public static char LastChar(string src)
		{
			return src[src.Length - 1];
		}

		/// <summary>
		/// Returns all but the last character of the source.
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		public static string RemoveLastChar(string src)
		{
			return src.Substring(0, src.Length - 1);
		}

		/// <summary>
		/// Right of the n'th occurance of c
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">The search char.</param>
		/// <param name="n">The occurance.</param>
		/// <returns>Returns everything to the right of c, or an empty string if c is not found.</returns>
		public static string RightOf(string src, char c, int n)
		{
			string ret=String.Empty;
			int idx=-1;
			
			while (n > 0)
			{
				idx=src.IndexOf(c, idx+1);
			
				if (idx==-1)
				{
					break;
				}
				
				--n;
			}

			if (idx != -1)
			{
				ret=src.Substring(idx+1);
			}

			return ret;
		}

		/// <summary>
		/// Returns everything to the left of the righmost char c.
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">The search char.</param>
		/// <returns>Everything to the left of the rightmost char c, or the entire string.</returns>
		public static string LeftOfRightmostOf(string src, char c)
		{
			string ret=src;
			int idx=src.LastIndexOf(c);
			
			if (idx != -1)
			{
				ret=src.Substring(0, idx);
			}
			
			return ret;
		}

		/// <summary>
		/// Returns everything to the right of the rightmost char c.
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="c">The seach char.</param>
		/// <returns>Returns everything to the right of the rightmost search char, or an empty string.</returns>
		public static string RightOfRightmostOf(string src, char c)
		{
			string ret=String.Empty;
			int idx=src.LastIndexOf(c);
			
			if (idx != -1)
			{
				ret=src.Substring(idx+1);
			}
			
			return ret;
		}

		/// <summary>
		/// Returns everything between the start and end chars, exclusive.
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="start">The first char to find.</param>
		/// <param name="end">The end char to find.</param>
		/// <returns>The string between the start and stop chars, or an empty string if not found.</returns>
		public static string Between(string src, char start, char end)
		{
			string ret=String.Empty;
			int idxStart=src.IndexOf(start);
			
			if (idxStart != -1)
			{
				++idxStart;
				int idxEnd=src.IndexOf(end, idxStart);
			
				if (idxEnd != -1)
				{
					ret=src.Substring(idxStart, idxEnd-idxStart);
				}
			}
			
			return ret;
		}

		public static string Between(string src, string start, string end)
		{
			string ret = String.Empty;
			int idxStart = src.IndexOf(start);

			if (idxStart != -1)
			{
				idxStart += start.Length;
				int idxEnd = src.IndexOf(end, idxStart);

				if (idxEnd != -1)
				{
					ret = src.Substring(idxStart, idxEnd - idxStart);
				}
			}

			return ret;
		}

		public static string BetweenEnds(string src, char start, char end)
		{
			string ret = String.Empty;
			int idxStart = src.IndexOf(start);

			if (idxStart != -1)
			{
				++idxStart;
				int idxEnd = src.LastIndexOf(end);

				if (idxEnd != -1)
				{
					ret = src.Substring(idxStart, idxEnd - idxStart);
				}
			}

			return ret;
		}

		/// <summary>
		/// Returns the number of occurances of "find".
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <param name="find">The search char.</param>
		/// <returns>The # of times the char occurs in the search string.</returns>
		public static int Count(string src, char find)
		{
			int ret=0;
			
			foreach(char s in src)
			{
				if (s==find)
				{
					++ret;
				}
			}
			
			return ret;
		}

		/// <summary>
		/// Returns the rightmost char in src.
		/// </summary>
		/// <param name="src">The source string.</param>
		/// <returns>The rightmost char, or '\0' if the source has zero length.</returns>
		public static char Rightmost(string src)
		{
			char c='\0';
			
			if (src.Length>0)
			{
				c=src[src.Length-1];
			}
			
			return c;
		}

		public static bool BeginsWith(string src, char c)
		{
			bool ret=false;

			if (src.Length > 0)
			{
				ret=src[0]==c;
			}

			return ret;
		}

		public static bool EndsWith(string src, char c)
		{
			bool ret=false;

			if (src.Length > 0)
			{
				ret=src[src.Length-1]==c;
			}

			return ret;
		}

		public static string EmptyStringAsNull(string src)
		{
			string ret = src;

			if (ret == String.Empty)
			{
				ret = null;
			}

			return ret;
		}

		public static string NullAsEmptyString(string src)
		{
			string ret = src;

			if (ret == null)
			{
				ret = String.Empty;
			}

			return ret;
		}

		public static bool IsNullOrEmpty(string src)
		{
			return ((src == null) || (src == String.Empty));
		}

		// Read about MD5 here: http://en.wikipedia.org/wiki/MD5
		public static string Hash(string src)
		{
			HashAlgorithm hashProvider = new MD5CryptoServiceProvider();
			byte[] bytes = Encoding.UTF8.GetBytes(src);
			byte[] encoded=hashProvider.ComputeHash(bytes);
			return Convert.ToBase64String(encoded);
		}

		/// <summary>
		/// Returns a camelcase string, where the first character is lowercase.
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		public static string CamelCase(string src)
		{
			return src[0].ToString().ToLower() + src.Substring(1).ToLower();
		}

		/// <summary>
		/// Returns a Pascalcase string, where the first character is uppercase.
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		public static string PascalCase(string src)
		{
			string ret = String.Empty;

			if (!String.IsNullOrEmpty(src))
			{
				ret = src[0].ToString().ToUpper() + src.Substring(1).ToLower();
			}

			return ret;
		}

		/// <summary>
		/// Returns a Pascal-cased string, given a string with words separated by spaces.
		/// </summary>
		/// <param name="src"></param>
		/// <returns></returns>
		public static string PascalCaseWords(string src)
		{
			StringBuilder sb = new StringBuilder();
			string[] s = src.Split(' ');
			string more=String.Empty;

			foreach (string s1 in s)
			{
				sb.Append(more);
				sb.Append(PascalCase(s1));
				more=" ";
			}

			return sb.ToString();
		}

		public static string SeparateCamelCase(string src)
		{
			StringBuilder sb = new StringBuilder();
			sb.Append(Char.ToUpper(src[0]));

			for (int i = 1; i < src.Length; i++)
			{
				char c = src[i];

				if (Char.IsUpper(c))
				{
					sb.Append(' ');
				}

				sb.Append(c);
			}

			return sb.ToString();
		}

        public static string[] Split(string source, char delimeter, char quoteChar)
        {
            List<string> retArray = new List<string>();
            int start = 0, end = 0;
            bool insideField = false;

            for (end = 0; end < source.Length; end++)
            {
                if (source[end] == quoteChar)
                {
                    insideField = !insideField;
                }
                else if (!insideField && source[end] == delimeter)
                {
                    retArray.Add(source.Substring(start, end - start));
                    start = end + 1;
                }
            }

            retArray.Add(source.Substring(start));

            return retArray.ToArray();
        }
	}
}
