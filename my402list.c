#include <stdlib.h>

#include "my402list.h"

int
My402ListLength( My402List* my402list )
{
	int returnValue = 0;
        if(my402list->num_members > 0){
                returnValue = my402list->num_members;
        }
        return returnValue;
}

int
My402ListEmpty( My402List* my402list )
{
	int returnValue = TRUE;
	if(my402list->num_members > 0){
		returnValue = FALSE;
	}
	return returnValue;
}

int
My402ListAppend( My402List* my402list, 
		 void* item )
{	
	int returnValue = TRUE;
	My402ListElem* my402listnewelem = malloc(sizeof(My402ListElem));
        if(!my402listnewelem){
               returnValue = FALSE;
         }else{
		if(my402list->num_members > 0){
			my402listnewelem->obj = item;
			my402listnewelem->prev = my402list->anchor.prev;
			my402listnewelem->next = &(my402list->anchor);
			my402list->anchor.prev->next = my402listnewelem;
                	my402list->anchor.prev = my402listnewelem;
		}else{
			my402listnewelem->obj = item;
                        my402listnewelem->prev = &(my402list->anchor);
                        my402listnewelem->next = &(my402list->anchor);
                        my402list->anchor.next = my402listnewelem;
                        my402list->anchor.prev = my402listnewelem;
		}
		my402list->num_members++;
	 }  
	return returnValue;
}

int
My402ListPrepend( My402List* my402list, 
		  void* item )
{
	 int returnValue = TRUE;
        My402ListElem* my402listnewelem = malloc(sizeof(My402ListElem));
        if(!my402listnewelem){
               returnValue = FALSE;
         }else{
                if(my402list->num_members > 0){
                        my402listnewelem->obj = item;
                        my402listnewelem->prev = &(my402list->anchor);
                        my402listnewelem->next = my402list->anchor.next;
                        my402list->anchor.next->prev = my402listnewelem;
                        my402list->anchor.next = my402listnewelem;
                }else{
                        my402listnewelem->obj = item;
                        my402listnewelem->prev = &(my402list->anchor);
                        my402listnewelem->next = &(my402list->anchor);
                        my402list->anchor.next = my402listnewelem;
                        my402list->anchor.prev = my402listnewelem;
                }
		my402list->num_members++;
         }
        return returnValue;
}

void
My402ListUnlink( My402List* my402list, 
		 My402ListElem* my402listelem )
{	
	my402listelem->next->prev = my402listelem->prev;
	my402listelem->prev->next = my402listelem->next;
	//free(my402listelem);
	my402list->num_members--;
}

void
My402ListUnlinkAll( My402List* my402list )
{
		int i = 0;
		My402ListElem* traverse = my402list->anchor.next;
		for(i=0;i<my402list->num_members;i++){
			My402ListElem* temp = traverse;
			traverse = traverse->next;
			free(temp);
		}
		my402list->num_members = 0;
}

int
My402ListInsertAfter( My402List* my402list, 
		      void* item, 
		      My402ListElem* my402listelem )
{
	int returnValue = TRUE;
	if(!my402listelem){
		returnValue = My402ListAppend(my402list,item);
	}else{
		My402ListElem* my402listnewelem = malloc(sizeof(My402ListElem));
		if(!my402listnewelem){
			returnValue = FALSE;
		}else{
			my402listnewelem->obj = item;
			my402listnewelem->prev = my402listelem;
			my402listnewelem->next = my402listelem->next;
			my402listelem->next->prev = my402listnewelem;
			my402listelem->next = my402listnewelem;
			my402list->num_members++;
		}
	}
	return returnValue;
}

int
My402ListInsertBefore( My402List* my402list, 
		       void* item, 
		       My402ListElem* my402listelem )
{
	int returnValue = TRUE;
        if(!my402listelem){
                returnValue = My402ListPrepend(my402list,item);
        }else{
                My402ListElem* my402listnewelem = malloc(sizeof(My402ListElem));
                if(!my402listnewelem){
                        returnValue = FALSE;
                }else{
                        my402listnewelem->obj = item;
                        my402listnewelem->prev = my402listelem->prev;
                        my402listnewelem->next = my402listelem;
                        my402listelem->prev->next = my402listnewelem;
                        my402listelem->prev = my402listnewelem;
                        my402list->num_members++;
                }
        }
        return returnValue;
}

My402ListElem*
My402ListFirst( My402List* my402list )
{
	My402ListElem* returnValue = NULL;
	if(my402list->num_members > 0){
		returnValue = my402list->anchor.next;
	}	
	return returnValue;
}

My402ListElem*
My402ListLast( My402List* my402list )
{
	My402ListElem* returnValue = NULL;
        if(my402list->num_members > 0){
                returnValue = my402list->anchor.prev;
        }
        return returnValue;
}

My402ListElem*
My402ListNext( My402List* my402list, 
	       My402ListElem* my402listelem )
{
	My402ListElem* returnValue = NULL;
	if(My402ListLast(my402list) != my402listelem){
		returnValue = my402listelem->next;
	}
	return returnValue;
}

My402ListElem*
My402ListPrev( My402List* my402list, 
	       My402ListElem* my402listelem )
{
	My402ListElem* returnValue = NULL;
        if(My402ListFirst(my402list) != my402listelem){
                returnValue = my402listelem->prev;
        }
        return returnValue;
}

My402ListElem*
My402ListFind( My402List* my402list, 
               void* item )
{
	My402ListElem* returnValue = NULL;
	My402ListElem* traverse = my402list->anchor.next;
	while(traverse != (&(my402list->anchor))){
		if(traverse->obj == item){
			returnValue = traverse;
			break;		
		}
		traverse = traverse->next;
	}
	return returnValue;
}

int 
My402ListInit( My402List* my402list )
{
	int returnValue = TRUE;
	if(!my402list){
		returnValue =  FALSE;
	}else{
		my402list->num_members = 0;
		my402list->anchor.next = my402list->anchor.prev;
		my402list->anchor.prev = my402list->anchor.next;
	}
	return returnValue;
}
