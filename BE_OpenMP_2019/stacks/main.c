#include "aux.h"
#include <omp.h>

void stacks_seq(stack_t *stacks, int n);
void stacks_par_critical(stack_t *stacks, int n);
void stacks_par_atomic(stack_t *stacks, int n);
void stacks_par_locks(stack_t *stacks, int n);


int main(int argc, char **argv){

  stack_t *stacks;
  int      i, j, n;
  long     t_start, t_end, save;

  
  if ( argc == 2 ) {
    n = atoi(argv[1]);    /* the number of stacks */
  } else {
    printf("Usage:\n\n ./main n\n\nwhere n is the number of stacks.\n");
    return 1;
  }

  printf("\n");
  
  init_stacks(&stacks, n);
  t_start = usecs();
  stacks_seq(stacks, n);
  t_end = usecs();
  printf("Sequential  version.       --------  time : %8.2f msec.     ",((double)t_end-t_start)/1000.0);
  check_result(stacks, n);
  free_stacks(&stacks, n);

  init_stacks(&stacks, n);
  t_start = usecs();
  stacks_par_critical(stacks, n);
  t_end = usecs();
  printf("Critical    version.       --------  time : %8.2f msec.     ",((double)t_end-t_start)/1000.0);
  check_result(stacks, n);
  free_stacks(&stacks, n);

  init_stacks(&stacks, n);
  t_start = usecs();
  stacks_par_atomic(stacks, n);
  t_end = usecs();
  printf("Atomic      version.       --------  time : %8.2f msec.     ",((double)t_end-t_start)/1000.0);
  check_result(stacks, n);
  free_stacks(&stacks, n);

  init_stacks(&stacks, n);
  t_start = usecs();
  stacks_par_locks(stacks, n);
  t_end = usecs();
  printf("Locks       version.       --------  time : %8.2f msec.     ",((double)t_end-t_start)/1000.0);
  check_result(stacks, n);
  free_stacks(&stacks, n);

  return 0;
  
}


void stacks_seq(stack_t *stacks, int n){

  int s;
  
  
  for(;;){

    /* Get the stack number s */
    s = get_random_stack();

    if(s==-1) break;
    
    /* Push some value on stack s */
    stacks[s].elems[stacks[s].cnt++] = process();

  }
  
}



void stacks_par_critical(stack_t *stacks, int n){

  int s , temp;
  
  #pragma omp parallel private(s,temp)
  {
    for(;;){

      /* Get the stack number s */
      s = get_random_stack();

      if(s==-1) break;
      temp = process();
      #pragma omp critical
      {
      /* Push some value on stack s */
      stacks[s].elems[stacks[s].cnt++] = temp;
      }
    }
  }
}




void stacks_par_atomic(stack_t *stacks, int n){

  int s, temp;
  
  #pragma omp parallel private(s,temp)
  {
      for(;;){

        /* Get the stack number s */
        s = get_random_stack();

        if(s==-1) break;
        #pragma omp atomic capture
        temp = stacks[s].cnt++ ;

       

        /* Push some value on stack s */
        stacks[s].elems[temp] = process();
        
      }
  }
}



void stacks_par_locks(stack_t *stacks, int n){

  int s, temp;
  omp_lock_t *locks;
  
  locks = (omp_lock_t*)malloc(1000*sizeof(omp_lock_t));
  
  /* omp_init_lock(&lock) */
  for ( int i=0 ; i < 1000 ; i++ ) {
    omp_init_lock(locks+i);
  }  
  
  for(;;){

    /* Get the stack number s */
    s = get_random_stack();

    if(s==-1) break;
    temp = process();
    /* Push some value on stack s */
    omp_set_lock(locks+s);
    stacks[s].elems[stacks[s].cnt++] =temp;
    omp_unset_lock(locks+s);

  }
  /* omp_destroy_lock(&lock) */
 for ( int i=0 ; i < 1000 ; i++ ) {
    omp_destroy_lock(locks+i);
  }  

}
