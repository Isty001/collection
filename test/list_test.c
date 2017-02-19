#include <string.h>
#include <stdlib.h>
#include "minunit.h"
#include "../src/list.h"


MU_TEST(test_prepend)
{
    List *list = list_new();
    int a = 10, b = 20;

    list->prepend(list, &a);
    mu_assert_int_eq(10, *(int *) list->head(list));
    mu_assert_int_eq(10, *(int *) list->last(list));

    list->prepend(list, &b);
    mu_assert_int_eq(20, *(int *) list->head(list));

    mu_assert_int_eq(2, list->count);

    mu_assert_int_eq(20, *(int *) list->shift(list));
    mu_assert_int_eq(10, *(int *) list->shift(list));

    mu_assert(NULL == list->shift(list), "Should be empty");
    mu_assert_int_eq(0, list->count);

    list->free(list);
}

MU_TEST(test_append)
{
    List *list = list_new();

    int a = 10;

    list->append(list, &a);
    mu_assert_int_eq(10, *(int *) list->last(list));
    mu_assert_int_eq(10, *(int *) list->head(list));

    mu_assert_int_eq(1, list->count);

    int b = 90;
    list->append(list, &b);
    mu_assert_int_eq(2, list->count);

    mu_assert_int_eq(90, *(int *) list->pop(list));
    mu_assert_int_eq(10, *(int *) list->pop(list));

    mu_assert(NULL == list->pop(list), "Should be empty");
    mu_assert_int_eq(0, list->count);

    list->free(list);
}

MU_TEST(test_foreach)
{
    List *list = list_new();
    char left[20] = "", right[20] = "";

    list
        ->append(list, " ")
        ->prepend(list, "Hello")
        ->append(list, "World")
        ->foreach_l(list, function(void, (void *str) {
            sprintf(left, "%s%s", left, (char *) str);
        }))
        ->foreach_r(list, function(void, (void *str) {
            sprintf(right, "%s%s", right, (char *) str);
        }));

    mu_assert_int_eq(0, strcmp("Hello World", left));
    mu_assert_int_eq(0, strcmp("World Hello", right));

    list->free(list);
}

MU_TEST(test_map)
{
    List *list = list_new();
    list->free_item = free;

    list
        ->append(list, "Hello")
        ->append(list, "World")
        ->map(list, function(void *, (void *str) {
            char *buff = malloc(50);
            sprintf(buff, "I say: %s", (char *) str);

            return buff;
        }));

    mu_assert_int_eq(0, strcmp(list->get(list, 0), "I say: Hello"));
    mu_assert_int_eq(0, strcmp(list->get(list, 1), "I say: World"));


    list->free(list);
}

MU_TEST(test_get_index)
{
    List *list = list_new();

    int a = 1, b = 200, c = 500;

    list->append(list, &a)
        ->append(list, &b)
        ->append(list, &c);

    mu_assert_int_eq(a, *(int *) list->get(list, 0));
    mu_assert_int_eq(b, *(int *) list->get(list, 1));
    mu_assert_int_eq(c, *(int *) list->get(list, 2));

    mu_assert_int_eq(c, *(int *) list->get(list, -1));
    mu_assert_int_eq(b, *(int *) list->get(list, -2));
    mu_assert_int_eq(a, *(int *) list->get(list, -3));

    mu_assert(NULL == list->get(list, 100), "This should be NULL");

    list->free(list);
}

MU_TEST(test_fold)
{
    List *list = list_new();
    char *left = malloc(30);
    char *right = malloc(30);
    memcpy(left, "I say:", 7);
    memcpy(right, "I say:", 7);

    list
        ->append(list, "Hello")
        ->append(list, "World");

    void *(*concat)(void *, void *) = function(void *, (void *val, void *item){
        sprintf(val, "%s %s", (char *) val, (char *) item);
        return val;
    });

    left = list->fold_l(list, left, concat);
    right = list->fold_r(list, right, concat);

    mu_assert_int_eq(0, strcmp("I say: Hello World", left));
    mu_assert_int_eq(0, strcmp("I say: World Hello", right));

    list->free(list);
    free(left);
    free(right);
}

MU_TEST(test_clone)
{
    List *list = list_new();
    list->prepend(list, "Test");

    List *new = list->clone(list);

    new->map(new, function(void *, (void *item) {
    (void)item;
        return "Else";
    }));

    mu_assert_int_eq(0, strcmp("Test", list->get(list, 0)));
    mu_assert_int_eq(0, strcmp("Else", new->get(new, 0)));

    list->free(list);
    new->free(new);
}

MU_TEST(test_delete)
{
    List *list = list_new();
    list->append(list, "Hello");

    list->delete_at(list, 0);

    mu_assert(NULL == list->get(list, 0), "Should be deleted");

    int a = 10;

    list->append(list, &a);
    mu_assert(list->has(list, &a), "Should have");

    list->delete(list, &a);
    mu_assert(false == list->has(list, &a), "Shouldn't have");

    list->append(list, "Something else");
    list->set(list, 0, "OK");
    mu_assert_int_eq(0, strcmp("OK", list->get(list, 0)));

    list->free(list);
}

MU_TEST(test_concat)
{
    char *common = "Test";

    List *list = list_new();
    list
        ->append(list, "Unit")
        ->append(list, &common)
        ->append(list, "Test");

    List *other = list_new();
    other
        ->append(other, &common)
        ->append(other, "Hello");

    list->concat_f(list, other);

    mu_assert_int_eq(5, list->count);
    mu_assert_int_eq(0, strcmp("Hello", list->get(list, 4)));

    list->free(list);
}

MU_TEST(test_merge)
{
    char *common = "Hello";

    List *list = list_new();
    list
        ->append(list, "Test")
        ->append(list, common);

    List *other = list_new();
    other
        ->append(other, common)
        ->append(other, "Unit");

    list->merge_f(list, other);

    mu_assert_int_eq(3, list->count);
    mu_assert_int_eq(0, strcmp(common, list->get(list, 1)));

    list->free(list);
}

MU_TEST(test_filter)
{
    List *list = list_new();
    list->append(list, "Unit")
        ->append(list, "Test")
        ->filter(list, function(bool, (void *item) {
            return 0 == strcmp("Test", item);
        }));

    mu_assert_int_eq(1, list->count);
    mu_assert_int_eq(0, strcmp("Test", list->get(list, 0)));

    list->free(list);
}

MU_TEST(test_exists)
{
    int a = 10;
    int b = 10;
    List *list = list_new();

    list->append(list, &a);

    mu_assert(list->has(list, &a), "Should be in the list");
    mu_assert(false == list->has(list, &b), "Should not be in the list");

    Predicate is_ten = function(bool, (void *item) {
        return 10 == *(int *)item;
    });

    bool exists = list->exists(list, is_ten);
    mu_assert(exists, "Should exist");

    mu_assert(&a == list->find(list, is_ten), "Should be the same");

    exists = list->exists(list, function(bool, (void *item) {
        return 943 == *(int *)item;
    }));

    mu_assert(false == exists, "Should not exist");

    list->free(list);
}

MU_TEST(test_complex_op)
{
    int x = 100, y = 999;

    List *other = list_new();
    other->append(other, &x)->append(other, &y);

    List *list = list_new();

    int a = 10, b = 15, c = 20, d = 60;
    int e = 0;

    int sum = *(int *) list
        ->append(list, &a)
        ->prepend(list, &b)
        ->prepend(list, &c)
        ->append(list, &d)
        ->delete(list, &d)
        ->map(list, function(void *, (void *item) {
            *(int *)item += 2;
            return item;
        }))
        ->concat(list, other)
        ->filter(list, function(bool, (void *item) {
            return 0 == *(int *)item % 2;
        }))
        ->fold_l(list, &e, function(void *, (void *val, void *item) {
            *(int *)val += *(int *)item;

            return val;
        }));

    mu_assert_int_eq(134, sum);

    list->free(list);
    other->free(other);
}

int main(void)
{
    MU_RUN_TEST(test_prepend);
    MU_RUN_TEST(test_append);
    MU_RUN_TEST(test_foreach);
    MU_RUN_TEST(test_map);
    MU_RUN_TEST(test_get_index);
    MU_RUN_TEST(test_fold);
    MU_RUN_TEST(test_clone);
    MU_RUN_TEST(test_delete);
    MU_RUN_TEST(test_concat);
    MU_RUN_TEST(test_merge);
    MU_RUN_TEST(test_filter);
    MU_RUN_TEST(test_exists);
    MU_RUN_TEST(test_complex_op);

    MU_REPORT();

    return 0;
}
