namespace Nito
{


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Interface
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
Component_Allocator get_component_allocator()
{
    return [](const Cpp_Utils::JSON & data) -> Component
    {
        return new T(data.get<T>());
    };
}


template<typename T>
Component_Deallocator get_component_deallocator()
{
    return [](Component component) -> void
    {
        delete (T *)component;
    };
}


} // namespace Nito
