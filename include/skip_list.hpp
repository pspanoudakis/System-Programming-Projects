#ifndef SKIP_LIST_HPP
#define SKIP_LIST_HPP

#include "utils.hpp"

#define SKIP_LIST_MAX_LAYERS 10

class SkipList
{
    private:
        struct SkipListNode
        {
            const int layers;
            SkipListNode **next_nodes;
            void *data;
            SkipListNode(int total_layers);
            ~SkipListNode();
        };
        const int max_layer;
        int curr_layer;
        SkipListNode **layer_heads;
        //CompareFunc compare;
        DestroyFunc destroyElement;
        int getRandomLayer(void);
        
    public:
        SkipList(int layers, DestroyFunc dest);
        ~SkipList();
        void* find(void *element, CompareFunc compare);
        bool insert(void *element, void **present, CompareFunc compare);
        void remove(void *element, CompareFunc compare);
        void display(DisplayFunc f);
        void displayElements(DisplayFunc print);
};

#endif
