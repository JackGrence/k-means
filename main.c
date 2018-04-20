#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>
#include "version.h"
#include "linklist.h"
#include "display.h"
#include "jpeglib.h"

struct my_error_mgr
{
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

void my_error_exit (j_common_ptr cinfo);
rgb_list *read_JPEG_file (char *filename);
rgb_list **clustering (int k, rgb_list * rgbList, int rgbList_len);
int get_distance_square (rgb_list * list1, rgb_list * list2);
int get_target_pos (int k, rgb_list ** cluster_ary, rgb_list * rgbList);
bool clustering_with_clusterAry (int k, rgb_list ** cluster_ary);
void set_image_buffer (int k, rgb_list ** cluster_ary, rgb_list * rgbList);
void
write_JPEG_file (char *filename, int quality, int k, rgb_list ** cluster_ary,
		 rgb_list * rgbList);

JSAMPLE *image_buffer;		/* Points to large array of R,G,B-order data */
int image_height;		/* Number of rows in image */
int image_width;		/* Number of columns in image */
int image_pixel_len;

int
main (int argc, char *argv[])
{
  if (argc < 3)
    {
      printf ("Usage: ./kmeans file.jpg k\n");
      return 1;
    }
  int i;
  int k = strtol (argv[2], NULL, 10);
  rgb_list *rgbList;
  rgb_list **final_ary;
  rgbList = read_JPEG_file (argv[1]);
  final_ary = clustering (k, rgbList->next, image_pixel_len);

  rgbList->next = reverse_rgbList (rgbList->next, NULL);

  image_buffer =
    (unsigned char *) calloc (image_width * 3, sizeof (unsigned char));
  argv[1][0] = argv[2][0];
  write_JPEG_file (argv[1], 75, k, final_ary, rgbList->next);

  for (i = 0; i < k; i++)
    {
      free_rgbList (final_ary[i]);
    }
  free (final_ary);
  free_rgbList (rgbList);
  free (image_buffer);
  return 0;
}

void
set_image_buffer (int k, rgb_list ** cluster_ary, rgb_list * rgbList)
{
  int pixel_pos;
  int target;
  rgb_list *cur_rgbList;
  image_buffer =
    (unsigned char *) calloc (image_pixel_len * 3, sizeof (unsigned char));
  // scan every pixel
  for (pixel_pos = image_pixel_len - 1, cur_rgbList = rgbList;
       cur_rgbList != NULL; pixel_pos--, cur_rgbList = cur_rgbList->next)
    {
      target = get_target_pos (k, cluster_ary, cur_rgbList);
      memcpy (&image_buffer[pixel_pos * 3], cluster_ary[target], 3);
    }
}

int
get_target_pos (int k, rgb_list ** cluster_ary, rgb_list * rgbList)
{
  int i;
  int min;
  int distance;
  int target;

  target = 0;
  min = get_distance_square (cluster_ary[0], rgbList);
  for (i = 1; i < k; i++)
    {
      distance = get_distance_square (cluster_ary[i], rgbList);
      if (distance < min)
	{
	  min = distance;
	  target = i;
	}
    }
  return target;
}

rgb_list **
clustering (int k, rgb_list * rgbList, int rgbList_len)
{
  int i;
  int list_index;
  int rand_index;
  rgb_list **cluster_ary;
  rgb_list *cur_rgbList;

  if (k <= 0)
    return NULL;

  cur_rgbList = rgbList;

  cluster_ary = (rgb_list **) calloc (k, sizeof (rgb_list *));

  srand (time (NULL));

  // init center point
  for (i = 0; i < k; i++)
    {
      cluster_ary[i] = create_rgbList ();
      rand_index = rand () % (rgbList_len / k);
      for (list_index = 0; list_index < rand_index; list_index++)
	{
	  cur_rgbList = cur_rgbList->next;
	}
      memcpy (cluster_ary[i], cur_rgbList, 3);
      cur_rgbList = cur_rgbList->next;
    }

  int min;
  int distance;;
  rgb_list *target_list;
  // scan every pixel
  for (cur_rgbList = rgbList; cur_rgbList != NULL;
       cur_rgbList = cur_rgbList->next)
    {
      target_list = cluster_ary[0];
      min = get_distance_square (cluster_ary[0], cur_rgbList);
      for (i = 1; i < k; i++)
	{
	  distance = get_distance_square (cluster_ary[i], cur_rgbList);
	  if (distance < min)
	    {
	      target_list = cluster_ary[i];
	      min = distance;
	    }
	}
      add_rgbList (target_list, (unsigned char *) cur_rgbList);
    }

  while (clustering_with_clusterAry (k, cluster_ary));

  return cluster_ary;
}

bool
clustering_with_clusterAry (int k, rgb_list ** cluster_ary)
{
  int i;
  int list_len;
  int RGBsum[3];
  rgb_list *cur_rgbList;
  rgb_list **scanList_ary;
  bool result;
  result = false;

  scanList_ary = (rgb_list **) calloc (k, sizeof (rgb_list *));

  // reset center point
  for (i = 0; i < k; i++)
    {
      scanList_ary[i] = cluster_ary[i]->next;
      memset (&RGBsum, 0, 3 * sizeof (int));
      list_len = 0;
      for (cur_rgbList = cluster_ary[i]->next; cur_rgbList != NULL;
	   cur_rgbList = cur_rgbList->next)
	{
	  RGBsum[0] += cur_rgbList->r;
	  RGBsum[1] += cur_rgbList->g;
	  RGBsum[2] += cur_rgbList->b;
	  list_len++;
	}
      if (list_len != 0)
	{
	  RGBsum[0] /= list_len;
	  RGBsum[1] /= list_len;
	  RGBsum[2] /= list_len;
	  cluster_ary[i]->r = RGBsum[0];
	  cluster_ary[i]->g = RGBsum[1];
	  cluster_ary[i]->b = RGBsum[2];
	}
    }

  int min;
  int scan;
  int distance;
  rgb_list *target_list;
  rgb_list *prev_list;
  prev_list = NULL;
  for (scan = 0; scan < k; scan++)
    {
      // find prev list
      for (cur_rgbList = cluster_ary[scan]; cur_rgbList != NULL;
	   cur_rgbList = cur_rgbList->next)
	{
	  if (cur_rgbList->next == scanList_ary[scan])
	    {
	      prev_list = cur_rgbList;
	      break;
	    }
	}

      if (prev_list == NULL)
	{
	  printf ("something error, exit.\n");
	  exit (1);
	}

      // categorize each list
      for (cur_rgbList = scanList_ary[scan]; cur_rgbList != NULL;
	   cur_rgbList = cur_rgbList->next)
	{
	  target_list = cluster_ary[scan];
	  min = get_distance_square (cluster_ary[scan], cur_rgbList);
	  for (i = 0; i < k; i++)
	    {
	      if (i == scan)
		continue;
	      distance = get_distance_square (cluster_ary[i], cur_rgbList);
	      if (distance < min)
		{
		  target_list = cluster_ary[i];
		  min = distance;
		  result = true;
		}
	    }

	  if (target_list != cluster_ary[scan])
	    {
	      add_rgbList (target_list, (unsigned char *) cur_rgbList);

	      // remove current element from cur list
	      prev_list->next = cur_rgbList->next;
	      cur_rgbList->next = NULL;
	      free (cur_rgbList);
	      cur_rgbList = prev_list;
	    }

	  prev_list = cur_rgbList;
	}
    }
  return result;
}

int
get_distance_square (rgb_list * list1, rgb_list * list2)
{
  int sum;
  int diff;
  diff = (int) list1->r - (int) list2->r;
  diff = abs (diff);
  sum = diff * diff;
  diff = (int) list1->g - (int) list2->g;
  diff = abs (diff);
  sum += diff * diff;
  diff = (int) list1->b - (int) list2->b;
  diff = abs (diff);
  sum += diff * diff;
  return sum;
}

rgb_list *
read_JPEG_file (char *filename)
{
  struct jpeg_decompress_struct cinfo;
  struct my_error_mgr jerr;
  FILE *infile;			/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  int row_stride;		/* physical row width in output buffer */
  if ((infile = fopen (filename, "rb")) == NULL)
    {
      fprintf (stderr, "can't open %s\n", filename);
      return 0;
    }

  /* Step 1: allocate and initialize JPEG decompression object */
  cinfo.err = jpeg_std_error (&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  if (setjmp (jerr.setjmp_buffer))
    {
      jpeg_destroy_decompress (&cinfo);
      fclose (infile);
      return 0;
    }
  jpeg_create_decompress (&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src (&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header (&cinfo, TRUE);

  /* Step 4: set parameters for decompression */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress (&cinfo);
  row_stride = cinfo.output_width * cinfo.output_components;
  buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  image_pixel_len = 0;
  image_width = row_stride / 3;
  image_height = 0;
  rgb_list *rgbList;
  rgbList = create_rgbList ();
  while (cinfo.output_scanline < cinfo.output_height)
    {
      (void) jpeg_read_scanlines (&cinfo, buffer, 1);
      int i;
      for (i = 0; i < row_stride; i += 3)
	{
	  add_rgbList (rgbList, (unsigned char *) &buffer[0][i]);
	  image_pixel_len++;
	}
      image_height++;
    }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress (&cinfo);

  /* Step 8: Release JPEG decompression object */

  jpeg_destroy_decompress (&cinfo);

  fclose (infile);

  return rgbList;
}

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp (myerr->setjmp_buffer, 1);
}

void
write_JPEG_file (char *filename, int quality, int k, rgb_list ** cluster_ary,
		 rgb_list * rgbList)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE *outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */
  cinfo.err = jpeg_std_error (&jerr);
  jpeg_create_compress (&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  if ((outfile = fopen (filename, "wb")) == NULL)
    {
      fprintf (stderr, "can't open %s\n", filename);
      exit (1);
    }
  jpeg_stdio_dest (&cinfo, outfile);

  /* Step 3: set parameters for compression */
  cinfo.image_width = image_width;	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;	/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB;	/* colorspace of input image */
  jpeg_set_defaults (&cinfo);
  jpeg_set_quality (&cinfo, quality,
		    TRUE /* limit to baseline-JPEG values */ );

  /* Step 4: Start compressor */
  jpeg_start_compress (&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  int i;
  int target;
  while (cinfo.next_scanline < cinfo.image_height)
    {
      for (i = 0; i < image_width; i++)
	{
	  target = get_target_pos (k, cluster_ary, rgbList);
	  memcpy (&image_buffer[i * 3], cluster_ary[target], 3);
	  rgbList = rgbList->next;
	}
      row_pointer[0] = image_buffer;
      (void) jpeg_write_scanlines (&cinfo, row_pointer, 1);
    }

  /* Step 6: Finish compression */

  jpeg_finish_compress (&cinfo);
  fclose (outfile);

  /* Step 7: release JPEG compression object */
  jpeg_destroy_compress (&cinfo);
}
