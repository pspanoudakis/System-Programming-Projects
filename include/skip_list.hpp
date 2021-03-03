#include "utils.hpp"

#define MAXLAYERS 10

class SkipList
{
    private:
        CompareFunc compare;
        struct SkipListNode
        {
            SkipListNode **next_nodes;
            void *data;
            SkipListNode(int layers);
            ~SkipListNode();
        };
        const int max_layers;
        int curr_layer;
        SkipListNode **layer_heads;
        void* search(void *element, SkipListNode *start, int search_layer);
        
    public:
        SkipList(int layers, CompareFunc function);
        ~SkipList();
        void* find(void *element);
        void insert(void *element);
};