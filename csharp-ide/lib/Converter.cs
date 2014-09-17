/*
Copyright (c) 2005, Marc Clifton
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list
  of conditions and the following disclaimer. 

* Redistributions in binary form must reproduce the above copyright notice, this 
  list of conditions and the following disclaimer in the documentation and/or other
  materials provided with the distribution. 
 
* Neither the name of MyXaml nor the names of its contributors may be
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
using System.Data;
using System.ComponentModel;
using System.Reflection;

namespace Clifton.Tools.Data
{
	public class ConverterException : ApplicationException
	{
		public ConverterException(string msg)
			: base(msg)
		{
		}
	}

	public class Converter
	{
		public static object Convert(object src, Type destType)
		{
			object ret = src;

			if ((src != null) && (src != DBNull.Value))
			{
				Type srcType = src.GetType();

				if ((srcType.FullName == "System.Object") || (destType.FullName == "System.Object"))
				{
					ret = src;
				}
				else
				{
					if (srcType != destType)
					{
						TypeConverter tcSrc = TypeDescriptor.GetConverter(srcType);
						TypeConverter tcDest = TypeDescriptor.GetConverter(destType);

						if (tcSrc.CanConvertTo(destType))
						{
							ret = tcSrc.ConvertTo(src, destType);
						}
						else if (tcDest.CanConvertFrom(srcType))
						{
							if (srcType.FullName == "System.String")
							{
								ret = tcDest.ConvertFromInvariantString((string)src);
							}
							else
							{
								ret = tcDest.ConvertFrom(src);
							}
						}
						else
						{
							// If no conversion exists, throw an exception.
							throw new ConverterException("Can't convert from " + src.GetType().FullName + " to " + destType.FullName);
						}
					}
				}
			}
			else if (src == DBNull.Value)
			{
				if (destType.FullName == "System.String")
				{
					// convert DBNull.Value to null for strings.
					ret = null;
				}
			}

			return ret;
		}
	}
}
