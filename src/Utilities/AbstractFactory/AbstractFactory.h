// Copyright(c) 2019 - 2020, #Momo
// All rights reserved.
// 
// Redistributionand use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
// 
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditionsand the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditionsand the following disclaimer in the documentation
// and /or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Utilities/UUID/UUID.h"
#include "Utilities/VectorPool/VectorPool.h"
#include "Utilities/Memory/Memory.h"

namespace MxEngine
{
    template<typename T>
    struct ManagedResource
    {
        UUID uuid;
        T value;
        size_t refCount = 0;

        template<typename... Args>
        ManagedResource(UUID uuid, Args&&... value)
            : uuid(uuid), value(std::forward<Args>(value)...)
        {
        }

        ManagedResource(const ManagedResource&) = delete;

        ~ManagedResource()
        {
            uuid = UUIDGenerator::GetNull();
        }
    };

    template<typename T, typename Factory>
    class Resource
    {
        UUID uuid;
        size_t handle;

        #if defined(MXENGINE_DEBUG)
        mutable ManagedResource<T>* _resourcePtr = nullptr;
        #endif

        static constexpr size_t InvalidHandle = std::numeric_limits<size_t>::max();

        void IncRef()
        {
            if (this->IsValid())
                this->DereferenceHandle(handle).refCount++;
        }

        void DecRef()
        {
            if (this->IsValid())
                this->DereferenceHandle(handle).refCount--;
        }

        ManagedResource<T>& DereferenceHandle(size_t handle) const
        {
            auto& pool = Factory::Get((T*)nullptr);
            auto& resource = pool[handle];

            #if defined(MXENGINE_DEBUG)
            this->_resourcePtr = &resource;
            #endif

            return resource;
        }
    public:
        Resource()
            : uuid(UUIDGenerator::GetNull()), handle(InvalidHandle)
        {

        }

        Resource(UUID uuid, size_t handle)
            : uuid(uuid), handle(handle)
        {
            this->IncRef();
        }

        Resource(const Resource& wrapper)
            : uuid(wrapper.uuid), handle(wrapper.handle)
        {
            this->IncRef();
        }

        Resource& operator=(const Resource& wrapper)
        {
            this->DecRef();

            this->uuid = wrapper.uuid;
            this->handle = wrapper.handle;
            this->IncRef();

            return *this;
        }

        Resource(Resource&& wrapper) noexcept
            : uuid(wrapper.uuid), handle(wrapper.handle)
        {
            wrapper.handle = InvalidHandle;
        }

        Resource& operator=(Resource&& wrapper) noexcept
        {
            this->DecRef();
            this->uuid = wrapper.uuid;
            this->handle = wrapper.handle;
            wrapper.handle = InvalidHandle;

            return *this;
        }

        bool IsValid() const
        {
            return handle != InvalidHandle && DereferenceHandle(handle).uuid == uuid;
        }

        T* operator->()
        {
            MX_ASSERT(this->IsValid());
            if (!this->IsValid()) return nullptr;
            return this->GetUnchecked();
        }

        const T* operator->() const
        {
            MX_ASSERT(this->IsValid());
            if (!this->IsValid()) return nullptr;
            return this->GetUnchecked();
        }

        T* GetUnchecked()
        {
            return &this->DereferenceHandle(handle).value;
        }

        const T* GetUnchecked() const
        {
            return &this->DereferenceHandle(handle).value;
        }

        auto GetHandle()
        {
            return this->handle;
        }

        ~Resource()
        {
            this->DecRef();
        }
    };

    template<typename T, typename... Args>
    struct FactoryImpl : FactoryImpl<Args...>
    {
        using Base = FactoryImpl<Args...>;
        using Pool = VectorPool<ManagedResource<T>>;
        Pool pool;

        template<typename U>
        auto& GetPool()
        {
            if constexpr (std::is_same<T, U>::value)
                return this->pool;
            else
                return ((Base*)this)->GetPool<U>();
        }

        template<typename F>
        void ForEach(F&& func)
        {
            func(this->pool);
            ((Base*)this)->ForEach(std::forward<F>(func));
        }
    };

    template<typename T>
    struct FactoryImpl<T>
    {
        using Pool = VectorPool<ManagedResource<T>>;
        Pool pool;

        template<typename U>
        auto& GetPool()
        {
            static_assert(std::is_same<T, U>::value, "cannot find appropriate Factory<T>");
            return this->pool;
        }

        template<typename F>
        void ForEach(F&& func)
        {
            func(this->pool);
        }
    };

    template<typename... Args>
    class AbstractFactoryImpl
    {
    public:
        using Factory = FactoryImpl<Args...>;
    private:
        inline static Factory* factory = nullptr;
    public:
        static Factory* GetImpl()
        {
            return factory;
        }

        static void Init()
        {
            if (factory == nullptr)
                factory = Alloc<Factory>();
        }

        static void Clone(Factory* other)
        {
            factory = other;
        }

        template<typename U>
        static auto& Get(U* = nullptr)
        {
            return factory->GetPool<U>();
        }

        template<typename T, typename... ConstructArgs>
        static Resource<T, AbstractFactoryImpl<Args...>> Create(ConstructArgs&&... args)
        {
            UUID uuid = UUIDGenerator::Get();
            auto& pool = factory->GetPool<T>();
            size_t index = pool.Allocate(uuid, std::forward<ConstructArgs>(args)...);
            return Resource<T, AbstractFactoryImpl<Args...>>(uuid, index);
        }

        template<typename T>
        static void Destroy(Resource<T, AbstractFactoryImpl<Args...>>& resource)
        {
            factory->GetPool<T>().Deallocate(resource.GetHandle());
        }
    };
}