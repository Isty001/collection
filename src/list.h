#ifndef ROGUE_CRAFT_LIST_H
#define ROGUE_CRAFT_LIST_H


#include <stdint.h>
#include <stdbool.h>


#define function(return_type, function_body) ({ return_type __fn__ function_body __fn__; })


typedef struct Node Node;
typedef struct List List;
typedef bool (*Predicate)(void *);
typedef void (*Foreach)(void *);
typedef void *(*Map)(void *);
typedef void *(*Fold)(void *value, void *current);

struct List {
    Node *head_node;
    Node *last_node;
    uint32_t count;
    List *(*prepend)(List *, void *);
    void *(*shift)(List *);
    List *(*append)(List *, void *);
    void *(*pop)(List *);
    void *(*head)(List *);
    void *(*last)(List *);
    List *(*foreach_l)(List *, Foreach);
    List *(*foreach_r)(List *, Foreach);
    List *(*map)(List *, Map);
    List *(*filter)(List *, Predicate);
    List *(*concat)(List *, List *);
    List *(*clone)(List *);
    void *(*fold_l)(List *, void *, Fold);
    void *(*fold_r)(List *, void *, Fold);
    void *(*get)(List *, int);
    List *(*set)(List *, int, void *);
    bool (*has)(List *, void *);
    bool (*exists)(List *, Predicate);
    void *(*find)(List *, Predicate);
    List *(*delete_at)(List *, int);
    List *(*delete)(List *, void *);
    void (*free)(List *);
    void (*free_item)(void *);
};


List *list_new(void);


#endif
