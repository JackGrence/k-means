#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>
#include <time.h>
#include "linklist.h"
#include "display.h"
#include "jpeglib.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo);
GLOBAL(int) read_JPEG_file (char * filename);
rgb_list ** clustering(int k, rgb_list *rgbList, int rgbList_len);
int get_distance_square (rgb_list *list1, rgb_list *list2);
bool clustering_with_clusterAry (int k, rgb_list **cluster_ary);

JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;		/* Number of columns in image */

int
main(int argc, char *argv[])
{
    char ary[] = {1, 2, 3, 4, 5, 6};
    rgb_list *rgbList;
    rgb_list **final_ary;
    rgbList = create_rgbList ();
    add_rgbList (rgbList, &ary[0]);
    add_rgbList (rgbList, &ary[1]);
    add_rgbList (rgbList, &ary[2]);
    add_rgbList (rgbList, &ary[3]);
    final_ary = clustering (2, rgbList->next, 4);
    int i;
    printf ("main\n");
    print_rgbList (rgbList->next);
    for (i = 0; i < 2; i++)
    {
        print_rgbList (final_ary[i]);
    }
    return 0;
}

rgb_list **
clustering(int k, rgb_list *rgbList, int rgbList_len)
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
    for (cur_rgbList = rgbList; cur_rgbList != NULL; cur_rgbList = cur_rgbList->next)
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
        add_rgbList (target_list, (char *) cur_rgbList);
    }

    while (clustering_with_clusterAry (k, cluster_ary));

    return cluster_ary;
}

bool
clustering_with_clusterAry (int k, rgb_list **cluster_ary)
{
    int i;
    int list_len;
    rgb_list *cur_rgbList;
    rgb_list **scanList_ary;
    bool result;
    result = false;

    scanList_ary = (rgb_list **) calloc (k, sizeof (rgb_list *));

    // reset center point
    for (i = 0; i < k; i++)
    {
        scanList_ary[i] = cluster_ary[i]->next;
        memset (cluster_ary[i], 0, 3);
        list_len = 0;
        for (cur_rgbList = cluster_ary[i]->next; cur_rgbList != NULL; cur_rgbList = cur_rgbList->next)
        {
            cluster_ary[i]->r += cur_rgbList->r;
            cluster_ary[i]->g += cur_rgbList->g;
            cluster_ary[i]->b += cur_rgbList->b;
            list_len++;
        }
        cluster_ary[i]->r /= list_len;
        cluster_ary[i]->g /= list_len;
        cluster_ary[i]->b /= list_len;
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
        for (cur_rgbList = cluster_ary[scan]; cur_rgbList != NULL; cur_rgbList = cur_rgbList->next)
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
            exit(1);
        }

        // categorize each list
        for (cur_rgbList = scanList_ary[scan]; cur_rgbList != NULL; cur_rgbList = cur_rgbList->next)
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
                add_rgbList (target_list, (char *) cur_rgbList);

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
get_distance_square (rgb_list *list1, rgb_list *list2)
{
    int sum;
    int diff;
    diff = list1->r - list2->r;
    diff = abs (diff);
    sum = diff * diff;
    diff = list1->g - list2->g;
    diff = abs (diff);
    sum += diff * diff;
    diff = list1->b - list2->b;
    diff = abs (diff);
    sum += diff * diff;
    return sum;
}

GLOBAL(int)
read_JPEG_file (char * filename)
{
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    FILE * infile;		/* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */
    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }

    /* Step 1: allocate and initialize JPEG decompression object */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */

    (void) jpeg_read_header(&cinfo, TRUE);

    /* Step 4: set parameters for decompression */

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&cinfo);
    row_stride = cinfo.output_width * cinfo.output_components;
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        int i;
        for (i = 0; i < row_stride; i++)
        {
            printf("%d ", buffer[0][i]);
        }
        printf ("\n");
    }

    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);

    /* Step 8: Release JPEG decompression object */

    jpeg_destroy_decompress(&cinfo);

    fclose(infile);

    return 1;
}

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}
