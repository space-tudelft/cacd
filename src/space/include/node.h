/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*! @brief Structure to hold information about a node in the circuit model.
 */
typedef struct Node {
    unsigned substr : 4;
    unsigned term   : 4;
    unsigned area   : 4;
    unsigned flag   : 4;
    unsigned flag2  : 4;
    unsigned flag3  : 4;
    unsigned keep : 4;
    unsigned delayed : 4;
    struct Terminal * names;

    /*------------------------------------------------------------------------*/
    /*! @brief A doubly linked list of subnodes.
     *//*---------------------------------------------------------------------*/

    struct subnode * subs;

    /*------------------------------------------------------------------------*/
    /*! @brief The conducting elements connected to this node.
     *
     * This field is a pointer to an array of <code>resSortTabSize</code>
     * pointers (or NULL).
     *
     * Each element of the array points to a linked list of
     * <code>element_r</code> objects (elements).
     *
     * The pointer may be NULL when the array is considered "not available".
     *
     * The array is indexed by "sort" (see <code>resSortTab</code>).
     *
     * The array is allocated physically somewhere at the end of this struct,
     * and hence does not need to be freed separately (but each individual element
     * in the linked list pointed to by the array should still be freed; elements
     * are (de)allocated using a free-list mechanism, see the documentation
     * of <code>element_r</code>).
     *//*---------------------------------------------------------------------*/

    struct elementR ** con;

    /*------------------------------------------------------------------------*/
    /*! @brief The capacitive elements connected to this node.
     *
     * This field is a pointer to an array of <code>capSortTabSize</code>
     * pointers (or NULL).
     *
     * Each element of the array points to a linked list of <code>element_c</code>
     * objects (elements).
     *
     * The pointer may be NULL when the array is considered "not available".
     *
     * The array is indexed by "sort" (see <code>capSortTab</code>).
     *
     * Basically, this data structure is accessed in a similar fashion as
     * <code>con</code>.
     *//*---------------------------------------------------------------------*/

    struct elementC ** cap;

    /*------------------------------------------------------------------------*/
    /*! @brief Number of resistances connected to this node.
     *
     *  The value of this field is used during node elimination. Nodes are
     *  stored in a priority queue, ordered by semi-"degree" (a heuristic
     *  value, see function <code>nqDegree</code>), and this semi-degree may
     *  depend on the value of this field.
     *//*---------------------------------------------------------------------*/

    int res_cnt;

    /*------------------------------------------------------------------------*/
    /*! @brief Number of capacitances connected to this node.
     *
     *  The value of this field is used during node elimination.
     *
     *  @see res_cnt
     *//*---------------------------------------------------------------------*/

    int cap_cnt;

    int n_n_cnt;  /* names and netEq count */
    double * gndCap;
    double * substrCap;

    /*------------------------------------------------------------------------*/
    /*! @brief Total admittance to the substrate node.
     *
     *  This field contains a pointer to an array of
     *  <code>resSortTabSize</code> values, each representing a total
     *  admittance to the substrate node.
     *
     *  @see resSortTabSize
     *//*---------------------------------------------------------------------*/

    double * substrCon;

    double help; /* used for reducMaxParRes */

    struct netEquiv * netEq;
#ifdef CONFIG_SPACE2
    struct netEquiv * netEq2;
    struct cluster * clr;
    struct Node * cl_next;
#endif
    struct group * grp;
    struct Node *gprev;
    struct Node *gnext;

    /*------------------------------------------------------------------------*/
    /*! @brief Pointer to the "prev" and "next" node in the priority queue.
     *
     *  The "next" pointer is also used to link nodes in the "free list" of nodes
     *  together, which is used for faster allocation.
     *//*---------------------------------------------------------------------*/

    struct Node *next, *prev;

    /*------------------------------------------------------------------------*/
    /*! @brief The conductor mask number of the node.
     *
     *  Substrate nodes have mask number -1.
     *//*---------------------------------------------------------------------*/

    int    mask;

    /*------------------------------------------------------------------------*/
    /*! @brief The x and y coordinates assigned to the node.
     *//*---------------------------------------------------------------------*/
    /* @@: Are these coordinates always set properly, or may they be undefined?
       @@: When exactly are the coordinates of a node set?
     */
    coor_t node_x, node_y;
    coor_t node_h, node_w;

    /*------------------------------------------------------------------------*/
    /*! @brief The id of the node.
     *
     *  The ids are currently generated at random, so ids should be considered
     *  only "pseudo unique". The ids are used for node hashing.
     *//*---------------------------------------------------------------------*/

    int id;

    /*------------------------------------------------------------------------*/
    /*! @brief The extra moments to ground of this node.
     *
     *  The "extra" moments of a node are the moments of order >= 2 related to
     *  the admittance to ground of the node (the moment of order 1, a
     *  capacitance value, is represented by gndCap[cx], where cx is the sort
     *  of capacitance being investigated; the moment of order 0 is not
     *  possible, since it would imply the existence of a resistance to
     *  ground, which is not permitted in this implementation). These values
     *  are used by the SNE elimination algorithm.
     *
     *  The size of the moments array is stored in the global variable
     *  extraMoments.
     *
     *  Thus: the mathematical 1st moment is stored in gndCap[].
     *  Thus: the mathematical 2nd moment is stored in moments[0].
     *
     *  @see momentsElimOrWeight
     *//*---------------------------------------------------------------------*/
#ifdef MOMENTS
    double *moments;
    double *moments2;
#endif

    struct nodeLink * pols; /* BIPOLAR */

#ifndef CONFIG_SPACE2
    int weight; /* SNE */
#endif
    int degree;
} node_t;

