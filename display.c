#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "linklist.h"
#include "display.h"

void
print_ary (int *ary, int len)
{
  if (!DEBUG_PRINT)
    return;
  int i;
  printf ("[ ");
  for (i = 0; i < len; i++)
    {
      printf ("%d ", ary[i]);
    }
  printf ("]\n");
}

void
print_tableList (table_list * tableL)
{
  if (!DEBUG_PRINT)
    return;
  int cnt = 0;
  int i;
  while (tableL != NULL)
    {
      printf ("Find table: %d\n[ ", cnt);
      for (i = 0; i < TABLE_LIST_SIZE; i++)
	{
	  printf ("%d ", tableL->table[i]);
	}
      printf ("]\n");
      tableL = tableL->next_table;
      cnt++;
    }
}

void
calc_time (const char *msg)
{
  if (!DEBUG_PRINT)
    return;
  end = clock ();
  spent = (double) (end - begin) / CLOCKS_PER_SEC;
  printf ("%s, spent: %fs\n", msg, spent);
}

void
print_intList (int_list * intList)
{
  if (!DEBUG_PRINT)
    return;
  printf ("%d ", intList->val);
  if (intList->next != NULL)
    print_intList (intList->next);
  else
    printf ("\n");
}

void
print_rgbList (rgb_list * rgbList)
{
  if (!DEBUG_PRINT)
    return;
  printf ("[%d %d %d] ", rgbList->r, rgbList->g, rgbList->b);
  if (rgbList->next != NULL)
    print_rgbList (rgbList->next);
  else
    printf ("\n");
}
