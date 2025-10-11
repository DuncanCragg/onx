
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tests.h>
#include <onx/log.h>
#include <onx/items.h>

// ---------------------------------------------------------------------------------

void test_list() {

  char buf[32];

  list* li=list_new(4);
                                 list_add(  li, value_new("y"));
                                 list_add(  li, value_new("3"));

  tests_assert(                   list_size( li)==2,              "size should be 2");
  tests_assert(                  !list_get_n(li,-1),              "-1th item is null");
  tests_assert(                  !list_get_n(li,0),               "0th item is null");
  tests_assert_equal(item_to_text(list_get_n(li,1),buf,32), "y", "1st item is y");
  tests_assert_equal(item_to_text(list_get_n(li,2),buf,32), "3", "2nd item is 3");
  tests_assert(                  !list_get_n(li,3),               "3rd item is null");

                                 list_set_n(li,2,value_new("5"));

  tests_assert(                   list_size( li)==2,               "size should still be 2");
  tests_assert_equal(item_to_text(list_get_n(li,2),buf,32), "5",  "2nd item is now 5");

  tests_assert(                  !list_set_n(li,3,value_new("+")), "can't set item out of range");

  tests_assert(                   list_size( li)==2,               "size should still be 2");

  tests_assert(                   list_add(  li,value_new("N")),   "can add a 3rd item");
  tests_assert(                   list_size( li)==3,               "size should be 3");

  tests_assert(                   list_set_n(li,3,value_new("+")), "now can set 3rd item");

  tests_assert(                   list_add(  li,value_new("N")),   "should be able to add a fourth item");
  tests_assert(                   list_size( li)==4,               "size should be 4");

  tests_assert(                  !list_add(  li,value_new("X")),   "shouldn't be able to add a fifth item");
  tests_assert(                   list_size( li)==4,               "size should still be 4");

  list* lj=list_new(4);
  list_add(lj, value_new("z"));
  tests_assert(                  !list_vals_equal(li, lj),         "lists not equal");

  list_free(lj, true);
  lj = list_vals_new_from("y 5 + N", 5);
  tests_assert(                   list_vals_equal(li, lj),         "lists equal");
  list_free(lj, true);

  lj=list_new_from("a",0,value_new("banana"));
  tests_assert_equal_num(         list_size(lj), 3,                   "list_new_from correct size");
  tests_assert_equal(             list_get_n(lj,1),         "a",      "list_new_from correct 1st el");
  tests_assert(                   list_get_n(lj,2)     ==    0,       "list_new_from correct 2nd el");
  tests_assert_equal(item_to_text(list_get_n(lj,3),buf,32), "banana", "list_new_from correct 3rd el");
  list_free(lj, true);

  tests_assert_equal_num(         list_items_find( li, (item*)value_new("y")), 1, "y is found at location 1");
  tests_assert_equal_num(         list_items_find( li, (item*)value_new("5")), 2, "5 is found at location 2");
  tests_assert_equal_num(         list_items_find( li, (item*)value_new("+")), 3, "+ is found at location 3");
  tests_assert_equal_num(         list_items_find( li, (item*)value_new("X")), 0, "X is not found ");

  tests_assert_equal(item_to_text(li, buf, 32), "y 5 + N", "serialise to string works");

  tests_assert(                   list_del_n(li,2),                "can delete 2nd item");
  tests_assert(                   list_del_n(li,3),                "can delete 3rd item");
  tests_assert(                   list_size( li)==2,               "size now 2");

  tests_assert_equal(item_to_text(li, buf, 32), "y +", "serialise to string works");

  tests_assert(                   list_del_n(li,2),                "can delete 2nd item");
  tests_assert(                   list_size( li)==1,               "size now 1");

  tests_assert_equal(item_to_text(li, buf, 32), "y", "serialise to string works");

  tests_assert(                   list_del_n(li,1),                "can delete 1st item");
  tests_assert(                   list_size( li)==0,               "size now 0");

  tests_assert_equal(item_to_text(li, buf, 32), "", "serialise to string works");

  tests_assert(                   list_ins_n(li,1,value_new("3")), "can insert an item into empty");
  tests_assert(                   list_size( li)==1,               "size should be 1");

  tests_assert(                   list_ins_n(li,1,value_new("1")), "can insert another item before it");
  tests_assert(                   list_size( li)==2,               "size should be 2");

  tests_assert_equal(item_to_text(li, buf, 32), "1 3", "serialise to string works");

  tests_assert(                   list_ins_n(li,2,value_new("2")), "can insert another item before it");
  tests_assert(                   list_size( li)==3,               "size should be 3");

  tests_assert_equal(item_to_text(li, buf, 32), "1 2 3", "serialise to string works");

  tests_assert(                   list_ins_n(li,4,value_new("4")), "can insert item at end using index size+1");
  tests_assert(                   list_size( li)==4,               "size should be 4");

  tests_assert_equal(item_to_text(li, buf, 32), "1 2 3 4", "serialise to string works");

                                 list_clear(li, true);
  tests_assert_equal_num(         list_size( li), 0,               "can clear the list");

  list_free(li, false);
  li=list_vals_new_from(" one\n", 1);
  tests_assert(         list_size(li)==1,              "size should be 1");
  tests_assert_equal(item_to_text(li, buf, 32), "one", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from(" one\n ", 1);
  tests_assert(         list_size(li)==1,              "size should be 1");
  tests_assert_equal(item_to_text(li, buf, 32), "one", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from(" one\n two", 2);
  tests_assert(         list_size(li)==2,                  "size should be 2");
  tests_assert_equal(item_to_text(li, buf, 32), "one two", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from(" one\n two  three  \n ", 3);
  tests_assert(         list_size(li)==3,                        "size should be 3");
  tests_assert_equal(item_to_text(li, buf, 32), "one two three", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from_fixed(" one\n");
  tests_assert(         list_size(li)==1,              "size should be 1");
  tests_assert_equal(item_to_text(li, buf, 32), "one", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from_fixed(" one\n ");
  tests_assert(         list_size(li)==1,              "size should be 1");
  tests_assert_equal(item_to_text(li, buf, 32), "one", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from_fixed(" one\n two");
  tests_assert(         list_size(li)==2,                  "size should be 2");
  tests_assert_equal(item_to_text(li, buf, 32), "one two", "can parse whitespace separated lists");

  list_free(li, false);
  li=list_vals_new_from_fixed(" one\n two  three  \n ");
  tests_assert(         list_size(li)==3,                        "size should be 3");
  tests_assert_equal(item_to_text(li, buf, 32), "one two three", "can parse whitespace separated lists");

  list_free(li, false);
}

void run_list_tests() {

  log_write("------list tests-----\n");

  test_list();
}

