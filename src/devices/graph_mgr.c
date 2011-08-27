/**
* @cond ham_internals
*/

/*
pagesize detect: door 2 dingen: phys geeft met mask aan wat pagesize is; virtual geeft met offset additief aan
hoeveel 'overhead' er max bij komt.

graph reg. met cycle detect, odat loops in graph detect worden en errormelding opleveren bij reg.
--> hou bij hoeveel nodes we hebben en geef nodes een max.depth nummer; reg. voegt node toe,
die dan recursief voor hernummering van depth zorgt: indien depth > aantal nodes, dan cycle detected.

filenamen: virtual nodes manipuleren die: \1, \2, etc. regex substitutie praktijken?
*/




/*
Proof of concept

Building, initializing and using a device driver graph, consisting of 

- 1 'entry point': the 'top' device driver

- zero or more 'virtual' device driver nodes

- 1 or more 'physical device driver' nodes


The idea is that the device driver nodes/layers each act as an object and together 
form a 'device chain' which supports partitioned and disparate I/O storage access methods.

'virtual' device driver layers are those device drivers which expect one or more 
device drivers registered as children. In terms of the directed graph, a 'virtual' 
node points to one or more other device driver nodes.

'physical' device layers (nodes) are expected to read and write data to storage 
devices available in the system. Network connections and other 
'application-external' I/O data channels are regarded as just some other 
'storage device' each.

In fact, a 'physical' device driver node is expected to be an end node of a directed 
graph path, while 'virtual' device driver nodes are by definition NEVER end nodes of 
a directed path.


This proof of concept is meant to showcase two different aspects of this device driver
setup:

- the ability to split I/O across disparate devices in a non-uniform way (flexible 
multi-level partitioning schemes) -- of particular interest are the technical details
ensuring a uniform page size presented at the 'top node' level, irrespective the
underlying graph, second there's the need to transform a single flat space 64-bit
top-level 'rid' atom address to an unambiguous address suitable for each device layer 
in the graph.

- test performance and limitations of various virtual device drivers, such as crypto
layers, compression layers, cache layers, partitioners, etc.


The design is done in an Object Oriented manner, where method-overloading and various
design patterns are useful (e.g. the factory pattern).

The implementation is done in portable C do allow easy migration into the HamsterDB 
code proper.
*/




#include "internal_preparation.h"






/**
Manage the device graph: add/remove/connect devices.
*/










/*
Assume the user constructs his own device graph; that 
way he can construct arbitrary complex device graphs without us
having to provide an API for such.

All we require is the user constructs that device graph in such
a way that cleanup is doable.

THOUGHT: as devices can be linked to others in multiple ways, we need
offer some sort of reference counting hook, which pops the next question:

what if I want to play nasty and wish to keep device 'alive' after the
related env or db has been closed already? This could easily be done
by upping the reference count so that the hamster cleanup code does
not destroy the device after the final refcount decrement.

Consequences? Usefulness? --> Think network devices or other I/O devices,
which can be 're-used'. Keeping the 'alive' would be one way to keep the
device graph intact.

To make sure hamster is transparent to this outside stuff, CREATE/OPEN
should INCREMENT all the refcounters, and CLOSE should DECREMENT these.
Then the cleanup code can do the refcount decrementing, just like the
open/create callbacks in there can do the refcount upping, and final cleanup
will happen when refcount finally drops back to zero. Presto.

Should we provide helper functions for this graph construction stuff anyway?

devices can be statically allocated, on the stack or on the heap. The allocator 
and cleanup callback must cope with that. hamster should be oblivious to this.

How do we link up devices in the directed graph? links. --> pointers. (to/from bidirectional or
'to' unidirectional? Well, sometimes, you need to traverse the graph in the reverse
direction and not only when you are invoking various devices (when such can be done
using the call stack fake ham_device_invocation_t ) so bidirectional is probably preferred.)
*/

typedef struct
{
    ham_device_t *culprit;
    int reason;
    union
    {
        ham_device_t *from;
        
        ham_device_t *to;

        struct
        {
            ham_size_t lambda;
            ham_size_t mu;
        } cycle;

    } extra_info;
} ham_device_graph_check_report_t;

/*
An (x_i, i) stack item for use with the Navisch-derived cycle detection algorithm.
*/
typedef struct
{
    ham_device_t *dev_i; /**< this is our version of Navisch' x_i */
    ham_size_t call_depth; /**< this is our version of Navisch' i */
} ham_device_graph_pathcheck_t;


typedef enum cycle_seek_state_t
{
    FIND_MU,
    MU_FOUND
} cycle_seek_state_t;

typedef struct
{
    mem_allocator_t *mm;
    ham_device_graph_pathcheck_t *stack;
    ham_size_t alloc_size;
    ham_size_t node_count;

    /* detect the start of the cycle: use this array of length lambda to recognize when we hit start (mu) */
    ham_device_graph_pathcheck_t *cycle;
    ham_size_t cycle_size;
    ham_size_t cycle_fill;
    cycle_seek_state_t cycle_seek_state;

} traverse4check_data_t;


#define device_get_to_nodecount(dev)        (dev)->out.count
#define device_get_to_node(dev, idx)        (dev)->out.multi[idx]


/*
Nivasch:

keep a stack of (x_i, i), where, at all times, both the i's and x_i's in the stack
form strictly increasing sequences. 

The stack is initially empty.

At each step j, pop from the stack all entries (x_i, i) where x_i > x_j. If an x_i == x_j
is found in the stack, we are done; then the cycle length is 

  lambda = j - i

Otherwise, push (x_j, j) on top of the sack and continue.
*/
static ham_status_t 
traverse_graph4check(traverse4check_data_t *data, 
        ham_device_t *entry_node, ham_size_t call_depth, 
        ham_device_graph_check_report_t *report_output)
{
    /*
    traverse through all 'to' child nodes
    */
    ham_status_t st = 0;
    ham_size_t i;
    ham_device_graph_pathcheck_t *stack = data->stack;

    for (i = 0; i < entry_node->out.count; i++)
    {
        ham_device_t *to = entry_node->out.devs[i];
        ham_size_t j;
        
        for (j = data->node_count; j-- > 0; )
        {
            if (stack[j].dev_i > to)
            {
                /* pop item off the stack */
                data->node_count = j;
            }
            else if (stack[j].dev_i == to)
            {
                /* cycle detected! */
                ham_size_t lambda = call_depth - stack[j].call_depth;

                report_output->reason = HAM_CYCLE_IN_DEVICE_GRAPH;
                report_output->culprit = to;
                report_output->extra_info.cycle.lambda = lambda;
                report_output->extra_info.cycle.mu = call_depth;

                data->cycle = allocator_calloc(data->mm, lambda * sizeof(data->stack[0]));
                if (!data->cycle)
                    return HAM_OUT_OF_MEMORY;
                data->cycle_size = lambda;
                data->cycle_fill = 1;
                data->cycle_seek_state = FIND_MU;

                data->cycle[0].call_depth = call_depth;
                data->cycle[0].dev_i = to;

                return HAM_CYCLE_IN_DEVICE_GRAPH;
            }
        }

        /* push (x_i, i) on the stack */
        if (data->node_count == data->alloc_size)
        {
            ham_size_t newlen = data->alloc_size * 2;

            data->stack = allocator_realloc(data->mm, data->stack, newlen * sizeof(data->stack[0]));
            if (!data->stack)
                return HAM_OUT_OF_MEMORY;

            memset(&data->stack[data->alloc_size], 0, (newlen - data->alloc_size) * sizeof(data->stack[0]));
        }
        data->stack[data->node_count].call_depth = call_depth;
        data->stack[data->node_count++].dev_i = to;
                                                       
        st = traverse_graph4check(data, to, call_depth + 1, report_output);
        switch (st)
        {
        case HAM_SUCCESS:
            continue;

        default:
            return st;

        case HAM_CYCLE_IN_DEVICE_GRAPH:
            /* 
            collect cycle sequence or try locate the start?

            As we matched x_i == x_j before, we know there's one ENTIRE cycle sitting in the call stack,
            so we can collect first and afterwards, we can go look for mu (start of cycle)
            */
            if (data->cycle_fill < data->cycle_size)
            {
                ham_assert(data->cycle_seek_state == FIND_MU, (0));

                data->cycle[data->cycle_fill].call_depth = call_depth;
                data->cycle[data->cycle_fill].dev_i = to;

                report_output->culprit = to;
                report_output->extra_info.cycle.mu = call_depth;

                return st;
            }
            else if (data->cycle_seek_state == FIND_MU)
            {
                /* 
                check whether we are in the cycle; if not, then the 
                inner call we just exited was element MU.
                
                This scan can be made a lot faster by using a hash table instead of
                an array here, but we were lazy and this is fringe functionality, 
                so no use expending a lot of breath on speeding up this baby any further. 
                */
                ham_size_t k;

                data->cycle_seek_state = MU_FOUND;

                for (k = 0; k < data->cycle_size; k++)
                {
                    if (to == data->cycle[k].dev_i)
                    {
                        /* keep on scanning for MU */
                        report_output->culprit = to;
                        report_output->extra_info.cycle.mu = call_depth;
                        data->cycle_seek_state = FIND_MU;
                        return st;
                    }
                }

                /*
                when we get here, the current device is the last before the cycle.

                Fortunately we tracked the 'mu' while we backwards-scanned up the
                call stack, so we have what we need by now.
                */
            }
            else 
            {
                ham_assert(data->cycle_seek_state == MU_FOUND, (0));
            }
            break;
        }
    }

    return st;
}

/**
Check for illegal cycles in the graph and require every terminal to be a physical device.

Use 'Cycle Detection Using A Stack', Gabriel Nivasch (2004) for cycle detection.
*/
ham_status_t ham_validate_device_graph(ham_device_t *entry_node, ham_device_graph_check_report_t *report_output)
{
    ham_status_t st;
    mem_allocator_t *mm = device_get_allocator(entry_node);
    /*
    determine the number of devices in the graph.

    WARNING: thee may be illegal cycles in the graph, which shouldn't cause us to run ad infinitum!
    */
    traverse4check_data_t data = {0};
    
    data.mm = mm;

    data.alloc_size = 16;

    if (!entry_node || !report_output)
        return HAM_INV_PARAMETER;

    data.stack = allocator_calloc(mm, data.alloc_size * sizeof(data.stack[0]));
    if (!data.stack)
        return HAM_OUT_OF_MEMORY;

    /*
    CAVEAT

    Should make sure the entry_node does not have any parent nodes, but that's
    being anal; we won't be using those devices anyhow, so we don't care about them
    in the end.
    */

    data.stack[0].call_depth = 0;
    data.stack[0].dev_i = entry_node;

    data.node_count = 1;

    st = traverse_graph4check(&data, entry_node, 1, report_output); 

    allocator_free(mm, data.stack);
    if (data.cycle)
    {
        allocator_free(mm, data.cycle);
    }

    return st;
}





/*
Nivasch, 2004 algorithm derivative
*/


/**
* @endcond 
*/

