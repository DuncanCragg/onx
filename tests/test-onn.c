
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <onx/lib.h>
#include <onx/log.h>
#include <onx/time.h>

#include <persistence.h>
#include <onn.h>

#include <tests.h>

// ---------------------------------------------------------------------------------

#define TEXTBUFFLEN 256
char textbuff[TEXTBUFFLEN];

uint8_t evaluate_setup_called=0;

bool evaluate_setup(object* n1, void* d) {
  evaluate_setup_called++;
  return true;
}

void test_object_set_up() {

  object* nr=object_new(0, 0, "random uid", 4);
  char* random_uid_1 = object_property(nr, "UID");
  nr=object_new(0, 0, "random uid", 4);
  char* random_uid_2 = object_property(nr, "UID");

  tests_assert(      strlen(random_uid_1)==23,                "UID generation returns long string");
  tests_assert(      strlen(random_uid_2)==23,                "UID generation returns long string");
  tests_assert(      strcmp(random_uid_1, random_uid_2),      "UID generation creates unique UIDs");

  tests_assert(      object_property_length(  nr, "is")==2,           "property 'is' is a list of 2 items");
  tests_assert_equal(object_property(         nr, "is"), "random",    "pull off 1st value in list" );
  tests_assert(      object_property_contains(nr, "is",    "random"), "object_new parses 'is' first list item" );
  tests_assert(      object_property_contains(nr, "is",    "uid"   ), "object_new parses 'is' first list item" );
  tests_assert_equal(object_property(         nr, "is:1"), "random",  "object_new parses 'is' first list item" );
  tests_assert_equal(object_property_get_n(   nr, "is", 1),"random",  "object_new parses 'is' first list item passing index" );
  tests_assert(      object_property_is(      nr, "is:1",  "random"), "object_new parses 'is' first list item" );
  tests_assert(      object_property_is(      nr, "is:2",  "uid"),    "object_new parses 'is' second list item" );
  tests_assert(     !object_property(         nr, "is:1:"),           "cannot end in :" );
  tests_assert(     !object_property_is(      nr, "is:2:",  "uid"),   "cannot end in :" );

  onn_set_evaluators("eval_default", evaluate_setup, 0);
  object* n1=object_new("uid-1", "eval_default", "setup", 4);
  // UID: uid-1  is: setup
  tests_assert(      n1,                                     "object_new created an object");
  tests_assert(      onn_get_from_cache("uid-1")==n1,        "onn_get_from_cache can find uid-1");
  tests_assert(     !onn_get_from_cache("uid-2"),            "onn_get_from_cache can't find uid-2");

  tests_assert(      object_property_is(    n1, "UID", "uid-1"),  "object_new saves uid as a (virtual) property");
  tests_assert_equal(object_property(       n1, "is"), "setup",   "object_property returns 'is'" );
  tests_assert(      object_property_is(    n1, "is",  "setup"),  "object_property_is says 'is' is 'setup'");
  tests_assert_equal_num(object_property_length(n1, "is"), 1,     "property 'is' is a single value");

                    object_property_set(           n1, "state", "good");
  tests_assert(      object_property_is(            n1, "state", "good"), "object_property_is says 'state' is 'good'");
  tests_assert(      object_property_contains(      n1, "state", "good"), "object_property_contains says it is 'good'");
  tests_assert(      object_property_length(        n1, "state")==1,      "property 'state' is a single value");
  tests_assert(      object_property_length(        n1, "banana")==0,     "property 'banana' is empty");
  tests_assert(      object_property_size(          n1, "state")== -1,    "property 'state' is not a properties");
  // UID: uid-1  is: setup  state: good
                    object_property_add(           n1, "state", "mostly");
  tests_assert(     !object_property_is(            n1, "state", "good"),   "object_property_is says 'state' is not all 'good'");
  tests_assert(      object_property_contains(      n1, "state", "good"),   "object_property_contains says it is 'good'");
  tests_assert(      object_property_contains(      n1, "state", "mostly"), "object_property_contains says it is 'mostly'");

                        object_property_set(   n1, "number", "2147483647");
  tests_assert_equal_num(object_property_int32( n1, "number"), 2147483647, "object_property_int32() parses numbers: 2147483647");
                        object_property_set(   n1, "number", "000");
  tests_assert_equal_num(object_property_int32( n1, "number"), 000,        "object_property_int32() parses numbers: 000");
                        object_property_set(   n1, "number", "-2147483648");
  tests_assert_equal_num(object_property_int32( n1, "number"), -2147483648,"object_property_int32() parses numbers: -2147483648");
                        object_property_set(   n1, "number", "-2147483648.0~%");
  tests_assert_equal_num(object_property_int32( n1, "number"), -2147483648,"object_property_int32() parses numbers: -2147483648.0~%");
                        object_property_set(   n1, "number", "2.87654321");
  tests_assert_equal_num(object_property_int32( n1, "number"), 2,          "object_property_int32() parses numbers: '2.87654321'");

                        object_property_set(   n1, "number", "!");
  tests_assert_equal_num(object_property_int32( n1, "number"), 0,          "object_property_int32() parses numbers not '!'");
                        object_property_set(   n1, "number", "876543218765432187654321");
  tests_assert_equal_num(object_property_int32( n1, "number"), 0,          "object_property_int32() parses numbers not '876543218765432187654321'");
                        object_property_set(   n1, "number", 0);
  tests_assert_equal_num(object_property_int32( n1, "number"), 0,          "object_property_int32() parses numbers not nothing there");

  tests_assert_equal_num(object_property_length(  n1, "state"), 2,   "property 'state' is now a list of two");
  tests_assert_equal_num(object_property_size(    n1, "state"),  -1, "property 'state' is not a properties");
  // UID: uid-1  is: setup  state: good mostly
  tests_assert(          object_property_add(     n1, "1", "a"),  "can add new property");
  tests_assert_equal_num(object_property_length(  n1, "1"), 1,    "property '1' is a single value");
  tests_assert(          object_property_size(    n1, "1")== -1,  "not a properties");
  // UID: uid-1  is: setup  state: good mostly  1: a
  tests_assert_equal(object_property(         n1, "1:1"), "a",  "1st value in list can be found by path-indexing and is 'a'");
  tests_assert(     !object_property(         n1, "1:1:"),      "cannot end in :");
  tests_assert(     !object_property(         n1, "1:2"),       "2nd value in list by path index is null");
  tests_assert(     !object_property(         n1, "1:0"),       "0th value in list by path index is null");
  tests_assert_equal(object_property_get_n(   n1, "1", 1), "a", "1st value in list can be found by index and is 'a'");
  tests_assert(     !object_property_get_n(   n1, "1", 2),      "2nd value in list by index is null");
  tests_assert(     !object_property_get_n(   n1, "1", 0),      "0th value in list by index is null");
  tests_assert(     !object_property_val(     n1, "1", 1),      "1st value in list can't be found by indexing which is for properties only");

  tests_assert(      object_property_add(     n1, "1", "b"),    "can add another");
  tests_assert(      object_property_length(  n1, "1")==2,      "property '1' is now a list of two");
  tests_assert(      object_property_size(    n1, "1")== -1,    "not a properties");
  tests_assert(     !object_property_val(     n1, "1", 2),      "2nd value in list by index is null");
  tests_assert(     !object_property(         n1, "1:0"),       "0th value in list is null");
  tests_assert_equal(object_property(         n1, "1:1"), "a",  "1st value in list is 'a'");
  tests_assert_equal(object_property(         n1, "1:2"), "b",  "2nd value in list is 'b'");
  tests_assert(     !object_property(         n1, "1:1:"),      "cannot end in :");
  tests_assert(     !object_property(         n1, "1:2:"),      "cannot end in :'");
  tests_assert(     !object_property(         n1, "1:3"),       "3rd value in list is null");
  tests_assert(     !object_property_get_n(   n1, "1", 0),      "0th value in list is null");
  tests_assert_equal(object_property_get_n(   n1, "1", 1), "a", "1st value in list is 'a'");
  tests_assert_equal(object_property_get_n(   n1, "1", 2), "b", "2nd value in list is 'b'");
  tests_assert(     !object_property_get_n(   n1, "1:", 1),     "cannot end in :");
  tests_assert(     !object_property_get_n(   n1, "1:", 2),     "cannot end in :");
  tests_assert(     !object_property_get_n(   n1, "1", 3),      "3rd value in list is null");
  tests_assert(     !object_property(         n1, "1:four"),    "four-th value in list is null");
  // UID: uid-1  is: setup  state: good mostly  1: a b
  tests_assert(      object_property_add(     n1, "1", "c"),    "can add a third to existing list");
  tests_assert(      object_property_length(  n1, "1")==3,      "three items in the list");
  tests_assert(     !object_property_val(     n1, "1", 3),      "3rd value in list by index is null");
  tests_assert_equal(object_property(         n1, "1:1"), "a",  "1st value in list is 'a'");
  tests_assert_equal(object_property(         n1, "1:2"), "b",  "2nd value in list is 'b'");
  tests_assert_equal(object_property(         n1, "1:3"), "c",  "3rd value in list is 'c'");
  tests_assert(     !object_property(         n1, "1:4"),       "4th value in list is null");
  // UID: uid-1  is: setup  state: good mostly  1: a b c
  tests_assert(      object_property_set(     n1, "1:2", "B"),  "can set 2nd value in list");
  tests_assert_equal(object_property(         n1, "1:2"), "B",  "2nd value in list is 'B'");
  tests_assert(     !object_property_set(     n1, "1:4", "X"),  "can't set 4th value in list");
  // UID: uid-1  is: setup  state: good mostly  1: a B c
  tests_assert(      object_property_set(     n1, "1:2", ""),   "can set 2nd value in list to empty to delete");
  tests_assert(      object_property_length(  n1, "1")==2,      "now two items in the list");
  // UID: uid-1  is: setup  state: good mostly  1: a c
  tests_assert(      object_property_set(     n1, "1:2", 0),    "can set 2nd value in list to null to delete");
  tests_assert(      object_property_length(  n1, "1")==1,      "now one item in the list");
  tests_assert_equal(object_property(         n1, "1"), "a",    "property '1' is 'a'");
  // UID: uid-1  is: setup  state: good mostly  1: a
  tests_assert(      object_property_set(     n1, "1:1", ""),   "can set 1st value in 'list' to empty to delete");
  tests_assert(      object_property_length(  n1, "1")==0,      "now no property");
  tests_assert(     !object_property(         n1, "1"),         "now no such property");
  // UID: uid-1  is: setup  state: good mostly
  tests_assert(      object_property_set(     n1, "1", "a"),    "can set property back");
  tests_assert(      object_property_add(     n1, "1", "c"),    "and another");
  // UID: uid-1  is: setup  state: good mostly  1: a c
  tests_assert(      object_property_set(     n1, "2", "ok"),   "can set 2 more properties");
  tests_assert(      object_property_length(  n1, "2")==1,      "property '1' is a value");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok
  tests_assert(     !object_property_set(     n1, "3", "not ok"),  "can't set 3 more properties");
  tests_assert(     !object_property_set(     n1, "4", "not ok"),  "can't set 4 more properties");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok
  tests_assert(     !object_property(         n1, "4"),       "empty property returns null");
  tests_assert(      object_property_is(      n1, "4", ""),   "empty property is empty");
  tests_assert(      object_property_contains(n1, "4", ""),   "empty property is empty");
  tests_assert(     !object_property_length(  n1, "4"),       "empty property is not a value");
  // UID: uid-1  is: setup  state: good mostly  1: a c    2: ok
  tests_assert(      object_property_append(  n1, "1", "e"), "add another using append macro");
  tests_assert_equal(object_property(         n1, "1:1"), "a",  "1st is a");
  tests_assert_equal(object_property(         n1, "1:2"), "c",  "2nd is c");
  tests_assert_equal(object_property(         n1, "1:3"), "e",  "3rd is e");
  // UID: uid-1  is: setup  state: good mostly  1: a c e  2: ok
  tests_assert(      object_property_set(     n1, "1:3", 0),      "delete 'e'");
  tests_assert(     !object_property(         n1, "1:3"),         "3rd is not e");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok
  tests_assert(      object_property_append(  n1, "2", "mate"),   "add another using append macro");
  tests_assert_equal(object_property(         n1, "2:2"), "mate", "2nd is mate");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok mate
  tests_assert(      object_property_length(  n1, "2")==2,        "2 els");
  tests_assert(      object_property_set(     n1, "2", 0),        "can delete whole list");
  tests_assert(      object_property_length(  n1, "2")==0,        "check empty");
  // UID: uid-1  is: setup  state: good mostly  1: a c
  tests_assert(      object_property_prepend( n1, "2", "mate"),   "can prepend to list using prepend macro");
  tests_assert_equal(object_property(         n1, "2:1"), "mate", "1st is mate");
  tests_assert(      object_property_length(  n1, "2")==1,        "1 el");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: mate
  tests_assert(      object_property_insert(  n1, "2", "ok"),     "can insert (prepend) into list");
  tests_assert_equal(object_property(         n1, "2:1"), "ok",   "1st is now 'ok'");
  tests_assert(      object_property_length(  n1, "2")==2,        "2 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok mate
  tests_assert(      object_property_insert(  n1, "2", "well"),   "can insert (prepend) into list");
  tests_assert_equal(object_property(         n1, "2:1"), "well", "1st is now 'well'");
  tests_assert_equal(object_property(         n1, "2:2"), "ok",   "2nd is now 'ok'");
  tests_assert_equal(object_property(         n1, "2:3"), "mate", "3rd is now 'mate'");
  tests_assert(      object_property_length(  n1, "2")==3,        "3 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: well ok mate
  tests_assert(      object_property_insert(  n1, "2:2",  "that's"),"can do indexed insert");
  tests_assert_equal(object_property(         n1, "2:1"), "well",   "1st is 'well'");
  tests_assert_equal(object_property(         n1, "2:2"), "that's", "2nd is now 'thats'");
  tests_assert_equal(object_property(         n1, "2:3"), "ok",     "3rd is now 'ok'");
  tests_assert_equal(object_property(         n1, "2:4"), "mate",   "4th is now 'mate'");
  tests_assert(      object_property_length(  n1, "2")==4,          "4 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: well that's ok mate
  tests_assert(      object_property_insert(  n1, "2:1",  "so"),    "can do indexed insert @1");
  tests_assert_equal(object_property(         n1, "2:1"), "so",     "1st is 'so'");
  tests_assert_equal(object_property(         n1, "2:2"), "well",   "2nd is now 'well'");
  tests_assert(      object_property_length(  n1, "2")==5,          "5 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: so well that's ok mate
  tests_assert(      object_property_insert(  n1, "2:6",  "innit"), "can do indexed insert @ end");
  tests_assert_equal(object_property(         n1, "2:5"), "mate",   "5th is 'mate'");
  tests_assert_equal(object_property(         n1, "2:6"), "innit",  "6th is now 'innit'");
  tests_assert(      object_property_length(  n1, "2")==6,          "6 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: so well that's ok mate innit
  tests_assert(     !object_property_insert(  n1, "2:8", "yeah?"),  "can't do indexed insert @ end +1");
  tests_assert(     !object_property(         n1, "2:8"),           "no 8th el");
  tests_assert(      object_property_length(  n1, "2")==6,          "6 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: so well that's ok mate innit
  tests_assert(      object_property_append(  n1, "2:2",  "hmmm"),   "can do indexed append");
  tests_assert_equal(object_property(         n1, "2:1"), "so",      "1st is 'so'");
  tests_assert_equal(object_property(         n1, "2:2"), "well",    "2nd is 'well'");
  tests_assert_equal(object_property(         n1, "2:3"), "hmmm",    "3rd is now 'hmmm'");
  tests_assert_equal(object_property(         n1, "2:4"), "that\'s", "4th is now 'that\'s'");
  tests_assert(      object_property_length(  n1, "2")==7,           "7 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: so well hmmm that's ok mate innit
  tests_assert(      object_property_append(  n1, "2:7",  "bruv"),   "can do indexed append @ end");
  tests_assert_equal(object_property(         n1, "2:6"), "mate",    "6th is 'mate'");
  tests_assert_equal(object_property(         n1, "2:7"), "innit",   "7th is 'innit'");
  tests_assert_equal(object_property(         n1, "2:8"), "bruv",    "8th is now 'bruv'");
  tests_assert(      object_property_length(  n1, "2")==8,           "8 els");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: so well hmmm that's ok mate innit bruv
  tests_assert(      object_property_set(     n1, "2", ""),   "can set property to empty");
  tests_assert(     !object_property(         n1, "2"),       "empty property returns null");
  tests_assert(      object_property_is(      n1, "2", 0),    "empty property is empty");
  tests_assert(      object_property_contains(n1, "2", 0),    "empty property is empty");
  tests_assert(     !object_property_length(  n1, "2"),       "empty property is not a value");
  tests_assert(      object_property_set(     n1, "2", "ok"), "can set empty property back");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok
  tests_assert(      object_property_set(     n1, "2", 0),    "can set property to null");
  tests_assert(     !object_property(         n1, "2"),       "empty property returns null");
  tests_assert(      object_property_is(      n1, "2", ""),   "empty property is empty");
  tests_assert(      object_property_contains(n1, "2", 0),    "empty property is empty");
  tests_assert(     !object_property_length(  n1, "2"),       "empty property is not a value");
  // UID: uid-1  is: setup  state: good mostly  1: a c
  tests_assert(      object_property_set(     n1, "2", "ok m8"), "can set property to two items");
  tests_assert(      object_property_contains(n1, "2", "ok"),    "can get 'ok' back");
  tests_assert(      object_property_contains(n1, "2", "m8"),    "can get 'm8' back");
  tests_assert(      object_property_length(  n1, "2")==2,       "property '2' is a list of two");
  tests_assert(      object_property_is(      n1, "2:1", "ok"),  "list items are correct");
  tests_assert(      object_property_is(      n1, "2:2", "m8"),  "list items are correct");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok m8
  tests_assert(      object_property_size(    n1, ":")==4,       "there are four properties");
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok m8
  tests_assert_equal(object_property_key(     n1, ":", 1), "is",    "key of 1st item is 'is'");
  tests_assert_equal(object_property_val(     n1, ":", 1), "setup", "val of 1st item is 'setup'");
  tests_assert_equal(object_property_key(     n1, ":", 2), "state", "key of 2nd item is 'state'");
  tests_assert(     !object_property_val(     n1, ":", 2),          "val of 2nd item is not just a value");
  tests_assert_equal(object_property_key(     n1, ":", 3), "1",     "key of 3rd item is '1'");
  tests_assert(     !object_property_val(     n1, ":", 3),          "val of 3rd item is not just a value");
  tests_assert_equal(object_property_key(     n1, ":", 4), "2",     "key of 4th item is '2'");
  tests_assert(     !object_property_val(     n1, ":", 4),          "val of 4th item is not just a value");

  tests_assert(     !object_property_key(     n1, ":", 5),          "key of 5th item is 0");
  tests_assert(     !object_property_val(     n1, ":", 5),          "val of 5th item is 0");
  tests_assert(     !object_property_key(     n1, ":", 0),          "key of 0th item is 0");
  tests_assert(     !object_property_val(     n1, ":", 0),          "val of 0th item is 0");

  object* nv=object_new("uid-6", "eval_default", "varargs", 4);
  tests_assert(      object_property_set(     nv, "v", "x"),                  "can set val");
  tests_assert(      object_property_add(     nv, "v", "y"),                  "can add val");
  tests_assert(      object_property_set_list(nv, "v", "a", "b", 0),          "can set two vals using varargs");
  tests_assert(      object_property_length(  nv, "v")==2,                    "property 'v' is a list of two");
  tests_assert(      object_property_is(      nv, "v:1", "a"),                "list items are correct");
  tests_assert(      object_property_is(      nv, "v:2", "b"),                "list items are correct");
  tests_assert(      object_property_add_list(nv, "v", "c", "d", 0),          "can add two more vals using varargs");
  tests_assert(      object_property_length(  nv, "v")==4,                    "property 'v' is a list of two");
  tests_assert(      object_property_is(      nv, "v:3", "c"),                "list items are correct");
  tests_assert(      object_property_is(      nv, "v:4", "d"),                "list items are correct");
  tests_assert(      object_property_add_list(nv, "w", "e", "f", 0),          "can add two more vals using varargs");
  tests_assert(      object_property_length(  nv, "w")==2,                    "property 'v' is a list of two");
  tests_assert(      object_property_is(      nv, "w:1", "e"),                "list items are correct");
  tests_assert(      object_property_is(      nv, "w:2", "f"),                "list items are correct");
  tests_assert(      object_property_set_fmt( nv, "x", "(%c) [%d]", '*', 20), "can add two vals using varargs and format");
  tests_assert(      object_property_length(  nv, "x")==2,                    "property 'x' is a list of two");
  tests_assert(      object_property_is(      nv, "x:1", "(*)"),              "list items are correct");
  tests_assert(      object_property_is(      nv, "x:2", "[20]"),             "list items are correct");
  // { UID: uid-6  Eval: eval_default  is: varargs  v: a b c d  w: e f  x: (*) [20] }
  tests_assert(      object_property_set_n(   nv, "w", 1, "E"),          "can set 1st el in list");
  tests_assert(      object_property_is(      nv, "w:1", "E"),           "list items are correct");
  tests_assert(      object_property_set_n(   nv, "v", 3, "C"),          "can set 3rd el in list");
  tests_assert(      object_property_is(      nv, "v:3", "C"),           "list items are correct");
  tests_assert(      object_property_set_n(   nv, "v", 3, ""),           "can set 3rd el in list to empty to delete");
  tests_assert(      object_property_is(      nv, "v:3", "d"),           "list items are correct");
  tests_assert(      object_property_set_n(   nv, "v", 1, 0),            "can set 1st el in list to null to delete");
  tests_assert(      object_property_is(      nv, "v:1", "b"),           "list items are correct");
  tests_assert(      object_property_is(      nv, "v:2", "d"),           "list items are correct");
  tests_assert(     !object_property_set_n(   nv, "v", 3, "C"),          "can't set 3rd el in list");
  tests_assert(      object_property_set_n(   nv, "v", 1, ""),           "can delete 1st el in list");
  tests_assert(      object_property_is(      nv, "v:1", "d"),           "list items are correct");
  tests_assert(      object_property_is(      nv, "v", "d"),             "non-list items are correct");
  tests_assert(     !object_property_set_n(   nv, "v", 2, "c"),          "can't set 2nd el in non-list");
  tests_assert(     !object_property_set_n(   nv, "v", 2, 0),            "can't del 2nd el in non-list");
  tests_assert(     !object_property_set(     nv, "v:2", "c"),           "can't set 2nd el in non-list");
  tests_assert(     !object_property_set(     nv, "v:2", 0),             "can't del 2nd el in non-list");
  tests_assert(     !object_property_get_n(   nv, "v", 2),               "can't get 2nd el in non-list");
  tests_assert(     !object_property(         nv, "v:2"),                "can't get 2nd el in non-list");
  // { UID: uid-6 Eval: eval_default is: varargs v: d w: E f x: 10 20 }
}

// ---------------------------------------------------------------------------------

void test_device() {

  log_write("------device behaviour tests-----\n");

  object* nd=onn_device_object;
  tests_assert(!!nd,                                             "device object is set");
  tests_assert(object_property_contains(nd, "is", "device"),     "it's a device");

  object_property_set(onn_device_object, "incoming", "uid-incomingdevice");
  object_property(onn_device_object, "incoming:UID");
  object* incomingdevice=onn_get_from_cache("uid-incomingdevice");
  object_property_set(incomingdevice, "is", "device");

  onn_loop();

  tests_assert_equal(object_property(onn_device_object, "peers"), "uid-incomingdevice", "device evaluator adds incoming device to peers");
}


// ---------------------------------------------------------------------------------

uint8_t evaluate_local_state_n2_called=0;

bool evaluate_local_state_n2(object* n2, void* d) {
  evaluate_local_state_n2_called++;
  return true;
}

uint8_t evaluate_local_state_n3_called=0;

#include <onx/items.h>

bool evaluate_local_state_n3(object* n3, void* d) {

  evaluate_local_state_n3_called++;
  // UID: uid-1  is: setup  state: good mostly  1: a c  2: ok m8
  // UID: uid-2  is: local-state  state: good
  // UID: uid-3  is: local-state  n2: uid-2  self: uid-3  n*: uid-1 uid-2 uid-3 uid-4 uid-5
  tests_assert(     !object_property(         n3, "is:foo"),                     "can't find is:foo");
  tests_assert_equal(object_property(         n3, "n2"), "uid-2",                "can see UID of local object immediately: n2");
  tests_assert(     !object_property(         n3, "n2:"),                        "cannot end in :");
  tests_assert_equal(object_property(         n3, "n2:1"), "uid-2",              "can see UID of local object immediately: n2:1");
  tests_assert(     !object_property(         n3, "n2:1:"),                      "cannot end in :");
  tests_assert_equal(object_property(         n3, "n2:UID"), "uid-2",            "can see UID of local object immediately: n2:UID");
  tests_assert_equal(object_property(         n3, "n2:1:UID"), "uid-2",          "can see UID of local object immediately: n2:1:UID");
  tests_assert_equal(object_property(         n3, "n2:is"),   "local-state",     "can see 'is' of local object immediately");
  tests_assert_equal(object_property(         n3, "n2:state"), "good",           "can see state prop of local object immediately");
  tests_assert(     !object_property(         n3, "n2:foo"),                     "can't find n2:foo");
  tests_assert_equal(object_property(         n3, "self:UID"), "uid-3",          "can see through link to self");
  tests_assert(      object_property_length(  n3, "self:n2")==1,                 "property 'n2' is a value (uid)");
  tests_assert(      object_property_length(  n3, "self:n2:")==1,                "property 'n2:' is a properties with length 1");
  tests_assert_equal(object_property(         n3, "self:n2"), "uid-2",           "property 'n2' is uid-2");
  tests_assert(      object_property_size(    n3, "self:n2")==-1,                "property 'n2' is not a properties");
  tests_assert(      object_property_size(    n3, "self:n2:")==3,                "property 'n2:' is a properties with length 3");
  tests_assert_equal(object_property_val(     n3, "self:n2:", 1), "local-state", "value of n2 first item is 'local-state'");
  tests_assert(     !object_property_val(     n3, "self:n2",  2),                "value of n2 second item is not found without :");
  tests_assert_equal(object_property_val(     n3, "self:n2:", 2), "good",        "value of n2 second item is 'good'");
  tests_assert_equal(object_property_val(     n3, "self:n2:", 3), "uid-1",       "value of n2 third item is uid-1");
  tests_assert(     !object_property_val(     n3, "self:n2:", 4),                "value of n2 third item is null");
  tests_assert(      object_property_size(    n3, "self:self:n2:")==3,           "there are three properties at n3:n2:");
  tests_assert(      object_property_length(  n3, "n*")==5,                      "property 'n*' is a list of 5 values/uids");
  tests_assert(      object_property_length(  n3, "n*:")==0,                     "cannot end in :");
//tests_assert_equal(object_property(         n3, "n*:UID"), "uid-1 uid-2..",    "n*:UID is uid-1 uid-2.. (one day)");
  tests_assert_equal(object_property(         n3, "n*:1:UID"), "uid-1",          "n*:1:UID is uid-1");
  tests_assert_equal(object_property(         n3, "n*:2:UID"), "uid-2",          "n*:2:UID is uid-2");
  tests_assert_equal(object_property(         n3, "n*:3:UID"), "uid-3",          "n*:3:UID is uid-3");
  tests_assert_equal(object_property(         n3, "n*:1"), "uid-1",              "n*:1 is uid-1");
  tests_assert(     !object_property(         n3, "n*:1:"),                      "cannot end in :");
  tests_assert_equal(object_property(         n3, "n*:2"), "uid-2",              "n*:2 is uid-2");
  tests_assert_equal(object_property(         n3, "n*:3"), "uid-3",              "n*:3 is uid-3");
  tests_assert_equal(object_property(         n3, "n*:4"), "uid-4",              "n*:4 is uid-4");
  tests_assert_equal(object_property_get_n(   n3, "n*", 1), "uid-1",             "n*,1 is uid-1");
  tests_assert_equal(object_property_get_n(   n3, "n*", 2), "uid-2",             "n*,2 is uid-2");
  tests_assert_equal(object_property_key(     n3, ":", 2), "n2",                 "key of 2nd item is 'n2'");
  tests_assert_equal(object_property_val(     n3, ":", 2), "uid-2",              "val of 1st item is 'uid-2'");
  tests_assert_equal(object_property_key(     n3, "n2:", 1), "is",               "key of 1st item in n2 is 'is'");
  tests_assert_equal(object_property_val(     n3, "n2:", 1), "local-state",      "val of 1st item in n2 is 'local-state'");
  tests_assert_equal(object_property_key(     n3, "n2:", 2), "state",            "key of 2nd item in n2 is 'state'");
  tests_assert_equal(object_property_val(     n3, "n2:", 2), "good",             "val of 2nd item in n2 is 'good'");
  tests_assert_equal(object_property_key(     n3, "self:self:n2:", 2), "state",  "key of 2nd item in n2 is 'state', via self");
  tests_assert_equal(object_property_val(     n3, "self:n2:", 2), "good",        "val of 2nd item in n2 is 'good', via self");
  tests_assert_equal(object_property(         n3, "self"), "uid-3",              "can see self as string when it's all uids");
  tests_assert_equal(object_property(         n3, "n2"), "uid-2",                "can see n2 as string when it's all uids");
  tests_assert_equal(object_property(         n3, "n*"), "uid-1",                "can see n* as 1st val, even though it's a list");
//tests_assert_equal(object_property(         n3, "n*:is"), "setup",             "can see n*:is as 1st link's val");
  tests_assert_equal(object_property(         n3, "n*:1:is"), "setup",           "can see n*:1:is anyway");
//tests_assert_equal(object_property(         n3, "n*:state"), "good",           "can see n*:state as 1st link's val");
  tests_assert_equal(object_property(         n3, "n*:1:state"), "good",         "can see n*:1:state anyway");
  tests_assert_equal(object_property(         n3, "n*:1:state:2"), "mostly",     "can see n* as 1st val, even though it's a list");
  tests_assert_equal(object_property(         n3, "n2:n1:state"), "good",        "can see through n2 to n1, getting 1st in list");
  tests_assert_equal(object_property(         n3, "n2:n1:state:1"), "good",      "can see through n2 to n1");
  tests_assert_equal(object_property(         n3, "n2:n1:state:2"), "mostly",    "can see through n2 to n1");

/*
{ UID: uid-3 is: local-state   n2: uid-2   self: uid-3   n*: uid-1 uid-2 uid-3 uid-4 uid-5 }
{ UID: uid-2 is: local-state   state: good          n1: uid-1 }
{ UID: uid-1 is: setup         state: good mostly   1: a c 2: ok m8 }
*/
  tests_assert_equal(    object_pathpair(         n3, "n2:n1", "state:1"),  "good",    "can see through n2 to n1 using pathpair");
  tests_assert(          object_pathpair_is(      n3, "n2:n1", "state:2",   "mostly"), "can see through n2 to n1 using pathpair and _is");
  tests_assert(          object_pathpair_contains(n3, "n2:n1", "state",     "mostly"), "can see through n2 to n1 using pathpair and _contains");
  tests_assert_equal_num(object_pathpair_length(  n3, "n2:n1", "state"),    2,         "pathpair length is 2");
  tests_assert_equal(    object_pathpair_get_n(   n3, "n2:n1", "state", 1), "good",    "can see through n2 to n1 using pathpair");
  tests_assert_equal(    object_pathpair_get_n(   n3, "n2:n1", "state", 2), "mostly",  "can see through n2 to n1 using pathpair");
  return true;
}

void test_local_state() {

  onn_set_evaluators("eval_local_state_n2", evaluate_local_state_n2, 0);
  onn_set_evaluators("eval_local_state_n3", evaluate_local_state_n3, 0);
  // UID: uid-2  is: local-state
  // UID: uid-3  is: local-state
  object* n2=object_new("uid-2", "eval_local_state_n2", "local-state", 5);
  object* n3=object_new("uid-3", "eval_local_state_n3", "local-state", 5);
  tests_assert(      onn_get_from_cache("uid-2")==n2,   "onn_get_from_cache can find uid-2");
  tests_assert_equal_num(object_property_int32(  n2, "Ver"), 1,     "version 1");
  tests_assert(      onn_get_from_cache("uid-3")==n3,   "onn_get_from_cache can find uid-3");
  // UID: uid-2  is: local-state  state: ok
  tests_assert(          object_property_set(    n2, "state", "OK"),    "can add 'state' = OK");
  tests_assert_equal_num(object_property_int32(  n2, "Ver"), 2,         "version 2");
  tests_assert(         !object_property_set(    n2, "state", "OK"),    "can't re-set 'state' to same val");
  tests_assert(          object_property_set(    n2, "state", "OK OK"), "can add 'state' = OK OK");
  tests_assert_equal_num(object_property_int32(  n2, "Ver"), 3,         "version 3");
  tests_assert(         !object_property_set(    n2, "state", "OK OK"), "can't re-set 'state' to same list");
  tests_assert(          object_property_set(    n2, "state", "ok"),    "can change 'state' to 'ok'");
  tests_assert_equal_num(object_property_int32(  n2, "Ver"), 4,         "version 4");
  tests_assert(          object_property_set(    n2, "n1",    "uid-1"), "can add 'n1'");
  tests_assert_equal_num(object_property_int32(  n2, "Ver"), 5,         "version 5");
  // UID: uid-3  is: local-state  n2: uid-2  self: uid-3
  tests_assert(      object_property_set(    n3, "n2",    "uid-2"), "can add 'n2'");
  tests_assert(      object_property_set(    n3, "self",  "uid-3"), "can add 'self'");
  // UID: uid-3  is: local-state  n2: uid-2  self: uid-3  n* uid-1 uid-2 uid-3 uid-4
  tests_assert(      object_property_set(    n3, "n*", "uid-1"),    "can set uid-1 in n*");
  tests_assert(      object_property_add(    n3, "n*", "uid-2"),    "can add uid-2 to n*");
  tests_assert(      object_property_add(    n3, "n*", "uid-3"),    "can add uid-3 to n*");
  tests_assert(     !object_property_add(    n3, "n*", ""),         "can't add empty to n*");
  tests_assert(     !object_property_add(    n3, "n*", 0),          "can't add empty to n*");
  tests_assert(      object_property_add(    n3, "n*", "uid-4"),    "can add uid-4 to n*");
  tests_assert(      object_property_add(    n3, "n*", "uid-5"),    "can add uid-5 to n*");
//tests_assert(     !object_property_add(    n3, "n*", "uid-6"),    "can't add uid-6 to n*");
  tests_assert(      object_property_length( n3, "n*")==5,          "there are 5 items in n*");
  tests_assert_equal(object_property(        n3, "n*:1"), "uid-1",  "n*:1 is uid-1");
  tests_assert_equal(object_property(        n3, "n*:2"), "uid-2",  "n*:2 is uid-2");
  tests_assert_equal(object_property_get_n(  n3, "n*", 3), "uid-3", "n*:3 is uid-3");
  tests_assert_equal(object_property_get_n(  n3, "n*", 4), "uid-4", "n*:4 is uid-4");
  tests_assert_equal(object_property_get_n(  n3, "n*", 5), "uid-5", "n*:5 is uid-5");
  // UID: uid-2  is: local-state  state: good
  tests_assert_equal(object_property(        n3, "n2:state"), "ok", "can see through n2 to uid-2's state");
  tests_assert(      object_property_set(    n2, "state", "good"),  "can set 'state' to trigger evaluate_local_state_n3");
}

// ---------------------------------------------------------------------------------

uint8_t pre_evaluate_n2_called=0;

bool pre_evaluate_n2(object* n2, void* args) {
  pre_evaluate_n2_called++;
  if(pre_evaluate_n2_called==1){
    tests_assert_equal((char*)args, "arguments", "pre-evaluator gets passed the arguments");
    tests_assert_equal(object_property(n2, "state"), "good",  "can see state");
  }
  return true;
}

uint8_t evaluate_local_notify_n2_called=0;

bool evaluate_local_notify_n2(object* n2, void* args) {
  evaluate_local_notify_n2_called++;
  if(evaluate_local_notify_n2_called==1){
    tests_assert_equal((char*)args, "arguments", "evaluator gets passed the arguments");
    object_property_set(n2, "state", "better:");
    object_property_set(n2, "state", "better:");
    object_property_set(n2, "state", "better:");
  }
  return true;
}

uint8_t post_evaluate_n2_called=0;

bool post_evaluate_n2(object* n2, void* args) {
  post_evaluate_n2_called++;
  if(post_evaluate_n2_called==1){
    tests_assert_equal((char*)args, "arguments", "post-evaluator gets passed the arguments");
    tests_assert_equal(object_property(n2, "state"), "better:",  "can see evaluator's state update");
  }
  return true;
}

uint8_t evaluate_local_notify_n3_called=0;

bool evaluate_local_notify_n3(object* n3, void* d) {

  evaluate_local_notify_n3_called++;

  int i=1;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(n3, "Alerted"),                  "uid-2",   "1: n3/uid-3 can see that it was uid-2 update that triggered eval");
    tests_assert_equal(object_property(n3, "Alerted:UID"),              "uid-2",   "1: n3/uid-3 can see that it was uid-2 update that triggered eval");
    tests_assert_equal(object_property(n3, "Alerted:state"),            "better:", "1: n3/uid-3 can see state update 1");
    tests_assert_equal(object_property(n3, "n2:state"),                 "better:", "1: n3/uid-3 can see state update 2");
  }
  i++;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(n3, "Alerted"),                  "uid-3",   "2: n3/uid-3 can see that it was itself that triggered eval");
  }
  i++;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(n3, "Alerted"),                  "uid-3",   "3: n3/uid-3 can see that it was itself that triggered eval");
    tests_assert_equal(object_property(n3, "Alerted:UID"),              "uid-3",   "3: n3/uid-3 can see that it was itself that triggered eval");
    tests_assert_equal(object_property(n3, "Alerted:Alerted:n2:state"), "better:", "3: n3/uid-3 can see through itself");
    tests_assert_equal(object_property(n3, "Alerted:is:1"),             "local",   "3: n3/uid-3 can see state update through itself 1");
    tests_assert_equal(object_property(n3, "Alerted:is:2"),             "state",   "3: n3/uid-3 can see state update through itself 2");
  }
  i++;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(   n3, "Alerted"),               "uid-2",   "4: n3/uid-3 can see that it was uid-2 update that triggered eval");
    tests_assert_equal(object_property(   n3, "Alerted:UID"),           "uid-2",   "4: n3/uid-3 can see that it was uid-2 update that triggered eval");
    tests_assert_equal(object_property(   n3, "Alerted:n5"),            "uid-5",   "4: n3/uid-3 can see state update");
  }
  i++;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(         n3, "Alerted"),         "uid-1",   "5: n3/uid-3 can see that it was uid-1 update that triggered eval");
    tests_assert_equal(object_property(         n3, "Alerted:UID"),     "uid-1",   "5: n3/uid-3 can see that it was uid-1 update that triggered eval");
    tests_assert_equal(object_property(         n3, "Alerted:state:1"), "good:",   "5: n3/uid-3 can see state update 3");
    tests_assert_equal(object_property(         n3, "Alerted:state:2"), "good",    "5: n3/uid-3 can see state update 4");
    tests_assert_equal(object_property(         n3, "n2:n1:state:1"),   "good:",   "5: n3/uid-3 can see state update 5");
    tests_assert_equal(object_property(         n3, "n2:n1:state:2"),   "good",    "5: n3/uid-3 can see state update 5");
  }
  i++;
  if(evaluate_local_notify_n3_called==i){
    tests_assert_equal(object_property(         n3, "Alerted"),         "uid-1",   "6: n3/uid-3 can see that it was uid-1 update that triggered eval");
    tests_assert_equal(object_property(         n3, "Alerted:UID"),     "uid-1",   "6: n3/uid-3 can see that it was uid-1 update that triggered eval");
    tests_assert_equal(object_property(         n3, "Alerted:state:1"), ":better", "6: n3/uid-3 can see the state update");
    tests_assert_equal(object_property(         n3, "Alerted:state:2"), "better:", "6: n3/uid-3 can see the state update");
    tests_assert_equal(object_property(         n3, "n2:n1:state:1"),   ":better", "6: n3/uid-3 can see the state update");
    tests_assert_equal(object_property(         n3, "n2:n1:state:2"),   "better:", "6: n3/uid-3 can see the state update");
  }
  return true;
}

void test_remote_object() {

  object* n2=onn_get_from_cache("uid-2");
  object_property_set(n2, "n5", "uid-5");

  object* n3=onn_get_from_cache("uid-3");

  tests_assert_equal(object_property(n2, "n5:UID"),   "uid-5", "n2: n5:UID is uid-5 from shell");
  tests_assert_equal(object_property(n3, "n*:5:UID"), "uid-5", "n3: n*:5:UID is uid-5 from shell");

  tests_assert(     !object_property(n2, "n5:x"),   "n2: n5:x is null");
  tests_assert(     !object_property(n3, "n*:5:x"), "n3: n*:5:x is null");

  object* n5=onn_get_from_cache("uid-5");
  char* n5text="UID: uid-5 Ver: 0 Devices: shell Notify: uid-2 uid-3";

  tests_assert_equal(object_to_text(n5,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), n5text, "converts remote shell uid-5 to correct text");
}

void test_local_notify() {

  object* n2=onn_get_from_cache("uid-2");
  object* n3=onn_get_from_cache("uid-3");

  tests_assert_equal(object_property(n3, "n2"      ), "uid-2",   "uid-3 links to uid-2");
  tests_assert_equal(object_property(n3, "n2:state"), "good",    "can still see through link");

  onn_set_evaluators("eval_local_notify_n2", pre_evaluate_n2, evaluate_local_notify_n2, post_evaluate_n2, 0);
  onn_set_evaluators("eval_local_notify_n3", evaluate_local_notify_n3, 0);
  object_set_evaluator(n2, "eval_local_notify_n2");
  object_set_evaluator(n3, "eval_local_notify_n3");

  onn_run_evaluators("uid-2", "arguments");

  onn_loop();

  tests_assert_equal(object_property(n2, "state"), "better:",  "can see state update 6");

  object_property_add(n3, "Notifying", "uid-2");
  object_property_set(n3, "state", "changed");
}

// ---------------------------------------------------------------------------------

void test_to_text() {

  object* n1=onn_get_from_cache("uid-1");
  object* n2=onn_get_from_cache("uid-2");
  object* n3=onn_get_from_cache("uid-3");
  object* n4=onn_get_from_cache("uid-4");

  char* n1text="UID: uid-1 Ver: 37 Eval: eval_remote_notify_n1 Notify: uid-3 is: setup state: good mostly 1: a c 2: ok m8";
  char* n2text="UID: uid-2 Ver: 7 Eval: eval_remote_notify_n2 Notify: uid-3 is: local-state state: better\\: n1: uid-1";
  char* n3text="UID: uid-3 Ver: 10 Eval: eval_local_notify_n3 Notify: uid-3 uid-2 uid-4 is: local state n2: uid-2 self: uid-3 n*: uid-1 uid-2 uid-3 uid-4 uid-5 state: changed";
  char* n4text="UID: uid-4 Ver: 322 Eval: eval_remote_notify_n4 Notify: uid-1 uid-2 is: remote state ab: m\\: :c:d\\: n n3: uid-3 x:y: a :z:q\\: b last: one state: good";

  tests_assert_equal(object_to_text(n1,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), n1text, "converts uid-1 to correct text");
  tests_assert_equal(object_to_text(n2,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), n2text, "converts uid-2 to correct text");
  tests_assert_equal(object_to_text(n3,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), n3text, "converts uid-3 to correct text");
  tests_assert_equal(object_to_text(n4,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), n4text, "converts uid-4 to correct text");
}

// ---------------------------------------------------------------------------------

uint8_t evaluate_remote_notify_n1_called=0;

bool evaluate_remote_notify_n1(object* n1, void* d) {
  evaluate_remote_notify_n1_called++;
  return true;
}

uint8_t evaluate_remote_notify_n2_called=0;

bool evaluate_remote_notify_n2(object* n2, void* d) {
  evaluate_remote_notify_n2_called++;
  return true;
}

uint8_t evaluate_remote_notify_n4_called=0;

bool evaluate_remote_notify_n4(object* n4, void* d) {
  evaluate_remote_notify_n4_called++;
  return true;
}

void test_from_text() {

  tests_assert(     !find_unescaped_colon("abc"),                 "no colon if no colon");
  tests_assert_equal(find_unescaped_colon(":abc"),        ":abc", "found colon at start");
  tests_assert_equal(find_unescaped_colon("a:bc"),         ":bc", "found colon");
  tests_assert_equal(find_unescaped_colon("ab:c"),          ":c", "found colon");
  tests_assert_equal(find_unescaped_colon("abc:"),           ":", "found colon at end");
  tests_assert_equal(find_unescaped_colon("a:b:c"),       ":b:c", "found first colon");
  tests_assert_equal(find_unescaped_colon("a\\:b:c"),       ":c", "found first unescaped colon");
  tests_assert_equal(find_unescaped_colon("a\\::bc"),      ":bc", "found first unescaped colon");
  tests_assert_equal(find_unescaped_colon("a:b\\:c"),   ":b\\:c", "found first colon not the unescaped one");
  tests_assert(     !find_unescaped_colon("a\\:b\\:c"),           "no colon if all escaped");

  char banana_rem[]="banana";
  tests_assert_equal(remove_char_in_place(banana_rem, 'a'), "bnn", "can remove all of a given char");

  char banana_pre[32]; memcpy(banana_pre, "banana", strlen("banana")+1);
  tests_assert_equal(prefix_char_in_place(banana_pre, 'a', 'n'), "baanaana", "can prefix all of a given char");
  tests_assert_equal_num(num_tokens("   aaaa b\t c\n"), 3, "found 3 tokens");
  tests_assert_equal_num(num_tokens("a bbbb c"),        3, "found 3 tokens");
  tests_assert_equal_num(num_tokens("a   ccc"),         2, "found 2 tokens");
  tests_assert_equal_num(num_tokens("a b "),            2, "found 2 tokens");
  tests_assert_equal_num(num_tokens(" a b"),            2, "found 2 tokens");
  tests_assert_equal_num(num_tokens("a  "),             1, "found 1 tokens");
  tests_assert_equal_num(num_tokens("  "),              0, "found 0 tokens");
  tests_assert_equal_num(num_tokens(" "),               0, "found 0 tokens");
  tests_assert_equal_num(num_tokens(""),                0, "found 0 tokens");
  tests_assert_equal_num(num_tokens(0),                 0, "found 0 tokens");

  tests_assert(string_is_blank(0),     "null string is blank");
  tests_assert(string_is_blank(""),    "empty string is blank");
  tests_assert(string_is_blank(" "),   "single space string is blank");
  tests_assert(string_is_blank("  "),  "2 space string is blank");
  tests_assert(string_is_blank("\n "), "newline/space string is blank");

  tests_assert(!string_is_blank("."),     "string is not blank");
  tests_assert(!string_is_blank(" . "),   "string is not blank");
  tests_assert(!string_is_blank(" . \n"), "string is not blank");
  tests_assert(!string_is_blank("\n . "), "string is not blank");

  object* n1=onn_get_from_cache("uid-1");
  object* n2=onn_get_from_cache("uid-2");
  object* n3=onn_get_from_cache("uid-3");
  onn_set_evaluators("eval_remote_notify_n1", evaluate_remote_notify_n1, 0);
  onn_set_evaluators("eval_remote_notify_n2", evaluate_remote_notify_n2, 0);
  onn_set_evaluators("eval_remote_notify_n4", evaluate_remote_notify_n4, 0);
  object_set_evaluator(n1, "eval_remote_notify_n1");
  object_set_evaluator(n2, "eval_remote_notify_n2");

  char* text=" UID:   uid-4 Ver: 321  Eval:  eval_remote_notify_n4 Notify: uid-1   uid-2   is: remote  state ab: m\\: :c:d\\:  n n3: uid-3 x:y:  a :z:q\\:  b  last: one\n";
  object* n4=object_new_from(text, 6);
  tests_assert(!!n4, "input text was parsed into an object 1");
  if(!n4) return;

  tests_assert_equal(    object_property(         n4, "UID"), "uid-4",     "object_new_from parses uid");
  tests_assert_equal_num(object_property_int32(   n4, "Ver"), 321,         "object_new_from parses version");
  tests_assert(          object_property_contains(n4, "is", "remote"),     "object_new_from parses is");
  tests_assert(          object_property_contains(n4, "is", "state"),      "object_new_from parses is");
  tests_assert_equal(    object_property(         n4, "is:1"), "remote",   "object_new_from parses 'is' first list item" );
  tests_assert_equal(    object_property(         n4, "is:2"), "state",    "object_new_from parses 'is' second list item" );
  tests_assert_equal(    object_property(         n4, "n3"), "uid-3",      "object_new_from parses n3");
  tests_assert_equal(    object_property(         n4, "ab:1"), "m:",       "object_new_from parses all the escaped colons");
  tests_assert_equal(    object_property(         n4, "ab:2"), ":c:d:",    "object_new_from parses all the escaped colons");
  tests_assert_equal(    object_property(         n4, "ab:3"), "n",        "object_new_from parses all the escaped colons");
  tests_assert_equal(    object_property(         n4, "x\\:y:1"), "a",     "object_new_from parses all the escaped colons");
  tests_assert_equal(    object_property(         n4, "x\\:y:2"), ":z:q:", "object_new_from parses all the escaped colons");
  tests_assert_equal(    object_property(         n4, "x\\:y:3"), "b",     "object_new_from parses all the escaped colons");

  tests_assert_equal(object_property_key(     n4, ":", 4),             "x:y",   "key of 4th item is 'x:y'");
  char keyesc[64];
  tests_assert_equal(object_property_key_esc( n4, ":", 4, keyesc, 64), "x\\:y", "key of 4th item escaped is 'x\\:y'");

  tests_assert_equal(object_property(         n4, "last"), "one",      "object_new_from parses last one as single value without newline");

  char* totext="UID: uid-4 Ver: 321 Eval: eval_remote_notify_n4 Notify: uid-1 uid-2 is: remote state ab: m\\: :c:d\\: n n3: uid-3 x:y: a :z:q\\: b last: one";
  tests_assert_equal(object_to_text(        n4,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), totext, "gives same text back from reconstruction 1");

                    object_property_set(   n4, "state", "good");
  tests_assert_equal(object_property(       n4, "n3:is"), "local-state",   "object_new_from traverses n3:is" );
  tests_assert_equal(object_property(       n4, "state"), "good",          "object_new_from creates usable object");
                    object_property_set(   n3, "is", "local state");

  text="UID: uid-nx Ver: 543 Cache: keep-active Devices: uid-x Notify: uid-1 uid-2 is: remote state n3: uid-3";
  object* nx=object_new_from(text, 4);  // note object_new_from isn't meant for parsing remote object text!

  tests_assert(!!nx, "input text was parsed into an object 2");
  if(!nx) return;

  char* nxuid=object_property(nx, "UID");

  tests_assert(      is_local("uid-4"),                   "n4 is local");
  tests_assert(     !is_local(nxuid),                     "nx is remote");
  tests_assert_equal(object_get_cache(nx), "keep-active", "nx is Cache: keep-active");

  tests_assert_equal(object_to_text(nx,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), text, "gives same text back from reconstruction 2");

  text="is: remote state n3: uid-3";
  object* nb=object_new_from(text, 4);
  char* nbuid=object_property(nb, "UID");
  char fulltext[256]; snprintf(fulltext, 256, "UID: %s Ver: 1 %s", nbuid, text);
  tests_assert_equal(object_to_text(nb,textbuff,TEXTBUFFLEN,OBJECT_TO_TEXT_PERSIST), fulltext, "gives same text back from reconstruction 3");

  text="is: messed up  : --";
  object* nm=object_new_from(text, 4);
  tests_assert(!nm, "does not parse messed up text");

  text="UID: uid-1--2-3 Ver: 2 is: messed up name: mangoUID: uid-1-2-3 Ver: 2 is: messed up name: mango";
  nm=object_new_from(text, 4);
  tests_assert(!nm, "does not parse messed up text");
}

uint8_t evaluate_persistence_n1_called=0;

bool evaluate_persistence_n1(object* n1, void* d) {

  if(evaluate_persistence_n1_called) return true;
  tests_assert_equal(object_property(    n1, "state:1"), "good:",            "can still see n1's state because it's refetched from persistence");
  tests_assert_equal(object_property(    n1, "state:2"), "good",             "can still see n1's state because it's refetched from persistence");
  tests_assert(      object_property_set(n1, "state",    ":better better:"), "can change n1 to :better better: (awaiting n3/n4 to be notified)");
  evaluate_persistence_n1_called++;
  return true;
}

uint8_t evaluate_persistence_n4_before_called=0;

bool evaluate_persistence_n4_before(object* n4, void* d) {
  tests_assert_equal(object_property(n4, "n3:n2:n1:state:1"), "good:", "n4 can look through objects in the cache on notify before persist");
  tests_assert_equal(object_property(n4, "n3:n2:n1:state:2"), "good",  "n4 can look through objects in the cache on notify before persist");
  evaluate_persistence_n4_before_called++;
  return true;
}

uint8_t evaluate_persistence_n4_after_called=0;

bool evaluate_persistence_n4_after(object* n4, void* d) {
  tests_assert_equal(object_property(n4, "n3:n2:n1:state:1"), ":better", "n4 can look through objects in the cache on notify after persist");
  tests_assert_equal(object_property(n4, "n3:n2:n1:state:2"), "better:", "n4 can look through objects in the cache on notify after persist");
  evaluate_persistence_n4_after_called++;
  return true;
}

void test_persistence() {

  log_write("\n------------ Persistence ----------------\n\n");

  onn_show_cache();

  object* n4=onn_get_from_cache("uid-4");
  object* n1=onn_get_from_cache("uid-1");

  object_set_cache(n1, "keep-active");

  onn_set_evaluators("eval_persistence_n1", evaluate_persistence_n1, 0);
  onn_set_evaluators("eval_persistence_n4", evaluate_persistence_n4_before, 0);

  object_set_evaluator(n1, "eval_persistence_n1");
  object_set_evaluator(n4, "eval_persistence_n4");

  tests_assert_equal(object_property(n4, "n3:n2:n1:state:1"), "good",   "n4 can look through objects in the cache");
  tests_assert_equal(object_property(n4, "n3:n2:n1:state:2"), "mostly", "n4 can look through objects in the cache");
  tests_assert(object_property_set(  n1, "state", "good: good"),        "can change n1 to good: good (awaiting n3/n4 to be notified)");

  onn_loop();

  onn_set_evaluators("eval_persistence_n4", evaluate_persistence_n4_after, 0);

  // not currently testing anything here tbh
  object* nn=object_new("uid-n", "eval_default", "volatile", 4);
  object_set_persist(nn, "none");

  onn_show_cache();

  onn_un_cache("uid-n");
  onn_un_cache("uid-5");
  onn_un_cache("uid-4");
  onn_un_cache("uid-3");
  onn_un_cache("uid-2");
  onn_un_cache("uid-1");

  onn_show_cache();

  persist_reload();
}

uint8_t evaluate_timer_n4_called=0;

bool evaluate_timer_n4(object* n4, void* d) {
  evaluate_timer_n4_called++;
  if(evaluate_timer_n4_called==1){
    tests_assert_equal(object_property(n4, "Alerted"), "uid-2", "uid-4 was alerted by uid-2 first");
    tests_assert_equal(object_property(n4, "Timer"),   "150",   "Timer is 150 on notification from uid-2");
  }
  if(evaluate_timer_n4_called==2){
    tests_assert_equal(object_property(n4, "Timer"),     "0",   "Timer is zero on timeout");
  }
  return true;
}

void test_timer() {

  object* n2=onn_get_from_cache("uid-2");
  object* n4=onn_get_from_cache("uid-4");

  onn_set_evaluators("eval_timer_n4", evaluate_timer_n4, 0);
  object_set_evaluator(n4, "eval_timer_n4");

  object_property_set(n2, "n4", "uid-4");

  object_property_set(n4, "Timer", "150");

  tests_assert_equal(object_property(    n4, "Timer"),    "150", "Timer is 150");
  tests_assert_equal(object_property(    n2, "n4:Timer"), "150", "n4:Timer is 150");
}

// ---------------------------------------------------------------------------------

void run_onn_tests() {

  log_write("\n----------ONN tests-----------\n");

  test_object_set_up();

  onn_loop();

  test_device();

  onn_loop();

  test_local_state();

  onn_loop();

  tests_assert(         !evaluate_setup_called,              "evaluate_setup was not called");
  tests_assert(         !evaluate_local_state_n2_called,     "evaluate_local_state_n2 was not called");
  tests_assert_equal_num(evaluate_local_state_n3_called, 1,  "evaluate_local_state_n3 was called");

  test_local_notify();

  onn_loop();
  onn_loop();
  onn_loop();

  tests_assert_equal_num(pre_evaluate_n2_called, 2,           "pre_evaluate_n2 was called twice");
  tests_assert_equal_num(evaluate_local_notify_n2_called, 2,  "evaluate_local_notify_n2 was called twice");
  tests_assert_equal_num(post_evaluate_n2_called, 2,          "post_evaluate_n2 was called twice");
  tests_assert_equal_num(evaluate_local_notify_n3_called, 2,  "evaluate_local_notify_n3 was called twice");

  test_from_text();
  test_to_text();

  test_remote_object();

  onn_loop();
  onn_loop();
  onn_loop();
  onn_loop();
  onn_loop();

  tests_assert_equal_num(evaluate_remote_notify_n1_called, 1, "evaluate_remote_notify_n1 was called");
  tests_assert_equal_num(evaluate_remote_notify_n2_called, 2, "evaluate_remote_notify_n2 was called twice");
  tests_assert_equal_num(evaluate_remote_notify_n4_called, 1, "evaluate_remote_notify_n4 was called");
  tests_assert_equal_num(evaluate_local_notify_n3_called,  4, "evaluate_local_notify_n3 was called four times");

  test_persistence();

  onn_loop();
  onn_loop();

  tests_assert_equal_num(evaluate_persistence_n1_called, 1,        "evaluate_persistence_n1 was called");
  tests_assert_equal_num(evaluate_persistence_n4_before_called, 1, "evaluate_persistence_n4_before was called");
  tests_assert_equal_num(evaluate_persistence_n4_after_called, 1,  "evaluate_persistence_n4_after was called");
  tests_assert_equal_num(evaluate_local_notify_n3_called, 6,       "evaluate_local_notify_n3 was called six times");

  onn_show_cache();
  persistence_dump();

  uint32_t s=(uint32_t)time_ms();
  uint32_t e;

  test_timer();

  e=(uint32_t)time_ms()-s;
  log_write(">>>> time passed after test_timer %ud\n", e);

  onn_loop();
  onn_loop();

  tests_assert_equal_num(evaluate_timer_n4_called, 1, "evaluate_timer_n4 was not called immediately");

  e=(uint32_t)time_ms()-s;
  log_write(">>>> time passed after evaluators %ud\n", e);

  e=(uint32_t)time_ms()-s;
  time_delay_ms(100-e);

  e=(uint32_t)time_ms()-s;
  log_write(">>>> time passed after waiting %ud\n", e);

  tests_assert_equal_num(evaluate_timer_n4_called, 1, "evaluate_timer_n4 was not called after 100");

  e=(uint32_t)time_ms()-s;
  time_delay_ms(180-e);

  e=(uint32_t)time_ms()-s;
  log_write(">>>> time passed after waiting %ud\n", e);

  onn_loop();

  tests_assert_equal_num(evaluate_timer_n4_called, 2, "evaluate_timer_n4 was called again, after 180");
}

// ---------------------------------------------------------------------------------

