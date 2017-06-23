#pragma once

namespace FactUtilEmbedded { namespace experimental {

// experimentation with compile-time dependency tree
// didn't get very far

namespace layer1 {

template <uint16_t id>
class NodeBase
{
};


template <uint16_t id, uint16_t parent_id>
class Node : public NodeBase<id>
{
    typedef NodeBase<id> node_base_t;
    typedef Node<id, parent_id> node_t;

public:

    static constexpr uint16_t _id = id;
    static constexpr uint16_t _parent_id = parent_id;

};


template <class ...TNodes>
class Tree
{
    template <bool dummy>
    static uint16_t _get_parent(uint16_t id)
    {
        return -1;
    }

    template <bool dummy, class TNode, class ...TNodes2>
    static uint16_t _get_parent(uint16_t id)
    {
        if(TNode::_id == id) return TNode::_parent_id;

        return _get_parent<true, TNodes2...>(id);
    }

    template <bool dummy>
    static uint16_t _get_child(uint16_t id, uint16_t index)
    {
        return -1;
    }

    template <bool dummy, class TNode, class ...TNodes2>
    static uint16_t _get_child(uint16_t id, uint16_t index)
    {
        if(TNode::_parent_id == id)
        {
            // Decrement index counter to "move up" the indexes
            // to grab specific child
            if(index-- == 0) return TNode::_id;
        }

        return _get_child<true, TNodes2...>(id, index);
    }


    template <bool dummy>
    static uint16_t _child_count(uint16_t id)
    {
        return 0;
    }

    template <bool dummy, class TNode, class ...TNodes2>
    static uint16_t _child_count(uint16_t id)
    {
        if(TNode::_parent_id == id)
        {
            return _child_count<true, TNodes2...>(id) + 1;
        }
        else
            return _child_count<true, TNodes2...>(id);
    }

public:
    static inline uint16_t get_parent(uint16_t id)
    {
        return _get_parent<true, TNodes...>(id);
    }


    static inline uint16_t get_child(uint16_t id, uint16_t index)
    {
        return _get_child<true, TNodes...>(id, index);
    }


    static inline size_t child_count(uint16_t id)
    {
        return _child_count<true, TNodes...>(id);
    }
};



}

namespace layer2 {


template <uint16_t id, class T>
class NodeBase
{
    const T value;

public:
    NodeBase(T value) : value(value) {}
};


template <uint16_t id, uint16_t parent_id, class T>
class Node : public NodeBase<id, T>
{
    typedef NodeBase<id, T> node_base_t;
    typedef Node<id, parent_id, T> node_t;

public:
    Node(T value) : node_base_t(value) {}
};


template <class ...TNodes>
class Tree
{
public:
    Tree(TNodes...nodes)
    {
    }

};

}

}}