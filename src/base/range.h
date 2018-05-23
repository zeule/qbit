/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2018  Eugene Shalygin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#ifndef QBT_RANGE_H
#define QBT_RANGE_H

#include <type_traits>
#include <utility>

template <typename T>
class has_size_method
{
private:
    template<typename U> static auto test(int) -> decltype(std::declval<U>().size() == 0, std::true_type());
    template<typename> static std::false_type test(...);

public:
    static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template <typename T>
class has_end_index_method
{
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().end_index() == decltype(std::declval<U>().end_index())(), std::true_type());
    template<typename> static std::false_type test(...);

public:
    static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template <typename T>
class has_data_method
{
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().data() == nullptr, std::true_type());
    template<typename> static std::false_type test(...);

public:
    static constexpr bool value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template <typename Iter>
struct iterator_range
{
    Iter _begin;
    Iter _end;
    Iter begin() { return _begin; }
    Iter end() { return _end; }
};

template <typename Container, typename Index>
class by_index_iterator
{
public:
    using container_type = Container;
    using index_type = Index;

    by_index_iterator(Container& c, Index i)
        : index_{i}
        , container_{c}
    {
    }

    auto operator*() -> decltype(std::declval<Container>()[std::declval<Index>()])
    {
        return container_[index_];
    }

    by_index_iterator& operator++()
    {
        ++index_;
    }

    bool operator==(const by_index_iterator& other) const
    {
        return &container_ == &other.container_ && index_ == other.index_;
    }

    bool operator!=(const by_index_iterator& other) const
    {
        return &container_ != &other.container_ || index_ != other.index_;
    }

private:
    Index index_;
    Container& container_;
};

template <typename IndexType>
class index_pseudo_iterator
{
public:
    using index_type = IndexType;

    index_pseudo_iterator(index_type i)
        : index_{i}
    {
    }

    index_type operator*()
    {
        return index_;
    }

    index_pseudo_iterator& operator++()
    {
        ++index_;
    }

    bool operator==(const index_pseudo_iterator& other) const
    {
        return index_ == other.index_;
    }

    bool operator!=(const index_pseudo_iterator& other) const
    {
        return index_ != other.index_;
    }

private:
    IndexType index_;
};

template <typename T>
class simplest_iterator_type
{
    static_assert(has_data_method<T>::value || has_end_index_method<T>::value || has_size_method<T>::value);

    template <typename U> static
    std::enable_if_t<has_data_method<U>::value, decltype(std::declval<U>().data())> test(int);

    template <typename U>
    std::enable_if_t<!has_data_method<U>::value && has_end_index_method<U>::value,
        by_index_iterator<U, decltype(std::declval<U>().end_index())>> test(int);

    template <typename U>
    std::enable_if_t<!has_data_method<U>::value && !has_end_index_method<U>::value && has_size_method<U>::value,
        by_index_iterator<U, decltype(std::declval<U>().size())>> test(int);
public:
    using type = decltype(test<T>(0));
};

template <typename T>
using simplest_iterator_type_t = typename simplest_iterator_type<T>::type;

template <typename Container>
using container_element_type_t = std::conditional_t<std::is_const<Container>::value,
    const typename Container::value_type, typename Container::value_type>;

template <typename Iter>
iterator_range<Iter> range(Iter begin, Iter end)
{ return { begin, end}; assert(begin != end);}

template <typename Container, typename IndexType>
std::enable_if_t<has_data_method<Container>::value, iterator_range<simplest_iterator_type_t<Container>>>
range(Container& c, IndexType begin, IndexType end)
{
    // the presence of Container::data() indicates that the container stores elements sequentially
    // and we may use pointers
    return {&c[begin], &c[end]};
}

template <typename Container, typename IndexType>
std::enable_if_t<!has_data_method<Container>::value, iterator_range<simplest_iterator_type_t<Container>>>
range(Container& c, IndexType begin, IndexType end)
{
    return {{c, begin}, {c, end}};
}

template <typename Container>
std::enable_if_t<has_end_index_method<Container>::value, iterator_range<simplest_iterator_type_t<Container>>>
range(Container& c)
{
    using IndexType = decltype(std::declval<Container>().end_index());
    return range(c, IndexType(0), c.end_index());
}

template <typename Container>
std::enable_if_t<!has_end_index_method<Container>::value && has_size_method<Container>::value,
    iterator_range<simplest_iterator_type_t<Container>>>
range(Container& c)
{
    using IndexType = decltype(std::declval<Container>().size());
    return range(c, IndexType(0), c.size());
}

template <typename IndexType>
iterator_range<index_pseudo_iterator<IndexType>> indexRange(IndexType end)
{
    return {index_pseudo_iterator(IndexType(0)), index_pseudo_iterator(end)};
}

#endif
