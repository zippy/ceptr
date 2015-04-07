/*
    Copyright 2104 Higher Order Programming

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

// Copyright © 2010-2014 The CefSharp Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

using System.Windows.Forms;

using CefSharp;

namespace csharp_ide
{
	internal class DefaultFocusHandler : IFocusHandler
	{
		private readonly ChromiumWebBrowser browser;

		public DefaultFocusHandler(ChromiumWebBrowser browser)
		{
			this.browser = browser;
		}

		public void OnGotFocus()
		{
		}

		public bool OnSetFocus(CefFocusSource source)
		{
			return false;
		}

		public void OnTakeFocus(bool next)
		{
			browser.BeginInvoke(new MethodInvoker(() => browser.SelectNextControl(browser, next, true, true, true)));
		}
	}
}
