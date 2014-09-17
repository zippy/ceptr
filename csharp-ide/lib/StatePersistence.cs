using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

using Clifton.ExtensionMethods;

namespace Clifton.ApplicationStateManagement
{
	/// <summary>
	/// Persists the state of an item, associating the item's key with a value.
	/// </summary>
	public class State
	{
		public string Key { get; set; }
		public string Value { get; set; }

		public State()
		{
		}

		/// <summary>
		/// Instantiates a key-value.
		/// </summary>
		/// <param name="key">The item's key.</param>
		/// <param name="val">The item's value.</param>
		public State(string key, int val)
		{
			Key = key;
			Value = val.ToString();
		}

		/// <summary>
		/// Instantiates a key-value.
		/// </summary>
		/// <param name="key">The item's key.</param>
		/// <param name="val">The item's value.</param>
		public State(string key, string val)
		{
			Key = key;
			Value = val;
		}

		public State(string key, bool val)
		{
			Key = key;
			Value = val.ToString();
		}
	}

	/// <summary>
	/// Persists the state of all keys for an object.
	/// </summary>
	public class ObjectState
	{
		/// <summary>
		/// The object's key.
		/// </summary>
		public string StateObjectKey { get; set; }
		/// <summary>
		/// The key-values of all persistable items for the object key.
		/// </summary>
		public List<State> States { get; set; }

		public ObjectState()
		{
			States = new List<State>();
		}
	}

	/// <summary>
	/// Persists the state of all objects.
	/// </summary>
	public class ApplicationState
	{
		public List<ObjectState> ObjectStates { get; set; }

		public ApplicationState()
		{
			ObjectStates = new List<ObjectState>();
		}
	}

	/// <summary>
	/// State get/set functions to be associated with a specific stateable object.
	/// </summary>
	internal class SaveAndRestoreActions
	{
		public Action<List<State>> SetState { get; set; }
		public Func<List<State>> GetState { get; set; }
	}

	/// <summary>
	/// State persistence is implemented with this class, allowing objects
	/// to register themselves and persist object-specific key-values.
	/// </summary>
	public class StatePersistence
	{
		private Dictionary<string, SaveAndRestoreActions> stateObjects;
		private ApplicationState appState;

		public StatePersistence()
		{
			stateObjects = new Dictionary<string, SaveAndRestoreActions>();
		}

		/// <summary>
		/// Registers an object as state persistable.
		/// </summary>
		/// <param name="objectKey">The object's key.  This must be unique.</param>
		public void Register(string objectKey, Func<List<State>> getState, Action<List<State>> setState, bool replace=false)
		{
			if (!replace)
			{
				stateObjects.ContainsKey(objectKey).Then(() => { throw new ApplicationException("The instance " + objectKey + " has already been registered as persistable."); });
			};

			stateObjects[objectKey] = new SaveAndRestoreActions() { GetState = getState, SetState = setState };
		}

		/// <summary>
		/// Loads the application states from the specified file.
		/// </summary>
		public void ReadState(string filename)
		{
			if (File.Exists(filename))
			{
				XmlSerializer xs = new XmlSerializer(typeof(ApplicationState));
				XmlTextReader xtr = new XmlTextReader(filename);
				appState = xs.Deserialize(xtr) as ApplicationState;
				xtr.Close();
			}
		}

		/// <summary>
		/// Writes the application states to the specified file.
		/// </summary>
		public void WriteState(string filename)
		{
			XmlSerializer xs = new XmlSerializer(typeof(ApplicationState));
			XmlTextWriter xtr = new XmlTextWriter(filename, Encoding.UTF8);
			xs.Serialize(xtr, appState);
			xtr.Close();
		}

		/// <summary>
		/// Save the state of all persistable objects.
		/// </summary>
		public void SaveAllStates()
		{
			appState = new ApplicationState();

			stateObjects.ForEach(kvp =>
				{
					ObjectState objState = new ObjectState();
					objState.StateObjectKey = kvp.Key;
					objState.States = kvp.Value.GetState();
					appState.ObjectStates.Add(objState);
				});
		}

		/// <summary>
		/// Saves the state of the specified object key.
		/// </summary>
		public void SaveState(string objectKey)
		{
			SaveAndRestoreActions stateObject = stateObjects[objectKey];
			ObjectState objState = new ObjectState();
			objState.StateObjectKey = objectKey;
			objState.States = stateObject.GetState();
			appState.ObjectStates = (List<ObjectState>)appState.ObjectStates.Replace(objState, (a, b) => (a.StateObjectKey == b.StateObjectKey));
		}

		/// <summary>
		/// Restores the state of all persistable objects.
		/// </summary>
		public void RestoreAllStates(string fileName)
		{
			stateObjects.ForEach(kvp =>
				{
					ObjectState objState = appState.ObjectStates.SingleOrDefault(t => t.StateObjectKey == kvp.Key);
					objState.IfNotNull(t =>
						{
							stateObjects[kvp.Key].SetState(t.States);
						});
				});
		}

		/// <summary>
		/// Restore the state of a specific object key.
		/// </summary>
		public void RestoreState(string objectKey)
		{
			ObjectState objState = appState.ObjectStates.SingleOrDefault(t => t.StateObjectKey == objectKey);
			objState.IfNotNull(t =>
			{
				stateObjects[objectKey].SetState(t.States);
			});
		}

		/// <summary>
		/// Returns the state of a specific object and item key.  Useful for single item objects, such as "last file opened."
		/// </summary>
		public string GetState(string objectKey, string itemKey)
		{
			ObjectState objState = appState.ObjectStates.SingleOrDefault(t => t.StateObjectKey == objectKey);
			string ret = objState.States.Single(t=>t.Key==itemKey).Value;

			return ret;

		}
	}
}
