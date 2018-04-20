#include <stdbool.h>

#ifndef LINKLIST
#define LINKLIST

#define TABLE_LIST_SIZE 500

typedef struct _table_list table_list;

struct _table_list
{
  int table[TABLE_LIST_SIZE];
  table_list *next_table;
};

typedef struct _array_list array_list;

struct _array_list
{
  int size;
  int *ary;
  array_list *next;
};

typedef struct _int_list int_list;

struct _int_list
{
  int val;
  int_list *next;
};

typedef struct _rgb_list rgb_list;

struct _rgb_list
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  rgb_list *next;
};

table_list *create_tableList (void);
void free_tableList (table_list * victim);
void tableL_insert (int val, table_list * des, bool needcount);
int get_tableL_val (int ind, table_list * tableL);
int set_tableL_val (int ind, table_list * tableL, int val);
int_list *create_intList (void);
void free_intList (int_list * intList);
int_list *add_intList (int_list * intList, int val);
rgb_list *create_rgbList (void);
rgb_list *create_rgbList_by_RGB (unsigned char rgb[3]);
void free_rgbList (rgb_list * rgbList);
rgb_list *add_rgbList (rgb_list * rgbList, unsigned char rgb[3]);
rgb_list *reverse_rgbList (rgb_list * rgbList, rgb_list * prevList);

#endif
