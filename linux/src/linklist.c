#include<stdio.h>
#include<stdlib.h>
#include<string.h>

// #include "heap_4.h"
#include "mem.h"
#include "misc.h"
#include "linklist.h"

// static List_Header timer_list = INIT_LIST_HEADER;
//
//                     Header.first                            Header.tail
//                     V                                       V     
//                      _____     _____     _____     _____     _____  
//             NULL <--|     |<--|     |<--|     |<--|     |<--|     | (Header.node_cnt = 5) 
//                     |  1  |   |  2  |   |  3  |   |  4  |   |  5  |
//                     |_____|-->|_____|-->|_____|-->|_____|-->|_____|--> NULL
// Node Index :           0         1         2         3         4
// Insert Idx :      0         1         2         3         4         5

int list_insert_tail(list_header head, void *ins)
{
    if(!head || !ins) return -1;

    list_node *new = (list_node *)omalloc("list node", sizeof(list_node));
    if(!new) return -1;

    // first node add
    if(!head->first) {
        head->first = head->tail = new;
        new->next = NULL;
        new->prev = NULL;
        new->ins = ins;
    }
    else {
        head->tail->next = new;
        new->prev = head->tail;
        new->next = NULL;
        new->ins = ins;
        head->tail = new;
    }

    return head->node_cnt++;
}

int list_insert_first(list_header head, void *ins)
{
    if(!head || !ins) return -1;

    list_node *new = (list_node *)omalloc("list node", sizeof(list_node));

    // first node add
    if(!head->first) {
        head->first = head->tail = new;
        new->next = NULL;
        new->prev = NULL;
        new->ins = ins;
    }
    else {
        head->first->prev = new;
        new->prev = NULL;
        new->next = head->first;
        new->ins = ins;
        head->first = new;
    }

    head->node_cnt++;

    return 0;
}

int list_insert_index(list_header head, void *ins, unsigned int idx)
{
    if(!head || !ins) return -1;

    if(!idx) return list_insert_first(head, ins);
    if(idx >= head->node_cnt) list_insert_tail(head, ins);

    list_node *new = (list_node *)omalloc("list node", sizeof(list_node));

    list_node *p_node = head->first;

    for(unsigned int cnt = 1; p_node; ++cnt)
    {
        if(cnt == idx)
        {
            p_node->next->prev = new;
            new->next = p_node->next;

            new->prev = p_node;
            p_node->next = new;

            head->node_cnt++;

            return cnt;
        }
        else p_node = p_node->next;
    }

    return -1;
}

void *list_get_tail(list_header head)
{
    if(!head || !head->node_cnt) return NULL;

    return head->tail->ins;
}

void *list_get_first(list_header head)
{
    if(!head || !head->node_cnt) return NULL;

    return head->first->ins;
}

void *list_get_index(list_header head, unsigned int idx)
{
    if(!head || idx > head->node_cnt) return NULL;

    if(!idx) return list_get_first(head);

    list_node *p_node = head->first;

    for(unsigned int cnt = 0; p_node; ++cnt)
    {
        if(cnt == idx) return p_node->ins;
        else p_node = p_node->next;
    }

    return NULL;
}

int list_remove_tail(list_header head)
{
    if(!head) return -1;

    if(!head->tail) return 0;

    list_node *tail_node = head->tail;

    // have prev node
    if (tail_node->prev) {
        tail_node->prev->next = tail_node->next;
    }

    head->tail = tail_node->prev;

    // only one node
    if (head->first == tail_node) {
        head->first = tail_node->next;
    }

    ofree(tail_node);
    head->node_cnt--;

    return 0;
}

int list_remove_first(list_header head)
{
    if(!head) return -1;

    if(!head->first) return 0;

    list_node *first_node = head->first;

    // have next node
    if (first_node->next) {
        first_node->next->prev = first_node->prev;
    }

    head->first = first_node->next;

    // only one node
    if (head->tail == first_node) {
        head->tail = first_node->prev;
    }

    ofree(first_node);
    head->node_cnt--;

    return 0;
}

int list_remove_index(list_header head, unsigned int idx)
{
    if(!head || idx > head->node_cnt) return -1;

    if(!idx) return list_remove_first(head);

    list_node *p_node = head->first;

    for(unsigned int cnt = 0; p_node; ++cnt) {
        if(cnt == idx) {
            if(p_node == head->first)
                return list_remove_first(head);
            else if(p_node == head->tail)
                return list_remove_tail(head);
            else {
                p_node->prev->next = p_node->next;
                p_node->next->prev = p_node->prev;

                ofree(p_node);
                head->node_cnt--;

                return 0;
            }
        }
        else p_node = p_node->next;
    }

    return -1;
}

int list_header_init(list_header *head)
{
    if(!head) return -1;
    if(*head) return 0;

    *head = (list_header)omalloc("list header", sizeof(struct __list_header));
    if(*head) memset(*head, 0, sizeof(struct __list_header));
    else return -1;

    return 0;
}

int list_header_delete(list_header *head)
{
    if(!head) return -1;
    if(!*head) return 0;

    while(!(*head)->first && !(*head)->tail)
        list_remove_first(*head);

    ofree(*head);

    return 0;
}

int list_header_length(list_header head)
{
    if(!head) return 0;

    return head->node_cnt;
}

// ---- Linklist Object ----


int Linklist_AddFirst(linklist *ll, void *node)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    ll->lock(ll);
    int ret = list_insert_first(&(ll->head), node);
    ll->unlock(ll);

    return ret;
}
int Linklist_AddEnd(linklist *ll, void *node)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    ll->lock(ll);
    int ret = list_insert_tail(&(ll->head), node);
    ll->unlock(ll);

    return ret;
}
int Linklist_AddMiddle(linklist *ll, int idx , void *node)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    ll->lock(ll);
    int ret = list_insert_index(&(ll->head), node, idx);
    ll->unlock(ll);

    return ret;
}

void *Linklist_GetFirst(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return NULL;
    }

    ll->lock(ll);
    void *ret = list_get_first(&(ll->head));
    ll->unlock(ll);

    return ret;
}
void *Linklist_GetEnd(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return NULL;
    }

    ll->lock(ll);
    void *ret = list_get_tail(&(ll->head));
    ll->unlock(ll);

    return ret;
}
void *Linklist_GetMiddle(linklist *ll, int idx)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return NULL;
    }

    ll->lock(ll);
    void *ret = list_get_index(&(ll->head), idx);
    ll->unlock(ll);

    return ret;
}

int Linklist_RemoveFirst(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    return list_remove_first(&(ll->head));
}
int Linklist_RemoveEnd(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    return list_remove_tail(&(ll->head));
}
int Linklist_RemoveMiddle(linklist *ll, int idx)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    return list_remove_index(&(ll->head), idx);
}

int Linklist_Lock(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    return ll->mutex->lock(ll->mutex);
}
int Linklist_Unlock(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    return ll->mutex->unlock(ll->mutex);
}
int Linklist_Length(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    ll->lock(ll);
    int len = list_header_length(&(ll->head));
    ll->unlock(ll);

    return len;
}
int Linklist_Destroy(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    if (ll->len(ll)) {
        PRINT_ERR("%s() link list busy ( please remove all node first )\n", __func__);
        return -1;
    }

    if (ll->mutex) {
        ll->mutex->destroy(ll->mutex);
        ll->mutex = NULL;
    }

    ofree(ll);

    return 0;
}
int Linklist_Init(linklist *ll)
{
    if (!ll) {
        PRINT_ERR("%s() invliad args\n", __func__);
        return -1;
    }

    memset(&(ll->head), 0, sizeof(struct __list_header));

    ll->mutex = mutex_new();
    if (!ll->mutex) {
        PRINT_ERR("%s() mutex new failed\n", __func__);
        return -1;
    } else if (ll->mutex->init(ll->mutex)) {
        PRINT_ERR("%s() mutex init failed\n", __func__);
        return -1;
    }

    return 0;
}
linklist *LinklistNew(void)
{
    linklist *new_link = (linklist *)omalloc("link list obj", sizeof(linklist));
    if (!new_link) {
        PRINT_ERR("%s() omalloc failed\n", __func__);
        return NULL;
    }

    new_link->init = Linklist_Init;
    new_link->destroy = Linklist_Destroy;
    new_link->len = Linklist_Length;
    new_link->lock = Linklist_Lock;
    new_link->unlock = Linklist_Unlock;

    new_link->add_first = Linklist_AddFirst;
    new_link->add_end = Linklist_AddEnd;
    new_link->add_middle = Linklist_AddMiddle;

    new_link->get_first = Linklist_GetFirst;
    new_link->get_end = Linklist_GetEnd;
    new_link->get_middle = Linklist_GetMiddle;

    new_link->rm_first = Linklist_RemoveFirst;
    new_link->rm_end = Linklist_RemoveEnd;
    new_link->rm_middle = Linklist_RemoveMiddle;

    return new_link;
}
