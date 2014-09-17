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
 
* Neither the name Marc Clifton nor the names of its contributors may be
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
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Reflection;
using System.Xml;

using Clifton.ExtensionMethods;
using Clifton.Tools.Strings;

namespace Clifton.MycroParser
{
	public delegate void InstantiateClassDlgt(object sender, ClassEventArgs cea);
	public delegate void AssignPropertyDlgt(object sender, PropertyEventArgs pea);
	public delegate void UnknownPropertyDlgt(object sender, UnknownPropertyEventArgs pea);
	public delegate void CustomAssignPropertyDlgt(object sender, CustomPropertyEventArgs pea);
	public delegate void AssignEventDlgt(object sender, EventEventArgs eea);
	public delegate void SupportInitializeDlgt(object sender, SupportInitializeEventArgs siea);
	public delegate void AddToCollectionDlgt(object sender, CollectionEventArgs cea);
	public delegate void UseReferenceDlgt(object sender, UseReferenceEventArgs urea);
	public delegate void AssignReferenceDlgt(object sender, AssignReferenceEventArgs area);
	public delegate void CommentDlgt(object sender, CommentEventArgs cea);

	public interface IMycroParserInstantiatedObject
	{
		Dictionary<string, object> ObjectCollection { get; set; }
	}

	public class HandledEventArgs : EventArgs
	{
		protected bool handled;

		public bool Handled
		{
			get { return handled; }
			set { handled = value; }
		}
	}

	public class ClassEventArgs : HandledEventArgs
	{
		protected Type t;
		protected XmlNode node;
		protected object result;

		public Type Type
		{
			get { return t; }
		}

		public XmlNode Node
		{
			get { return node; }
		}

		public object Result
		{
			get { return result; }
			set { result = value; }
		}

		public ClassEventArgs(Type t, XmlNode node)
		{
			this.t = t;
			this.node = node;
			result = null;
			handled = false;
		}
	}

	public class PropertyEventArgs : HandledEventArgs
	{
		protected PropertyInfo pi;
		protected object src;
		protected object val;
		protected string valStr;

		public PropertyInfo PropertyInfo
		{
			get { return pi; }
		}

		public object Source
		{
			get { return src; }
		}

		public object Value
		{
			get { return val; }
		}

		public string AsString
		{
			get { return valStr; }
		}

		public PropertyEventArgs(PropertyInfo pi, object src, object val, string valStr)
		{
			this.pi = pi;
			this.src = src;
			this.val = val;
			this.valStr = valStr;
			handled = false;
		}
	}

	public class CustomPropertyEventArgs : HandledEventArgs
	{
		protected PropertyInfo pi;
		protected object src;
		protected object val;

		public PropertyInfo PropertyInfo
		{
			get { return pi; }
		}

		public object Source
		{
			get { return src; }
		}

		public object Value
		{
			get { return val; }
		}

		public CustomPropertyEventArgs(PropertyInfo pi, object src, object val)
		{
			this.pi = pi;
			this.src = src;
			this.val = val;
		}
	}

	public class UnknownPropertyEventArgs : HandledEventArgs
	{
		protected string propertyName;
		protected object src;
		protected string propertyValue;

		public string PropertyName
		{
			get { return propertyName; }
		}

		public object Source
		{
			get { return src; }
		}

		public string PropertyValue
		{
			get { return propertyValue; }
		}

		public UnknownPropertyEventArgs(string pname, object src, string pvalue)
		{
			this.propertyName = pname;
			this.src = src;
			this.propertyValue = pvalue;
		}
	}

	public class EventEventArgs : HandledEventArgs
	{
		protected EventInfo ei;
		protected object ret;
		protected object sink;
		protected string srcName;
		protected string methodName;

		public EventInfo EventInfo
		{
			get { return ei; }
		}

		public object Return
		{
			get { return ret; }
		}

		public object Sink
		{
			get { return sink; }
		}

		public string SourceName
		{
			get { return srcName; }
		}

		public string MethodName
		{
			get { return methodName; }
		}

		public EventEventArgs(EventInfo ei, object ret, object sink, string srcName, string methodName)
		{
			this.ei = ei;
			this.ret = ret;
			this.sink = sink;
			this.srcName = srcName;
			this.methodName = methodName;
		}
	}

	public class SupportInitializeEventArgs : HandledEventArgs
	{
		protected Type t;
		protected object obj;

		public object Object
		{
			get { return obj; }
		}

		public Type Type
		{
			get { return t; }
		}

		public SupportInitializeEventArgs(Type t, object obj)
		{
			this.t = t;
			this.obj = obj;
		}
	}

	public class CollectionEventArgs : HandledEventArgs
	{
		protected PropertyInfo pi;
		protected Type instanceType;
		protected Type parentType;

		public PropertyInfo PropertyInfo
		{
			get { return pi; }
		}

		public Type InstanceType
		{
			get { return instanceType; }
		}

		public Type ParentType
		{
			get { return parentType; }
		}

		public CollectionEventArgs(PropertyInfo pi, Type instanceType, Type parentType)
		{
			this.pi = pi;
			this.instanceType = instanceType;
			this.parentType = parentType;
		}
	}

	public class UseReferenceEventArgs : HandledEventArgs
	{
		protected Type t;
		protected string refName;
		protected object ret;

		public Type Type
		{
			get { return t; }
		}

		public string RefName
		{
			get { return refName; }
		}

		public object Return
		{
			get { return ret; }
			set { ret = value; }
		}

		public UseReferenceEventArgs(Type t, string refName)
		{
			this.t = t;
			this.refName = refName;
			ret = null;
		}
	}

	public class AssignReferenceEventArgs : HandledEventArgs
	{
		protected PropertyInfo pi;
		protected string refName;
		protected object obj;

		public PropertyInfo PropertyInfo
		{
			get { return pi; }
		}

		public string RefName
		{
			get { return refName; }
		}

		public object Object
		{
			get { return obj; }
		}

		public AssignReferenceEventArgs(PropertyInfo pi, string refName, object obj)
		{
			this.pi = pi;
			this.refName = refName;
			this.obj = obj;
		}
	}

	public class CommentEventArgs
	{
		protected string comment;

		public string Comment
		{
			get { return comment; }
		}

		public CommentEventArgs(string comment)
		{
			this.comment = comment;
		}
	}

	//	public interface IMycroXaml
	//	{
	//		void Initialize(object parent);
	//		object ReturnedObject
	//		{
	//			get;
	//		}
	//	}

	public class MycroParser
	{
		protected List<Tuple<Type, object>> objectsToEndInit;
		protected Dictionary<string, object> nsMap;
		protected Dictionary<string, object> objectCollection;
		protected object eventSink;
		protected XmlNode baseNode;

		public event InstantiateClassDlgt InstantiateClass;
		public event AssignPropertyDlgt AssignProperty;
		public event UnknownPropertyDlgt UnknownProperty;
		public event CustomAssignPropertyDlgt CustomAssignProperty;
		public event AssignEventDlgt AssignEvent;
		public event SupportInitializeDlgt BeginInitCheck;
		public event SupportInitializeDlgt EndInitCheck;
		public event EventHandler EndChildProcessing;
		public event AddToCollectionDlgt AddToCollection;
		public event UseReferenceDlgt UseReference;
		public event AssignReferenceDlgt AssignReference;
		public event CommentDlgt Comment;

		public Dictionary<string, object> NamespaceMap
		{
			get { return nsMap; }
		}

		public Dictionary<string, object> ObjectCollection
		{
			get { return objectCollection; }
		}

		public MycroParser()
		{
			objectCollection = new Dictionary<string, object>();
			objectsToEndInit = new List<Tuple<Type, object>>();
		}

		public static T InstantiateFromFile<T>(string filename, Action<MycroParser> AddInstances=null, object eventSink=null)
		{
			MycroParser mp = new MycroParser();
			AddInstances.IfNotNull(t => t(mp));
			XmlDocument doc = new XmlDocument();
			doc.Load(filename);
			mp.Load(doc, "Form", eventSink);
			T obj = (T)mp.Process();

			// Pass object collection to the instantiated class if it implements IMycroParserInstantiatedObject.
			if (obj is IMycroParserInstantiatedObject)
			{
				((IMycroParserInstantiatedObject)obj).ObjectCollection = mp.ObjectCollection;
			}

			return obj;
		}

		public T Load<T>(string filename, object eventSink)
		{
			XmlDocument doc = new XmlDocument();
			doc.Load(filename);
			Load(doc, "Form", eventSink);
			T obj = (T)Process();

			// Pass object collection to the instantiated class if it implements IMycroParserInstantiatedObject.
			if (obj is IMycroParserInstantiatedObject)
			{
				((IMycroParserInstantiatedObject)obj).ObjectCollection = ObjectCollection;
			}

			return obj;
		}

		public void Load(XmlDocument doc, string objectName, object eventSink)
		{
			this.eventSink = eventSink;

			XmlNode node;

			if (objectName != null)
			{
				node = doc.SelectSingleNode("//MycroXaml[@Name='" + objectName + "']");
				Trace.Assert(node != null, "Couldn't find MycroXaml element " + objectName);
				Trace.Assert(node.ChildNodes.Count <= 1, "Only one child of the root is allowed.");
				// The last valid node instantiated is returned.
				// The xml root should only have one child.
				ProcessNamespaces(node);

				if (node.ChildNodes.Count == 1)
				{
					baseNode = node.ChildNodes[0];
				}
			}
			else
			{
				node = doc.DocumentElement;
				baseNode = node;
				ProcessNamespaces(node);
			}
		}

		public object Process()
		{
			object ret = null;

			if (baseNode != null)
			{
				Type t;
				ret = ProcessNode(baseNode, null, out t);
			}

			DoEndInit();

			return ret;
		}

		public bool HasInstance(string name)
		{
			return objectCollection.ContainsKey(name);
		}

		public object GetInstance(string name)
		{
			Trace.Assert(objectCollection.ContainsKey(name), "The object collection does not have an entry for " + name);
			return objectCollection[name];
		}

		public void AddInstance(string name, object obj)
		{
			// We don't care if we overwrite an existing object.
			objectCollection[name] = obj;
		}

		protected void DoEndInit()
		{
			foreach (var endInit in objectsToEndInit)
			{
				OnEndInitCheck(endInit.Item1, endInit.Item2);
			}
		}

		protected void ProcessNamespaces(XmlNode node)
		{
			nsMap = new Dictionary<string, object>();

			foreach (XmlAttribute attr in node.Attributes)
			{
				if (attr.Prefix == "xmlns")
				{
					nsMap[attr.LocalName] = attr.Value;
				}
			}
		}

		protected virtual object OnInstantiateClass(Type t, XmlNode node)
		{
			object ret = null;
			ClassEventArgs args = new ClassEventArgs(t, node);

			if (InstantiateClass != null)
			{
				InstantiateClass(this, args);
				ret = args.Result;
			}

			if (!args.Handled)
			{
				ret = Activator.CreateInstance(t);
			}

			return ret;
		}

		protected virtual void OnAssignProperty(PropertyInfo pi, object ret, object val, string origVal)
		{
			PropertyEventArgs args = new PropertyEventArgs(pi, ret, val, origVal);

			if (AssignProperty != null)
			{
				AssignProperty(this, args);
			}

			if (!args.Handled)
			{
				pi.SetValue(ret, val, null);
			}
		}

		protected virtual bool OnCustomAssignProperty(PropertyInfo pi, object ret, object val)
		{
			CustomPropertyEventArgs args = new CustomPropertyEventArgs(pi, ret, val);

			if (CustomAssignProperty != null)
			{
				CustomAssignProperty(this, args);
			}

			return args.Handled;
		}

		protected virtual bool OnUnknownProperty(string pname, object ret, string pvalue)
		{
			UnknownPropertyEventArgs args = new UnknownPropertyEventArgs(pname, ret, pvalue);

			if (UnknownProperty != null)
			{
				UnknownProperty(this, args);
			}

			return args.Handled;
		}

		protected virtual void OnAssignEvent(EventInfo ei, object ret, object sink, string srcName, string methodName)
		{
			EventEventArgs args = new EventEventArgs(ei, ret, sink, srcName, methodName);

			if (AssignEvent != null)
			{
				AssignEvent(this, args);
			}

			if (!args.Handled)
			{
				Delegate dlgt = null;

				try
				{
					MethodInfo mi = sink.GetType().GetMethod(methodName, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.Static);
					dlgt = Delegate.CreateDelegate(ei.EventHandlerType, sink, mi.Name);
				}
				catch (Exception e)
				{
					Trace.Fail("Couldn't create a delegate for the event " + srcName + "." + methodName + ":\r\n" + e.Message);
				}

				try
				{
					ei.AddEventHandler(ret, dlgt);
				}
				catch (Exception e)
				{
					Trace.Fail("Binding to event " + ei.Name + " failed: " + e.Message);
				}
			}
		}

		protected virtual void OnBeginInitCheck(Type t, object obj)
		{
			SupportInitializeEventArgs args = new SupportInitializeEventArgs(t, obj);

			if (BeginInitCheck != null)
			{
				BeginInitCheck(this, args);
			}

			if (!args.Handled)
			{
				// support the ISupportInitialize interface
				if (obj is ISupportInitialize)
				{
					((ISupportInitialize)obj).BeginInit();
				}
			}
		}

		protected virtual void OnEndInitCheck(Type t, object obj)
		{
			SupportInitializeEventArgs args = new SupportInitializeEventArgs(t, obj);

			if (EndInitCheck != null)
			{
				EndInitCheck(this, args);
			}

			if (!args.Handled)
			{
				// support the ISupportInitialize interface
				if (obj is ISupportInitialize)
				{
					((ISupportInitialize)obj).EndInit();
				}
			}
		}

		protected virtual void OnEndChildProcessing()
		{
			if (EndChildProcessing != null)
			{
				EndChildProcessing(this, EventArgs.Empty);
			}
		}

		protected virtual object OnUseReference(Type t, string refVar)
		{
			object ret = null;

			UseReferenceEventArgs args = new UseReferenceEventArgs(t, refVar);

			if (UseReference != null)
			{
				UseReference(this, args);
			}

			if (!args.Handled)
			{
				if (HasInstance(refVar))
				{
					ret = GetInstance(refVar);
				}
			}
			else
			{
				ret = args.Return;
			}

			return ret;
		}

		protected virtual void OnAssignReference(PropertyInfo pi, string refName, object obj)
		{
			AssignReferenceEventArgs args = new AssignReferenceEventArgs(pi, refName, obj);

			if (AssignReference != null)
			{
				AssignReference(this, args);
			}

			if (!args.Handled)
			{
				object val = GetInstance(refName);

				try
				{
					pi.SetValue(obj, val, null);
				}
				catch (Exception e)
				{
					if (!OnCustomAssignProperty(pi, obj, val))
					{
						Trace.Fail("Couldn't set property " + pi.Name + " to an instance of " + refName + ":\r\n" + e.Message);
					}
				}
			}
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="pi">The PropertyInfo of the collection property.</param>
		/// <param name="propObject">The instance of the collection.</param>
		/// <param name="obj">The instance to add to the collection.</param>
		/// <param name="t">The instance type (being added to the collection).</param>
		/// <param name="parentType">The parent type.</param>
		/// <param name="parent">The parent instance.</param>
		protected virtual void OnAddToCollection(PropertyInfo pi, object propObject, object obj, Type t, Type parentType, object parent)
		{
			CollectionEventArgs args = new CollectionEventArgs(pi, t, parentType);

			if (AddToCollection != null)
			{
				AddToCollection(this, args);
			}

			if (!args.Handled)
			{
				// A null return is valid in cases where a class implementing the IMicroXaml interface
				// might want to take care of managing the instance it creates itself.  See DataBinding
				if (obj != null)
				{

					// support for ICollection and IList objects.
					// If the object is a collection or list, then even if it's writeable, treat as a list.
					if ( (!pi.CanWrite) || (propObject is ICollection) || (propObject is IList) )
					{
						if (propObject is ICollection)
						{
							MethodInfo mi = propObject.GetType().GetMethod("Add", new Type[] { obj.GetType() });

							if (mi != null)
							{
								try
								{
									mi.Invoke(propObject, new object[] { obj });
								}
								catch (Exception e)
								{
									Trace.Fail("Adding to collection failed:\r\n" + e.Message);
								}
							}
							else if (propObject is IList)
							{
								try
								{
									((IList)propObject).Add(obj);
								}
								catch (Exception e)
								{
									Trace.Fail("List/Collection add failed:\r\n" + e.Message);
								}
							}
						}
						else
						{
							Trace.Fail("Unsupported read-only property: " + pi.Name);
						}
					}
					else
					{
						// direct assignment if not a collection
						try
						{
							pi.SetValue(parent, obj, null);
						}
						catch (Exception e)
						{
							Trace.Fail("Property setter for " + pi.Name + " failed:\r\n" + e.Message);
						}
					}
				}
			}
		}

		protected virtual void OnComment(string text)
		{
			if (Comment != null)
			{
				CommentEventArgs args = new CommentEventArgs(text);
				Comment(this, args);
			}
		}

		protected object ProcessNode(XmlNode node, object parent, out Type t)
		{
			t = null;
			object ret = null;

			// Special case for String objects
			if (node.LocalName == "String")
			{
				return node.InnerText;
			}

			bool useRef = false;
			int attributeCount = 0;

			string ns = node.Prefix;
			string cname = node.LocalName;

			Trace.Assert(nsMap.ContainsKey(ns), "Namespace '" + ns + "' has not been declared.");
			string asyName = (string)nsMap[ns];
			string qname = StringHelpers.LeftOf(asyName, ',') + "." + cname + ", " + StringHelpers.RightOf(asyName, ',');
			t = Type.GetType(qname, false);
			Trace.Assert(t != null, "Type " + qname + " could not be determined.");

			// Do ref:Name check here and call OnReferenceInstance if appropriate.
			if (node.Attributes != null)
			{
				attributeCount = node.Attributes.Count;

				if (attributeCount > 0)
				{
					// We're making a blatant assumption that the ref:Name is going to be
					// the first attribute in the node.
					if (node.Attributes[0].Name == "ref:Name")
					{
						string refVar = node.Attributes[0].Value;
						ret = OnUseReference(t, refVar);
						useRef = true;
					}
				}
			}

			if (!useRef)
			{
				// instantiate the class
				try
				{
					ret = OnInstantiateClass(t, node);
					AddToInstanceCollection(node, ret);		// Allows for reference of the node by any child nodes.
				}
				catch (Exception e)
				{
					while (e.InnerException != null)
					{
						e = e.InnerException;
					}

					Trace.Fail("Type " + qname + " could not be instantiated:\r\n" + e.Message);
				}
			}

			// Optimization, to remove SuspendLayout followed by ResumeLayout when no 
			// properties are being set (the ref only has a Name attribute).
			// If the referenced object has additional properties that have been set (attributeCount > 1, as the first attribute is the ref:),
			// then we call EndInit again because the object might need to do initialization with the attribute values that have now been assigned.
			// Unfortunately, we leave it up to the object to determine how to handle potential multiple inits!
			if (!useRef)
			{
				OnBeginInitCheck(t, ret);
			}

			// If the instance implements the IMicroXaml interface, then it may need 
			// access to the parser.
			//				if (ret is IMycroXaml)
			//				{
			//					((IMycroXaml)ret).Initialize(parent);
			//				}

			// TODO: Here we process attributes first, as some WinForm attributes, like Anchor, on child controls,
			// requires that the parent attributes (such as Size) have already been set!!!
			string refName = ProcessAttributes(node, ret, t);

			// implements the class-property-class model
			ProcessChildProperties(node, ret, t);

			OnEndChildProcessing();

			// TODO: ProcessAttributes was here.

			// Optimization, to remove SuspendLayout followed by ResumeLayout when no 
			// properties are being set (the ref only has a Name attribute).
			// If the referenced object has additional properties that have been set (attributeCount > 1, as the first attribute is the ref:),
			// then we call EndInit again because the object might need to do initialization with the attribute values that have now been assigned.
			// Unfortunately, we leave it up to the object to determine how to handle potential multiple inits!
			if (!useRef)
			{
				objectsToEndInit.Add(new Tuple<Type, object>(t, ret));
			}

			// If the instance implements the IMicroXaml interface, then it has the option
			// to return an object that replaces the instance created by the parser.
			//				if (ret is IMycroXaml)
			//				{
			//					ret=((IMycroXaml)ret).ReturnedObject;
			//					
			//					if ( (ret != null) && (refName != String.Empty) )
			//					{
			//						AddInstance(refName, ret);
			//					}
			//				}

			return ret;
		}

		protected void ProcessChildProperties(XmlNode node, object parent, Type parentType)
		{
			Type t;
			object obj;

			// children of a class must always be properties
			foreach (XmlNode child in node.ChildNodes)
			{
				if (child is XmlComment)
				{
					OnComment(child.Value);
				}
				else
					if (child is XmlElement)
					{
						string pname = child.LocalName;
						PropertyInfo pi = parentType.GetProperty(pname); //, BindingFlags.FlattenHierarchy | BindingFlags.Public | BindingFlags.NonPublic);

						if ((pi == null)) // || (node.Prefix != child.Prefix))
						{
							// Special case--we're going to assume that the child is a class instance
							// not associated with the parent object
							ProcessNode(child, null, out t);
							continue;
						}

						// a property can only have one child node unless it's a collection
						foreach (XmlNode grandChild in child.ChildNodes)
						{
							if (grandChild is XmlComment)
							{
								OnComment(grandChild.Value);
							}
							else
								if (grandChild is XmlElement)
								{
									object propObject = null;

									if (parent != null)
									{
										propObject = pi.GetValue(parent, null);

										if (propObject == null)
										{
											// The grandChild type is a property of the child, which is itself a property/instance of the parent.
											// Instantiate the child and assign it to the parent, then process the grandchild as a collection property.
											// TODO: This could be iterated more levels!
											obj = ProcessNode(child, null, out t);
											pi.SetValue(parent, obj, null);
											continue;
										}
									}

									obj = ProcessNode(grandChild, propObject, out t);
									OnAddToCollection(pi, propObject, obj, t, parentType, parent);
								}
						}
					}
			}
		}

		protected void AddToInstanceCollection(XmlNode node, object ret)
		{
			foreach (XmlAttribute attr in node.Attributes)
			{
				string pname = attr.Name;
				string pvalue = attr.Value;

				// auto-add to our object collection
				if ((pname == "Name") || (pname == "def:Name"))
				{
					AddInstance(pvalue, ret);
				}
			}
		}


		protected string ProcessAttributes(XmlNode node, object ret, Type t)
		{
			string refName = String.Empty;

			// process attributes
			foreach (XmlAttribute attr in node.Attributes)
			{
				string pname = attr.Name;
				string pvalue = attr.Value;

				// it's either a property or an event.  Allow assignment to protected / private properties.
				PropertyInfo pi = t.GetProperty(pname, BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
				EventInfo ei = t.GetEvent(pname);

				if (pi != null)
				{
					// it's a property!
					if (pvalue.StartsWith("{") && pvalue.EndsWith("}"))
					{
						// And the value is a reference to an instance!
						// Get the referenced object.  Late binding is not supported!
						OnAssignReference(pi, StringHelpers.Between(pvalue, '{', '}'), ret);
					}
					else
					{
						// it's string, so use a type converter.
						if (pi.PropertyType.FullName == "System.Object")
						{
							OnAssignProperty(pi, ret, pvalue, pvalue);
						}
						else
						{
							TypeConverter tc = TypeDescriptor.GetConverter(pi.PropertyType);

							if (tc.CanConvertFrom(typeof(string)))
							{
								object val = tc.ConvertFrom(pvalue);

								try
								{
									OnAssignProperty(pi, ret, val, pvalue);
								}
								catch (Exception e)
								{
									Trace.Fail("Property setter for " + pname + " failed:\r\n" + e.Message);
								}
							}
							else
							{
								if (!OnCustomAssignProperty(pi, ret, pvalue))
								{
									Trace.Fail("Property setter for " + pname + " cannot be converted to property type " + pi.PropertyType.FullName + ".");
								}
							}
						}
					}

					// auto-add to our object collection
					if ((pname == "Name") || (pname == "def:Name"))
					{
						refName = pvalue;
						// AddInstance(pvalue, ret);
					}
				}
				else if (ei != null)
				{
					// it's an event!
					string src = pvalue;
					string methodName = String.Empty;
					object sink = eventSink;

					if ((StringHelpers.BeginsWith(src, '{')) && (StringHelpers.EndsWith(src, '}')))
					{
						src = StringHelpers.Between(src, '{', '}');
					}

					if (src.IndexOf('.') != -1)
					{
						string[] handler = src.Split('.');
						src = handler[0];
						methodName = handler[1];
						sink = GetInstance(src);
					}
					else
					{
						methodName = src;
					}

					OnAssignEvent(ei, ret, sink, src, methodName);
				}
				else
				{
					// auto-add to our object collection
					if ((pname == "Name") || (pname == "def:Name"))
					{
						refName = pvalue;
						// AddInstance(pvalue, ret);
					}
					else if (pname == "ref:Name")
					{
						// Do nothing.
					}
					else
					{
						if (!OnUnknownProperty(pname, ret, pvalue))
						{
							// who knows what it is???
							Trace.Fail("Failed acquiring property information for '" + pname + "' with value '"+pvalue+"'");
						}
					}
				}
			}
			return refName;
		}
	}
}
