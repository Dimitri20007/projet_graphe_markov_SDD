#include <malloc.h>
#include "graph.h"

/* removeTransitiveLinks: supprime les liens transitifs du t_link_array */
void removeTransitiveLinks(t_link_array *p_link_array)
{
    int i = 0;
    while (i < p_link_array->size)
    {
        t_link link1 = p_link_array->links[i];
        int j = 0;
        int to_remove = 0;
        while (j < p_link_array->size && !to_remove)
        {
            if (j != i)
            {
                t_link link2 = p_link_array->links[j];
                if (link1.from == link2.from)
                {
                    // rechercher un lien de link2.to vers link1.to
                    int k = 0;
                    while (k < p_link_array->size && !to_remove)
                    {
                        if (k != j && k != i)
                        {
                            t_link link3 = p_link_array->links[k];
                            if ((link3.from == link2.to) && (link3.to == link1.to))
                            {
                                to_remove = 1;
                            }
                        }
                        k++;
                    }
                }
            }
            j++;
        }
        if (to_remove)
        {
            // supprimer link1 en le remplaçant par le dernier lien
            p_link_array->links[i] = p_link_array->links[p_link_array->size - 1];
            p_link_array->size--;
        }
        else
        {
            i++;
        }
    }
}