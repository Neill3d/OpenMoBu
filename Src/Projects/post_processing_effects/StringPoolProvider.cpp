#include "StringPoolProvider.h"
#include "StringPool.h"

/// <summary>
/// Implementation of a StringPoolProvider
///  we hide the StringPool template class here and only provide interface to communicate with
/// </summary>
class CStringPoolProviderImpl
{
public:
    std::string_view AddString(std::string_view str)
    {
        return m_StringPool.add(str);
    }

private:
    StringPool<char, true> m_StringPool;
};

//////////////////////////////////////////////////////////////////////////////
//

CStringPoolProvider::CStringPoolProvider()
{
    impl = new CStringPoolProviderImpl();
}

std::string_view CStringPoolProvider::AddString(std::string_view str)
{
    return impl->AddString(str);
}
