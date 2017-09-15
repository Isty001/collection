## C Collection

Doubly linked list based collection in C. 
GCC is required to compile, because of the nested functions, and statement expression.

Check the full API reference below. A quick example:

```c
int main(void)
{
    int initial = 0;
    int a = 10, b = 20, c = 99, d = 50;
    List *first = list_new();
    List *second = list_new();
    
    first
        ->append(first, &c)
        ->append(first, &d);

    int sum = *(int *) second
        ->append(second, &a)
        ->append(second, &b)
        ->concat(second, first->filter(first, (Predicate) function(bool, (int *item) {
            return 0 == *item % 5;
        })))
        ->map(second, (Map) function(int *, (int *item) {
            *item += 5;
            return item;
        }))
        ->fold_l(second, &initial, (Fold) function(int *, (int *val, int *item) {
            *val += *item;
            return val;
        }));

    printf("%d", sum); // Will output 95 

    second->free(second);
    first->free(first);

    return 0;
}
```

## API

The API uses a kinda 'oo' interface, storing the function pointers on the `List` instance
mainly for a bit of syntactic sugar to allow chainable consecutive calls. Thus the functions
return the `List` pointer they received. Of course when passing a `List *` to a method, it does not
matter on which instance it's stored on.

Functions ending with `_f` will free the argument they got, except the first 'self' `List` argument.

GCC allows the following cool macro, (found [here](http://stackoverflow.com/questions/10405436/anonymous-functions-using-gcc-statement-expressions)) that makes possible to simulate anonymous functions, as you can 
see in the examples

```c
#define function(return_type, function_body) ({ return_type __fn__ function_body __fn__; })
```

#### Create and free a List

```c
List *list = list_new();
//
list->free(list);
```

If you need custom allocator functions you can set the default via `list_set_allocators();`
The first two arguments are the `List` node allocator and free functions, the third is a free, for
your items stored. If you pass `NULL` at any argument, the defaults will be used. For custom behavior
on a given instance, just override `release_item`, `alloc_node`, or `release_node` members.


It's also possible to create a copy of an existing `List`. It can be useful if you don't want to
modify it, but rather create a modified version.


```c
List *new = original->clone(original);
```


#### Foreach

You can iterate the `List` from both direction. (left = from head, right = from last)
```c
list
    ->foreach_l(list, function(void, (void *item) {
        //
    }))
    ->foreach_r(list, function(void, (void *item) {
        //
    }));
```


#### Fold

Fold operation is also possible from both directions.

```c
char initial[50] = "";

char *folded = list
    ->append(list, "Unit")
    ->append(list, "Test")
    ->fold_l(list, initial, (Fold) function(char *, (char *val, char *item) {
        sprintf(val, "%s%s", val, item);
        return val;
    }));

puts(folded); // UnitTest

```
With `fold_r` the output will be `TestUnit`


#### Map

Mapping new values to the `List` is also done by a callback function

```c
char hello[10] = "Hello";
char hi[10] = "Hi";
    
greetings
    ->append(greetings, hello)
    ->append(greetings, hi)
    ->map(greetings, (Map) function(char *, (char *greeting) {
        sprintf(greeting, "%s!!", greeting);
        return greeting;
    }))
    ->foreach_l(greetings, (Foreach) puts);
    
    //Hello!!
    //Hi!!
```


#### Filter

If the callback returns `false`, the item will be removed

```c
list->append(list, "Unit")
    ->append(list, "Test")
    ->filter(list, function(bool, (void *item) {
        return 0 == strcmp("Test", item);
    }))
    ->foreach_l(list, (Foreach) puts);
    
    //Test
```


#### Concat and Merge

Items of the other `List` will be simply appended to the other

```c
char *unit = "Unit ";

List *list_1 = list_new();
list_1
    ->append(list_1, unit)
    ->append(list_1, "Test");

List *list_2 = list_new();
list_2
    ->append(list_2, "I ")
    ->append(list_2, "said: ")
    ->append(list_2, unit)
    ->concat(list_2, list_1)
    ->foreach_l(list_2, (Foreach) printf);
    
    // I said: Unit Unit Test
```

Merge method is the same, except that it will only add items that are not already in the `List`.
Using `merge()` with the above example, the otput will be `I said: Unit Test`


#### Manipulating the ends of the List:

add: 
```c
list->prepend(list, &item)->append(list, &other_item);
```

remove:
```c
void *first = list->shift(list);
void *last = list->pop(list);
```

replace:
```c
list->replace(list, &from, &to);
```

get the first/last item without removing
```c
void *first = list->head(list);
void *last = list->last(list);
```

If the list is empty, `NULL` will be returned


#### Accessing elements of the List

by index:

```c
if (0 == strcmp("delete_me", list->get(list, 0))) {
    list->delete_at(list, 0);
} else if (0 == strcmp("replace_me", list->get(list, 10))) {
    list->set(list, 10, "Something else");
}
```
Negative indexes can also be used. For example, -2 will be the second from the last.
`get()` will return `NULL` if the index is out of bounds.
`set()`, `delete_at()` and `delete()` will ignore the invalid indexes.


You can always check the size of the `List` via the `count` field.


or by pointer:

```c
if (list->has(list, &item)) {
    list->delete(list, &item);
}
```

You can also find/test items by predicates, this will return on the first match:

```c
Predicate is_ten = function(bool, (void *item) {
    return 10 == *(int *)item;
});

bool exists = list->exists(list, is_ten);
int *item = list->find(list, is_ten);
```
