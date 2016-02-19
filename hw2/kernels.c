/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
	"GSTARRR",              /* Team name */

    "Daniel Chiang",     /* First member full name */
    "d.chiang@mail.utoronto.ca",  /* First member email address */

    "Kunsu Chen",                   /* Second member full name (leave blank if none) */
    "kunsu.chen@mail.utoronto.ca"                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	    dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst)
{
	//tiling with vertical vectors
	int register B = 32;

	int register i,j,k;
	int register ib;
	int register dimMinusOne = dim-1;
	int register dimcalc;
	for(i=0; i<dim; i+=B){
		for(j=0;j<dim; j++){

			ib = i+B;
			dimcalc = (dimMinusOne-j)*dim;
			//walk through sub blocks
			for(k=i;k<ib ;k++){ //vertical
				dst[dimcalc+k] = src[RIDX(k, j, dim)];
			}
		}
	}

}

////////////////////////////////////////////////////////////////////////////////////////
//* second attempt (commented out for now)
char rotate_two_descr[] = "naive LICM + register";
void attempt_two(int dim, pixel *src, pixel *dst)
{
    int register i, j;
    int register dimi;
    int register dimMinusOne = dim-1;

    for (i = 0; i < dim; i++){
    	dimi = i*dim;
    	for (j = 0; j < dim; j++){
	    dst[RIDX(dimMinusOne-j, i, dim)] = src[dimi+j];
    	}
    }
}

char rotate_three_descr[] = "tiling block";
void attempt_three(int dim, pixel *src, pixel *dst)
{
	//tiling with square matrix
	int register B = 32;

	//walk through blocks
	int register i,j,k,l;
	for(i=0; i<dim; i+=B){
		for(j=0;j<dim; j+=B){
			//walk through sub blocks
			for(k=i;k<i+B;k++)
				for(l=j;l<j+B;l++)
					dst[RIDX(dim-1-l, k, dim)] = src[RIDX(k, l, dim)];
		}
	}

}

char rotate_four_descr[] = "tiling block + LICM";
void attempt_four(int dim, pixel *src, pixel *dst)
{
	//tiling with square matrix
	int register B = 32;

	//walk through blocks
	int register i,j,k,l;
	int register ib, jb;
	int register dimMinusOne = dim-1;
	int register kdim;

	for(i=0; i<dim; i+=B){
		for(j=0;j<dim; j+=B){

			ib = i+B;
			jb = j+B;

			//walk through sub blocks
			for(k=i;k<ib;k++){

				kdim = k*dim;

				for(l=j;l<jb;l++){
					dst[RIDX(dimMinusOne-l, k, dim)] = src[kdim+l];
				}
			}
		}
	}
}

char rotate_five_descr[] = "tiling horizontal";
void attempt_five(int dim, pixel *src, pixel *dst)
{
	//tiling with vertical vectors
	int register B = 32;

	int register i,j,k;
	int register jb;
	int register dimMinusOne = dim-1;
	int register dimi;
	for(i=0; i<dim; i++){
		for(j=0;j<dim; j+=B){

			jb = j+B;
			dimi = i*dim;
			//walk through sub blocks
			for(k=j;k<jb ;k++){ //horizontal
				dst[RIDX(dimMinusOne-k, i, dim)] = src[dimi+k];
			}
		}
	}

}


/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);
//
//    add_rotate_function(&attempt_two, rotate_two_descr);
//    add_rotate_function(&attempt_three, rotate_three_descr);
//    add_rotate_function(&attempt_four, rotate_four_descr);
//    add_rotate_function(&attempt_five, rotate_five_descr);
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

