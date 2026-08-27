#include <stdexcept>
#include <cstdlib>
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// This class implements the C++11 shared_ptr template. A shared_ptr is like  
// the C++ Standard Library unique_ptr except that it allows sharing of pointers 
// between instances via reference counting. shared_ptr objects can safely be 
// copied and can  safely be used in containers such as vector or list.
//
// Notes regarding safe usage of shared_ptr:
//     - http://www.boost.org/doc/libs/1_53_0/libs/smart_ptr/shared_ptr.htm#ThreadSafety
//     - If you construct a shared_ptr with a raw pointer, you cannot construct another shared_ptr
//       with just that raw pointer. Insted you need to construct additional shared_ptrs with 
//       the originally created shared_ptr. Otherwise you will get a crash.
//     - Usage of shared_ptr is thread-safe, but what it points to isn't automatically thread safe. 
//       Multiple shared_ptrs that refer to the same object can be used arbitrarily by multiple threads.
//     - You can use a single shared_ptr between multiple threads in all ways except one: assigment
//       to that shared_ptr. The following is not thread-safe, and needs to be guarded by a mutex 
//       or the shared_ptr atomic functions:
//           shared_ptr<Foo> pFoo;
//           // Thread 1:
//           shared_ptr<Foo> pFoo2 = pFoo;
//           // Thread 2:
//           pFoo = make_shared<Foo>();
//
// Compatibility note:
// This version of shared_ptr updates the previous version to have full C++11 
// compatibility. However, in order to add C++11 compatibility there needed to 
// be a few breaking changes which may affect some users. It's likely that most
// or all breaking changes can be rectified by doing the same thing in a slightly
// different way. Here's a list of the primary signficant breaking changes:
//     - shared_ptr now takes just one template parameter instead of three.
//       (allocator and deleter). You now specify the allocator and deleter
//       as part of the shared_ptr constructor at runtime.
//     - shared_ptr has thread safety, which 
//
///////////////////////////////////////////////////////////////////////////////


#ifndef EASTL_SHARED_PTR_H
#define EASTL_SHARED_PTR_H


#include <EASTL/internal/config.h>
#include <EASTL/internal/smart_ptr.h>
#include <EASTL/internal/thread_support.h>
#include <EASTL/unique_ptr.h>
#include <EASTL/functional.h>
#include <EASTL/allocator.h>
#include <EASTL/atomic.h>
#include <EASTL/memory.h>
#if EASTL_RTTI_ENABLED
	#include <typeinfo>
#endif
#if EASTL_EXCEPTIONS_ENABLED
	#include <exception>
#endif

EA_DISABLE_ALL_VC_WARNINGS()
#include <new>
#include <stddef.h>
EA_RESTORE_ALL_VC_WARNINGS()

// 4530 - C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
// 4571 - catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught.
// 4512/4626 - 'class' : assignment operator could not be generated.  // This disabling would best be put elsewhere.
EA_DISABLE_VC_WARNING(4530 4571 4512 4626);

#if defined(EA_PRAGMA_ONCE_SUPPORTED)
	#pragma once // Some compilers (e.g. VC++) benefit significantly from using this. We've measured 3-4% build speed improvements in apps as a result.
#endif



namespace eastl
{
	///////////////////////////////////////////////////////////////////////////
	// shared_ptr
	///////////////////////////////////////////////////////////////////////////

	/// EASTL_SHARED_PTR_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_SHARED_PTR_DEFAULT_NAME
		#define EASTL_SHARED_PTR_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " shared_ptr" // Unless the user overrides something, this is "EASTL shared_ptr".
	#endif


	/// EASTL_SHARED_PTR_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_SHARED_PTR_DEFAULT_ALLOCATOR
		#define EASTL_SHARED_PTR_DEFAULT_ALLOCATOR EASTLAllocatorType(EASTL_SHARED_PTR_DEFAULT_NAME)
	#endif


	// Forward declarations
	template <typename T, typename Deleter> class unique_ptr;
	template <typename T> class weak_ptr;
	template <typename T> class enable_shared_from_this;



	#if EASTL_EXCEPTIONS_ENABLED
		// We define eastl::bad_weak_ptr as opposed to std::bad_weak_ptr. The reason is that 
		// we can't easily know of std::bad_weak_ptr exists and we would have to #include <memory>
		// to use it. EASTL "owns" the types that are defined in EASTL headers, and std::bad_weak_ptr 
		// is declared in <memory>.

		struct bad_weak_ptr : std::exception
		{
			const char* what() const EA_NOEXCEPT EA_OVERRIDE
				{ return "bad weak_ptr"; }
		};
	#endif


	/// ref_count_sp
	///
	/// This is a small utility class used by shared_ptr and weak_ptr.
	struct ref_count_sp
	{
		atomic<int32_t> mRefCount;            /// Reference count on the contained pointer. Starts as 1 by default.
		atomic<int32_t> mWeakRefCount;        /// Reference count on contained pointer plus this ref_count_sp object itself. Starts as 1 by default.

	public:
		ref_count_sp(int32_t refCount = 1, int32_t weakRefCount = 1) EA_NOEXCEPT;
		virtual ~ref_count_sp() EA_NOEXCEPT {}

		int32_t       use_count() const EA_NOEXCEPT;
		void          addref() EA_NOEXCEPT;
		void          release();
		void          weak_addref() EA_NOEXCEPT;
		void          weak_release();
		ref_count_sp* lock() EA_NOEXCEPT;

		virtual void free_value() EA_NOEXCEPT = 0;          // Release the contained object.
		virtual void free_ref_count_sp() EA_NOEXCEPT = 0;   // Release this instance.

		#if EASTL_RTTI_ENABLED
			virtual void* get_deleter(const std::type_info& type) const EA_NOEXCEPT = 0;
		#else
			virtual void* get_deleter() const EA_NOEXCEPT = 0;
		#endif
	};


	inline ref_count_sp::ref_count_sp(int32_t refCount, int32_t weakRefCount) EA_NOEXCEPT
		: mRefCount(refCount), mWeakRefCount(weakRefCount) {
    __builtin_trap() /* STUB: not implemented */;
}

	inline int32_t ref_count_sp::use_count() const EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void ref_count_sp::addref() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void ref_count_sp::release()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void ref_count_sp::weak_addref() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void ref_count_sp::weak_release()
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline ref_count_sp* ref_count_sp::lock() EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}



	/// ref_count_sp_t
	///
	/// This is a version of ref_count_sp which is used to delete the contained pointer.
	template <typename T, typename Allocator, typename Deleter>
	class ref_count_sp_t : public ref_count_sp
	{
	public:
		typedef ref_count_sp_t<T, Allocator, Deleter>   this_type;
		typedef T                                       value_type;
		typedef Allocator                               allocator_type;
		typedef Deleter                                 deleter_type;

		value_type     mValue; // This is expected to be a pointer.
		deleter_type   mDeleter;
		allocator_type mAllocator;

		ref_count_sp_t(value_type value, deleter_type deleter, allocator_type allocator)
			: ref_count_sp(), mValue(value), mDeleter(eastl::move(deleter)), mAllocator(eastl::move(allocator))
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void free_value() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void free_ref_count_sp() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		#if EASTL_RTTI_ENABLED
			void* get_deleter(const std::type_info& type) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
		#else
			void* get_deleter() const EA_NOEXCEPT
			{
				return (void*)&mDeleter;
			}
		#endif
	};

	/// ref_count_sp_t_inst
	///
	/// This is a version of ref_count_sp which is used to actually hold an instance of
	/// T (instead of a pointer). This is useful to allocate the object and ref count
	/// in a single memory allocation.
	template<typename T, typename Allocator>
	class ref_count_sp_t_inst : public ref_count_sp
	{
	public:
		typedef ref_count_sp_t_inst<T, Allocator>                                        this_type;
		typedef T                                                                        value_type;
		typedef Allocator                                                                allocator_type;
		typedef typename aligned_storage<sizeof(T), eastl::alignment_of<T>::value>::type storage_type;

		storage_type   mMemory;
		allocator_type mAllocator;

		value_type* GetValue() {
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename... Args>
		ref_count_sp_t_inst(allocator_type allocator, Args&&... args)
			: ref_count_sp(), mAllocator(eastl::move(allocator))
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void free_value() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void free_ref_count_sp() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		#if EASTL_RTTI_ENABLED
			void* get_deleter(const std::type_info&) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
		#else
			void* get_deleter() const EA_NOEXCEPT
			{
				return nullptr;
			}
		#endif
	};


	/// do_enable_shared_from_this
	///
	/// If a user calls this function, it sets up mWeakPtr member of 
	/// the enable_shared_from_this parameter to point to the ref_count_sp 
	/// object that's passed in. Normally, the user doesn't need to call 
	/// this function, as the shared_ptr constructor will do it for them.
	///
	template <typename T, typename U>
	void do_enable_shared_from_this(const ref_count_sp* pRefCount,
	                                const enable_shared_from_this<T>* pEnableSharedFromThis,
	                                const U* pValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	inline void do_enable_shared_from_this(const ref_count_sp*, ...) {
    __builtin_trap() /* STUB: not implemented */;
} // Empty specialization. This no-op version is
	                                                                    // called by shared_ptr when shared_ptr's T type
	                                                                    // is anything but an enabled_shared_from_this
	                                                                    // class.


	/// shared_ptr_traits
	/// This exists for the sole purpose of creating a typedef called
	/// reference_type which is specialized for type void. The reason
	/// for this is that shared_ptr::operator*() returns a reference
	/// to T but if T is void, it needs to return void, not *void, 
	/// as the latter is not valid C++.
	template <typename T> struct shared_ptr_traits
		{ typedef T& reference_type; };

	template <> struct shared_ptr_traits<void>
		{ typedef void reference_type; };

	template <> struct shared_ptr_traits<void const>
		{ typedef void reference_type; };

	template <> struct shared_ptr_traits<void volatile>
		{ typedef void reference_type; };

	template <> struct shared_ptr_traits<void const volatile>
		{ typedef void reference_type; };



	/// shared_ptr
	///
	/// This class implements the C++11 shared_ptr template. A shared_ptr is like the C++
	/// Standard Library unique_ptr except that it allows sharing of pointers between
	/// instances via reference counting. shared_ptr objects can safely be copied and
	/// can safely be used in C++ Standard Library containers such as std::vector or
	/// std::list.
	///
	/// This class is not thread safe in that you cannot use an instance of it from 
	/// two threads at the same time and cannot use two separate instances of it, which 
	/// own the same pointer, at the same time. Use standard multithread mutex techniques
	/// to address the former problems and use shared_ptr_mt to address the latter.
	/// Note that this is contrary to the C++11 standard.
	///
	/// As of this writing, arrays aren't supported, but they are planned in the future 
	/// based on the C++17 proposal: http://isocpp.org/files/papers/N3920.html
	///
	template <typename T>
	class shared_ptr
	{
	public:
		typedef shared_ptr<T>                                    this_type;
		typedef T                                                element_type; 
		typedef typename shared_ptr_traits<T>::reference_type    reference_type;   // This defines what a reference to a T is. It's always simply T&, except for the case where T is void, whereby the reference is also just void.
		typedef EASTLAllocatorType                               default_allocator_type;
		typedef default_delete<T>                                default_deleter_type;
		typedef weak_ptr<T>                                      weak_type;

	protected:
		element_type*  mpValue;
		ref_count_sp*  mpRefCount;           /// Base pointer to Reference count for owned pointer and the owned pointer.

	public:
		/// Initializes and "empty" shared_ptr.
		/// Postcondition: use_count() == zero and get() == 0
		shared_ptr() EA_NOEXCEPT
			: mpValue(nullptr),
			  mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// Takes ownership of the pointer and sets the reference count
		/// to the pointer to 1. It is OK if the input pointer is null.
		/// The shared reference count is allocated on the heap using the
		/// default eastl allocator.
		/// Throws: bad_alloc, or an implementation-defined exception when 
		///         a resource other than memory could not be obtained.
		/// Exception safety: If an exception is thrown, delete p is called.
		/// Postcondition in the event of no exception: use_count() == 1 && get() == p
		template <typename U>
		explicit shared_ptr(U* pValue,
		                    typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(nullptr), mpRefCount(nullptr) // alloc_internal will set this.
		{
    __builtin_trap() /* STUB: not implemented */;
}


		shared_ptr(std::nullptr_t) EA_NOEXCEPT
			: mpValue(nullptr),
			  mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// Takes ownership of the pointer and sets the reference count
		/// to the pointer to 1. It is OK if the input pointer is null.
		/// The shared reference count is allocated on the heap using the
		/// default eastl allocator. The pointer will be disposed using the
		/// provided deleter.
		/// If an exception occurs during the allocation of the shared 
		/// reference count, the owned pointer is deleted and the exception
		/// is rethrown.
		/// Postcondition: use_count() == 1 && get() == p
		template <typename U, typename Deleter>
		shared_ptr(U* pValue,
		           Deleter deleter,
		           typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(nullptr), mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Deleter>
		shared_ptr(std::nullptr_t, Deleter deleter)
		    : mpValue(nullptr), mpRefCount(nullptr) // alloc_internal will set this.
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// Takes ownership of the pointer and sets the reference count
		/// to the pointer to 1. It is OK if the input pointer is null.
		/// The shared reference count is allocated on the heap using the
		/// supplied allocator. The pointer will be disposed using the
		/// provided deleter.
		/// If an exception occurs during the allocation of the shared 
		/// reference count, the owned pointer is deleted and the exception
		/// is rethrown.
		/// Postcondition: use_count() == 1 && get() == p
		template <typename U, typename Deleter, typename Allocator>
		explicit shared_ptr(U* pValue,
		                    Deleter deleter,
		                    const Allocator& allocator,
		                    typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(nullptr), mpRefCount(nullptr) // alloc_internal will set this.
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename Deleter, typename Allocator>
		shared_ptr(std::nullptr_t, Deleter deleter, Allocator allocator)
			: mpValue(nullptr),
			  mpRefCount(nullptr) // alloc_internal will set this.
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// shared_ptr
		/// construction with self type.
		/// If we want a shared_ptr constructor that is templated on shared_ptr<U>,
		/// then we need to make it in addition to this function, as otherwise 
		/// the compiler will generate this function and things will go wrong.
		/// To accomplish this in a thread-safe way requires use of shared_ptr atomic_store.
		shared_ptr(const shared_ptr& sharedPtr) EA_NOEXCEPT
			: mpValue(sharedPtr.mpValue),
			  mpRefCount(sharedPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// shared_ptr
		/// Shares ownership of a pointer with another instance of shared_ptr.
		/// This function increments the shared reference count on the pointer.
		/// To accomplish this in a thread-safe way requires use of shared_ptr atomic_store.
		template <typename U>
		shared_ptr(const shared_ptr<U>& sharedPtr,
		           typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
		    : mpValue(sharedPtr.mpValue),
		      mpRefCount(sharedPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// shared_ptr
		/// 
		/// 20.7.2.2.1p13: Constructs a shared_ptr instance that stores p and shares ownership with r.
		/// Postconditions: get() == pValue && use_count() == sharedPtr.use_count().
		/// To avoid the possibility of a dangling pointer, the user of this constructor must 
		/// ensure that pValue remains valid at least until the ownership group of sharedPtr is destroyed. 
		/// This constructor allows creation of an empty shared_ptr instance with a non-NULL stored pointer. 
		///
		/// Shares ownership of a pointer with another instance of shared_ptr while storing a potentially 
		/// different pointer. This function increments the shared reference count on the sharedPtr if it exists.
		/// If sharedPtr has no shared reference then a shared reference is not created an pValue is not 
		/// deleted in our destructor and effectively the pointer is not actually shared.
		///
		/// To accomplish this in a thread-safe way requires the user to maintain the lifetime of sharedPtr
		/// as described above.
		///
		template <typename U>
		shared_ptr(const shared_ptr<U>& sharedPtr, element_type* pValue) EA_NOEXCEPT
			: mpValue(pValue),
			  mpRefCount(sharedPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		shared_ptr(shared_ptr&& sharedPtr) EA_NOEXCEPT
			: mpValue(sharedPtr.mpValue),
			  mpRefCount(sharedPtr.mpRefCount)
		{
			sharedPtr.mpValue = nullptr;
			sharedPtr.mpRefCount = nullptr;
		}


		template <typename U>
		shared_ptr(shared_ptr<U>&& sharedPtr,
		           typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
		    : mpValue(sharedPtr.mpValue),
		      mpRefCount(sharedPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// unique_ptr constructor
		template <typename U, typename Deleter>
		shared_ptr(unique_ptr<U, Deleter>&& uniquePtr,
		           typename eastl::enable_if<!eastl::is_array<U>::value && !is_lvalue_reference<Deleter>::value &&
		                                     eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(nullptr), mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// unique_ptr constructor
		// The following is not in the C++11 Standard.
		template <typename U, typename Deleter, typename Allocator>
		shared_ptr(unique_ptr<U, Deleter>&& uniquePtr,
		           const Allocator& allocator,
		           typename eastl::enable_if<!eastl::is_array<U>::value && !is_lvalue_reference<Deleter>::value &&
		                                     eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(nullptr), mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// shared_ptr(weak_ptr)
		/// Shares ownership of a pointer with an instance of weak_ptr.
		/// This function increments the shared reference count on the pointer.
		template <typename U>
		explicit shared_ptr(const weak_ptr<U>& weakPtr,
		                    typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0)
		    : mpValue(weakPtr.mpValue)
		    , mpRefCount(weakPtr.mpRefCount ?
		                     weakPtr.mpRefCount->lock() :
		                     weakPtr.mpRefCount) // mpRefCount->lock() addref's the return value for us.
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// ~shared_ptr
		/// Decrements the reference count for the owned pointer. If the 
		/// reference count goes to zero, the owned pointer is deleted and
		/// the shared reference count is deleted.
		~shared_ptr()
		{
			if (mpRefCount)
			{
				mpRefCount->release();
			}
			// else if mpValue is non-NULL then we just lose it because it wasn't actually shared (can happen with
			// shared_ptr(const shared_ptr<U>& sharedPtr, element_type* pValue) constructor).

			#if EASTL_DEBUG
				mpValue = nullptr;
				mpRefCount = nullptr;
			#endif
		}


		// The following is disabled because it is not specified by the C++11 Standard, as it leads to 
		// potential collisions. Use the reset(p) and reset() functions instead.
		//
		// template <typename U>
		// typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		// operator=(const U* pValue) EA_NOEXCEPT
		// {
		//     reset(pValue);
		//     return *this;
		// }
		//
		// template <typename U>
		// this_type& operator=(std::nullptr_t) EA_NOEXCEPT
		// {
		//     reset();
		//     return *this;
		// }


		/// operator=
		/// Assignment to self type.
		/// If we want a shared_ptr operator= that is templated on shared_ptr<U>,
		/// then we need to make it in addition to this function, as otherwise 
		/// the compiler will generate this function and things will go wrong.
		shared_ptr& operator=(const shared_ptr& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=
		/// Copies another shared_ptr to this object. Note that this object
		/// may already own a shared pointer with another different pointer
		/// (but still of the same type) before this call. In that case,
		/// this function releases the old pointer, decrementing its reference
		/// count and deleting it if zero, takes shared ownership of the new 
		/// pointer and increments its reference count.
		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=
		/// Assignment to self type.
		/// If we want a shared_ptr operator= that is templated on shared_ptr<U>,
		/// then we need to make it in addition to this function, as otherwise 
		/// the compiler will generate this function and things will go wrong.
		this_type& operator=(shared_ptr&& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=
		/// Moves another shared_ptr to this object. Note that this object
		/// may already own a shared pointer with another different pointer
		/// (but still of the same type) before this call. In that case,
		/// this function releases the old pointer, decrementing its reference
		/// count and deleting it if zero, takes shared ownership of the new 
		/// pointer and increments its reference count.
		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(shared_ptr<U>&& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		// unique_ptr operator=
		template <typename U, typename Deleter>
		typename eastl::enable_if<!eastl::is_array<U>::value && eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(unique_ptr<U, Deleter>&& uniquePtr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		/// Releases the owned pointer.
		void reset() EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		/// Releases the owned pointer and takes ownership of the 
		/// passed in pointer.
		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, void>::type
		reset(U* pValue)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		/// Releases the owned pointer and takes ownership of the 
		/// passed in pointer.
		template <typename U, typename Deleter>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, void>::type
		reset(U* pValue, Deleter deleter)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// reset
		/// Resets the shared_ptr
		template <typename U, typename Deleter, typename Allocator>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, void>::type
		reset(U* pValue, Deleter deleter, const Allocator& allocator)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// swap
		/// Exchanges the owned pointer between two shared_ptr objects.
		/// This function is not intrinsically thread-safe. You must use atomic_exchange(shared_ptr<T>*, shared_ptr<T>)
		/// or manually coordinate the swap.
		void swap(this_type& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator*
		/// Returns the owner pointer dereferenced.
		/// Example usage:
		///    shared_ptr<int> ptr(new int(3));
		///    int x = *ptr;
		reference_type operator*() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// operator->
		/// Allows access to the owned pointer via operator->()
		/// Example usage:
		///    struct X{ void DoSomething(); }; 
		///    shared_ptr<int> ptr(new X);
		///    ptr->DoSomething();
		element_type* operator->() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// operator[]
		/// Index into the array pointed to by the owned pointer.
		/// The behaviour is undefined if the owned pointer is nullptr, if the user specified index is negative, or if
		/// the index is outside the referred array bounds.
		///
		/// When T is not an array type, it is unspecified whether this function is declared. If the function is declared,
		/// it is unspecified what its return type is, except that the declaration (although not necessarily the
		/// definition) of the function is guaranteed to be legal.
		//
		// TODO(rparolin): This is disabled because eastl::shared_ptr needs array support.
		// element_type& operator[](ptrdiff_t idx)
		// {
		//     return get()[idx];
		// }

		/// get
		/// Returns the owned pointer. Note that this class does 
		/// not provide an operator T() function. This is because such
		/// a thing (automatic conversion) is deemed unsafe.
		/// Example usage:
		///    struct X{ void DoSomething(); }; 
		///    shared_ptr<int> ptr(new X);
		///    X* pX = ptr.get();
		///    pX->DoSomething();
		element_type* get() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// use_count
		/// Returns: the number of shared_ptr objects, *this included, that share ownership with *this, or 0 when *this is empty.
		int use_count() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// unique
		/// Returns: use_count() == 1.
		bool unique() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// owner_before
		/// C++11 function for ordering.
		template <typename U>
		bool owner_before(const shared_ptr<U>& sharedPtr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename U>
		bool owner_before(const weak_ptr<U>& weakPtr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		template <typename Deleter>
		Deleter* get_deleter() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		#ifdef EA_COMPILER_NO_EXPLICIT_CONVERSION_OPERATORS
			/// Note that below we do not use operator bool(). The reason for this
			/// is that booleans automatically convert up to short, int, float, etc.
			/// The result is that this: if(sharedPtr == 1) would yield true (bad).
			typedef T* (this_type::*bool_)() const;
			operator bool_() const EA_NOEXCEPT
			{
				if(mpValue)
					return &this_type::get;
				return nullptr;
			}

			bool operator!() const EA_NOEXCEPT
			{
				return (mpValue == nullptr);
			}
		#else
			/// Explicit operator bool
			/// Allows for using a shared_ptr as a boolean. 
			/// Example usage:
			///    shared_ptr<int> ptr(new int(3));
			///    if(ptr)
			///        ++*ptr;
			explicit operator bool() const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
		#endif

		/// Returns true if the given shared_ptr ows the same T pointer that we do.
		template <typename U>
		bool equivalent_ownership(const shared_ptr<U>& sharedPtr) const
		{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		// Friend declarations.
		template <typename U> friend class shared_ptr;
		template <typename U> friend class weak_ptr;
		template <typename U> friend void allocate_shared_helper(shared_ptr<U>&, ref_count_sp*, U*);

		// Handles the allocating of mpRefCount, while assigning mpValue.
		// The provided pValue may be NULL, as with constructing with a deleter and allocator but NULL pointer.
		template <typename U, typename Allocator, typename Deleter>
		void alloc_internal(U pValue, Allocator allocator, Deleter deleter)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	}; // class shared_ptr


	/// get_pointer
	/// returns shared_ptr::get() via the input shared_ptr. 
	template <typename T>
	inline typename shared_ptr<T>::element_type* get_pointer(const shared_ptr<T>& sharedPtr) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// get_deleter
	/// returns the deleter in the input shared_ptr.
	template <typename Deleter, typename T>
	Deleter* get_deleter(const shared_ptr<T>& sharedPtr) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	/// swap
	/// Exchanges the owned pointer beween two shared_ptr objects.
	/// This non-member version is useful for compatibility of shared_ptr
	/// objects with the C++ Standard Library and other libraries.
	template <typename T>
	inline void swap(shared_ptr<T>& a, shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// shared_ptr comparison operators
	template <typename T, typename U> 
	inline bool operator==(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T, typename U>
	std::strong_ordering operator<=>(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
		return a.get() <=> b.get();
	}
#else
	template <typename T, typename U> 
	inline bool operator!=(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> 
	inline bool operator<(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> 
	inline bool operator>(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> 
	inline bool operator<=(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> 
	inline bool operator>=(const shared_ptr<T>& a, const shared_ptr<U>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif

	template <typename T>
	inline bool operator==(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	#if defined(EA_COMPILER_HAS_THREE_WAY_COMPARISON)
	template <typename T>
	inline std::strong_ordering operator<=>(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
		return a.get() <=> nullptr;
	}
	#else
	template <typename T>
	inline bool operator==(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator!=(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator!=(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator<(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator<(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator>(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator>(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator<=(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator<=(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator>=(const shared_ptr<T>& a, std::nullptr_t) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool operator>=(std::nullptr_t, const shared_ptr<T>& b) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}
#endif



	/// reinterpret_pointer_cast
	///
	/// Returns a shared_ptr<T> reinterpret-casted from a const shared_ptr<U>&.
	/// http://isocpp.org/files/papers/N3920.html
	///
	/// Requires: The expression reinterpret_cast<T*>(sharedPtr.get()) shall be well formed.
	/// Returns: If sharedPtr is empty, an empty shared_ptr<T>; otherwise, a shared_ptr<T>
	///          object that stores const_cast<T*>(sharedPtr.get()) and shares ownership with sharedPtr.
	/// Postconditions: w.get() == const_cast<T*>(sharedPtr.get()) and w.use_count() == sharedPtr.use_count(), 
	///                 where w is the return value.
	template <typename T, typename U>
	inline shared_ptr<T> reinterpret_pointer_cast(shared_ptr<U> const& sharedPtr) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}


	/// static_pointer_cast
	///
	/// Returns a shared_ptr<T> static-casted from a shared_ptr<U>&.
	///
	/// Requires: The expression const_cast<T*>(sharedPtr.get()) shall be well formed.
	/// Returns: If sharedPtr is empty, an empty shared_ptr<T>; otherwise, a shared_ptr<T> 
	/// object that stores const_cast<T*>(sharedPtr.get()) and shares ownership with sharedPtr.
	/// Postconditions: w.get() == const_cast<T*>(sharedPtr.get()) and w.use_count() == sharedPtr.use_count(), 
	///                 where w is the return value.
	template <typename T, typename U> 
	inline shared_ptr<T> static_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> // Retained for support for pre-C++11 shared_ptr.
	inline shared_ptr<T> static_shared_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}



	/// const_pointer_cast
	///
	/// Returns a shared_ptr<T> const-casted from a const shared_ptr<U>&.
	/// Normally, this means that the source shared_ptr holds a const data type.
	//
	/// Requires: The expression const_cast<T*>(sharedPtr.get()) shall be well formed.
	/// Returns: If sharedPtr is empty, an empty shared_ptr<T>; otherwise, a shared_ptr<T>
	///          object that stores const_cast<T*>(sharedPtr.get()) and shares ownership with sharedPtr.
	/// Postconditions: w.get() == const_cast<T*>(sharedPtr.get()) and w.use_count() == sharedPtr.use_count(), 
	///                 where w is the return value.
	template <typename T, typename U> 
	inline shared_ptr<T> const_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename U> // Retained for support for pre-C++11 shared_ptr.
	inline shared_ptr<T> const_shared_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}



	#if EASTL_RTTI_ENABLED
		/// dynamic_pointer_cast
		/// 
		/// Returns a shared_ptr<T> dynamic-casted from a const shared_ptr<U>&.
		/// 
		/// Requires: The expression dynamic_cast<T*>(sharedPtr.get()) shall be well formed and shall have well defined behavior.
		/// Returns: When dynamic_cast<T*>(sharedPtr.get()) returns a nonzero value, a shared_ptr<T> object that stores 
		///          a copy of it and shares ownership with sharedPtr; Otherwise, an empty shared_ptr<T> object.
		/// Postcondition: w.get() == dynamic_cast<T*>(sharedPtr.get()), where w is the return value
		/// 
		template <typename T, typename U>
		inline shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template <typename T, typename U> // Retained for support for pre-C++11 shared_ptr.
		inline typename eastl::enable_if<!eastl::is_array<T>::value && !eastl::is_array<U>::value, shared_ptr<T> >::type
		dynamic_shared_pointer_cast(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
	#endif


	/// hash specialization for shared_ptr.
	/// It simply returns eastl::hash(x.get()). If your unique_ptr pointer type (the return value of shared_ptr<T>::get) is 
	/// a custom type and not a built-in pointer type then you will need to independently define eastl::hash for that type.
	template <typename T> 
	struct hash< shared_ptr<T> >
	{ 
		size_t operator()(const shared_ptr<T>& x) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};


	template <typename T>
	void allocate_shared_helper(eastl::shared_ptr<T>& sharedPtr, ref_count_sp* pRefCount, T* pValue)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename Allocator, typename... Args>
	shared_ptr<T> allocate_shared(const Allocator& allocator, Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T, typename... Args>
	shared_ptr<T> make_shared(Args&&... args)
	{
    __builtin_trap() /* STUB: not implemented */;
}



	///////////////////////////////////////////////////////////////////////////
	// shared_ptr atomic access
	//
	// These functions allow shared_ptr to act like other C++11 atomic operations.
	// So the same way you can use atomic_load on a raw pointer, you can also
	// use it on a shared_ptr. This allows for transparent use of shared_ptr in
	// place of raw pointers (e.g. in templates). You do not need to use these
	// functions for regular thread-safe direct usage of shared_ptr construction
	// and copying, as it's intrinsically thread-safe for that already.
	//
	// That being said, the following is not thread-safe and needs to be guarded by 
	// a mutex or the following atomic functions, as it's assigning the *same*
	// shared_ptr object from multiple threads as opposed to different shared_ptr
	// objects underlying object:
	//      shared_ptr<Foo> pFoo;
	//      // Thread 1:
	//      shared_ptr<Foo> pFoo2 = pFoo;
	//      // Thread 2:
	//      pFoo = make_shared<Foo>();
	///////////////////////////////////////////////////////////////////////////

	template <typename T>
	inline bool atomic_is_lock_free(const shared_ptr<T>*)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline shared_ptr<T> atomic_load(const shared_ptr<T>* pSharedPtr)
	{
    __builtin_trap() /* STUB: not implemented */;
}
  
	template <typename T>
	inline shared_ptr<T> atomic_load_explicit(const shared_ptr<T>* pSharedPtr, ... /*std::memory_order memoryOrder*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline void atomic_store(shared_ptr<T>* pSharedPtrA, shared_ptr<T> sharedPtrB)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline void atomic_store_explicit(shared_ptr<T>* pSharedPtrA, shared_ptr<T> sharedPtrB, ... /*std::memory_order memoryOrder*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	shared_ptr<T> atomic_exchange(shared_ptr<T>* pSharedPtrA, shared_ptr<T> sharedPtrB)
	{
    __builtin_trap() /* STUB: not implemented */;
}
  
	template <typename T>
	inline shared_ptr<T> atomic_exchange_explicit(shared_ptr<T>* pSharedPtrA, shared_ptr<T> sharedPtrB, ... /*std::memory_order memoryOrder*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	// Compares the shared pointers pointed-to by p and expected. If they are equivalent (share ownership of the 
	// same pointer and refer to the same pointer), assigns sharedPtrNew into *pSharedPtr using the memory ordering constraints 
	// specified by success and returns true. If they are not equivalent, assigns *pSharedPtr into *pSharedPtrCondition using the 
	// memory ordering constraints specified by failure and returns false.
	template <typename T>
	bool atomic_compare_exchange_strong(shared_ptr<T>* pSharedPtr, shared_ptr<T>* pSharedPtrCondition, shared_ptr<T> sharedPtrNew)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool atomic_compare_exchange_weak(shared_ptr<T>* pSharedPtr, shared_ptr<T>* pSharedPtrCondition, shared_ptr<T> sharedPtrNew)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T> // Returns true if pSharedPtr was equivalent to *pSharedPtrCondition.
	inline bool atomic_compare_exchange_strong_explicit(shared_ptr<T>* pSharedPtr, shared_ptr<T>* pSharedPtrCondition, shared_ptr<T> sharedPtrNew, ... /*memory_order memoryOrderSuccess, memory_order memoryOrderFailure*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}

	template <typename T>
	inline bool atomic_compare_exchange_weak_explicit(shared_ptr<T>* pSharedPtr, shared_ptr<T>* pSharedPtrCondition, shared_ptr<T> sharedPtrNew, ... /*memory_order memoryOrderSuccess, memory_order memoryOrderFailure*/)
	{
    __builtin_trap() /* STUB: not implemented */;
}




	///////////////////////////////////////////////////////////////////////////
	// weak_ptr
	///////////////////////////////////////////////////////////////////////////

	/// EASTL_WEAK_PTR_DEFAULT_NAME
	///
	/// Defines a default container name in the absence of a user-provided name.
	///
	#ifndef EASTL_WEAK_PTR_DEFAULT_NAME
		#define EASTL_WEAK_PTR_DEFAULT_NAME EASTL_DEFAULT_NAME_PREFIX " weak_ptr" // Unless the user overrides something, this is "EASTL weak_ptr".
	#endif


	/// EASTL_WEAK_PTR_DEFAULT_ALLOCATOR
	///
	#ifndef EASTL_WEAK_PTR_DEFAULT_ALLOCATOR
		#define EASTL_WEAK_PTR_DEFAULT_ALLOCATOR allocator_type(EASTL_WEAK_PTR_DEFAULT_NAME)
	#endif


	/// weak_ptr
	///
	/// The weak_ptr class template stores a "weak reference" to an object 
	/// that's already managed by a shared_ptr. To access the object, a weak_ptr 
	/// can be converted to a shared_ptr using the shared_ptr constructor or 
	/// the lock() member function. When the last shared_ptr to the object goes 
	/// away and the object is deleted, the attempt to obtain a shared_ptr 
	/// from the weak_ptr instances that refer to the deleted object will fail via
	/// lock() returning an empty shared_ptr.
	///
	/// The Allocator template argument manages the memory of the shared reference
	/// count and not the stored object. weak_ptr will not delete the stored object
	/// but instead can only delete the reference count on that object.
	///
	template <typename T>
	class weak_ptr
	{
	public:
		typedef weak_ptr<T>     this_type;
		typedef T               element_type;

	public:
		/// weak_ptr
		weak_ptr() EA_NOEXCEPT
			: mpValue(nullptr),
			  mpRefCount(nullptr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// weak_ptr
		/// Construction with self type.
		weak_ptr(const this_type& weakPtr) EA_NOEXCEPT
			: mpValue(weakPtr.mpValue),
			  mpRefCount(weakPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// weak_ptr
		/// Move construction with self type.
		weak_ptr(this_type&& weakPtr) EA_NOEXCEPT
			: mpValue(weakPtr.mpValue),
			  mpRefCount(weakPtr.mpRefCount)
		{
			weakPtr.mpValue = nullptr;
			weakPtr.mpRefCount = nullptr;
		}


		/// weak_ptr
		/// Constructs a weak_ptr from another weak_ptr.
		template <typename U>
		weak_ptr(const weak_ptr<U>& weakPtr, typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
			: mpValue(weakPtr.mpValue),
				mpRefCount(weakPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// weak_ptr
		/// Move constructs a weak_ptr from another weak_ptr.
		template <typename U>
		weak_ptr(weak_ptr<U>&& weakPtr,
		         typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
		    : mpValue(weakPtr.mpValue),
		      mpRefCount(weakPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// weak_ptr
		/// Constructs a weak_ptr from a shared_ptr.
		template <typename U>
		weak_ptr(const shared_ptr<U>& sharedPtr,
		         typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
		    : mpValue(sharedPtr.mpValue),
		      mpRefCount(sharedPtr.mpRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// ~weak_ptr
		~weak_ptr()
		{
			if(mpRefCount)
				mpRefCount->weak_release();
		}


		/// operator=(weak_ptr)
		/// assignment to self type.
		this_type& operator=(const this_type& weakPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		this_type& operator=(this_type&& weakPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=(weak_ptr)
		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(const weak_ptr<U>& weakPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(weak_ptr<U>&& weakPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// operator=(shared_ptr)
		/// Assigns to a weak_ptr from a shared_ptr.
		template <typename U>
		typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value, this_type&>::type
		operator=(const shared_ptr<U>& sharedPtr) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		shared_ptr<T> lock() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// Returns: 0 if *this is empty ; otherwise, the number of shared_ptr instances that share ownership with *this.
		int use_count() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		// Returns: use_count() == 0
		bool expired() const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void reset()
		{
    __builtin_trap() /* STUB: not implemented */;
}

		void swap(this_type& weakPtr)
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// assign
		///
		/// Assignment via another weak_ptr. 
		///
		template <typename U>
		void assign(const weak_ptr<U>& weakPtr,
		            typename eastl::enable_if<eastl::is_convertible<U*, element_type*>::value>::type* = 0) EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// owner_before
		/// C++11 function for ordering.
		template <typename U>
		bool owner_before(const weak_ptr<U>& weakPtr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		/// owner_before
		template <typename U>
		bool owner_before(const shared_ptr<U>& sharedPtr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// less_than
		/// For compatibility with pre-C++11 weak_ptr. Use owner_before instead.
		template <typename U>
		bool less_than(const weak_ptr<U>& weakPtr) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}


		/// assign
		///
		/// Assignment through a T/ref_count_sp pair. This is used by 
		/// external utility functions.
		///
		void assign(element_type* pValue, ref_count_sp* pRefCount)
		{
    __builtin_trap() /* STUB: not implemented */;
}

	protected:
		element_type*  mpValue;       /// The (weakly) owned pointer.
		ref_count_sp*  mpRefCount;    /// Reference count for owned pointer.

		// Friend declarations
		template <typename U> friend class shared_ptr;
		template <typename U> friend class weak_ptr;

	}; // class weak_ptr



	/// Note that the C++11 Standard does not specify that weak_ptr has comparison operators,
	/// though it does specify that the owner_before function exists in weak_ptr.
	template <typename T, typename U>
	inline bool operator<(const weak_ptr<T>& weakPtr1, const weak_ptr<U>& weakPtr2)
	{
    __builtin_trap() /* STUB: not implemented */;
}


	template <typename T>
	void swap(weak_ptr<T>& weakPtr1, weak_ptr<T>& weakPtr2)
	{
    __builtin_trap() /* STUB: not implemented */;
}






	///////////////////////////////////////////////////////////////////////////
	// owner_less
	//
	// Implements less (operator <) for shared_ptr and thus allows it to participate
	// in algorithms and containers that use strict weak ordering, such as map.
	///////////////////////////////////////////////////////////////////////////

	template <typename T> 
	struct owner_less;

	template <typename T>
	struct owner_less< shared_ptr<T> >
	{
		bool operator()(shared_ptr<T> const& a, shared_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bool operator()(shared_ptr<T> const& a, weak_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bool operator()(weak_ptr<T> const& a, shared_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};

	template <typename T>
	struct owner_less< weak_ptr<T> >
	{
		bool operator()(weak_ptr<T> const& a, weak_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bool operator()(weak_ptr<T> const& a, shared_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}

		bool operator()(shared_ptr<T> const& a, weak_ptr<T> const& b) const EA_NOEXCEPT
			{
    __builtin_trap() /* STUB: not implemented */;
}
	};

	template <>
	struct owner_less<void>
	{
		typedef int is_transparent;

		template<typename T, typename U>
		bool operator()(shared_ptr<T> const& a, shared_ptr<U> const& b) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename T, typename U>
		bool operator()(shared_ptr<T> const& a, weak_ptr<U> const& b) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename T, typename U>
		bool operator()(weak_ptr<T> const& a, shared_ptr<U> const& b) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}

		template<typename T, typename U>
		bool operator()(weak_ptr<T> const& a, weak_ptr<U> const& b) const EA_NOEXCEPT
		{
    __builtin_trap() /* STUB: not implemented */;
}
	};


} // namespace eastl


EA_RESTORE_VC_WARNING();


// We have to either #include enable_shared.h here or we need to move the enable_shared source code to here.
#include <EASTL/internal/enable_shared.h>


#endif // Header include guard
