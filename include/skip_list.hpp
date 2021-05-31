/**
 * File: skip_list.hpp
 * Skip List definition.
 * Pavlos Spanoudakis (sdi1800184)
 */

#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include "utils.hpp"

#define SKIP_LIST_MAX_LAYERS 30     // The maximum number of levels allowed in a Skip List.

/**
 * A probabilistic ordered List structure, which allows significantly
 * faster element lookup & insertion.
 */
class SkipList
{
    private:
        /**
         * The Skip List Node structure.
         * Each node has an array of pointers to other Nodes, one for each
         * level where the node is present.
         */
        struct SkipListNode
        {
            const int layers;
            SkipListNode **next_nodes;
            void *data;
            SkipListNode(int total_layers);
            ~SkipListNode();
        };
        const int max_layer;        // The max level selected for the Skip List.
        int curr_layer;             // The currently higher level of the list.
        SkipListNode **layer_heads; // An array of pointers to the first node of each level.
        DestroyFunc destroyElement; // Used for destroying elements when the skip list is being destroyed.
        int getRandomLayer(void);
        
    public:
        SkipList(int layers, DestroyFunc dest);
        ~SkipList();
        void* find(void *element, CompareFunc compare);
        bool insert(void *element, void **present, CompareFunc compare);
        void remove(void *element, void **present, CompareFunc compare);
        void displayElements(DisplayFunc print);
};

#endif
