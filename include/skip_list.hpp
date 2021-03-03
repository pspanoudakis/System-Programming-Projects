#include "utils.hpp"

class SkipList
{
    private:
        CompareFunc compare;
        struct SkipListNode
        {
            SkipListNode *next;
            SkipListNode *down;
            void *data;
            SkipListNode();
            ~SkipListNode();
        };
        SkipListNode *layers;
        void* search(void *element, SkipListNode *start);
        
    public:
        SkipList(CompareFunc function);
        ~SkipList();
        void* find(void *element);
        void insert(void *element);
};