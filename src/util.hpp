#ifndef FRITZMON_UTIL_HPP
#define FRITZMON_UTIL_HPP

#include <QtCore/QDebug>

#include <algorithm>
#include <memory>
#include <vector>

template<typename T>
std::unique_ptr<T>
removeSmartpointerFromVector(std::vector<std::unique_ptr<T>> &container, T *value)
{
    using std::begin;
    using std::end;

    // convenience iterator instances
    const auto b = begin(container);
    const auto e = end(container);

    // smartpointer instance to return
    auto ptr = std::unique_ptr<T>();

    // find pointer value in the container
    auto iter = std::find_if(b, e, [&](const std::unique_ptr<T> &ptrval){
        return ptrval.get() == value;
    });

    // if not found, return empty (null) smartpointer
    if (iter == e)
        return ptr;

    // swap the pointer from the container into the local instance
    iter->swap(ptr);
    // remove the now empty pointer from the container
    container.erase(iter);

    return ptr;
}

#endif // FRITZMON_UTIL_HPP

