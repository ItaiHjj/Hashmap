#include "hashmap.h"
#define MINIMIZE 0
#define MAGNIFY 1

/**
 * Allocates dynamically new hash map element.
 * @param func a function which "hashes" keys.
 * @return pointer to dynamically allocated hashmap.
 * @if_fail return NULL.
 */
hashmap *hashmap_alloc (hash_func func)
{
  hashmap *hash_map = (hashmap *) malloc (sizeof (hashmap));
  if (hash_map == NULL)
    return NULL;
  hash_map->buckets = (vector **) calloc
      (HASH_MAP_INITIAL_CAP, sizeof (vector *));
  if (hash_map->buckets == NULL)
    {
      free (hash_map);
      hash_map = NULL;
      return NULL;
    }
  hash_map->size = 0;
  hash_map->capacity = HASH_MAP_INITIAL_CAP;
  hash_map->hash_func = func;
  return hash_map;
}

/**
 * Frees a hash map and the elements the hash map itself allocated.
 * @param p_hash_map pointer to dynamically allocated pointer to hash_map.
 */
void hashmap_free (hashmap **p_hash_map)
{
  if ((p_hash_map == NULL) || (*p_hash_map == NULL))
    return;
  for (size_t i = 0; i < (*p_hash_map)->capacity; i++)
    if ((*p_hash_map)->buckets[i] != NULL)
      vector_free (&(*p_hash_map)->buckets[i]);
  free ((*p_hash_map)->buckets);
  free (*p_hash_map);
  *p_hash_map = NULL;
}

/**
 * Free the vector in the bucket list and free the bucket itself.
 * @param buckets The buckets to free.
 * @return 0
 */
void delete_buckets (vector **buckets, size_t buckets_capacity)
{
  for (size_t i = 0; i < buckets_capacity; i++)
    if (buckets[i] != NULL)
      vector_free (&buckets[i]);
  free (buckets);
  buckets = NULL;
}

/**
 * In case the hash map need to be reorganized. Make new buckets hash-table.
 * * @param hash_map the hash map to rehash.
 * @param action 1 to magnify, 0 to minimize.
 * @return new buckets for re-hashing table worked, NULL otherwise
 */
vector **resize_buckets (const hashmap *hash_map, int magnify)
{
  size_t new_buckets_capacity;
  if (magnify)
    new_buckets_capacity = (hash_map->capacity * HASH_MAP_GROWTH_FACTOR);
  else  // Minimize Case
    new_buckets_capacity = (hash_map->capacity / HASH_MAP_GROWTH_FACTOR);

  vector **new_buckets = calloc (new_buckets_capacity, sizeof (vector *));
  if (new_buckets == NULL)
    return NULL;

  vector **old_buckets = hash_map->buckets;
  size_t new_ind;
  pair *curr_pair;

  for (size_t i = 0; i < hash_map->capacity; i++)
    if (hash_map->buckets[i] != NULL)
      for (size_t j = 0; j < old_buckets[i]->size; j++)
        {
          curr_pair = old_buckets[i]->data[j];
          new_ind = hash_map->hash_func (curr_pair->key)
                    & (new_buckets_capacity - 1);
          if (new_buckets[new_ind] == NULL) // Need allocate new vector
            {
              new_buckets[new_ind] = vector_alloc
                  (pair_copy, pair_cmp, pair_free);
              if (new_buckets[new_ind] == NULL) // Allocation didn't work
                {
                  delete_buckets (new_buckets, new_buckets_capacity);
                  return NULL;
                }
            }
          if (!vector_push_back (new_buckets[new_ind], curr_pair))
            {
              delete_buckets (new_buckets, new_buckets_capacity);
              return NULL;
            }
        }
  return new_buckets;
}

/**
 * Inserts a new in_pair to the hash map.
 * The function inserts *new*, *copied*, *dynamically allocated* in_pair,
 * NOT the in_pair it receives as a parameter.
 * @param hash_map the hash map to be inserted with new element.
 * @param in_pair a in_pair the hash map would contain.
 * @return returns 1 for successful insertion, 0 otherwise.
 */
int hashmap_insert (hashmap *hash_map, const pair *in_pair)
{
  if ((hash_map == NULL) || (in_pair == NULL))
    return 0;
  if (hashmap_at (hash_map, in_pair->key) != NULL)
    return 0;
  size_t ind;
  hash_map->size++;
  if (HASH_MAP_MAX_LOAD_FACTOR < hashmap_get_load_factor (hash_map))
    {
      ind = hash_map->hash_func (in_pair->key)
            & ((hash_map->capacity * HASH_MAP_GROWTH_FACTOR) - 1);

      vector **new_buckets = resize_buckets (hash_map, MAGNIFY);
      if (new_buckets == NULL)
        {
          hash_map->size--;
          return 0;
        }
      if (new_buckets[ind] == NULL)
        {
          new_buckets[ind] = vector_alloc (pair_copy, pair_cmp, pair_free);
          if (new_buckets[ind] == NULL)
            {
              hash_map->size--;
              return 0;
            }
        }
      if (vector_push_back (new_buckets[ind], in_pair))
        {
          delete_buckets (hash_map->buckets, hash_map->capacity);
          hash_map->capacity *= HASH_MAP_GROWTH_FACTOR;
          hash_map->buckets = new_buckets;
          return 1;
        }
      hash_map->size--;
      return 0;
    }
// Below its case Not needed Resize Hashtable
  ind = hash_map->hash_func (in_pair->key) & (hash_map->capacity - 1);
  if (hash_map->buckets[ind] == NULL) // Case of allocate new vector
    {
      hash_map->buckets[ind] =
          vector_alloc (pair_copy, pair_cmp, pair_free);
      if (hash_map->buckets[ind] == NULL)
        {
          hash_map->size--;
          return 0;
        }
    }
  if (vector_push_back (hash_map->buckets[ind], in_pair))
    return 1;
  hash_map->size--;
  return 0;
}

/**
 * The function returns the value associated with the given key.
 * @param hash_map a hash map.
 * @param key the key to be checked.
 * @return the value associated with key if exists, NULL otherwise
 * (the value itself, not a copy of it).
 */
valueT hashmap_at (const hashmap *hash_map, const_keyT key)
{
  if (hash_map == NULL || key == NULL)
    return NULL;
  size_t ind = hash_map->hash_func (key) & (hash_map->capacity - 1);
  pair *curr = NULL;
  if (hash_map->buckets[ind] != NULL)
    for (size_t i = 0; i < hash_map->buckets[ind]->size; i++)
      {
        curr = hash_map->buckets[ind]->data[i];
        if (curr->key_cmp (key, curr->key))
          return curr->value;
      }
  return NULL;
}

/**
 * The function erases the pair associated with key.
 * @param hash_map a hash map.
 * @param key a key of the pair to be erased.
 * @return 1 if the erasing was done successfully, 0 otherwise.
 * (if key not in map, considered fail).
 */
int hashmap_erase (hashmap *hash_map, const_keyT key)
{

  if (hash_map == NULL || key == NULL)
      return 0;
  if (hashmap_at (hash_map, key) == NULL)
      return 0;



  size_t ind = hash_map->hash_func (key) & (hash_map->capacity - 1);
  vector *curr_vector = hash_map->buckets[ind];
  pair *curr_pair;
  hash_map->size--;
  if (hashmap_get_load_factor (hash_map) < HASH_MAP_MIN_LOAD_FACTOR)
    {
      vector **new_buckets = resize_buckets (hash_map, MINIMIZE);
      if (new_buckets == NULL)
        {
          hash_map->size++;
          return 0;
        }

      ind = hash_map->hash_func (key)
            & ((hash_map->capacity / HASH_MAP_GROWTH_FACTOR) - 1);
      curr_vector = new_buckets[ind];
      for (size_t i = 0; i < curr_vector->size; i++)
        {
          curr_pair = curr_vector->data[i];
          if (curr_pair->key_cmp (curr_pair->key, key))
            {
              if (vector_erase (curr_vector, i))
                {
                  delete_buckets (hash_map->buckets, hash_map->capacity);
                  hash_map->buckets = new_buckets;
                  hash_map->capacity /= HASH_MAP_GROWTH_FACTOR;
                  return 1;
                }
              return 0;
            }
        }
    }

  for (size_t i = 0; i < curr_vector->size; i++)
    {
      curr_pair = curr_vector->data[i];
      if (curr_pair->key_cmp (curr_pair->key, key))
        if (vector_erase (curr_vector, i))
          return 1;
    }
  hash_map->size++;
  return 0;
}

/**
 * This function returns the load factor of the hash map.
 * @param hash_map a hash map.
 * @return the hash map's load factor, -1 if the function failed.
 */
double hashmap_get_load_factor (const hashmap *hash_map)
{
  if (hash_map == NULL || hash_map->capacity == 0)
    return -1;
  return ((double) hash_map->size) / ((double) hash_map->capacity);
}

/**
 * This function receives a hashmap and 2 functions, the first
 * checks a condition on the keys, and the seconds apply some modification
 * on the values. The function should apply the modification
 * only on the values that are associated with keys that meet the condition.
 * Example: if the hashmap maps char->int, keyT_func checks
 * if the char is a capital letter (A-Z), and val_t_func multiples the number
 * by 2, hashmap_apply_if will change the map:
 * {('C',2),('#',3),('X',5)}, to: {('C',4),('#',3),('X',10)}, and the
 * return value will be 2.
 * @param hash_map a hashmap
 * @param keyT_func a function that checks a condition on keyT and
 * return 1 if true, 0 else
 * @param valT_func a function that modifies valueT, in-place
 * @return number of changed values
 */
int hashmap_apply_if
    (const hashmap *hash_map, keyT_func keyT_func, valueT_func valT_func)
{
  if (hash_map == NULL || keyT_func == NULL || valT_func == NULL)
    return -1;
  int counter = 0;
  for (size_t i = 0; i < hash_map->capacity; i++)
    if (hash_map->buckets[i] != NULL)
      {
        for (size_t j = 0; j < hash_map->buckets[i]->size; j++)
          {
            pair *curr = hash_map->buckets[i]->data[j];
            if (keyT_func (curr->key) == 1)
              {
                valT_func (curr->value);
                counter++;
              }
          }
      }
  return counter;
}