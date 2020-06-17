/*
 * CBLAS Error handling.
 * Rewrite by Martin Koehler <koehlerm(AT)mpi-magdeburg.mpg.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "cblas.h"
#include "cblas_f77.h"

// typedef int (*cblas_xerbla_function_t)(void *, int, const char*, const char*, va_list);
// cblas_xerbla_function_t cblas_xerbla_set(cblas_xerbla_function_t new_handler);
// void * cblas_xerbla_set_aux(void *aux);

/* Initialize the default  */
static void *errhandler_aux = NULL;
static cblas_xerbla_function_t errorhandler = NULL;

cblas_xerbla_function_t cblas_xerbla_set(cblas_xerbla_function_t new_handler)
{
    cblas_xerbla_function_t old_handler = errorhandler;
    errorhandler = new_handler;
    return old_handler;
}

void *cblas_xerbla_set_aux(void *aux)
{
    void *oldaux = errhandler_aux;
    errhandler_aux = aux;
    return oldaux;
}


void cblas_xerbla(int info, const char *rout, const char *form, ...)
{
   extern int RowMajorStrg;
   char empty[1] = "";
   va_list argptr;
   int ret_handler = 1;

   va_start(argptr, form);

   if ( errorhandler != NULL )
   {
        va_list argptr2;
        va_copy(argptr, argptr2);
        ret_handler = errorhandler(errhandler_aux, info, rout, form, argptr2);
        va_end(argptr2);
   }


   if ( ret_handler == 0 )
       return;

   if (RowMajorStrg)
   {
      if (strstr(rout,"gemm") != 0)
      {
         if      (info == 5 ) info =  4;
         else if (info == 4 ) info =  5;
         else if (info == 11) info =  9;
         else if (info == 9 ) info = 11;
      }
      else if (strstr(rout,"symm") != 0 || strstr(rout,"hemm") != 0)
      {
         if      (info == 5 ) info =  4;
         else if (info == 4 ) info =  5;
      }
      else if (strstr(rout,"trmm") != 0 || strstr(rout,"trsm") != 0)
      {
         if      (info == 7 ) info =  6;
         else if (info == 6 ) info =  7;
      }
      else if (strstr(rout,"gemv") != 0)
      {
         if      (info == 4)  info = 3;
         else if (info == 3)  info = 4;
      }
      else if (strstr(rout,"gbmv") != 0)
      {
         if      (info == 4)  info = 3;
         else if (info == 3)  info = 4;
         else if (info == 6)  info = 5;
         else if (info == 5)  info = 6;
      }
      else if (strstr(rout,"ger") != 0)
      {
         if      (info == 3) info = 2;
         else if (info == 2) info = 3;
         else if (info == 8) info = 6;
         else if (info == 6) info = 8;
      }
      else if ( (strstr(rout,"her2") != 0 || strstr(rout,"hpr2") != 0)
                 && strstr(rout,"her2k") == 0 )
      {
         if      (info == 8) info = 6;
         else if (info == 6) info = 8;
      }
   }
   if (info)
      fprintf(stderr, "Parameter %d to routine %s was incorrect\n", info, rout);
   vfprintf(stderr, form, argptr);
   va_end(argptr);
   if (info && !info)
      F77_xerbla(empty, &info); /* Force link of our F77 error handler */
   exit(-1);
}
