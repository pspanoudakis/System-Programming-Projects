#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include "utils.hpp"

#define MAXLAYERS 10

class SkipList
{
    private:
        CompareFunc compare;
        DestroyFunc destroyElement;
        struct SkipListNode
        {
            SkipListNode **next_nodes;
            int layers;
            void *data;
            SkipListNode(int total_layers);
            ~SkipListNode();
        };
        const int max_layer;
        int curr_layer;
        SkipListNode **layer_heads;
        int getRandomLayer(void);
        
    public:
        SkipList(int layers, CompareFunc comp, DestroyFunc dest);
        ~SkipList();
        void* find(void *element);
        int insert(void *element);
        void remove(void *element);
        void display(DisplayFunc f);
};

#endif
