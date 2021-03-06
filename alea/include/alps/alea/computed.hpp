/*
 * Copyright (C) 1998-2017 ALPS Collaboration. See COPYRIGHT.TXT
 * All rights reserved. Use is subject to license terms. See LICENSE.TXT
 * For use in publications, see ACKNOWLEDGE.TXT
 */
#pragma once

#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <complex>

#include <vector>
#include <Eigen/Dense>

#include <alps/alea/core.hpp>
#include <alps/alea/util.hpp>

// Forward declarations

namespace alps { namespace alea {
    template <typename T> class value_adapter;
    template <typename T> class vector_adapter;
    template <typename T, typename Derived> class eigen_adapter;
}}

// Actual declarations

namespace alps { namespace alea {

template <typename T>
class value_adapter
    : public computed<T>
{
public:
    typedef T value_type;

public:
    value_adapter(T in) : in_(in) { }

    size_t size() const { return 1; }

    void add_to(sink<T> out) const
    {
        if (out.size() != 1)
            throw size_mismatch();
        out.data()[0] += in_;
    }

    ~value_adapter() { }

private:
    T in_;
};

template <typename T>
class vector_adapter
    : public computed<T>
{
public:
    typedef T value_type;

public:
    vector_adapter(const std::vector<T> &in) : in_(in) { }

    size_t size() const { return in_.size(); }

    void add_to(sink<T> out) const
    {
        if (out.size() != in_.size())
            throw size_mismatch();
        for (size_t i = 0; i != in_.size(); ++i)
            out.data()[i] += in_[i];
    }

    ~vector_adapter() { }

private:
    const std::vector<T> &in_;
};

template <typename T, typename Derived>
class eigen_adapter
    : public computed<T>
{
public:
    typedef T value_type;

public:
    eigen_adapter(const Eigen::DenseBase<Derived> &in)
        : in_(in)
    {
        EIGEN_STATIC_ASSERT_VECTOR_ONLY(Eigen::DenseBase<Derived>);
        static_assert(std::is_same<T, typename Derived::Scalar>::value,
                      "Type mismatch -- use explicit cast");
    }

    size_t size() const { return in_.size(); }

    void add_to(sink<T> out) const
    {
        if (out.size() != (size_t)in_.rows())
            throw size_mismatch();

        typename eigen<T>::col_map out_map(out.data(), out.size());
        out_map += in_;
    }

    ~eigen_adapter() { }

private:
    const Eigen::DenseBase<Derived> &in_;
};

/**
 * Proxy object for computed results.
 */
template <typename T, typename Parent>
class computed_cmember
    : public computed<T>
{
public:
    typedef T value_type;
    typedef void (Parent::*adder_type)(sink<T>) const;

public:
    computed_cmember(const Parent &parent, adder_type adder, size_t size)
        : parent_(parent)
        , adder_(adder)
        , size_(size)
    { }

    size_t size() const { return size_; }

    void add_to(sink<T> out) const { (parent_.*adder_)(out); }

    void fast_add_to(sink<T> out) { (parent_.*adder_)(out); }

    const Parent &parent() const { return parent_; }

    const adder_type &adder() const { return adder_; }

    ~computed_cmember() { }

private:
    const Parent &parent_;
    adder_type adder_;
    size_t size_;
};

}}
