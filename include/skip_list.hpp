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
        const int max_layer;
        int curr_layer;
        SkipListNode **layer_heads;
        void* search(void *element, SkipListNode *start, int search_layer);
        int getRandomLayer(void);
        
    public:
        SkipList(int layers, CompareFunc function);
        ~SkipList();
        void* find(void *element);
        int insert(void *element);
        void remove(void *element);
};