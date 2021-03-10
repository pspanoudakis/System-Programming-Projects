#include <cstddef>
#include "../include/rb_tree.hpp"

RBTreeNode::RBTreeNode(Color c, void *element, RBTreeNode *parent_node):
color(c), data(element), left(NULL), right(NULL), parent(parent_node) { }

// To be deleted
RBTreeNode::~RBTreeNode()
{
    //delete (int*)data;
}

RBTreeNode* RBTreeNode::sibling()
{
    if (this->parent == NULL) { return NULL; }
    if (this->isLeftChild())
    {
        return this->parent->right;
    }
    return this->parent->left;
}

void RBTreeNode::destroyDescendants()
{
    if (this->left != NULL)
    {
        this->left->destroyDescendants();
        delete left;
    }
    if (this->right != NULL)
    {
        this->right->destroyDescendants();
        delete right;
    }
}

bool RBTreeNode::isLeftChild()
{
    if (this->parent == NULL) { return false; }

    return (this->parent->left == this);
}

void RBTreeNode::swapColor(RBTreeNode *other)
{
    Color temp = this->color;
    this->color = other->color;
    other->color = temp;
}

void RBTreeNode::replaceWithNewParent(RBTreeNode *new_parent)
{
    if (this->parent == NULL) { return; }

    if ( this->isLeftChild() )
    {
        this->parent->left = new_parent;
    }
    else
    {
        this->parent->right = new_parent;
    }
    new_parent->parent = this->parent;
    this->parent = new_parent;
    
}

RedBlackTree::RedBlackTree(CompareFunc comp):
num_elements(0), compare(comp), root(NULL) { }

RedBlackTree::~RedBlackTree()
{
    if (root != NULL)
    {
        root->destroyDescendants();
        delete root;
    }
}

void RedBlackTree::leftRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->right;

    if (this->root == target)
    {
        this->root = new_parent;
    }
    target->replaceWithNewParent(new_parent);
    target->right = new_parent->left;
    if (target->right != NULL)
    {
        target->right->parent = target;
    }
    new_parent->left = target;
}

void RedBlackTree::rightRotation(RBTreeNode *target)
{
    if (target == NULL) { return; }
    RBTreeNode *new_parent = target->left;

    if (this->root == target)
    {
        this->root = new_parent;
    }
    target->replaceWithNewParent(new_parent);
    target->left = new_parent->right;
    if (target->left != NULL)
    {
        target->left->parent = target;
    }
    new_parent->right = target;
}

void RedBlackTree::fixRedRedViolation(RBTreeNode *target)
{
    if ( this->root == target )
    {
        target->color = BLACK;
        return;
    }
    RBTreeNode *parent = target->parent;
    if (parent->color == BLACK) { return; }

    RBTreeNode *uncle = parent->sibling();

    // We know that parent is not BLACK 
    // (so it's not the root), so grandparent exists
    RBTreeNode *grandparent = parent->parent;

    if (uncle == NULL || (uncle->color == BLACK))
    {
        if (parent->isLeftChild())
        {
            if (target->isLeftChild())
            // Left-Left case
            {
                rightRotation(grandparent);
                parent->swapColor(grandparent);
            }
            else
            // Left-Right case
            {
                leftRotation(parent);
                rightRotation(grandparent);
                target->swapColor(grandparent);
            }            
        }
        else
        {
            if (target->isLeftChild())
            // Right-Left case
            {
                rightRotation(parent);
                leftRotation(grandparent);
                target->swapColor(grandparent);
            }
            else
            // Right-Right case
            {
                leftRotation(grandparent);
                parent->swapColor(grandparent);
            }            
        }        
    }
    else
    // uncle is RED
    {
        parent->color = BLACK;
        uncle->color = BLACK;
        grandparent->color = RED;
        fixRedRedViolation(grandparent);
    }
}

void RedBlackTree::recursiveInsert(RBTreeNode *start, void *element)
{
    int cmp = compare(element, start->data);

    if (cmp == 0) { return; }
    
    if (cmp > 0)
    {
        if (start->right == NULL)
        {
            start->right = new RBTreeNode(RED, element, start);
            fixRedRedViolation(start->right);
            this->num_elements++;
            return;
        }
        recursiveInsert(start->right, element);
    }
    else
    {
        if (start->left == NULL)
        {
            start->left = new RBTreeNode(RED, element, start);
            fixRedRedViolation(start->left);
            this->num_elements++;
            return;
        }
        recursiveInsert(start->left, element);
    }
}

void RedBlackTree::insert(void *element)
{
    if (root == NULL)
    {
        root = new RBTreeNode(BLACK, element, NULL);
        return;
    }
    recursiveInsert(root, element);
}

void* RedBlackTree::search(void *element)
{
    int cmp;
    RBTreeNode *current;
    current = this->root;

    while (current != NULL)
    {
        cmp = compare(element, current->data);
        if (cmp == 0)
        {
            return current->data;
        }
        else if(cmp > 0)
        {
            current = current->right;
        }
        else
        {
            current = current->left;   
        }
    }
    return NULL;
}

unsigned int RedBlackTree::getNumElements()
{
    return this->num_elements;
}