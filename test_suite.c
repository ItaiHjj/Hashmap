#include <stdlib.h>
#include "test_suite.h"
#include "test_pairs.h"
#include "hash_funcs.h"

#define PAIRS_LST_SIZE 34
#define START_CAPACITY 16
#define GROW_FACTOR 2

/**
 * Make allocated array of pairs.
 * @return  pairs array of void**.
 */
void **make_pairs ()
{
  void **pair_lst = malloc (PAIRS_LST_SIZE * sizeof (pair *));
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    {
      char name = (char) (i + 32);
      int number = i;
      keyT key = &name;
      valueT value = &number;
      pair_lst[i] = pair_alloc (key, value,
                                char_key_cpy, int_value_cpy,
                                char_key_cmp, int_value_cmp,
                                char_key_free, int_value_free);
    }
  return pair_lst;
}

/**
 * Free the array of lst
 * @param pair_lst
 */
void free_pair_lst (void **pair_lst)
{
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    pair_free (&pair_lst[i]);
  free (pair_lst);
  pair_lst = NULL;
}

/**
 * This function checks the hashmap_insert function of the hashmap library.
 * If hashmap_insert fails at some points, the functions
 * exits with exit code 1.
 */
void test_hash_map_insert (void)
{
  hashmap *t = hashmap_alloc (hash_char);
  void **pair_lst = make_pairs ();


  // Try push PAIRS_LST_SIZE news pair in the hashtable.
  // Check whether the table is resizing.
  // additionally check that the hashmap capacity and size are correct.
  for (size_t j = 0; j < PAIRS_LST_SIZE; j++)
    {
      pair *in_pair = pair_lst[j];
      hashmap_insert (t, in_pair);

      if (j == 0)
        {
          assert (t->capacity == 16 && "OK this why");
          assert(t->size == 1);
        }
      if (j == 7)
        {
          assert (t->capacity == 16);
          assert(t->size == 8);
        }
      if (j == 12)
        {
          assert (t->capacity == 32);
          assert(t->size == 13);
        }
      if (j == 23)
        {
          assert (t->capacity == 32);
          assert(t->size == 24);
        }
      if (j == 24)
        {
          assert (t->capacity == 64);
          assert(t->size == 25);
        }
      if (j == 30)
        {
          assert (t->capacity == 64);
          assert(t->size == 31);
        }
    }


  // Try push a pair that his key already exists in the hashmap.
  // Its should dont work.
  int pair_pushed = hashmap_insert (t, (pair *) pair_lst[0]);
  assert (pair_pushed != 1);


  // Try push a pair that his value already exits in the hashmap
  // And his key is not in hashmap.
  // its should work
  char name = (char) (67);
  int number = 0;
  keyT key = &name;
  valueT value = &number;
  pair *same_val_pair = pair_alloc (key, value,
                                    char_key_cpy, int_value_cpy,
                                    char_key_cmp, int_value_cmp,
                                    char_key_free, int_value_free);
  int same_val_pair_pushed = hashmap_insert (t, same_val_pair);
  assert (same_val_pair_pushed == 1);

  void *pair_to_free = (void *) same_val_pair;
  pair_free (&pair_to_free);
  free_pair_lst (pair_lst);
  hashmap_free (&t);
}

/**
 * This function checks the hashmap_at function of the hashmap library.
 * If hashmap_at fails at some points, the functions exits with exit code 1.
 */
void test_hash_map_at (void)
{
  hashmap *t = hashmap_alloc (hash_char);
  assert (t != NULL);  // Check map been allocated

  void **pair_lst = make_pairs ();

  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    hashmap_insert (t, pair_lst[i]);


  //check all the keys that been added to the hashmap.
  //All keys should be on the hashtable.
  for (int j = 0; j < PAIRS_LST_SIZE; j++)
    {
      pair *curr = pair_lst[j];
      valueT val = hashmap_at (t, curr->key);
      assert(*(int *) val == j);
    }


  // Check a key that is not on the hashmap
  char name = (char) 80;
  keyT key_not_in_hashmap = &name;
  valueT val = hashmap_at (t, key_not_in_hashmap);
  assert(val == NULL);

  free_pair_lst (pair_lst);
  hashmap_free (&t);
}

/**
 * This function checks the hashmap_erase function of the hashmap library.
 * If hashmap_erase fails at some points, the functions exits with exit code 1.
 */
void test_hash_map_erase (void)
{
  hashmap *t = hashmap_alloc (hash_char);
  void **pair_lst = make_pairs ();
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    {
      hashmap_insert (t, pair_lst[i]);
    }

  // Deletes all hashmap
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    {
      pair *curr = pair_lst[i];
      hashmap_erase (t, curr->key);
      assert(t->size == (size_t) (PAIRS_LST_SIZE - i - 1));
    }
  free_pair_lst (pair_lst);
  hashmap_free (&t);
}

/**
 * This function checks the hashmap_get_load_factor
 * function of the hashmap library. If hashmap_get_load_factor
 * fails at some points, the functions exits with exit code 1.
 */
void test_hash_map_get_load_factor (void)
{
  hashmap *t = hashmap_alloc (hash_char);
  void **pair_lst = make_pairs ();

  // Check this fnc while inserting and the while the hashtable is
  // growing.
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    {
      hashmap_insert (t, pair_lst[i]);
      if (i < 12)
        {
          assert ((double) (i + 1) / (double) START_CAPACITY ==
                  hashmap_get_load_factor (t));
        }

      if (12 <= i && i <= 23)
        {
          assert ((double) (i + 1) / (double) (START_CAPACITY * GROW_FACTOR) ==
                  hashmap_get_load_factor (t));
        }

      if (23 < i)
        {
          assert ((double) (i + 1) / (double)
              (START_CAPACITY * GROW_FACTOR * GROW_FACTOR)
                  == hashmap_get_load_factor (t));
        }
    }

//    Test the load factor while the hashtable is getting reduced to size 0.
  for (int i = PAIRS_LST_SIZE - 1; 0 <= i; i--)
    {
      pair *curr = pair_lst[i];
      hashmap_erase (t, curr->key);
      if (15 < i)
        {
          assert ((double) (double) (i)
                  / (double) (START_CAPACITY * GROW_FACTOR
                              * GROW_FACTOR)
                  == hashmap_get_load_factor (t));
        }

      if (8 <= i && i <= 15)
        {
          assert ((double) (double) (i)
                  / (double) (START_CAPACITY * GROW_FACTOR)
                  == hashmap_get_load_factor (t));
        }
      if (4 <= i && i <= 7)
        {
          assert ((double) (double) (i)
                  / (double) (START_CAPACITY)
                  == hashmap_get_load_factor (t));
        }

      if (2 <= i && i <= 3)
        {
          assert ((double) (double) (i)
                  / (double) ((double) START_CAPACITY / GROW_FACTOR)
                  == hashmap_get_load_factor (t));
        }
      if (0 <= i && i < 2)
        {
          assert ((double) (double) (i)
                  / (double) ((double) START_CAPACITY /
                              GROW_FACTOR / GROW_FACTOR)
                  == hashmap_get_load_factor (t));
        }
    }
  free_pair_lst (pair_lst);
  hashmap_free (&t);
}

/**
 * This function checks the HashMapGetApplyIf function of the hashmap library.
 * If HashMapGetApplyIf fails at some points,
 * the functions exits with exit code != 0.
 * Attention: IN THE PAIR LST WE MUST HAVE ASCII VAL: 48 <= VAL <= 57.
 * Hence if we minimize PAIRS_LST_SIZE we must not minimize too much.of
 * The min size of PAIRS_LST_SIZE is 25.
 */
void test_hash_map_apply_if ()
{
  hashmap *t = hashmap_alloc (hash_char);
  void **pair_lst = make_pairs ();
  for (int i = 0; i < PAIRS_LST_SIZE; i++)
    hashmap_insert (t, pair_lst[i]);

  int counts = hashmap_apply_if (t, is_digit, double_value);
  assert (counts == 10);

  hashmap_free (&t);
  free_pair_lst (pair_lst);
}

