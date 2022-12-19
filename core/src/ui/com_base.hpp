/*
 * Copyright © Windower Dev Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"),to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WINDOWER_UI_COM_BASE_HPP
#define WINDOWER_UI_COM_BASE_HPP

#include <unknwn.h>

#include <atomic>
#include <type_traits>

namespace windower::ui
{

template<typename T>
class com_base : public T
{
    static_assert(std::is_base_of_v<::IUnknown, T>);

public:
    com_base(com_base const&) = delete;
    com_base(com_base&&) = delete;

    virtual ~com_base() noexcept = default;

    com_base operator=(com_base const&) = delete;
    com_base operator=(com_base&&) = delete;

    // IUnknown
    ::HRESULT STDMETHODCALLTYPE
    QueryInterface(REFIID riid, void** ppvObject) noexcept override = 0;

    ::ULONG STDMETHODCALLTYPE AddRef() noexcept final { return ++m_ref_count; }

    ::ULONG STDMETHODCALLTYPE Release() noexcept final
    {
        auto const ref_count = --m_ref_count;
        if (ref_count == 0)
        {
            delete this;
        }
        return ref_count;
    }

protected:
    com_base() = default;

private:
    std::atomic<::ULONG> m_ref_count = 1;
};

template<typename T>
::HRESULT STDMETHODCALLTYPE
com_base<T>::QueryInterface(REFIID riid, void** ppvObject) noexcept
{
    if (!ppvObject)
    {
        return E_POINTER;
    }

    *ppvObject = nullptr;

    if (::IsEqualGUID(riid, ::IID_IUnknown))
    {
        ::IUnknown* const result = this;
        *ppvObject = result;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

}

#endif
