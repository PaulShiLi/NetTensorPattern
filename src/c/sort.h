#ifndef SORT_H
#define SORT_H
#include "UtilsOfTensor.h"

/*
 *  (C) Copyright by Ariel Faigon, 1987
 *  Released under the GNU GPL (General Public License) version 2
 *  or any later version (http://www.gnu.org/licenses/licenses.html)
 */
/*---------------            sort.h              --------------*/
/*--------------- sort library customizable file --------------*/

/*
 * This is the key TYPE.
 * Replace this typedef by YOUR key type.
 * e.g. if you're sorting an array of pointers to strings
 * You should do:
 *
 *	typedef char * KEY_T
 *
 * The keys are the items in the array that you're moving
 * around using the SWAP macro.
 * Note: the comparison function may compare any "function"
 * of this key, it doesn't necessarily need to compare the
 * key itself. example: you compare the strings pointed to
 * by the key itself.
 */ 
typedef double	KEY_T;

/*
 * These are the COMPARISON macros
 * Replace these macros by YOUR comparison operations.
 * e.g. if you are sorting an array of pointers to strings
 * you should define:
 *
 *	GT(x, y)  as   (strcmp((x),(y)) > 0) 	Greater than
 *	LT(x, y)  as   (strcmp((x),(y)) < 0) 	Less than
 *	GE(x, y)  as   (strcmp((x),(y)) >= 0) 	Greater or equal
 *	LE(x, y)  as   (strcmp((x),(y)) <= 0) 	Less or equal
 *	EQ(x, y)  as   (strcmp((x),(y)) == 0) 	Equal
 *	NE(x, y)  as   (strcmp((x),(y)) != 0) 	Not Equal
 */
#define GT(x, y) ((x) > (y))
#define LT(x, y) ((x) < (y))
#define GE(x, y) ((x) >= (y))
#define LE(x, y) ((x) <= (y))
#define EQ(x, y) ((x) == (y))
#define NE(x, y) ((x) != (y))

/*
 * This is the SWAP macro to swap between two keys.
 * Replace these macros by YOUR swap macro.
 * e.g. if you are sorting an array of pointers to strings
 * You can define it as:
 *
 *	#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp
 *
 * Bug: 'insort()' doesn't use the SWAP macro.
 */
#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp

/*-------------------- End of customizable part -----------------------*/
/*-------------------- DON'T TOUCH BEYOND THIS POINT ------------------*/
extern void    heapsort_inc ();
extern void    siftdown_inc ();
extern void    heapsort_dec ();
extern void    siftdown_dec ();



/* sort a value array and this input array is not moved at all. Only index array
  (unsigned int*) is rearranged according to the increasing order of the value array.
  After using "sortidx", you need to free it. */
extern void quicksortidx_inc_VEC_DOUBLE(VEC_DOUBLE v, VEC_UINT *sortidx);
/* sort a value array and this input array is not moved at all. Only index array
  (unsigned int*) is rearranged according to the decreasing order of the value array.
  After using "sortidx", you need to free it. */
extern void quicksortidx_dec_VEC_DOUBLE(VEC_DOUBLE v, VEC_UINT *sortidx);

#endif
