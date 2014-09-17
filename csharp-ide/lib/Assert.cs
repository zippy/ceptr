using System;

namespace Clifton.Assertions
{
	public static class Assert
	{
		public static string ErrorMessage = null;

		public static void ClearErrorMessage() { ErrorMessage = String.Empty; }

		public static void That(bool mustBeTrue, string message)
		{
			if (!mustBeTrue)
			{
				throw new ApplicationException(message);
			}
		}

		public static void IsNotNull(object obj, string message)
		{
			if (obj == null)
			{
				throw new ApplicationException(message);
			}
		}

		public static string LastErrorMessage(string exceptionMessage)
		{
			return (ErrorMessage == null ? exceptionMessage : ErrorMessage);
		}

		public static void SilentTry(Action f)
		{
			try
			{
				f();
			}
			catch (Exception ex)
			{
				// Silent catching of exceptions
				while (ex.InnerException != null) ex = ex.InnerException;
				System.Diagnostics.Debug.WriteLine(ex.Message);
			}
		}

	}
}
