#pragma once

namespace Lyre
{

	using TypeID = int;

	/** 
	 * The base class for all objects that require RTTI.
	 * Currently, supports only single inheritance.
	 * Usage:
	 *
	 *		class SomeClassThatNeedsRTTI : public InheritedFromLyreObject
	 *		{
	 *		public:
	 *			LYRE_OBJECT(SomeClassThatNeedsRTTI, InheritedFromLyreObject)
	 *		};
	 */
	class Object
	{
	public:
		virtual ~Object() = default;

		inline static TypeID ToTypeID()
		{
			return TypeInfo<Object>::GetID();
		}
		virtual bool IsTypeOf(TypeID typeId) const
		{
			return typeId == ToTypeID();
		}
		template<class Type>
		bool IsTypeOf() const
		{
			return IsTypeOf(Type::ToTypeID());
		}
		virtual TypeID GetType() const
		{
			return ToTypeID();
		}

	protected:
		template<class Type>
		struct TypeInfo
		{
			inline static TypeID GetID()
			{
				static TypeID typeID = generateTypeId();
				return typeID;
			}
		};

	private:

		inline static TypeID generateTypeId()
		{
			static TypeID IDGenerator = 0;
			return ++IDGenerator;
		}

	};


#define LYRE_OBJECT(ClassName, BaseName)								\
																		\
	inline static TypeID ToTypeID()										\
	{																	\
		return TypeInfo<ClassName>::GetID();							\
	}																	\
	bool IsTypeOf(TypeID typeId) const override							\
	{																	\
		return (typeId == ToTypeID()) || BaseName::IsTypeOf(typeId);	\
	}																	\
	template<class Type>												\
	bool IsTypeOf() const												\
	{																	\
		return IsTypeOf(Type::ToTypeID());								\
	}																	\
	TypeID GetType() const override										\
	{																	\
		return ToTypeID();												\
	}																	\


	template<class Type>
	Type const* DynamicCast(Object const& obj)
	{
		if (obj.IsTypeOf(Type::ToTypeID()))
		{
			return static_cast<Type const*>(&obj);
		}

		return nullptr;
	}

	template<class Type>
	Type const* DynamicCast(Object const* obj)
	{
		if (obj->IsTypeOf(Type::ToTypeID()))
		{
			return static_cast<Type const*>(obj);
		}

		return nullptr;
	}

	template<class Type>
	Type* DynamicCast(Object& obj)
	{
		if (obj.IsTypeOf(Type::ToTypeID()))
		{
			return static_cast<Type*>(&obj);
		}

		return nullptr;
	}

	template<class Type>
	Type* DynamicCast(Object* obj)
	{
		if (obj->IsTypeOf(Type::ToTypeID()))
		{
			return static_cast<Type*>(obj);
		}

		return nullptr;
	}

}
