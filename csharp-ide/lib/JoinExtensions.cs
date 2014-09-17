using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

// Code here is from the excellent article: http://www.codeproject.com/Articles/488643/LinQ-Extended-Joins

/*
Example of a join using Linq's Join method:  This is an inner join, where only the intersection of A & B is returned.
  
Linq:

var result = from p in Person.BuiltPersons()
             join a in Address.BuiltAddresses()
             on p.IdAddress equals a.IdAddress
             select new 
	   { 
                 Name             = a.MyPerson.Name,
                 Age              = a.MyPerson.Age,
                 PersonIdAddress  = a.MyPerson.IdAddress,
                 AddressIdAddress = a.MyAddress.IdAddress,
                 Street           = a.MyAddress.Street
	   };  

Lambda:

var resultJoint = Person.BuiltPersons().Join(                      /// Source Collection
                  Address.BuiltAddresses(),                        /// Inner Collection
                  p => p.IdAddress,                                /// PK
                  a => a.IdAddress,                                /// FK
                  (p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
                  .Select(a => new
                    {
                        Name             = a.MyPerson.Name,
                        Age              = a.MyPerson.Age,
                        PersonIdAddress  = a.MyPerson.IdAddress,
                        AddressIdAddress = a.MyAddress.IdAddress,
                        Street           = a.MyAddress.Street
                    });  */

namespace Clifton.Tools.Data
{
	public static class JoinExtensions
	{
		/*
		Lambda:

		var resultJoint = Person.BuiltPersons().LeftJoin(                    /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name             = a.MyPerson.Name,
								Age              = a.MyPerson.Age,
								PersonIdAddress  = a.MyPerson.IdAddress,
								AddressIdAddress = (a.MyAddress != null ? a.MyAddress.IdAddress : -1),
					Street           = (a.MyAddress != null ? a.MyAddress.Street    : "Null-Value")
							}); 
		*/
		/// <summary>
		/// Returns all of A with null values for B if B doesn't exist.
		/// </summary>
		public static IEnumerable<TResult> LeftJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																						 IEnumerable<TInner> inner,
																						 Func<TSource, TKey> pk,
																						 Func<TInner, TKey> fk,
																						 Func<TSource, TInner, TResult> result)
		{
			IEnumerable<TResult> _result = Enumerable.Empty<TResult>();

			_result = from s in source
					  join i in inner
					  on pk(s) equals fk(i) into joinData
					  from left in joinData.DefaultIfEmpty()
					  select result(s, left);

			return _result;
		}

		/*
		Lambda: 

		var resultJoint = Person.BuiltPersons().RightJoin(                   /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name           = (a.MyPerson != null ? a.MyPerson.Name : "Null-Value"),
								Age              = (a.MyPerson != null ? a.MyPerson.Age : -1),
								PersonIdAddress  = (a.MyPerson != null ? a.MyPerson.IdAddress : -1),
								AddressIdAddress = a.MyAddress.IdAddress,
								Street           = a.MyAddress.Street
							}); 
		 */
		/// <summary>
		/// Returns all of B with nulls for A if A doesn't exist.
		/// </summary>
		public static IEnumerable<TResult> RightJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																						 IEnumerable<TInner> inner,
																						 Func<TSource, TKey> pk,
																						 Func<TInner, TKey> fk,
																						 Func<TSource, TInner, TResult> result)
		{
			IEnumerable<TResult> _result = Enumerable.Empty<TResult>();

			_result = from i in inner
					  join s in source
					  on fk(i) equals pk(s) into joinData
					  from right in joinData.DefaultIfEmpty()
					  select result(right, i);

			return _result;
		}

		/*
		Lambda: 

		var resultJoint = Person.BuiltPersons().FullOuterJoinJoin(           /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name             = (a.MyPerson  != null ? a.MyPerson.Name       : "Null-Value"),
								Age              = (a.MyPerson  != null ? a.MyPerson.Age        : -1),
								PersonIdAddress  = (a.MyPerson  != null ? a.MyPerson.IdAddress  : -1),
								AddressIdAddress = (a.MyAddress != null ? a.MyAddress.IdAddress : -1),
								Street           = (a.MyAddress != null ? a.MyAddress.Street    : "Null-Value")
							});  
		*/
		/// <summary>
		/// Returns both A and B, with nulls where A or B doesn't have a correlation to B or A.
		/// </summary>
		public static IEnumerable<TResult> FullOuterJoinJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																						 IEnumerable<TInner> inner,
																						 Func<TSource, TKey> pk,
																						 Func<TInner, TKey> fk,
																						 Func<TSource, TInner, TResult> result)
		{

			var left = source.LeftJoin(inner, pk, fk, result).ToList();
			var right = source.RightJoin(inner, pk, fk, result).ToList();

			return left.Union(right);


		}

		/*
		Lambda:  

		var resultJoint = Person.BuiltPersons().LeftExcludingJoin(           /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name             = a.MyPerson.Name,
								Age              = a.MyPerson.Age,
								PersonIdAddress  = a.MyPerson.IdAddress,
								AddressIdAddress = (a.MyAddress != null ? a.MyAddress.IdAddress : -1),
								Street           = (a.MyAddress != null ? a.MyAddress.Street    : "Null-Value")
							}); 
		*/

		/// <summary>
		/// Returns only A where B does NOT exists for A.
		/// </summary>
		public static IEnumerable<TResult> LeftExcludingJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																						 IEnumerable<TInner> inner,
																						 Func<TSource, TKey> pk,
																						 Func<TInner, TKey> fk,
																						 Func<TSource, TInner, TResult> result)
		{
			IEnumerable<TResult> _result = Enumerable.Empty<TResult>();

			_result = from s in source
					  join i in inner
					  on pk(s) equals fk(i) into joinData
					  from left in joinData.DefaultIfEmpty()
					  where left == null
					  select result(s, left);

			return _result;
		}


		/*
		 Lambda:   

		var resultJoint = Person.BuiltPersons().RightExcludingJoin(          /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name             = (a.MyPerson != null ? a.MyPerson.Name      : "Null-Value"),
								Age              = (a.MyPerson != null ? a.MyPerson.Age       : -1),
								PersonIdAddress  = (a.MyPerson != null ? a.MyPerson.IdAddress : -1),
								AddressIdAddress = a.MyAddress.IdAddress,
								Street           = a.MyAddress.Street
							}); 
		*/
		/// <summary>
		/// Returns only B where A does not exist for B.
		/// </summary>
		public static IEnumerable<TResult> RightExcludingJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																				 IEnumerable<TInner> inner,
																				 Func<TSource, TKey> pk,
																				 Func<TInner, TKey> fk,
																				 Func<TSource, TInner, TResult> result)
		{
			IEnumerable<TResult> _result = Enumerable.Empty<TResult>();

			_result = from i in inner
					  join s in source
					  on fk(i) equals pk(s) into joinData
					  from right in joinData.DefaultIfEmpty()
					  where right == null
					  select result(right, i);

			return _result;
		}

		/*
		Lambda Expression:   
		Collapse | Copy Code

		var resultJoint = Person.BuiltPersons().FullExcludingJoin(          /// Source Collection
							Address.BuiltAddresses(),                        /// Inner Collection
							p => p.IdAddress,                                /// PK
							a => a.IdAddress,                                /// FK
							(p, a) => new { MyPerson = p, MyAddress = a })   /// Result Collection
							.Select(a => new
							{
								Name             = (a.MyPerson  != null ? a.MyPerson.Name       : "Null-Value"),
								Age              = (a.MyPerson  != null ? a.MyPerson.Age        : -1),
								PersonIdAddress  = (a.MyPerson  != null ? a.MyPerson.IdAddress  : -1),
								AddressIdAddress = (a.MyAddress != null ? a.MyAddress.IdAddress : -1),
								Street           = (a.MyAddress != null ? a.MyAddress.Street    : "Null-Value")
							}); 
		 */
		/// <summary>
		/// Returns A and B where A and B do not reference each other.
		/// </summary>
		public static IEnumerable<TResult> FullExcludingJoin<TSource, TInner, TKey, TResult>(this IEnumerable<TSource> source,
																					IEnumerable<TInner> inner,
																					Func<TSource, TKey> pk,
																					Func<TInner, TKey> fk,
																					Func<TSource, TInner, TResult> result)
		{
			var left = source.LeftExcludingJoin(inner, pk, fk, result).ToList();
			var right = source.RightExcludingJoin(inner, pk, fk, result).ToList();

			return left.Union(right);
		}
	}
}
