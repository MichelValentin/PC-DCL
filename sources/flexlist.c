    /*

        flexlist.c

        Copyright 1989
        John W. Small
        All rights reserved

        PSW / Power SoftWare
        P.O. Box 10072
        McLean, Virginia 22102 8072
        (703) 759-3838

        1/14/89

    */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flexlist.h"
 
#define FlistHalloc(hdsize)  malloc ( \
      sizeof(struct FlistHeader) - sizeof(FlistData) \
      + (hdsize))
#define FlistNalloc(ndsize)  malloc ( \
      sizeof(struct FlistNode) - sizeof(FlistData) \
      + (ndsize))
#define Flistfree(mptr) free(mptr)


    /* housekeeping primitives */

    Flist mkFlist   (size_t  ndsize, size_t  hdsize)
    {
      Flist lptr;

      if (ndsize)
         if ((lptr = (struct FlistHeader *) FlistHalloc(hdsize)) != NULL)  {
            lptr->front = lptr->current = lptr->rear = NULL;
            lptr->ncurrent = lptr->nodes = 0;
            lptr->ndlen = ndsize;
            lptr->hdlen = hdsize;
            return lptr;
         }
      return NULL;
    }



    int     clrFlist    (Flist lptr)
    {
      FlistN  nptr;

      if (lptr)  {
         while ((nptr = lptr->front) != NULL)  {
           lptr->front = nptr->next;
           Flistfree(nptr);
         }
         lptr->current = lptr->rear = NULL;
         lptr->ncurrent = lptr->nodes = 0;
         return 1;
      }
      return 0;
    }

    int      rmFlist    (Flist *lptrAddr)
    {
       if (clrFlist(*lptrAddr))  {
          Flistfree(*lptrAddr);
          *lptrAddr = NULL;
          return 1;
       }
       return 0;
    }

    unsigned nempty    (Flist lptr)
    {
      return lptr? lptr->nodes : 0;
    }

    void    *Flistdptr  (Flist lptr)
    {
      if (lptr)
         if (lptr->hdlen)
            return &lptr->data;
      return NULL;
    }



    /* stack and queue primitives */

    void    *pushdptr  (Flist lptr, void *bufAddr)
    {
      FlistN nptr;

      if (lptr)
         if ((nptr = (struct FlistNode *) FlistNalloc(lptr->ndlen)) != NULL)  {
            if (bufAddr)
               memcpy(&nptr->data,bufAddr,lptr->ndlen);
            nptr->prev = NULL;
            if ((nptr->next = lptr->front) != NULL)
               nptr->next->prev = nptr;
            else
               lptr->rear =  nptr;
            lptr->front = nptr;
            lptr->nodes++;
            if (lptr->ncurrent)
               lptr->ncurrent++;
            return &nptr->data;
         }
      return NULL;
    }

    int      pushn     (Flist lptr, FlistN nptr)
    {
      if (lptr && nptr)  {
         nptr->prev = NULL;
         if ((nptr->next = lptr->front) != NULL)
            nptr->next->prev = nptr;
         else
            lptr->rear =  nptr;
         lptr->front = nptr;
         lptr->nodes++;
         if (lptr->ncurrent)
            lptr->ncurrent++;
         return 1;
      }
      return 0;
    }



    int      popd      (Flist lptr, void *bufAddr)
    {
       FlistN nptr;

       if (lptr)
          if ((nptr = lptr->front) != NULL)  {
             if (bufAddr)
                memcpy(bufAddr,&nptr->data,lptr->ndlen);
             if (lptr->front == lptr->rear)
                lptr->rear = NULL;
             else
                nptr->next->prev = NULL;
             lptr->front = nptr->next;
             lptr->nodes--;
             Flistfree(nptr);
             if (lptr->ncurrent)
                if (!--lptr->ncurrent)
                   lptr->current = NULL;
             return 1;
          }
       return 0;
     }

    FlistN popn      (Flist lptr)
    {
       FlistN nptr;

       if (lptr)
          if ((nptr = lptr->front) != NULL)  {
             if (lptr->front == lptr->rear)
                lptr->rear = NULL;
             else
                nptr->next->prev = NULL;
             lptr->front = nptr->next;
             lptr->nodes--;
             if (lptr->ncurrent)
                if (!--lptr->ncurrent)
                   lptr->current = NULL;
             return nptr;
          }
       return NULL;
     }



    void    *topdptr   (Flist lptr, void *bufAddr)
    {
      if (lptr)
         if (lptr->front)  {
            if (bufAddr)
               memcpy (
                 bufAddr,&lptr->front->data,lptr->ndlen);
            return &lptr->front->data;
         }
      return NULL;
    }

    void    *iquedptr  (Flist lptr, void *bufAddr)
    {
      FlistN nptr;

      if (lptr)
         if ((nptr = (struct FlistNode *) FlistNalloc(lptr->ndlen)) != NULL)  {
            if (bufAddr)
               memcpy(&nptr->data,bufAddr,lptr->ndlen);
            nptr->next = NULL;
            if (lptr->rear)
               lptr->rear->next = nptr;
            else
               lptr->front = nptr;
            nptr->prev = lptr->rear;
            lptr->rear = nptr;
            lptr->nodes++;
            return &nptr->data;
         }
      return NULL;
    }

    int      iquen     (Flist lptr, FlistN nptr)
    {
      if (lptr && nptr)  {
         nptr->next = NULL;
         if (lptr->rear)
            lptr->rear->next = nptr;
         else
            lptr->front = nptr;
         nptr->prev = lptr->rear;
         lptr->rear = nptr;
         lptr->nodes++;
         return 1;
      }
      return 0;
    }



    /* current node primitives */

    unsigned ncur      (Flist lptr)
    {
      return lptr? lptr->ncurrent : 0;
    }

    void    *curdptr   (Flist lptr)
    {
      if (lptr)
         if (lptr->current)
            return &lptr->current->data;
      return NULL;
    }



    void    *mkcdptr   (Flist lptr, unsigned loc)
    {
      register FlistN nptr;
      register unsigned i;

      if (lptr)  {
         if ((loc < 1) || (loc > lptr->nodes))  {
            lptr->current = NULL;
            lptr->ncurrent = 0;
            return NULL;
         }
         else if (loc == lptr->ncurrent)
            return &lptr->current->data;
         else  {
            if (lptr->ncurrent)
               if (loc > lptr->ncurrent)
                  if (((lptr->nodes >> 1) +
                      (lptr->ncurrent >> 1)) < loc)
                     for (nptr=lptr->rear,
                          i=lptr->nodes-loc;i;i--)
                       nptr = nptr->prev;
                  else
                     for (nptr=lptr->current,
                          i=loc-lptr->ncurrent;i;i--)
                       nptr = nptr->next;
               else
                  if ((lptr->ncurrent >> 1) < loc)
                     for (nptr=lptr->current,
                          i=lptr->ncurrent-loc;i;i--)
                       nptr = nptr->prev;
                  else
                     for (nptr=lptr->front,i=loc-1;i;i--)
                       nptr = nptr->next;
            else if ((lptr->nodes >> 1) < loc)
               for (nptr=lptr->rear,i=lptr->nodes-loc;i;i--)
                 nptr = nptr->prev;
            else
               for (nptr=lptr->front,i=loc-1;i;i--)
                 nptr = nptr->next;
            lptr->ncurrent = loc;
            lptr->current = nptr;
            return &lptr->current->data;
         }
      }
      return NULL;
    }



    /* list primitives */

    void    *insdptr   (Flist lptr, void *bufAddr)
    {
      FlistN nptr;

      if (lptr)
         if ((nptr = (struct FlistNode *) FlistNalloc(lptr->ndlen)) != NULL)  {
            if (bufAddr)
               memcpy(&nptr->data,bufAddr,lptr->ndlen);
            if ((nptr->prev = lptr->current) != NULL)  {
               if ((nptr->next = lptr->current->next) != NULL)
                  nptr->next->prev = nptr;
               else
                  lptr->rear = nptr;
               lptr->current->next = nptr;
            }
            else {
               if ((nptr->next = lptr->front) != NULL)
                  nptr->next->prev = nptr;
               else
                  lptr->rear = nptr;
               lptr->front = nptr;
            }
            lptr->current = nptr;
            lptr->ncurrent++;
            lptr->nodes++;
            return &nptr->data;
         }
      return NULL;
    }



    int      insn      (Flist lptr, FlistN nptr)
    {
      if (lptr && nptr)  {
         if ((nptr->prev = lptr->current) != NULL)  {
            if ((nptr->next = lptr->current->next) != NULL)
               nptr->next->prev = nptr;
            else
               lptr->rear = nptr;
            lptr->current->next = nptr;
         }
         else {
            if ((nptr->next = lptr->front) != NULL)
               nptr->next->prev = nptr;
            else
               lptr->rear = nptr;
            lptr->front = nptr;
         }
         lptr->current = nptr;
         lptr->ncurrent++;
         lptr->nodes++;
         return 1;
      }
      return 0;
    }



    int      deld      (Flist lptr, void *bufAddr)
    {
      FlistN nptr;

      if (lptr)
         if ((nptr = lptr->current) != NULL)  {
            lptr->current = nptr->prev;
            lptr->ncurrent--;
            if (nptr->next)
               nptr->next->prev = nptr->prev;
            else
               lptr->rear = nptr->prev;
            if (nptr->prev)
               nptr->prev->next = nptr->next;
            else
               lptr->front = nptr->next;
            lptr->nodes--;
            if (bufAddr)
               memcpy(bufAddr,&nptr->data,lptr->ndlen);
            Flistfree(nptr);
            return 1;
         }
      return 0;
    }

    FlistN deln      (Flist lptr)
    {
      FlistN nptr;

      if (lptr)
         if ((nptr = lptr->current) != NULL)  {
            lptr->current = nptr->prev;
            lptr->ncurrent--;
            if (nptr->next)
               nptr->next->prev = nptr->prev;
            else
               lptr->rear = nptr->prev;
            if (nptr->prev)
               nptr->prev->next = nptr->next;
            else
               lptr->front = nptr->next;
            lptr->nodes--;
            return nptr;
         }
      return NULL;
    }



    void    *nextdptr  (Flist lptr, void *bufAddr)
    {
      if (lptr)  {
         if (lptr->current)
            lptr->current = lptr->current->next;
         else
            lptr->current = lptr->front;
         if (lptr->current)  {
            lptr->ncurrent++;
            if (bufAddr)
               memcpy (
                 bufAddr,&lptr->current->data,lptr->ndlen);
            return &lptr->current->data;
         }
         else
            lptr->ncurrent = 0;
      }
      return NULL;
    }

    void    *prevdptr  (Flist lptr, void *bufAddr)
    {
      if (lptr)  {
         if (lptr->current)  {
            lptr->current = lptr->current->prev;
            lptr->ncurrent--;
         }
         else {
            lptr->current = lptr->rear;
            lptr->ncurrent = lptr->nodes;
         }
         if (lptr->current)  {
            if (bufAddr)
               memcpy (
                 bufAddr,&lptr->current->data,lptr->ndlen);
            return &lptr->current->data;
         }
      }
      return NULL;
    }



    int      getd      (Flist lptr, void *bufAddr)
    {
      if (lptr)
         if (lptr->current && bufAddr)  {
            memcpy (
              bufAddr,&lptr->current->data,lptr->ndlen);
            return 1;
         }
      return 0;
    }

    int      putd      (Flist lptr, void *bufAddr)
    {
      if (lptr)
         if (lptr->current && bufAddr)  {
            memcpy (
            &lptr->current->data,bufAddr,lptr->ndlen);
            return 1;
         }
      return 0;
    }


    /* array primitives */

    int   stod  (Flist lptr, void *bufAddr, unsigned loc)
    {
      if (mkcdptr(lptr,loc) && bufAddr)  {
         memcpy(&lptr->current->data,bufAddr,lptr->ndlen);
         return 1;
      }
      return 0;
    }

    int   rcld  (Flist lptr, void *bufAddr, unsigned loc)
    {
      if (mkcdptr(lptr,loc) && bufAddr)  {
         memcpy(bufAddr,&lptr->current->data,lptr->ndlen);
         return 1;
      }
      return 0;
    }
