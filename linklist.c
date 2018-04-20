#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "linklist.h"

table_list *
create_tableList (void)
{
  table_list *new_table;
  new_table = (table_list *) malloc (sizeof (table_list));
  memset (new_table, 0, sizeof (table_list));
  return new_table;
}

void
free_tableList (table_list * victim)
{
  if (victim->next_table != NULL)
    free_tableList (victim->next_table);
  free (victim);
}

void
tableL_insert (int val, table_list * des, bool needcount)
{
  div_t val_pos;
  int i;

  val_pos = div (val, TABLE_LIST_SIZE);
  for (i = 0; i < val_pos.quot; i++)
    {
      if (des->next_table == NULL)
	{
	  des->next_table = create_tableList ();
	}
      des = des->next_table;
    }
  if (needcount)
    des->table[val_pos.rem]++;
}

int
get_tableL_val (int ind, table_list * tableL)
{
  div_t val_pos;
  int i;

  val_pos = div (ind, TABLE_LIST_SIZE);
  for (i = 0; i < val_pos.quot; i++)
    {
      if (tableL->next_table == NULL)
	return 0;
      tableL = tableL->next_table;
    }
  return tableL->table[val_pos.rem];
}

int
set_tableL_val (int ind, table_list * tableL, int val)
{
  div_t val_pos;
  int i;

  val_pos = div (ind, TABLE_LIST_SIZE);
  for (i = 0; i < val_pos.quot; i++)
    {
      if (tableL->next_table == NULL)
	tableL->next_table = create_tableList ();
      tableL = tableL->next_table;
    }
  tableL->table[val_pos.rem] = val;
  return tableL->table[val_pos.rem];
}

int_list *
create_intList (void)
{
  return (int_list *) calloc (1, sizeof (int_list));
}

void
free_intList (int_list * intList)
{
  if (intList->next != NULL)
    free_intList (intList->next);
  free (intList);
}


int_list *
add_intList (int_list * intList, int val)
{
  int_list *new_list;
  new_list = create_intList ();
  new_list->val = val;
  new_list->next = intList->next;
  intList->next = new_list;
  return new_list;
}

rgb_list *
create_rgbList (void)
{
  return (rgb_list *) calloc (1, sizeof (rgb_list));
}

void
free_rgbList (rgb_list * rgbList)
{
  rgb_list *nextList;

  do
    {
      nextList = rgbList->next;
      free (rgbList);
      rgbList = nextList;
    }
  while (rgbList != NULL);
}

rgb_list *
add_rgbList (rgb_list * rgbList, unsigned char rgb[3])
{
  rgb_list *new_list;
  new_list = create_rgbList ();
  memcpy (new_list, rgb, 3);
  new_list->next = rgbList->next;
  rgbList->next = new_list;
  return new_list;
}

rgb_list *
create_rgbList_by_RGB (unsigned char rgb[3])
{
  rgb_list *new_list;
  new_list = create_rgbList ();
  memcpy (new_list, rgb, 3);
  return new_list;
}

rgb_list *
reverse_rgbList (rgb_list * rgbList, rgb_list * prevList)
{
  rgb_list *cur_list;

  do
    {
      cur_list = rgbList->next;
      rgbList->next = prevList;
      prevList = rgbList;
      rgbList = cur_list;
    }
  while (cur_list != NULL);
  return prevList;
}
