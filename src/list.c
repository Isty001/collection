#include <malloc.h>
#include <stdlib.h>
#include "list.h"


#define node_walk(list, from, direction, ...)   \
        Node *node = list->from##_node;         \
        while (node) {                          \
            __VA_ARGS__;                        \
            node = node->direction;             \
        }                                       \


struct Node {
    Node *next;
    Node *prev;
    void *value;
};


static void *head(List *list)
{
    Node *head = list->head_node;

    return head ? head->value : NULL;
}

static void *end(List *list)
{
    Node *last = list->last_node;

    return last ? last->value : NULL;
}

static Node *node_new(Node *prev, Node *next, void *value)
{
    Node *node = malloc(sizeof(Node));
    node->prev = prev;
    node->next = next;
    node->value = value;

    return node;
}

static void node_free(List *list, Node *node)
{
    if (list->free_item) {
        list->free_item(node->value);
    }
    free(node);
}

static void add_first_node(List *list, Node *new)
{
    list->head_node = list->last_node = new;
    new->prev = new->next = NULL;
}

static List *prepend(List *list, void *value)
{
    Node *new = node_new(NULL, list->head_node, value);

    if (0 < list->count) {
        list->head_node->prev = new;
        list->head_node = new;
    } else {
        add_first_node(list, new);
    }
    list->count++;

    return list;
}

static List *append(List *list, void *value)
{
    Node *new = node_new(list->last_node, NULL, value);

    if (0 < list->count) {
        list->last_node->next = new;
        list->last_node = new;
    } else {
        add_first_node(list, new);
    }
    list->count++;

    return list;
}

static void *remove_end(List *list, Node *node)
{
    if (node == list->head_node) {
        list->head_node = node->next;
    }
    if (node == list->last_node) {
        list->last_node = node->prev;
    }

    list->count--;
    void *val = node->value;
    node_free(list, node);

    return val;
}

static void *pop(List *list)
{
    if (list->last_node) {
        Node *tmp = list->last_node;
        list->last_node = tmp->prev;

        if (tmp->prev) {
            tmp->prev->next = NULL;
        }

        return remove_end(list, tmp);
    }

    return NULL;
}

static void *shift(List *list)
{
    if (list->head_node) {
        Node *tmp = list->head_node;
        list->head_node = tmp->next;

        if (tmp->next) {
            tmp->next->prev = NULL;
        }

        return remove_end(list, tmp);
    }

    return NULL;
}

static List *foreach_l(List *list, Foreach foreach)
{
    node_walk(list, head, next, foreach(node->value));

    return list;
}

static List *foreach_r(List *list, Foreach foreach)
{
    node_walk(list, last, prev, foreach(node->value));

    return list;
}

static List *map(List *list, Map mapper)
{
    node_walk(list, head, next, node->value = mapper(node->value));

    return list;
}

static Node *node_at(List *list, int index)
{
    int i = 0;

    if (index < 0) {
        node_walk(list, last, prev,
                  if (0 == ++index) return node;
        )
    } else {
        node_walk(list, head, next,
                  if (i++ == index) return node;
        )
    }

    return NULL;
}

static void *get(List *list, int index)
{
    Node *node = node_at(list, index);

    if (node) {
        return node->value;
    }
    return NULL;
}

static List *set(List *list, int index, void *value)
{
    Node *node = node_at(list, index);

    if (node) {
        node->value = value;
        list->count++;
    }

    return list;
}

static void delete_node(List *list, Node *node)
{
    if (node) {
        if (node->prev) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
        if (node == list->head_node) {
            list->head_node = node->next;
        }
        if (node == list->last_node) {
            list->last_node = node->prev;
        }
        node_free(list, node);
        list->count--;
    }
}

static List *delete_at(List *list, int index)
{
    Node *node = node_at(list, index);
    delete_node(list, node);

    return list;
}

static List *delete(List *list, void *item)
{
    node_walk(list, head, next,
              if (item == node->value) {
                  delete_node(list, node);
                  break;
              }
    )

    return list;
}

static bool has(List *list, void *searched)
{
    return list->exists(list, function(bool, (void *item) {
        return item == searched;
    }));
}

static void *find(List *list, Predicate predicate)
{
    node_walk(list, head, next,
              if (predicate(node->value)) return node->value;
    )

    return NULL;
}

static bool exists(List *list, Predicate predicate)
{
    /** We can rely on find()'s NULL, because the user supplied value
     * could be also NULL */
    node_walk(list, head, next,
              if (predicate(node->value)) return true;
    )

    return false;
}

static List *filter(List *list, Predicate predicate)
{
    Node *node = list->head_node, *next;

    while (node) {
        next = node->next;
        if (!predicate(node->value)) {
            delete_node(list, node);
        }
        node = next;
    }

    return list;
}

static List *concat(List *list, List *other)
{
    other->foreach_l(other, function(void, (void *item) {
        list->append(list, item);
    }));

    return list;
}

static void *fold_l(List *list, void *value, Fold fold)
{
    node_walk(list, head, next, value = fold(value, node->value));

    return value;
}

static void *fold_r(List *list, void *value, Fold fold)
{
    node_walk(list, last, prev, value = fold(value, node->value));

    return value;
}

static List *clone(List *list)
{
    List *new = list_new();

    list->foreach_l(list, function(void, (void *item) {
        new->append(new, item);
    }));

    return new;
}

static void free_(List *list)
{
    Node *tmp, *head = list->head_node;

    while (head != NULL) {
        tmp = head;
        head = head->next;

        if (list->free_item) {
            list->free_item(tmp->value);
        }
        free(tmp);
    }
    free(list);
}

List *list_new(void)
{
    List *list = malloc(sizeof(List));
    list->count = 0;
    list->clone = clone;
    list->prepend = prepend;
    list->shift = shift;
    list->append = append;
    list->pop = pop;
    list->concat = concat;
    list->get = get;
    list->set = set;
    list->has = has;
    list->exists = exists;
    list->find = find;
    list->delete_at = delete_at;
    list->delete = delete;
    list->foreach_l = foreach_l;
    list->foreach_r = foreach_r;
    list->fold_l = fold_l;
    list->fold_r = fold_r;
    list->map = map;
    list->filter = filter;
    list->head = head;
    list->last = end;
    list->free = free_;
    list->head_node = NULL;
    list->last_node = NULL;
    list->free_item = NULL;

    return list;
}